#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include "gfast.h"

/*!
 * @brief Computes the predicted mangitude using PGD and Pd from
 *        the seismogeodetic data by solving the overdetermined system:
 *         \f[ 
 *            \left [B + C \log_{10}(r) \right ] \textbf{m}
 *          = \log_{10}(d) - A
 *         \f] 
 *        for the the magnitude m.  Here r is the hypocentral distance,
 *        d is the displacement, A, B, and C are defined in the routine. 
 *
 * @param[in] n         length of arrays 
 * @param[in] verbose   controls verbosity (0 is completely quiet)
 * @param[in] dist_tol  displacement tolerance (cm).  if the displacment
 *                      is less than dist_tol it would be set to dist_tol 
 * @param[in] dist_def  displacement default (cm) if d < dist_tol
 * @param[in] d         displacement (cm) [n] 
 * @param[in] r         hypocentral distance (km) [n]
 * @param[in] repi      epicentral distance (km) [n]
 *
 * @param[out] M        magnitude
 * @param[out] VR       variance reduction (percentage)
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @addtogroup PGD
 *
 */
int GFAST_scaling_PGD(int n, int verbose,
                      double dist_tol, double dist_def,
                      double *d, double *r, double *repi,
                      double *M, double *VR)
{
    const char *fcnm = "GFAST_scaling_PGD\0";
    double *b, *G, *UP, *Wb, *WG, M1[1],
           W, b10, repi_min, repi_min2, res, xden, xnum;
    int i, ierr;
    const double A = -6.687;
    const double B = 1.500;
    const double C = -0.214;
    //const double A = -4.434;
    //const double B = 1.047;
    //const double C = -0.138;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    *VR = 0.0;
    *M = 0.0;
    b = NULL;
    G = NULL;
    UP = NULL;
    Wb = NULL;
    WG = NULL;
    // Error check
    if (n < 1){
        if (verbose > 0){
            log_errorF("%s: Error invalid number of input stations: %d\n",
                       fcnm, n);
        }
        goto ERROR;
    }
    // Get the min epicentral distance 
    repi_min = numpy_min(n, repi);
    repi_min2 = pow(repi_min, 2);
    // Generate the over-determined system: [B + C*log10(r)]*m = log10(d) - A
    G  = (double *)calloc(n*1, sizeof(double));
    b  = (double *)calloc(n,   sizeof(double));
    ierr = GFAST_scaling_PGD__setForwardModel(n, verbose,
                                              B, C, r,
                                              G);
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error creating G matrix\n", fcnm);
        }
        goto ERROR;
    }
    ierr = GFAST_scaling_PGD__setRHS(n, verbose,
                                     dist_tol, dist_def,
                                     A, d,
                                     b);
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error creating RHS\n", fcnm);
        }
        goto ERROR;
    }
    // Apply weights
    WG = (double *)calloc(n, sizeof(double));
    Wb = (double *)calloc(n, sizeof(double));
    for (i=0; i<n; i++){
        W = exp(-pow(repi[i], 2)/8.0/repi_min2); // Compute weights
        WG[i] = W*G[i];
        Wb[i] = W*b[i];
    }
    // Solve the weighted least squares problem (M = lstsq(W*G,W*b)[0])
    ierr = numpy_lstsq(LAPACK_COL_MAJOR, 
                       n, 1, 1, WG, Wb,
                       NULL, M1, NULL, NULL); 
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error solving the least-squares problem\n", fcnm);
        }
        goto ERROR;
    }
    // Compute the estimates: UP = G*M
    UP = (double *)calloc(n, sizeof(double));
    cblas_dgemv(CblasColMajor, CblasNoTrans,
                n, 1, 1.0, G, n, M1, 1, 0.0, UP, 1);
    // Compute the variance reduction
    xnum = 0.0;
    xden = 0.0;
    for (i=0; i<n; i++){
        res = pow(10.0, b[i]) - pow(10.0, UP[i]);
        xnum = xnum + res*res;
        b10 = pow(10.0, b[i]);
        xden = xden + b10*b10;
    }
    // Copy results
    *M = M1[0];
    *VR = (1.0 - sqrt(xnum/xden))*100.0;
ERROR:; // An error was encountered
    // Free space
    if (b  != NULL){free(b);}
    if (G  != NULL){free(G);}
    if (UP != NULL){free(UP);}
    if (Wb != NULL){free(Wb);}
    if (WG != NULL){free(WG);}
    b  = NULL;
    G  = NULL;
    UP = NULL;
    Wb = NULL;
    WG = NULL;
    return 0;
}
//============================================================================//
/*!
 * @brief Sets the forward modeling matrix G s.t.
 *        \f$ G = \left [ B + C \log_{10}(r) \right ] \f$
 *        where B and C are scalar shifts and scale factors respectively
 *        and r the hypocentral distances for all stations.
 *
 * @param[in] n        length of r and G (> 0)
 * @param[in] verbose  controls verbosity (0 is quiet)
 * @param[in] B        linear shift in G = B + C*log10(r)
 * @param[in] C        scale factor in G = B + C*log10(r) 
 * @param[in] r        hypocentral distances (km) [n] 
 *
 * @param[out] G       forward modeling matrix for PGD [n x 1].  Note that
 *                     G is in column major format.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @addtogroup PGD
 * 
 */
int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                       double B, double C,
                                       double *__restrict__ r,
                                       double *__restrict__ G)
{ 
    const char *fcnm = "GFAST_scaling_PGD__setForwardModel\0";
    int i;
    if (n < 1){
        if (verbose > 0){
            log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        }
        return -1;
    }
    for (i=0; i<n; i++){
        G[i] = B + C*(log10(r[i]));
    }
    return 0;
}
//============================================================================//
/*!
 * @brief Computes the right hand side in the peak ground displacement 
 *        estimation s.t.
 *        \f$ \textbf{b} = \left \{ \log_{10}(d) - A \right \} \f$
 *        where A is a scalar shift and d is the displacement at each station.
 *
 * @param[in] n         number of points
 * @param[in] verbose   controls verbosity (0 is quiet)
 * @param[in] dist_tol  distance tolerance - if d is less than this then
 *                      it will be set to a default value (km)
 * @param[in] dist_def  distance default value (km)
 * @param[in] d         displacement (cm) [n]
 * @param[in] A         shift so that b = log10(d) - A
 *
 * @param[out] b        right hand side in Gm = b [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @addtogroup PGD
 *
 */
int GFAST_scaling_PGD__setRHS(int n, int verbose,
                              double dist_tol, double dist_def,
                              double A, double *__restrict__ d,
                              double *__restrict__ b) 
{
    const char *fcnm = "GFAST_scaling_PGD__setRHS\0";
    double dist;
    int i;
    if (n < 1){
        if (verbose > 0){
            log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        }
        return -1;
    }
    for (i=0; i<n; i++){
        dist = d[i];
        if (dist - dist_tol < 0.0){dist = dist_def;}
        b[i] = log10(dist) - A;
    }
    return 0;
}
