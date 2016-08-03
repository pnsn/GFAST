#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <stdbool.h>
#include <omp.h>
#include <lapacke.h>
#include "gfast.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

static int __verify_cmt_structs(struct GFAST_offsetData_struct cmt_data,
                                struct GFAST_cmtResults_struct *cmt);

/*!
 * @brief Drives the CMT estimation.
 *
 * @param[in] cmt_props  CMT inversion parameters
 * @param[in] SA_lat     event latitude (degrees) [-90,90]
 * @param[in] SA_lon     event longitude (degrees) [0,360]
 * @param[in] SA_dep     event depth (km)
 * @param[in] cmt_data   cmt offset data to invert
 *
 * @param[inout] cmt     on input contains the depths for the grid search
 *                       on output contains the corresponding variance
 *                       reduction, moment tensors, nodal planes at each
 *                       depth in the CMT grid search, and optimal depth
 *                       index.
 *
 * @result 0 indicates success
 *         1 indicates an error on the cmt structure
 *         2 indicates the offset data structure is invalid
 *         3 indicates there was insufficient data to invert
 *         4 indicates an error encountered during the computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_CMT__driver(struct GFAST_cmt_props_struct cmt_props,
                      double SA_lat, double SA_lon, double SA_dep,
                      struct GFAST_offsetData_struct cmt_data,
                      struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_CMT__driver\0";
    double *utmRecvEasting, *utmRecvNorthing, *staAlt,
           *eOffset, *eEst, *eWts, *nOffset, *nEst, *nWts,
           *uOffset, *uEst, *uWts,
           DC_pct, eres, nres, sum_res2, ures,
           utmSrcEasting, utmSrcNorthing, wte, wtn, wtu, x1, y1, x2, y2;
    int i, idep, ierr, ierr1, k, l1, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Verify the input data structure makes sense
    ierr = CMT_SUCCESS;
    luse = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    staAlt = NULL;
    uOffset = NULL;
    nOffset = NULL;
    eOffset = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    nEst = NULL;
    eEst = NULL;
    uEst = NULL;
    // Verify input data structures
    ierr =__verify_cmt_structs(cmt_data, cmt);
    if (ierr != CMT_SUCCESS)
    {
        log_errorF("%s: Error failed to verify data structures\n", fcnm);
    }
    // Warn in case hypocenter is outside of grid-search
    if (cmt_props.verbose > 1 &&
        (SA_dep < cmt->srcDepths[0] || SA_dep > cmt->srcDepths[cmt->ndeps-1]))
    {
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }
    // Initialize result
    cmt->opt_indx =-1;
    #pragma omp simd
    for (i=0; i<cmt->nsites*cmt->ndeps; i++)
    {
        cmt->EN[i] = 0.0;
        cmt->NN[i] = 0.0;
        cmt->UN[i] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<cmt->nsites; i++)
    {
        cmt->Einp[i] = 0.0;
        cmt->Ninp[i] = 0.0;
        cmt->Uinp[i] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<cmt->ndeps; i++)
    {
        cmt->objfn[i] = 0.0;
        cmt->str1[i] = 0.0;
        cmt->str2[i] = 0.0;
        cmt->dip1[i] = 0.0;
        cmt->dip2[i] = 0.0;
        cmt->rak1[i] = 0.0;
        cmt->rak2[i] = 0.0;
        cmt->Mw[i] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<6*cmt->ndeps; i++)
    {
        cmt->mts[i] = 0.0;
    }
    // Require there is a sufficient amount of data to invert
    luse = ISCL_memory_calloc__bool(cmt_data.nsites);
    l1 = 0;
    for (k=0; k<cmt_data.nsites; k++)
    {
        wtu = cmt_data.wtu[k];
        wtn = cmt_data.wtn[k];
        wte = cmt_data.wte[k];
        if (!cmt_data.lactive[k] || fmax(wtu, fmax(wtn, wte)) <= 0.0)
        {
             continue;
        }
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < cmt_props.min_sites)
    {
        if (cmt_props.verbose > 1)
        {
            log_warnF("%s: Insufficient data to invert %d < %d\n",
                      fcnm, l1, cmt_props.min_sites);
        }
        ierr = CMT_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Set space
    utmRecvNorthing = ISCL_memory_calloc__double(l1);
    utmRecvEasting  = ISCL_memory_calloc__double(l1);
    staAlt  = ISCL_memory_calloc__double(l1);
    uOffset = ISCL_memory_calloc__double(l1);
    nOffset = ISCL_memory_calloc__double(l1);
    eOffset = ISCL_memory_calloc__double(l1);
    uWts    = ISCL_memory_calloc__double(l1);
    nWts    = ISCL_memory_calloc__double(l1);
    eWts    = ISCL_memory_calloc__double(l1);
    nEst    = ISCL_memory_calloc__double(l1*cmt->ndeps);
    eEst    = ISCL_memory_calloc__double(l1*cmt->ndeps);
    uEst    = ISCL_memory_calloc__double(l1*cmt->ndeps);
    // Get the source location
    zone_loc = cmt_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    GFAST_core_coordtools_ll2utm(SA_lat, SA_lon,
                                 &y1, &x1,
                                 &lnorthp, &zone_loc);
    utmSrcNorthing = y1;
    utmSrcEasting = x1;
    // Get cartesian positions and observations onto local arrays
    l1 = 0;
    for (k=0; k<cmt_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        GFAST_core_coordtools_ll2utm(cmt_data.sta_lat[k],
                                     cmt_data.sta_lon[k],
                                     &y2, &x2,
                                     &lnorthp, &zone_loc);
        // Copy the pertinent data
        uOffset[l1] = cmt_data.ubuff[k];
        nOffset[l1] = cmt_data.nbuff[k];
        eOffset[l1] = cmt_data.ebuff[k];
        uWts[l1] = cmt_data.wtu[k];
        nWts[l1] = cmt_data.wtn[k];
        eWts[l1] = cmt_data.wte[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = cmt_data.sta_alt[k];
        l1 = l1 + 1;
    }
    // Invert!
    if (cmt_props.verbose > 2)
    { 
        log_debugF("%s: Inverting for CMT with %d sites\n", fcnm, l1);
    }
    ierr = GFAST_core_cmt_depthGridSearch(l1, cmt->ndeps,
                                          cmt_props.verbose,
                                          cmt_props.ldeviatoric,
                                          utmSrcEasting,
                                          utmSrcNorthing,
                                          cmt->srcDepths,
                                          utmRecvEasting,
                                          utmRecvNorthing,
                                          staAlt,
                                          nOffset,
                                          eOffset,
                                          uOffset,
                                          nWts,
                                          eWts,
                                          uWts,
                                          nEst,
                                          eEst,
                                          uEst,
                                          cmt->mts);
    if (ierr != 0)
    {   
        log_errorF("%s: Error in CMT gridsearch!\n", fcnm);
        ierr = CMT_COMPUTE_ERROR;
        goto ERROR;
    }
    // Get the estimates and observations
    i = 0;
    for (k=0; k<cmt->nsites; k++)
    {
        cmt->lsiteUsed[k] = luse[k];
        if (!luse[k]){continue;}
        cmt->Ninp[k] = nOffset[i];
        cmt->Einp[k] = eOffset[i];
        cmt->Uinp[k] = uOffset[i];
        cmt->lsiteUsed[k] = true;
        i = i + 1;
    }
    // Extract results and weight objective fn by percent double couple
    ierr = 0;
#ifdef PARALLEL_CMT
    #pragma omp parallel for \
     private(DC_pct, eres, i, idep, ierr1, k, sum_res2, nres, ures) \
     shared(cmt, eOffset, eEst, fcnm, l1, luse, nOffset, nEst, uOffset, uEst) \
     reduction(+:ierr), default(none) 
#endif
    for (idep=0; idep<cmt->ndeps; idep++)
    {
        // Compute the L2 norm
        sum_res2 = 0.0;
        #pragma omp simd reduction(+:sum_res2)
        for (i=0; i<l1; i++)
        {
            nres = nOffset[i] - nEst[idep*l1+i];
            eres = eOffset[i] - eEst[idep*l1+i];
            ures = uOffset[i] - uEst[idep*l1+i];
            sum_res2 = sum_res2 + nres*nres + eres*eres + ures*ures;
        }
        sum_res2 = sqrt(sum_res2);
        // Decompose the moment tensor
        ierr1 = GFAST_core_cmt_decomposeMomentTensor(1, &cmt->mts[6*idep],
                                                     &DC_pct,
                                                     &cmt->Mw[idep],
                                                     &cmt->str1[idep],
                                                     &cmt->str2[idep],
                                                     &cmt->dip1[idep],
                                                     &cmt->dip2[idep],
                                                     &cmt->rak1[idep],
                                                     &cmt->rak2[idep]);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error decomposing mt\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Prefer results with larger double couple percentages
        cmt->objfn[idep] = sum_res2/DC_pct;
        // Save the data
        i = 0;
        for (k=0; k<cmt->nsites; k++)
        {
            cmt->NN[idep*cmt->nsites+k] = 0.0;
            cmt->EN[idep*cmt->nsites+k] = 0.0;
            cmt->UN[idep*cmt->nsites+k] = 0.0;
            if (luse[k])
            {
                cmt->NN[idep*cmt->nsites+k] = nEst[idep*l1+i];
                cmt->EN[idep*cmt->nsites+k] = eEst[idep*l1+i];
                cmt->UN[idep*cmt->nsites+k] = uEst[idep*l1+i];
                i = i + 1;
            }
        }
    } // Loop on depths
    if (ierr != 0)
    {
        log_errorF("%s: Error decomposing moment tensor\n", fcnm);
        ierr = CMT_COMPUTE_ERROR;
    }
    // Get the optimimum index
    cmt->opt_indx = array_argmin__double(cmt->ndeps, cmt->objfn);
ERROR:;
    ISCL_memory_free__bool(&luse);
    ISCL_memory_free__double(&utmRecvNorthing);
    ISCL_memory_free__double(&utmRecvEasting);
    ISCL_memory_free__double(&staAlt);
    ISCL_memory_free__double(&uOffset);
    ISCL_memory_free__double(&nOffset);
    ISCL_memory_free__double(&eOffset);
    ISCL_memory_free__double(&nEst);
    ISCL_memory_free__double(&eEst);
    ISCL_memory_free__double(&uEst);
    ISCL_memory_free__double(&nWts);
    ISCL_memory_free__double(&eWts);
    ISCL_memory_free__double(&uWts);
    return ierr;
}
//============================================================================//
/*!
 * @brief Utility function for verifying input data structures
 *
 * @param[in] cmt_data   cmt_data structure to verify
 * @param[in] cmt        cmt results structure to verify
 *
 * @result CMT_SUCCESS indicates input structures are ready for use
 *
 * @author Ben Baker (ISTI)
 *
 */
