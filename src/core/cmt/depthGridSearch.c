#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <cblas.h>
#include <math.h>
#include <lapacke.h>
#include "gfast_core.h"
#include "iscl/linalg/linalg.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
#include "iscl/time/time.h"

/*!
 * @brief Performs the CMT depth grid search.
 *
 * @param[in] l1               number of sites
 * @param[in] ndeps            number of depths
 * @param[in] verbose          controls the verbosity (0 is quiet)
 * @param[in] deviatoric       if true then constrain the inversion s.t. 
 *                             resulting moment tensor is purely deviatoric.
 *                             otherwise, invert for all six moment tensor
 *                             terms.
 * @param[in] utmSrcEasting    source easting UTM position (m)
 * @param[in] utmSrcNorthing   source northing UTM position (m)
 * @param[in] srcDepths        source depths (km) in grid-search [ndeps]
 * @param[in] utmRecvEasting   receiver easting UTM positions (m) [l1]
 * @param[in] utmRecvNorthing  receiver northing UMT positions (m) [l1]
 * @param[in] staAlt           station elevations above sea level (m) [l1]
 * @param[in] nObsOffset       observed offset in north component (m)
 *                             for i'th site [l1]
 * @param[in] eObsOffset       observed offset in east component (m)
 *                             for i'th site [l1]
 * @param[in] uObsOffset       observed offset in vertical comonent (m)
 *                             for i'th site [l1]
 * @param[in] nWts             weight corresponding to i'th north offset
 *                             observation [l1]
 * @param[in] eWts             weight corresponding to i'th east offset
 *                             observation [l1]
 * @param[in] uWts             weight corresponding to i'th vertical offset
 *                             observation [l1]
 *
 * @param[out] nEst            estimate offset in the north component (m)
 *                             for the i'th site at all depths [l1*ndeps]
 *                             the i'th site at the idep'th depth is given by
 *                             idep*l1 + i
 * @param[out] eEst            estimate offset in the east component (m)
 *                             for the i'th site at all depths [l1*ndeps] 
 *                             the i'th site at the idep'th depth is given by
 *                             idep*l1 + i
 * @param[out] uEst            estimate offset in the vertical component (m)
 *                             for the i'th site at all depths [l1*ndeps]
 *                             the i'th site at the idep'th depth is given by
 *                             idep*l1 + i
 * @param[out] mts             the moment tensor terms (Nm) inverted for in
 *                             an NED system at each depth.  the id'th depth
 *                             is begins at index 6*id.  the moment tensors at
 *                             at each depth are packed:  
 *                             \f$ \{m_{xx}, m_{yy}, m_{zz},
 *                                   m_{xy}, m_{xz}, m_{yz} \} \f$. 
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @bug Only deviatoric constraint programmed.
 *
 */
