#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <stdbool.h>
#include "gfast.h"

#define n10 10
/*!
 * @defgroup CMT Centroid Moment Tensor and Finite Fault Estimation
 *
 * This module is responsible for estimating the centroid moment tensor 
 * and finite fault.
 *
 */

/*!
 * @brief Drives the centroid moment tensor inversion.
 *
 */
int GFAST_moment_tensor(int l1, int verbose, int zone_in, int *signal_ptr,
                        double *sta_lat, double *sta_lon,
                        double *n, double *e, double *u,
                        double SA_lat, double SA_lon, double SA_dep,
                        double *epidist,
                        double *S, double *VR, double *Mw)
{
    const char *fcnm = "GFAST_moment_tensor\0"; 
    double *U, *UP, *G, *azims, *backazi, *xrs, *yrs, *zrs,
           NN[n10], EE[n10], UU[n10], azi,
           E, Mo, N, r, rcond, res_norm2, t, u_norm2, x1, x2, y1, y2;
#ifndef WITH_LIBGEO
    double lon0_deg =-123.0;
#endif
    int efftime, i, ierr, indx, iwarn, k, ldg, m, rank, zone;
    bool lnorthp;
    const double sqrt22 = 0.7071067811865475; // 1/sqrt(2)
    const double pi180i = 180.0/M_PI;
    const int nrhs = 1;
    double alpha = 1.0, beta = 0.0;
    int n6 = 6;
    int incx = 1, incy = 1;
    //------------------------------------------------------------------------//
    //
    // Initialize
    *VR = 0.0;
    *Mw = 0.0;
    for (i=0; i<n6; i++){
        S[i] = 0.0;
    }
    // Reality check on sizes
    if (l1 < 1){
        printf("%s: Error no stations at which to invert\n", fcnm);
        ierr = 1;
        return ierr;
    }
    xrs = NULL;
    yrs = NULL;
    zrs = NULL;
    azims = NULL;
    backazi = NULL;
    U = NULL;
    G = NULL;
    // Allocate space
    m  = 3*l1;  // number of rows in G
    ldg = m;    // leading dimension of G
    xrs = (double *)calloc(l1, sizeof(double));
    yrs = (double *)calloc(l1, sizeof(double));
    zrs = (double *)calloc(l1, sizeof(double));
    azims = (double *)calloc(l1, sizeof(double));  // Azimuths
    backazi = (double *)calloc(l1, sizeof(double));// Backazimuth
    UP = (double *)calloc(m, sizeof(double));      // Estimates UP = G*S
    U = (double *)calloc(m, sizeof(double));       // Data G*S = U
    G = (double *)calloc(ldg*n6, sizeof(double));  // Forward modeling matrix; G
    S = (double *)calloc(n6, sizeof(double));      // Solution; G*S = U
    // Compute the source UTM location
#ifdef WITH_LIBGEO
    if (zone_in < 0 || zone_in > 60){
        zone =-1;
    }else{
        zone = zone_in;
    }
    ierr = geodetic_coordtools_ll2utm(sta_lat[i], sta_lon[i],
                                      &x2, &y2,
                                      &lnorthp, &zone);
    if (ierr != 0){ 
        if (verbose > 0){ 
            log_errorF("%s: Error computing source UTM\n", fcnm);
        }   
        goto ERROR;
    }
    for (i=0; i<l1; i++){
        ierr = geodetic_coordtools_ll2utm(sta_lat[i], sta_lon[i],
                                          &xrs[i], &yrs[i],
                                          &lnorthp, &zone);
        if (ierr != 0){ 
            if (verbose > 0){ 
                log_errorF("%s: Error computing station UTM\n", fcnm);
            }
            goto ERROR;
        }
    }
#else
    if (zone == 10){
        lon0_deg =-123.0;
    }
    GFAST_coordtools_ll2utm(SA_lon, SA_lat, lon0_deg,
                            &x2, &y2);
    for (i=0; i<l1; i++){
        GFAST_coordtools_ll2utm(SA_lon, SA_lat, lon0_deg,
                                &xrs[i], &yrs[i]);
    }
#endif
    // Compute distances, azimuths, and relative offsets in (x,y,z) coordinates
    for (i=0; i<l1; i++){
        // Compute the station UTM location
        x1 = xrs[i];
        y1 = yrs[i];
        // Compute azimuth and back-azimuth 
        azi = 90.0 - pi180i*atan2(x2-x1, y2-y1);
        if (azi  < 0.0)  {azi = azi + 360.0;}
        if (azi  > 360.0){azi = azi - 360.0;}
        if (azi  < 180.0){backazi[i] = azi + 180.0;}
        if (azi  > 180.0){backazi[i] = azi - 180.0;}
        if (azi == 180.0){backazi[i] = 0.0;}
        // Relative source receiver location in (x,y,z) coordinates
        xrs[i] = x1 - x2;
        yrs[i] = y1 - y2;
        zrs[i] = SA_dep*1000.0;
        azims[i] = 90.0 - azi + 180.0;
    } // Loop on stations
    // Extract the time series
    for (i=0; i<l1; i++){
        efftime = ceil(epidist[i]/2.0/1000.0);
        if (efftime < 290){
            indx = signal_ptr[i] + efftime;
        }else{
            indx = signal_ptr[i] + 290;
        }
        // Loop on last n10 points
        for (k=0; k<n10; k++){
            NN[k] = n[indx]; //[i,efftime:efftime+10]
            EE[k] = e[indx]; //[i,efftime:efftime+10]
            UU[k] = u[indx]; //[i,efftime:efftime+10]
            indx = indx + 1;
        }
        N = numpy_nanmean(n10, NN, &iwarn);
        E = numpy_nanmean(n10, EE, &iwarn);
        // Rotate
        obspy_rotate_NE2RT(1, &N, &E, backazi[i], &r, &t);
        // Fill the obervation array 
        indx = 3*i;
        U[indx  ] = r;
        U[indx+1] = t; 
        U[indx+2] = numpy_nanmean(n10, UU, &iwarn); 
    }
    // Compute the corresponding Green's functions
    ierr = GFAST_CMTgreenF(l1, verbose, 
                           xrs, yrs, zrs, azims,
                           G); 
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error computing Green's functions\n", fcnm);
        }
        goto ERROR; 
    }
    // Solve the least squares problem via the SVD
    rcond =-1.0; // Use machine epsilon for singular value cutoff
    ierr = numpy_lstsq(LAPACK_ROW_MAJOR, m, n6, nrhs, G, U,
                       &rcond, S, &rank, NULL);
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error solving least squares problem\n", fcnm);
        }
        goto ERROR;
    }
    if (rank != n6){
        if (verbose > 1){
            log_warnF("%s: Warning: G has rank=%d which is deficient\n",
                      fcnm, rank);
        }
    }
    // Compute the scalar moment
    Mo = sqrt22*sqrt(   pow(S[0],2) +   pow(S[1],2) +   pow(S[2],2)
                    + 2*pow(S[3],2) + 2*pow(S[4],2) + 2*pow(S[5],2));
    if (Mo == 0.0){
        *Mw = 0.0;
    }else{
        *Mw = 2.0*log10(Mo)/3.0 - 6.0;
    }
    // Compute the estimates
    cblas_dgemv(CblasRowMajor, CblasNoTrans,
                m, n6, alpha, G, ldg, S, incx, beta, UP, incy);
    // Compute the variance reduction 
    res_norm2 = 0.0;
    u_norm2 = 0.0;
    for (i=0; i<m; i++){
        r = U[i] - UP[i];
        res_norm2 = res_norm2 + r*r;
        u_norm2 = u_norm2 + U[i]*U[i];
    }
    *VR = (1.0 - sqrt(res_norm2/u_norm2))*100.0;
    // Compute the nodal planes
 
    // Free memory
ERROR:;
    if (xrs != NULL){free(xrs);}
    if (yrs != NULL){free(yrs);}
    if (zrs != NULL){free(zrs);}
    if (azims != NULL){free(azims);}
    if (backazi != NULL){free(backazi);}
    if (UP != NULL){free(UP);}
    if (U != NULL){free(U);}
    if (G != NULL){free(G);}
    return 0;
}