static int __verify_cmt_structs(struct GFAST_offsetData_struct cmt_data,
                                struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "__verify_cmt_structs\0";
    int ierr;
    ierr = CMT_SUCCESS;
    // Require there is offset data
    if (cmt_data.nsites < 1)
    {
        ierr = CMT_OS_DATA_ERROR;
        log_errorF("%s: No offset data\n", fcnm);
        goto ERROR;
    }
    if (cmt_data.nsites < 1)
    {
        ierr = CMT_OS_DATA_ERROR;
        log_errorF("%s: No peak displacement data\n", fcnm);
        goto ERROR;
    }
    // Verify the output data structures
    if (cmt->ndeps < 1)
    {
        log_errorF("%s: No depths in CMT gridsearch!\n", fcnm);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    if (cmt->objfn == NULL || cmt->mts == NULL ||
        cmt->str1 == NULL || cmt->str2 == NULL ||
        cmt->dip1 == NULL || cmt->dip2 == NULL ||
        cmt->rak1 == NULL || cmt->rak2 == NULL ||
        cmt->Mw == NULL || cmt->srcDepths == NULL ||
        cmt->EN == NULL || cmt->NN == NULL || cmt->UN == NULL ||
        cmt->Einp == NULL || cmt->Ninp == NULL || cmt->Uinp == NULL ||
        cmt->lsiteUsed == NULL)
    {
        if (cmt->objfn == NULL)
        {
            log_errorF("%s: Error cmt->objfn is NULL\n", fcnm);
        }
        if (cmt->mts == NULL)
        {
            log_errorF("%s: Error cmt->mts is NULL\n", fcnm);
        }
        if (cmt->str1 == NULL)
        {
            log_errorF("%s: Error cmt->str1 is NULL\n", fcnm);
        }
        if (cmt->str2 == NULL)
        {
            log_errorF("%s: Error cmt->str2 is NULL\n", fcnm);
        }
        if (cmt->dip1 == NULL)
        {
            log_errorF("%s: Error cmt->dip1 is NULL\n", fcnm);
        }
        if (cmt->dip2 == NULL)
        {
            log_errorF("%s: Error cmt->dip2 is NULL\n", fcnm);
        }
        if (cmt->rak1 == NULL)
        {
            log_errorF("%s: Error cmt->rak1 is NULL\n", fcnm);
        }
        if (cmt->rak2 == NULL)
        {
            log_errorF("%s: Error cmt->rak2 is NULL\n", fcnm);
        }
        if (cmt->Mw == NULL)
        {
            log_errorF("%s: Error Mw is NULL\n", fcnm);
        }
        if (cmt->srcDepths == NULL)
        {
            log_errorF("%s: Error srcDepths is NULL\n", fcnm);
        }
        if (cmt->EN == NULL)
        {
            log_errorF("%s: Error EN is NULL\n", fcnm);
        }
        if (cmt->NN == NULL)
        {
            log_errorF("%s: Error NN is NULL\n", fcnm);
        }
        if (cmt->UN == NULL)
        {
            log_errorF("%s: Error UN is NULL\n", fcnm);
        }
        if (cmt->Einp == NULL)
        {
            log_errorF("%s: Error Einp is NULL\n", fcnm);
        }
        if (cmt->Ninp == NULL)
        {
            log_errorF("%s: Error Ninp is NULL\n", fcnm);
        }
        if (cmt->Uinp == NULL)
        {
            log_errorF("%s: Error Uinp is NULL\n", fcnm);
        }
        if (cmt->lsiteUsed == NULL)
        {
            log_errorF("%s: Error lsiteUsed is NULL\n", fcnm);
        }
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Avoid a segfault
    if (cmt->nsites != cmt_data.nsites)
    {
        log_errorF("%s: nsites on cmt and cmt_data differs %d %d\n",
                   fcnm, cmt->nsites, cmt_data.nsites);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
ERROR:;
    return ierr;
}

