#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <omp.h>
#include "gfast.h"

/*!
 * @brief Computes the predicted mangitude using PGD and Pd from
 *        the geodetic data by solving the overdetermined system:
 *         \f[ 
 *            W \left [B + C \log_{10}(r) \right ] \textbf{m}
 *          = W \left \{ \log_{10}(d) - A \right \}
 *         \f] 
 *        for the the magnitude m.  Here r is the hypocentral distance,
 *        d is the displacement, A, B, and C are defined in the routine. 
 *        The weighting matrix is a diagonal matrix defined by: 
 *          \f[ W = diag
 *                  \left[
 *                     e^{-\frac{\Delta^2}{8 \min(\Delta^2) }}
 *                  \right ] \f]
 *        where \f$ \Delta \f$ is the epicentral distance.
 *
 * @param[in] l1               number of sites
 * @param[in] verbose          controls verbosity (< 2 is quiet)
 * @param[in] dep              event depth (km)
 * @param[in] dist_tol         displacement tolerance (cm).  if the displacment
 *                             is less than dist_tol it would be set to dist_tol
 * @param[in] dist_def         displacement default (cm) if d < dist_tol
 * @param[in] utmSrcNorthing   source UTM northing position (m)
 * @param[in] utmSrcEasting    source UTM easting position (m)
 * @param[in] srcDepths        source depth in grid search (km) [ndeps]
 * @param[in] utmRecvNorthing  receiver UTM northing position (m) [l1]
 * @param[in] utmRecvEasting   receiver UTM easting position (m) [l1]
 * @param[in] staAlt           station elevation (m) [l1]
 * @param[in] d                distance (cm) [n]
 * @param[in] repi             epicentral distance (km) [n]
 *
 * @param[out] M               magnitude at each depth [ndeps]
 * @param[out] VR              variance reduction (percentage) at each
 *                             depth [ndeps]
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_scaling_PGD__depthGridSearch(int l1, int ndeps,
                                       int verbose,
                                       double dist_tol,
                                       double dist_def,
                                       double utmSrcEasting,
                                       double utmSrcNorthing,
                                       double *__restrict__ srcDepths,
                                       double *__restrict__ utmRecvEasting,
                                       double *__restrict__ utmRecvNorthing,
                                       double *__restrict__ staAlt,
                                       double *__restrict__ d,
                                       double *__restrict__ repi,
                                       double *__restrict__ M,
                                       double *__restrict__ VR)
{
    const char *fcnm = "GFAST_scaling_PGD__depthGridSearch\0";
    double *b, *G, *r, *UP, *W, *Wb, *WG, M1[1],
           repi_min, repi_min2, res, srcDepth, xden, xnum;
    int i, idep, ierr, ierr1;
    const double A = -6.687;
    const double B = 1.500;
    const double C = -0.214;
    //const double A = -4.434; /* TODO remove with approval */
    //const double B = 1.047;  /* TODO remove with approval */
    //const double C = -0.138; /* TODO remove with approval */
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    r = NULL;
    b = NULL;
    G = NULL;
    UP = NULL;
    W  = NULL;
    Wb = NULL;
    WG = NULL;
    // Error check
    if (l1 < 1){
        log_errorF("%s: Error invalid number of input stations: %d\n",
                   fcnm, l1);
        goto ERROR;
    }
    if (ndeps < 1){
        log_errorF("%s: Error invalid number of source depths: %d\n",
                   fcnm, ndeps);
        goto ERROR;
    }
    // Initialize result to nothing
    #pragma omp simd
    for (idep=0; idep<ndeps; idep++){
        M[idep] = 0.0;
        VR[idep] = 0.0;
    }
    // Set space
    G  = GFAST_memory_calloc__double(l1*1);
    b  = GFAST_memory_calloc__double(l1);
    r  = GFAST_memory_calloc__double(l1);
    W  = GFAST_memory_calloc__double(l1);
    WG = GFAST_memory_calloc__double(l1*1);
    Wb = GFAST_memory_calloc__double(l1);
    UP = GFAST_memory_calloc__double(l1);
    // Get the min epicentral distance 
    repi_min = numpy_min(l1, repi);
    repi_min2 = pow(repi_min, 2);
    // Set the RHS log10(d) - A
    ierr = GFAST_scaling_PGD__setRHS(l1, verbose,
                                     dist_tol, dist_def,
                                     A, d,
                                     b);
    if (ierr != 0){
        log_errorF("%s: Error creating RHS\n", fcnm);
        goto ERROR;
    }
    // Loop on depths
#ifdef __PARALLEL_PGD
    #pragma omp parallel for \
     firstprivate(A, B, C, G, r, repi_min2, UP, verbose, W, WG, Wb) \
     private(i, idep, ierr1, M1, res, srcDepth, xden, xnum) \
     shared(b, d, dist_def, dist_tol, fcnm, l1, M, ndeps, repi, srcDepths, staAlt, utmRecvEasting, utmRecvNorthing, utmSrcNorthing, utmSrcEasting, VR) \
     reduction(+:ierr) default(none)
#endif
    for (idep=0; idep<ndeps; idep++){
        // Compute radial distance
        srcDepth = srcDepths[idep];
        #pragma omp simd aligned(r:CACHE_LINE_SIZE)
        for (i=0; i<l1; i++){
            r[i] = sqrt( pow(utmSrcEasting  - utmRecvEasting[i], 2)
                       + pow(utmSrcNorthing - utmRecvNorthing[i], 2)
                       + pow(srcDepth*1000.0 - staAlt[i], 2) )*1.e-3;
        }
        // Generate the over-determined system: [B + C*log10(r)]*m = RHS 
        ierr1 = GFAST_scaling_PGD__setForwardModel(l1, verbose,
                                                   B, C, r,
                                                   G);
        if (ierr1 != 0){
            log_errorF("%s: Error creating G matrix\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Apply weights
        #pragma omp simd aligned(WG,Wb:CACHE_LINE_SIZE)
        for (i=0; i<l1; i++){
            W[i] = exp(-pow(repi[i], 2)/8.0/repi_min2); // Compute weights
            WG[i] = W[i]*G[i];
            Wb[i] = W[i]*b[i];
        }
        // Solve the weighted least squares problem (M = lstsq(W*G,W*b)[0])
        ierr1 = numpy_lstsq__qr(LAPACK_COL_MAJOR,
                                l1, 1, 1, WG, Wb, M1);
        if (ierr1 != 0){
            log_errorF("%s: Error solving the least-squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the estimates: UP = G*M
        cblas_dgemv(CblasColMajor, CblasNoTrans,
                    l1, 1, 1.0, G, l1, M1, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
        #pragma omp simd reduction(+:xnum, xden) aligned(UP,b:CACHE_LINE_SIZE)
        for (i=0; i<l1; i++){
            res = W[i]*(d[i] - pow(10.0, UP[i] + A));
            xnum = xnum + sqrt(res*res);
            xden = xden + sqrt(pow(W[i]*d[i], 2));
        }
        // Copy results
        M[idep] = M1[0];
        VR[idep] = (1.0 - xnum/xden)*100.0;
    } // Loop on depths
    if (ierr != 0){
        log_errorF("%s: Errors detected during grid search\n", fcnm);
        ierr = 1;
    }
ERROR:; // An error was encountered
    // Free space
    GFAST_memory_free__double(&r);
    GFAST_memory_free__double(&b);
    GFAST_memory_free__double(&G);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&W);
    GFAST_memory_free__double(&Wb);
    GFAST_memory_free__double(&WG);
    return 0;
}