int core_cmt_depthGridSearch(const int l1, const int ndeps,
                             const int verbose,
                             const bool deviatoric,
                             const double utmSrcEasting,
                             const double utmSrcNorthing,
                             const double *__restrict__ srcDepths,
                             const double *__restrict__ utmRecvEasting,
                             const double *__restrict__ utmRecvNorthing,
                             const double *__restrict__ staAlt,
                             const double *__restrict__ nObsOffset,
                             const double *__restrict__ eObsOffset,
                             const double *__restrict__ uObsOffset,
                             const double *__restrict__ nWts,
                             const double *__restrict__ eWts,
                             const double *__restrict__ uWts,
                             double *__restrict__ nEst,
                             double *__restrict__ eEst,
                             double *__restrict__ uEst,
                             double *__restrict__ mts)
{
    const char *fcnm = "core_cmt_depthGridSearch\0";
    double *diagWt, *G, *U, *UP, *WG, *WU, *xrs, *yrs, *zrs_negative, S[6],
           eq_alt, m11, m12, m13, m22, m23, m33;
    int i, idep, ierr, ierr1, ldg, mrows, ncols;
    // Initialize
    ierr = 0;
    diagWt = NULL;
    WG = NULL;
    G = NULL;
    WU = NULL;
    U = NULL;
    UP = NULL;
    xrs = NULL;
    yrs = NULL;
    zrs_negative = NULL;
    // Error check
    if (l1 < 1)
    {
        log_errorF("%s: Error invalid number of input stations: %d\n",
                   fcnm, l1);
        ierr = 1;
        goto ERROR;
    }   
    if (srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        nObsOffset == NULL || eObsOffset == NULL || uObsOffset == NULL ||
        nEst == NULL || eEst == NULL || uEst == NULL || 
        mts == NULL)
    {
        if (srcDepths == NULL){log_errorF("%s: srcDepths is NULL!\n", fcnm);}
        if (utmRecvEasting == NULL)
        {
            log_errorF("%s: utmRecvEasting is NULL!\n", fcnm);
        }
        if (utmRecvNorthing == NULL)
        {
            log_errorF("%s: utmRecvNorthing is NULL!\n", fcnm);
        }
        if (staAlt == NULL){log_errorF("%s: staAlt is NULL\n", fcnm);}
        if (nObsOffset == NULL){log_errorF("%s: nObsOffset is NULL\n", fcnm);}
        if (eObsOffset == NULL){log_errorF("%s: eObsOffset is NULL\n", fcnm);}
        if (uObsOffset == NULL){log_errorF("%s: uObsOffset is NULL\n", fcnm);}
        if (nEst == NULL){log_errorF("%s: nEst is NULL\n", fcnm);}
        if (eEst == NULL){log_errorF("%s: eEst is NULL\n", fcnm);}
        if (uEst == NULL){log_errorF("%s: uEst is NULL\n", fcnm);}
        if (mts == NULL){log_errorF("%s: mts is NULL\n", fcnm);}
        ierr = 1;
        goto ERROR;
    }
    if (ndeps < 1)
    {
        log_errorF("%s: Error invalid number of source depths: %d\n",
                   fcnm, ndeps);
        ierr = 1;
        goto ERROR;
    }
    if (!deviatoric)
    {
        log_errorF("%s: Cannot perform general MT gridsearch!\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    // Initialize results to nothing
    #pragma omp simd
    for (idep=0; idep<ndeps; idep++)
    {
        mts[6*idep+0] = 0.0;
        mts[6*idep+1] = 0.0;
        mts[6*idep+2] = 0.0;
        mts[6*idep+3] = 0.0;
        mts[6*idep+4] = 0.0;
        mts[6*idep+5] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<ndeps*l1; i++)
    {
        uEst[i] = 0.0;
        nEst[i] = 0.0;
        eEst[i] = 0.0;
    }
    // Set space
    ncols = 6;
    if (deviatoric){ncols = 5;}
    mrows = 3*l1;
    ldg = ncols;  // In row major format
    diagWt       = ISCL_memory_calloc__double(mrows);
    G            = ISCL_memory_calloc__double(mrows*ncols);
    WG           = ISCL_memory_calloc__double(mrows*ncols);
    WU           = ISCL_memory_calloc__double(mrows);
    U            = ISCL_memory_calloc__double(mrows);
    UP           = ISCL_memory_calloc__double(mrows);
    xrs          = ISCL_memory_calloc__double(l1);
    yrs          = ISCL_memory_calloc__double(l1);
    zrs_negative = ISCL_memory_calloc__double(l1);
    // Compute the source-receiver offsets in (x, y) cartesian
    #pragma omp simd
    for (i=0; i<l1; i++)
    {
        xrs[i] = utmRecvEasting[i] - utmSrcEasting;
        yrs[i] = utmRecvNorthing[i] - utmSrcNorthing;
    }
    // Set the RHS
    ierr = core_cmt_setRHS(l1, nObsOffset, eObsOffset, uObsOffset, U);
    if (ierr != 0)
    {
        log_errorF("%s: error setting RHS!\n", fcnm);
        goto ERROR;
    }
    // Compute the diagonal data weights
    ierr = core_cmt_setDiagonalWeightMatrix(l1,
                                            nWts,
                                            eWts,
                                            uWts,
                                            diagWt);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to set weight matrix - will set to identity\n",
                   fcnm);
        for (i=0; i<mrows; i++)
        {
            diagWt[i] = 1.0;
        }
    }
    // Apply the diagonal data weights to the data
    ierr = core_cmt_weightObservations(mrows,
                                       diagWt,
                                       U,
                                       WU);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to apply data weights to data\n", fcnm);
        return -1;
    }
    // Grid search on source depths
    ISCL_time_tic();
    if (verbose > 2)
    {
        log_debugF("%s: Beginning search on depths...\n", fcnm);
    }
