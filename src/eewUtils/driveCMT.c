#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_eewUtils.h"
#ifdef GFAST_USE_INTEL
#include <mkl_lapacke.h>
#include <mkl_cblas.h>
#else
#include <lapacke.h>
#include <cblas.h>
#endif
#include <stdbool.h>
#include "gfast_core.h"
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
 * @param[in,out] cmt    on input contains the depths for the grid search
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
int eewUtils_driveCMT(struct GFAST_cmt_props_struct cmt_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_offsetData_struct cmt_data,
                      struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "eewUtils_driveCMT\0";
    double *utmRecvEasting, *utmRecvNorthing, *staAlt,
           *eOffset, *eEst, *eWts, *nOffset, *nEst, *nWts,
           *uOffset, *uEst, *uWts,
           DC_pct, eres, nres, sum_res2, ures,
           utmSrcEasting, utmSrcNorthing, wte, wtn, wtu, x1, y1, x2, y2;
    int i, idep, ierr, ierr1, ilat, ilon, indx, k, l1, nlld, zone_loc;
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
    if (cmt->nlats > 1 || cmt->nlons > 1)
    {
        log_errorF("%s: Error nlats > 1 | nlons > 1 not done\n", fcnm);
        ierr = 1;
        return ierr;
    }
    // Warn in case hypocenter is outside of grid-search
    if (cmt_props.verbose > 1 &&
        (SA_dep < cmt->srcDepths[0] || SA_dep > cmt->srcDepths[cmt->ndeps-1]))
    {
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }
    // Initialize result
    nlld = cmt->nlats*cmt->nlons*cmt->ndeps;
    cmt->opt_indx =-1;
    // Synthetics as a function of depth
    array_zeros64f_work(cmt->nsites*nlld, cmt->EN);
    array_zeros64f_work(cmt->nsites*nlld, cmt->NN);
    array_zeros64f_work(cmt->nsites*nlld, cmt->UN);
    // Observations
    array_zeros64f_work(cmt->nsites, cmt->Einp);
    array_zeros64f_work(cmt->nsites, cmt->Ninp);
    array_zeros64f_work(cmt->nsites, cmt->Uinp);
    // MT optimizaiton information as a function of depth 
    nlld = cmt->nlats*cmt->nlons*cmt->ndeps;
    array_zeros64f_work(nlld, cmt->l2);
    array_zeros64f_work(nlld, cmt->pct_dc);
    array_zeros64f_work(nlld, cmt->objfn);
    array_zeros64f_work(6*nlld, cmt->mts);
    array_zeros64f_work(nlld, cmt->str1);
    array_zeros64f_work(nlld, cmt->str2);
    array_zeros64f_work(nlld, cmt->dip1);
    array_zeros64f_work(nlld, cmt->dip2);
    array_zeros64f_work(nlld, cmt->rak1);
    array_zeros64f_work(nlld, cmt->rak2);
    array_zeros64f_work(nlld, cmt->Mw);
    // Require there is a sufficient amount of data to invert
    luse = memory_calloc8l(cmt_data.nsites);
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
    utmRecvNorthing = memory_calloc64f(l1);
    utmRecvEasting  = memory_calloc64f(l1);
    staAlt  = memory_calloc64f(l1);
    uOffset = memory_calloc64f(l1);
    nOffset = memory_calloc64f(l1);
    eOffset = memory_calloc64f(l1);
    uWts    = memory_calloc64f(l1);
    nWts    = memory_calloc64f(l1);
    eWts    = memory_calloc64f(l1);
    nEst    = memory_calloc64f(l1*cmt->ndeps);
    eEst    = memory_calloc64f(l1*cmt->ndeps);
    uEst    = memory_calloc64f(l1*cmt->ndeps);
    // Get the source location
    zone_loc = cmt_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    core_coordtools_ll2utm(SA_lat, SA_lon,
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
        core_coordtools_ll2utm(cmt_data.sta_lat[k],
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
    ierr = core_cmt_gridSearch(l1,
                               cmt->ndeps, cmt->nlats, cmt->nlons,
                               cmt_props.verbose,
                               cmt_props.ldeviatoric,
                               &utmSrcEasting,
                               &utmSrcNorthing,
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
/*
    ierr = core_cmt_depthGridSearch(l1, cmt->ndeps,
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
*/
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
    #pragma omp parallel for collapse(3) \
     private(DC_pct, eres, i, idep, ierr1, ilat, ilon, indx, k, sum_res2, nres, ures) \
     shared(cmt, eOffset, eEst, fcnm, l1, luse, nOffset, nEst, uOffset, uEst) \
     reduction(+:ierr), default(none) 
#endif
    for (ilon=0; ilon<cmt->nlons; ilon++)
    {
        for (ilat=0; ilat<cmt->nlats; ilat++)
        {
            for (idep=0; idep<cmt->ndeps; idep++)
            {
                // Get location in arrays
                indx = ilon*cmt->ndeps*cmt->nlats
                     + ilat*cmt->ndeps 
                     + idep;
                // Compute the L2 norm
                sum_res2 = 0.0;
#ifdef _OPENMP
                #pragma omp simd reduction(+:sum_res2)
#endif
                for (i=0; i<l1; i++)
                {
                    nres = nOffset[i] - nEst[indx*l1+i];
                    eres = eOffset[i] - eEst[indx*l1+i];
                    ures = uOffset[i] - uEst[indx*l1+i];
                    sum_res2 = sum_res2 + nres*nres + eres*eres + ures*ures;
                }
                sum_res2 = sqrt(sum_res2);
                // Decompose the moment tensor
                ierr1 = core_cmt_decomposeMomentTensor(1, &cmt->mts[6*idep],
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
                cmt->l2[indx] = 0.5*sqrt(sum_res2);
                cmt->pct_dc[indx] = DC_pct;
                cmt->objfn[indx] = sum_res2/DC_pct;
                // Save the data
                i = 0;
                for (k=0; k<cmt->nsites; k++)
                {
                    cmt->NN[indx*cmt->nsites+k] = 0.0;
                    cmt->EN[indx*cmt->nsites+k] = 0.0;
                    cmt->UN[indx*cmt->nsites+k] = 0.0;
                    if (luse[k])
                    {
                        cmt->NN[indx*cmt->nsites+k] = nEst[indx*l1+i];
                        cmt->EN[indx*cmt->nsites+k] = eEst[indx*l1+i];
                        cmt->UN[indx*cmt->nsites+k] = uEst[indx*l1+i];
                        i = i + 1;
                    }
               }
           } // Loop on depths
       } // loop on latiudes
    } // loop on longitudes
    if (ierr != 0)
    {
        log_errorF("%s: Error decomposing moment tensor\n", fcnm);
        ierr = CMT_COMPUTE_ERROR;
    }
    // Get the optimimum index
    cmt->opt_indx = array_argmin64f(cmt->ndeps, cmt->objfn);
ERROR:;
    memory_free8l(&luse);
    memory_free64f(&utmRecvNorthing);
    memory_free64f(&utmRecvEasting);
    memory_free64f(&staAlt);
    memory_free64f(&uOffset);
    memory_free64f(&nOffset);
    memory_free64f(&eOffset);
    memory_free64f(&nEst);
    memory_free64f(&eEst);
    memory_free64f(&uEst);
    memory_free64f(&nWts);
    memory_free64f(&eWts);
    memory_free64f(&uWts);
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
    if (cmt->l2 == NULL || cmt->pct_dc == NULL ||
        cmt->objfn == NULL || cmt->mts == NULL ||
        cmt->str1 == NULL || cmt->str2 == NULL ||
        cmt->dip1 == NULL || cmt->dip2 == NULL ||
        cmt->rak1 == NULL || cmt->rak2 == NULL ||
        cmt->Mw == NULL || cmt->srcDepths == NULL ||
        cmt->EN == NULL || cmt->NN == NULL || cmt->UN == NULL ||
        cmt->Einp == NULL || cmt->Ninp == NULL || cmt->Uinp == NULL ||
        cmt->lsiteUsed == NULL)
    {
        if (cmt->l2 == NULL)
        {
            log_errorF("%s: Error cmt->l2 is NULL\n", fcnm);
        }
        if (cmt->pct_dc == NULL)
        {
            log_errorF("%s: Error cmt->pct_dc is NULL\n", fcnm);
        }
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

