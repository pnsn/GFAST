#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <cblas.h>
#include <math.h>
#include <lapacke.h>
#include "gfast.h"

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
 * @param[in] utmRecvEasting   receiver easting UTM positions (m) [l1]
 * @param[in] utmRecvNorthing  receiver northing UMT positions (m) [l1]
 * @param[in] staAlt           station elevations above sea level (m) [l1]
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_CMT__depthGridSearch(int l1, int ndeps,
                               int verbose,
                               bool deviatoric,
                               double utmSrcEasting,
                               double utmSrcNorthing,
                               double *__restrict__ srcDepths,
                               double *__restrict__ utmRecvEasting,
                               double *__restrict__ utmRecvNorthing,
                               double *__restrict__ staAlt,
                               double *__restrict__ nAvgDisp,
                               double *__restrict__ eAvgDisp,
                               double *__restrict__ uAvgDisp,
                               double *__restrict__ cmt_vr,
                               double *__restrict__ mts,
                               double *__restrict__ str1,
                               double *__restrict__ str2,
                               double *__restrict__ dip1,
                               double *__restrict__ dip2,
                               double *__restrict__ rak1,
                               double *__restrict__ rak2,
                               double *__restrict__ Mw)
{
    const char *fcnm = "GFAST_CMT__depthGridSearch\0";
    double *G, *U, *UP, *xrs, *yrs, *zrs_negative, S[6],
           DC_pct, eq_alt, m11, m12, m13, m22, m23, m33,
           res, sum_res2;
    int i, idep, ierr, ierr1, ldg, mrows, ncols;
    // Initialize
    ierr = 0;
    G = NULL;
    U = NULL;
    UP = NULL;
    xrs = NULL;
    yrs = NULL;
    zrs_negative = NULL;
    // Error check
    if (l1 < 1){ 
        log_errorF("%s: Error invalid number of input stations: %d\n",
                   fcnm, l1);
        ierr = 1;
        goto ERROR;
    }   
    if (srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        nAvgDisp == NULL || eAvgDisp == NULL || uAvgDisp == NULL ||
        mts == NULL || cmt_vr == NULL)
    {
        if (srcDepths == NULL){log_errorF("%s: srcDepths is NULL!\n", fcnm);}
        if (utmRecvEasting == NULL){
            log_errorF("%s: utmRecvEasting is NULL!\n", fcnm);
        }
        if (utmRecvNorthing == NULL){
            log_errorF("%s: utmRecvNorthing is NULL!\n", fcnm);
        }
        if (staAlt == NULL){log_errorF("%s: staAlt is NULL\n", fcnm);}
        if (nAvgDisp == NULL){log_errorF("%s: nAvgDisp is NULL\n", fcnm);}
        if (eAvgDisp == NULL){log_errorF("%s: eAvgDisp is NULL\n", fcnm);}
        if (uAvgDisp == NULL){log_errorF("%s: uAvgDisp is NULL\n", fcnm);}
        if (mts == NULL){log_errorF("%s: mts is NULL\n", fcnm);}
        if (cmt_vr == NULL){log_errorF("%s: cmt_vr is NULL\n", fcnm);}
        ierr = 1;
        goto ERROR;
    }
    if (ndeps < 1){ 
        log_errorF("%s: Error invalid number of source depths: %d\n",
                   fcnm, ndeps);
        ierr = 1;
        goto ERROR;
    }
    if (!deviatoric){
        log_errorF("%s: Cannot perform general MT gridsearch!\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    // Initialize results to nothing
    #pragma omp simd
    for (idep=0; idep<ndeps; idep++){
        mts[6*idep+0] = 0.0;
        mts[6*idep+1] = 0.0;
        mts[6*idep+2] = 0.0;
        mts[6*idep+3] = 0.0;
        mts[6*idep+4] = 0.0;
        mts[6*idep+5] = 0.0;
        cmt_vr[idep] = 0.0;
        str1[idep] = 0.0;
        str2[idep] = 0.0;
        dip1[idep] = 0.0;
        dip2[idep] = 0.0;
        rak1[idep] = 0.0;
        rak2[idep] = 0.0;
        Mw[idep] = 0.0;
    }
    // Set space
    if (deviatoric){
        ncols = 5;
    }else{
        ncols = 6;
    }
    mrows = 3*l1;
    ldg = ncols;  // In row major format
    G = GFAST_memory_calloc__double(mrows*ncols);
    U = GFAST_memory_calloc__double(mrows);
    UP = GFAST_memory_calloc__double(mrows);
    xrs = GFAST_memory_calloc__double(l1);
    yrs = GFAST_memory_calloc__double(l1);
    zrs_negative = GFAST_memory_calloc__double(l1);
    // Compute the source-receiver offsets in (x, y) cartesian
    #pragma omp simd
    for (i=0; i<l1; i++){
        xrs[i] = utmRecvEasting[i] - utmSrcEasting;
        yrs[i] = utmRecvNorthing[i] - utmSrcNorthing;
    }
    // Set the RHS
    ierr = GFAST_CMT__setRHS(l1, verbose,
                             nAvgDisp, eAvgDisp, uAvgDisp, U);
    if (ierr != 0){
        log_errorF("%s: error setting RHS!\n", fcnm);
        goto ERROR;
    }
    // Grid search on source depths
#ifdef __PARALLEL_CMT
    #pragma omp parallel for \
     firstprivate(G, U, zrs_negative) \
     private (DC_pct, i, idep, ierr1, eq_alt, m11, m22, m33, m12, m13, m23, res, S, sum_res2 ) \
     shared (cmt_vr, dip1, dip2, fcnm, ldg, l1, mts, mrows, Mw, ncols, ndeps, rak1, rak2, srcDepths, staAlt, str1, str2, UP, xrs, yrs) \
     reduction(+:ierr) default(none)
#endif
    for (idep=0; idep<ndeps; idep++){
        eq_alt = srcDepths[idep]*1.e3;
        #pragma omp simd
        for (i=0; i<l1; i++){
            zrs_negative[i] =-(staAlt[i] + eq_alt);
        }
        // Set the forward modeling matrix - note convention of xrs and yrs 
        ierr1 = GFAST_CMT__setForwardModel__deviatoric(l1,
                                                       yrs,
                                                       xrs,
                                                       zrs_negative,
                                                       G);
        if (ierr1 != 0){
            log_errorF("%s: Error constructing Green's function matrix\n",
                       fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Solve the least squares problem
        ierr1 = numpy_lstsq__qr(LAPACK_ROW_MAJOR,
                                mrows, ncols, 1, G, U,
                                S);
        if (ierr1 != 0){
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
        // Compute the L2 misfit which is called the variance reduction
        sum_res2 = 0.0;
        #pragma omp simd aligned(UP:CACHE_LINE_SIZE) reduction(+:sum_res2)
        for (i=0; i<mrows; i++){
            res = U[i] - UP[i];
            sum_res2 = sum_res2 + res*res;
        }
        sum_res2 = sqrt(sum_res2); 
        // Compute the moment tensor decomposition
        ierr1 = GFAST_CMT__decomposeMomentTensor(&mts[6*idep],
                                                 &DC_pct,
                                                 &Mw[idep],
                                                 &str1[idep], &str2[idep],
                                                 &dip1[idep], &dip2[idep],
                                                 &rak1[idep], &rak2[idep]);
        if (ierr1 != 0){
            log_errorF("%s: Error decomposing moment tensor\n", fcnm);
            ierr = ierr + 1;
        }
        // Prefer results with larger double couple percentages
        cmt_vr[idep] = sum_res2/DC_pct;
    } // Loop on source depths
ERROR:;
    GFAST_memory_free__double(&G);
    GFAST_memory_free__double(&U);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&xrs);
    GFAST_memory_free__double(&yrs);
    GFAST_memory_free__double(&zrs_negative);
    return ierr;
}
 
