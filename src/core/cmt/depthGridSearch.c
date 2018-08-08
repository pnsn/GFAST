#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"
#ifdef GFAST_USE_ISCL
 #ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Weverything"
 #endif
 #include <mkl_lapacke.h>
 #include <mkl_cblas.h>
 #ifdef __clang__
  #pragma clang diagnostic pop
 #endif
#else
#include <lapacke.h>
#include <cblas.h>
#endif
#include "iscl/array/array.h"
#include "iscl/linalg/linalg.h"
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
        LOG_ERRMSG("Error invalid number of input stations: %d",l1);
        ierr = 1;
        goto ERROR;
    }   
    if (srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        nObsOffset == NULL || eObsOffset == NULL || uObsOffset == NULL ||
        nEst == NULL || eEst == NULL || uEst == NULL || 
        mts == NULL)
    {
        if (srcDepths == NULL){LOG_ERRMSG("%s", "srcDepths is NULL!");}
        if (utmRecvEasting == NULL)
        {
            LOG_ERRMSG("%s", "utmRecvEasting is NULL!");
        }
        if (utmRecvNorthing == NULL)
        {
            LOG_ERRMSG("%s", "utmRecvNorthing is NULL!");
        }
        if (staAlt == NULL){LOG_ERRMSG("%s", "staAlt is NULL");}
        if (nObsOffset == NULL){LOG_ERRMSG("%s", "nObsOffset is NULL");}
        if (eObsOffset == NULL){LOG_ERRMSG("%s", "eObsOffset is NULL");}
        if (uObsOffset == NULL){LOG_ERRMSG("%s", "uObsOffset is NULL");}
        if (nEst == NULL){LOG_ERRMSG("%s", "nEst is NULL");}
        if (eEst == NULL){LOG_ERRMSG("%s", "eEst is NULL");}
        if (uEst == NULL){LOG_ERRMSG("%s", "uEst is NULL");}
        if (mts == NULL){LOG_ERRMSG("%s", "mts is NULL");}
        ierr = 1;
        goto ERROR;
    }
    if (ndeps < 1)
    {
        LOG_ERRMSG("Error invalid number of source depths: %d", ndeps);
        ierr = 1;
        goto ERROR;
    }
    if (!deviatoric)
    {
        LOG_ERRMSG("%s", "Cannot perform general MT gridsearch!");
        ierr = 1;
        goto ERROR;
    }
    // Initialize results to nothing
    array_zeros64f_work(6*ndeps, mts);
    array_zeros64f_work(l1*ndeps, uEst);
    array_zeros64f_work(l1*ndeps, nEst);
    array_zeros64f_work(l1*ndeps, eEst);
    // Set space
    ncols = 6;
    if (deviatoric){ncols = 5;}
    mrows = 3*l1;
    ldg = ncols;  // In row major format
    diagWt = memory_calloc64f(mrows);
    WU     = memory_calloc64f(mrows);
    U      = memory_calloc64f(mrows);
    xrs    = memory_calloc64f(l1);
    yrs    = memory_calloc64f(l1);
    // Compute the source-receiver offsets in (x, y) cartesian
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<l1; i++)
    {
        xrs[i] = utmRecvEasting[i] - utmSrcEasting;
        yrs[i] = utmRecvNorthing[i] - utmSrcNorthing;
    }
    // Set the RHS
    ierr = core_cmt_setRHS(l1, nObsOffset, eObsOffset, uObsOffset, U);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting RHS!");
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
        LOG_ERRMSG("%s", "Failed to set weight matrix - will set to identity");
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
        LOG_ERRMSG("%s", "Failed to apply data weights to data");
        return -1;
    }
    // Grid search on source depths
    time_tic();
    if (verbose > 2)
    {
        LOG_DEBUGMSG("%s", "Beginning search on depths...");
    }
#ifdef PARALLEL_CMT
    #pragma omp parallel \
     private (G, UP, WG, zrs_negative) \
     private (i, idep, ierr1, eq_alt, m11, m22, m33, m12, m13, m23, S ) \
     shared (diagWt, eEst, ldg, mts, mrows, \
             ncols, nEst, srcDepths, staAlt, \
             uEst, U, WU, xrs, yrs) reduction(+:ierr) default (none)
    {
#endif
    G            = memory_calloc64f(mrows*ncols);
    WG           = memory_calloc64f(mrows*ncols);
    UP           = memory_calloc64f(mrows);
    zrs_negative = memory_calloc64f(l1);
#ifdef PARALLEL_CMT
    #pragma omp for
#endif
    for (idep=0; idep<ndeps; idep++)
    {
        // Get the z source receiver offset
        eq_alt = srcDepths[idep]*1.e3;
#ifdef _OPENMP
        #pragma omp simd
#endif
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
            LOG_ERRMSG("%s", "Error constructing Green's function matrix");
            ierr = ierr + 1;
            continue;
        }
        // Apply the data weights
        ierr1 = core_cmt_weightForwardModel(mrows, ncols, diagWt, G, WG);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error weighting forward modeling matrix!");
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
            LOG_ERRMSG("%s", "Error solving least squares problem");
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
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (i=0; i<l1; i++)
        {
            nEst[idep*l1+i] = UP[3*i+0];
            eEst[idep*l1+i] = UP[3*i+1];
            uEst[idep*l1+i] =-UP[3*i+2];
        }
    } // Loop on source depths
    memory_free64f(&G);
    memory_free64f(&WG);
    memory_free64f(&UP);
    memory_free64f(&zrs_negative);
#ifdef PARALLEL_CMT
    } // End the parallel region
#endif
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Errors were detect during the grid search");
    }
    else
    {
        if (verbose > 2)
        {
            LOG_DEBUGMSG("Grid-search time: %f (s)", time_toc());
        }
    }
ERROR:;
    memory_free64f(&diagWt);
    memory_free64f(&WU);
    memory_free64f(&U);
    memory_free64f(&xrs);
    memory_free64f(&yrs);
    return ierr;
}
 