#ifdef PARALLEL_CMT
    #pragma omp parallel for \
     firstprivate(G, UP, WG, zrs_negative) \
     private (i, idep, ierr1, eq_alt, m11, m22, m33, m12, m13, m23, S ) \
     shared (diagWt, eEst, fcnm, ldg, mts, mrows, \
             ncols, nEst, srcDepths, staAlt, \
             uEst, U, WU, xrs, yrs) \
     reduction(+:ierr) default(none)
#endif
    for (idep=0; idep<ndeps; idep++)
    {
        // Get the z source receiver offset
        eq_alt = srcDepths[idep]*1.e3;
        #pragma omp simd
        for (i=0; i<l1; i++)
        {
            zrs_negative[i] =-(staAlt[i] + eq_alt);
        }
        // Set the forward modeling matrix - note convention of xrs and yrs 
        ierr1 = core_cmt_setForwardModel(l1, deviatoric, 
                                         yrs,
                                         xrs,
                                         zrs_negative,
                                         G);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error constructing Green's function matrix\n",
                       fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Apply the data weights
        ierr1 = core_cmt_weightForwardModel(mrows, ncols, diagWt, G, WG);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error weighting forward modeling matrix!\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Solve the weighted least squares problem
        ierr1 = linalg_lstsq_qr64f_work(LAPACK_ROW_MAJOR,
                                        mrows, ncols, 1, false,
                                        WG, WU,
                                        S, NULL);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error solving least squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Collect the moment tensors in NED coordinates
        m12 = S[0];
        m13 = S[1];
        m33 = S[2];
        m23 = S[4];
        m11 = S[3] - 0.5*S[2];
        m22 =-S[3] - 0.5*S[2];
        mts[6*idep+0] = m11; // mxx
        mts[6*idep+1] = m22; // myy
        mts[6*idep+2] = m33; // mzz
        mts[6*idep+3] = m12; // mxy
        mts[6*idep+4] = m13; // mxz
        mts[6*idep+5] = m23; // myz
        // Compute the forward problem
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    mrows, ncols, 1.0, G, ldg, S, 1, 0.0, UP, 1);
        // Copy estimates back
        #pragma omp simd
        for (i=0; i<l1; i++)
        {
            nEst[idep*l1+i] = UP[3*i+0];
            eEst[idep*l1+i] = UP[3*i+1];
            uEst[idep*l1+i] =-UP[3*i+2];
        }
    } // Loop on source depths
    if (ierr != 0)
    {
        log_errorF("%s: Errors were detect during the grid search\n", fcnm);
    }
    else
    {
        if (verbose > 2)
        {
            log_debugF("%s: Grid-search time: %f (s)\n", fcnm, ISCL_time_toc());
        }
    }
ERROR:;
    ISCL_memory_free__double(&diagWt);
    ISCL_memory_free__double(&WG);
    ISCL_memory_free__double(&G);
    ISCL_memory_free__double(&WU);
    ISCL_memory_free__double(&U);
    ISCL_memory_free__double(&UP);
    ISCL_memory_free__double(&xrs);
    ISCL_memory_free__double(&yrs);
    ISCL_memory_free__double(&zrs_negative);
    return ierr;
}
 
