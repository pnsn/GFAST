#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <stdbool.h>
#include <omp.h>
#include <lapacke.h>
#include "gfast.h"

#define n10 10

static double _cmt_varianceReduction(int m, double *obs, double *est, int *ierr);
static double __cmt_doubleCouplePercentage(double M6[6], int *ierr);

/*!
 * @brief Drives the depth gridsearch centroid moment tensor inversion.
 *
 * @param[in] l1           number of sites
 * @param[in] ndeps        number of depths in CMT grid search 
 * @param[in] verbose      controls verbosity (< 2 is quiet)
 * @param[in] time_window 
 * @param[in] deviatoric   if true then the moment tensor is constrained
 *                         to be purely deviatoric i.e. 
 *                         \f$ m_{zz} =-(m_{xx} + m_{yy}) \f$.
 *                         otherwise, all six moment tensor terms are 
 *                         are inverted for. 
 * @param[in] weight_dc    if true then weight the variance reduction by
 *                         the double couple percentage like in Doug Dreger.
 *                         otherwise, leave the the variance reduction 
 *                         unperturbed. 
 * @param[in] zone_in      UTM zone.  If out of bounds [0,60] then the UTM zone
 *                         will be selected by computing the UTM zone for the
 *                         source and applying that too all the receivers
 * @param[in] signal_ptr 
 * @param[in] sta_lat      station latitudes (degrees) [l1]
 * @param[in] sta_lon      station latitudes (degrees) [l1]
 * @param[in] dt           station sampling periods (seconds) [l1]
 * @param[in] n            north displacements 
 * @param[in] e            east displacments
 * @param[in] u            vertical displacements
 * @param[in] SA_lat       source latitude (degrees)
 * @param[in] SA_lon       source longitude (degrees)
 * @param[in] SA_dep       source depths in grid search (km) [ndeps]
 * @param[in] epidist      source/receiver epicentral distances [l1]
 *
 * @param[out] S           best fitting moment tensor at each depth [6*ndep]
 *                         the moment tensors
 *                         \f$\{ m_{xx}^i, m_{yy}^i, m_{zz}^i
 *                               m_{xy}^i, m_{xz}^i, m_{yz}^i \}\f$ 
 *                         for the i'th depth begin at index 6*i.
 * @param[out] VR          variance reduction at each depth [ndeps] 
 * @param[out] Mw          moment magnitude at each depth [ndeps]
 * 
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * @date March 2016
 *
 */
int GFAST_CMT__depthGridSearch(int l1, int ndeps, int verbose,
                               double time_window,
                               bool deviatoric,
                               bool weight_dc,
                               int zone_in, int *signal_ptr,
                               double *__restrict__ sta_lat,
                               double *__restrict__ sta_lon,
                               double *__restrict__ dt,
                               double *__restrict__ n,
                               double *__restrict__ e,
                               double *__restrict__ u,
                               double SA_lat, double SA_lon,
                               double *__restrict__ SA_dep,
                               double *__restrict__ epidist,
                               double *S, double *VR, double *Mw)
{
    const char *fcnm = "GFAST_CMT__depthGridSearch\0";
    double *U, *UP, *G, *azims, *backazi, *xrs, *yrs, *zrs,
           Swork[6], NN[n10], EE[n10], UU[n10], dc_pct, azi,
           E, Mo, N, r, rcond, t, x1, x2, y1, y2;
    int efftime, i, idep, ierr, ierr1, indx, iwarn, k, ldg,
        m, ncol, nwin_max, rank, zone;
    bool lnorthp;
    //const double lon0_deg =-123.0; // historical purposes used with ll2utm_ori
    const double sqrt22 = 0.7071067811865475; // 1/sqrt(2)
    const double pi180i = 180.0/M_PI;
    const int nrhs = 1;
    const int n5 = 5;
    const int n6 = 6;
    //------------------------------------------------------------------------//
    //
    // Reality check on sizes
    ierr = 1;
    if (l1 < 1){
        log_errorF("%s: Error no stations at which to invert\n", fcnm);
        return ierr;
    }
    if (ndeps < 1){
        log_errorF("%s: Error no depths in grid search!\n", fcnm);
        return ierr;
    }
    if (time_window <= 0.0){
        log_errorF("%s: Error averaging window must be positive\n", fcnm);
        return ierr;
    }
    // Initialize
    ierr = 0;
    #pragma omp simd
    for (idep=0; idep<ndeps; idep++){
        VR[idep] = 0.0;
        Mw[idep] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<ndeps*n6; i++){
        S[i] = 0.0;
    }
    nwin_max = 0;
    #pragma omp simd reduction(max:nwin_max) 
    for (i=0; i<l1; i++){
        nwin_max = fmax(nwin_max, time_window/dt[i]);
    }
    // Allocate space
    m  = 3*l1;  // number of rows in G
    ldg = m;    // leading dimension of G
    if (deviatoric){
        ncol = n6;
    }else{
        ncol = n5;
    }
    xrs = GFAST_memory_calloc__double(l1);     // X position
    yrs = GFAST_memory_calloc__double(l1);     // Y position
    zrs = GFAST_memory_calloc__double(l1);     // Z position
    azims = GFAST_memory_calloc__double(l1);   // Azimuths (Cartesian)
    backazi = GFAST_memory_calloc__double(l1); // Backazimuth
    UP = GFAST_memory_calloc__double(m);       // Estimates UP = G*S
    U = GFAST_memory_calloc__double(m);        // Data G*S = U
    G = GFAST_memory_calloc__double(ldg*ncol); // Forward modeling matrix G
    // Compute the source UTM location
    if (zone_in < 0 || zone_in > 60){
        zone =-1;
    }else{
        zone = zone_in;
    }
    ierr = geodetic_coordtools_ll2utm(sta_lat[i], sta_lon[i],
                                      &x2, &y2,
                                      &lnorthp, &zone);
    if (ierr != 0){
        log_errorF("%s: Error computing source UTM\n", fcnm);
        goto ERROR;
    }
    // Compute the receiver locations with respect to the input zone
    for (i=0; i<l1; i++){
        ierr = geodetic_coordtools_ll2utm(sta_lat[i], sta_lon[i],
                                          &xrs[i], &yrs[i],
                                          &lnorthp, &zone);
        if (ierr != 0){
            log_errorF("%s: Error computing station UTM\n", fcnm);
            goto ERROR;
        }
    }
    // Compute distances, azimuths, and relative offsets in (x,y,z) coordinates
    #pragma omp simd
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
        //zrs[i] = SA_dep*1000.0; // I've moved this to later
        azims[i] = 90.0 - azi + 180.0; // Flip to cartesian coordinates
    } // Loop on stations
    // Extract the time series
    /* TODO move this to __setRHS and add in time_window */
    for (i=0; i<l1; i++){
        efftime = ceil(epidist[i]/2.0/1000.0);
        if (efftime < 290){
            indx = signal_ptr[i] + efftime;
        }else{
            indx = signal_ptr[i] + 290;
        }
        // Loop on last n10 points
        #pragma omp simd
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
    // Verify U is okay to invert
    for (i=0; i<m; i++){
        if (isnan(U[i])){
            log_errorF("%s: U array contains NaN's - lapack will fail!\n",
                       fcnm);
            goto ERROR;
        }
    }
    // Grid-search on depth
    ierr = 0;
    iwarn = 0;
    #pragma omp parallel for \
     firstprivate(deviatoric, G, l1, ldg, m, ncol, nrhs, sqrt22, UP, zrs), \
     private(dc_pct, i, idep, ierr1, Mo, rank, rcond, Swork), \
     shared(azims, Mw, S, SA_dep, U, VR, xrs, yrs), \
     reduction(+:ierr, iwarn)
    for (idep=0; idep<ndeps; idep++){
        // Fill the depths 
        #pragma omp simd
        for (i=0; i<l1; i++){
            zrs[i] = SA_dep[idep]*1000.0;
        } 
        // Compute the corresponding Green's functions
        ierr1 = GFAST_CMT__setForwardModel(l1, deviatoric,
                                           xrs, yrs, zrs, azims,
                                           G);
        if (ierr1 != 0){
            log_errorF("%s: Error computing Green's functions\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Solve the least squares problem via the SVD
        rcond =-1.0; // Use machine epsilon for singular value cutoff
        ierr1 = numpy_lstsq(LAPACK_ROW_MAJOR, m, ncol, nrhs, G, U,
                            &rcond, Swork, &rank, NULL);
        if (ierr != 0){
            log_errorF("%s: Error solving least squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        if (rank != ncol){
            if (verbose > 1){
                log_warnF("%s: Warning: G has rank=%d which is deficient\n",
                          fcnm, rank);
            }
            iwarn = iwarn + 1;
        }
        // Copy the model
        if (deviatoric){
            S[6*idep+0] =  Swork[0];
            S[6*idep+1] =  Swork[1];
            S[6*idep+2] =-(Swork[0] + Swork[1]); // Apply deviatoric constraint
            S[6*idep+3] =  Swork[2];
            S[6*idep+4] =  Swork[3];
            S[6*idep+5] =  Swork[4];
        }else{
            S[6*idep+0] = Swork[0];
            S[6*idep+1] = Swork[1];
            S[6*idep+2] = Swork[2];
            S[6*idep+3] = Swork[3];
            S[6*idep+4] = Swork[4];
            S[6*idep+5] = Swork[5];
        }
        // Compute the scalar moment
        Mo = sqrt22*sqrt(     pow(S[0],2) +     pow(S[1],2) +     pow(S[2],2)
                        + 2.0*pow(S[3],2) + 2.0*pow(S[4],2) + 2.0*pow(S[5],2));
        if (Mo == 0.0){
            Mw[idep] = 0.0;
        }else{
            Mw[idep] = 2.0*log10(Mo)/3.0 - 6.0;
        }
        // Compute the estimates
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    m, ncol, 1.0, G, ldg, Swork, 1, 0.0, UP, 1);
        // Compute the variance reduction 
        VR[idep] = _cmt_varianceReduction(m, U, UP, &ierr1);
        if (ierr1 != 0){
            log_errorF("%s: Error computing variance reduction\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the double couple percentage
        dc_pct = __cmt_doubleCouplePercentage(&S[6*idep], &ierr1);
        if (ierr1 != 0){
            log_errorF("%s: Error computing double couple percentage\n", fcnm);
            ierr = ierr + 1;
            VR[idep] = 0.0;
            continue;
        }
        if (weight_dc){VR[idep] = VR[idep]*dc_pct;}
    } // Loop on depths
    if (ierr != 0){
        log_errorF("%s: %d errors encountered during grid search\n", fcnm);
    }
    if (iwarn != 0 && verbose > 1){
        log_warnF("%s: %d warnings encountered during grid search\n", fcnm);
    }
ERROR:; // An error was encountered
    // Free memory
    GFAST_memory_free__double(&xrs);
    GFAST_memory_free__double(&yrs);
    GFAST_memory_free__double(&zrs);
    GFAST_memory_free__double(&azims);
    GFAST_memory_free__double(&backazi);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&U);
    GFAST_memory_free__double(&G);
    return 0;
}
//============================================================================//
/*!
 * @brief Computes matrix of Green's functions required in the CMT inversion
 *
 * @param[in] l1          length of arrays
 * @param[in] deviatoric 
 * @param[in] x1          x source receiver distance (meters) [l1]
 * @param[in] y1          y source receiver distance (meters) [l1]
 * @param[in] z1          z source receiver distance (meters) [l1]
 * @param[in] azi         source receiver angles in (x,y) Cartesian plane
 *                        (degrees) [l1]
 *
 * @param[out] G          matrix of Green's functions stored in row major
 *                        format [3*l1 x ncol] where ncol is 5 if 
 *                        deviatoric is true, otherwise, it is 6
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * @date March, 2016
 *
 */
int GFAST_CMT__setForwardModel(int l1, bool deviatoric,
                               double *__restrict__ x1, 
                               double *__restrict__ y1, 
                               double *__restrict__ z1, 
                               double *__restrict__ azi, 
                               double *__restrict__ G)
{
    const char *fcnm = "GFAST_CMTgreenF\0";
    double azi1, C1, C2, cosaz, cosaz_sinaz, cosaz2,  
           g111, g122, g133, g112, g113, g123,
           g211, g222, g233, g212, g213, g223,
           g311, g322, g333, g312, g313, g323,
           R, R3, sinaz, sinaz2, x, y, z;
    int i, indx;
    const double MU = 3.e10;
    const double K = 5.0*MU/3.0;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    //
    // Size check
    if (l1 < 1){
        log_errorF("%s: Error invalid number of points %d\n", fcnm, l1);
        return -1;
    }
    // Deviatoric constraints
    if (deviatoric){
        // Loop on points and fill up Green's function deviatoric matrix
        indx = 0;
        #pragma omp simd
        for (i=0; i<l1; i++){
            // compute coefficients in greens functions
            x = x1[i];
            y = y1[i];
            z = z1[i];
            R = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            C1= 1.0/pow(R,2)/MU/M_PI/8.0;
            C2 = (3.0*K + MU)/(3.0*K + 4.0*MU);
            R3 = pow(R, 3);
            // coefficients for row 1
            g111 = C1*(C2*3.0*x*x*x/R3 - 3.0*C2*x/R+2.0*x/R);
            g122 = C1*(C2*3.0*x*y*y/R3 - C2*x/R);
            g133 = C1*(C2*3.0*x*z*z/R3 - C2*x/R);
            g112 = C1*(C2*6.0*x*x*y/R3 - 2.0*C2*y/R + 2.0*y/R);
            g113 = C1*(C2*6.0*x*x*z/R3 - 2.0*C2*z/R + 2.0*z/R);
            g123 = C1*(C2*6.0*x*y*z/R3);
            // coefficients for row 2
            g211 = C1*(C2*3.0*y*x*x/R3 - C2*y/R);
            g222 = C1*(C2*3.0*y*y*y/R3 - 3.0*C2*y/R + 2.0*y/R);
            g233 = C1*(C2*3.0*y*z*z/R3 - C2*y/R);
            g212 = C1*(C2*6.0*y*x*y/R3 - 2.0*C2*x/R + 2.0*x/R);
            g213 = C1*(C2*6.0*y*x*z/R3);
            g223 = C1*(C2*6.0*y*y*z/R3 - 2.0*C2*z/R + 2.0*z/R);
            // coefficients for row 3
            g311 = C1*(C2*3.0*z*x*x/R3 - C2*z/R);
            g322 = C1*(C2*3.0*z*y*y/R3 - C2*z/R);
            g333 = C1*(C2*3.0*z*z*z/R3 - 3.0*C2*z/R + 2.0*z/R);
            g312 = C1*(C2*6.0*z*x*y/R3);
            g313 = C1*(C2*6.0*z*x*z/R3 - 2.0*C2*x/R + 2.0*x/R);
            g323 = C1*(C2*6.0*z*y*z/R3 - 2.0*C2*y/R + 2.0*y/R);
            // Fill row 1
            G[indx+0] = g112;
            G[indx+1] = g113;
            G[indx+2] = g133;
            G[indx+3] = 0.5*(g111 - g122);
            G[indx+4] = g123;
            // Fill row 2
            G[indx+5] = g212;
            G[indx+6] = g213;
            G[indx+7] = g233;
            G[indx+8] = 0.5*(g211 - g222);
            G[indx+9] = g223;
            // Fill row 3
            G[indx+10] = g312;
            G[indx+11] = g313;
            G[indx+12] = g333;
            G[indx+13] = 0.5*(g311 - g322);
            G[indx+14] = g323;
            // Update the index 3 rows
            indx = indx + 15;
        } // Loop on points
    // General moment tensor 
    }else{
        // Loop on points and fill up Green's functions matrix
        log_warnF("%s: This is deprecated\n", fcnm);
        indx = 0;
        #pragma omp simd
        for (i=0; i<l1; i++){
            // define some constants for this point
            azi1 = azi[i]*pi180;
            sinaz = sin(azi1);
            cosaz = cos(azi1);
            sinaz2 = pow(sinaz, 2);
            cosaz2 = pow(cosaz, 2);
            cosaz_sinaz = cosaz*sinaz;
            // compute coefficients in greens functions
            x = x1[i];
            y = y1[i];
            z = z1[i];
            R = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            C1= 1.0/pow(R,2)/MU/M_PI/8.0;
            C2 = (3.0*K + MU)/(3.0*K + 4.0*MU);
            R3 = pow(R, 3);
            // coefficients for row 1
            g111 = C1*( C2*(3.0*x*x*x/R3 - 3.0*x/R) + 2.0*x/R );
            g122 = C1*( C2*(3.0*x*y*y/R3 - x/R) );
            g133 = C1*( C2*(3.0*x*z*z/R3 - x/R) );
            g112 = C1*( C2*(3.0*x*x*y/R3 - y/R) + 2.0*y/R );
            g113 = C1*( C2*(3.0*x*x*z/R3 - z/R) + 2.0*z/R );
            g123 = C1*( C2*(3.0*x*y*z/R3) );
            // coefficients for row 2
            g211 = C1*( C2*(3.0*y*x*x/R3 - y/R) );
            g222 = C1*( C2*(3.0*y*y*y/R3 - 3.0*y/R) + 2.0*y/R );
            g233 = C1*( C2*(3.0*y*z*z/R3 - y/R) );
            g212 = C1*( C2*(3.0*y*x*y/R3 - x/R) );
            g213 = C1*( C2*(3.0*y*x*z/R3) );
            g223 = C1*( C2*(3.0*y*y*z/R3 - z/R) + 2.0*z/R );
            // coefficients for row 3
            g311 = C1*( C2*(3.0*z*x*x/R3 - z/R) );
            g322 = C1*( C2*(3.0*z*y*y/R3 - z/R) );
            g333 = C1*( C2*(3.0*z*z*z/R3 - 3.0*z/R) + 2.0*z/R );
            g312 = C1*( C2*(3.0*z*x*y/R3) );
            g313 = C1*( C2*(3.0*z*x*z/R3) + 2.0*x/R);
            g323 = C1*( C2*(3.0*z*y*z/R3) + 2.0*y/R);
            // Fill row 1
            G[indx+0]  = g111*cosaz2 + g122*sinaz2 + 2.0*g112*cosaz_sinaz;
            G[indx+1]  = g111*sinaz2 + g122*cosaz2 - 2.0*g112*cosaz_sinaz;
            G[indx+2]  = g133;
            G[indx+3]  =-g111*cosaz_sinaz + g112*cosaz2
                       - g112*sinaz2 + g122*cosaz_sinaz;
            G[indx+4]  = g113*cosaz + g123*sinaz;
            G[indx+5]  = g123*cosaz - g113*sinaz;
            // Fill row 2
            G[indx+6]  = g211*cosaz2 + g222*sinaz2 + 2.0*g212*cosaz_sinaz;
            G[indx+7]  = g211*sinaz2 + g222*cosaz2 - 2.0*g212*cosaz_sinaz;
            G[indx+8]  = g233;
            G[indx+9]  =-g211*cosaz_sinaz + g212*cosaz2
                       - g212*sinaz2 + g222*cosaz_sinaz;
            G[indx+10] = g213*cosaz + g223*sinaz;
            G[indx+11] = g223*cosaz - g213*sinaz;
            // Fill row 3
            G[indx+12] = g311*cosaz2 + g322*sinaz2 + 2.0*g312*cosaz_sinaz;
            G[indx+13] = g311*sinaz2 + g322*cosaz2 - 2.0*g312*cosaz_sinaz;
            G[indx+14] = g333;
            G[indx+15] =-g311*cosaz_sinaz + g312*cosaz2
                       - g312*sinaz2 + g322*cosaz_sinaz;
            G[indx+16] = g313*cosaz + g323*sinaz;
            G[indx+17] = g323*cosaz - g313*sinaz;
            // Update the index 3 rows
            indx = indx + 18;
        } // Loop on points
    }
    return 0;
}
//============================================================================//
/*
 * @brief Computes the variance reduction the CMT grid search inversion
 *
 * @param[in] m     length of arrays obs and est
 * @param[in] obs   observation array [m]
 * @param[in] est   estimate array [m]
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
double _cmt_varianceReduction(int m,
                              double *__restrict__ obs,
                              double *__restrict__ est,
                              int *ierr)
{
    const char *fcnm = "_cmt_varianceReduction\0";
    double r, res_norm2, u_norm2, vr; 
    int i;
    vr = 0.0;
    *ierr = 0;
    if (m < 1 || obs == NULL || est == NULL){
        if (m < 1){log_errorF("%s: Error no points\n", fcnm);}
        if (obs == NULL){log_errorF("%s: Error obs is NULL\n", fcnm);}
        if (est == NULL){log_errorF("%s: Error est is NULL\n", fcnm);}
        *ierr = 1;
        return vr;
    }
    res_norm2 = 0.0;
    u_norm2 = 0.0;
    #pragma omp simd reduction(+:res_norm2, u_norm2)
    for (i=0; i<m; i++){
        r = obs[i] - est[i];
        res_norm2 = res_norm2 + r*r;
        u_norm2 = u_norm2 + obs[i]*obs[i];
    }
    vr = (1.0 - sqrt(res_norm2/u_norm2))*100.0;
    return vr;
}
//============================================================================//
/*!
 * @brief Computes the double couple percentage ala MoPaD
 *
 * @param[in] M6       moment tensor in NED coordinates ordered 
 *                     \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                            m_{xy}, m_{xz}, m_{yz} \} \f$
 *
 * @param[out] ierr    0 indicates success
 *
 * @reference See Mopad's standard decomposition 
 * @reference A Student's Guide to and Review of Moment Tensors - 
 *            Jost and Herrmann (1989)
 * @reference Defining the scalar moment of a seismic source with 
 *            a general moment tensor - Bowers and Hudson (1999)
 *
 * @result percent double couple [0,100]
 *
 * @author Ben Baker (ISTI)
 *
 */
double __cmt_doubleCouplePercentage(double M6[6], int *ierr)
{
    const char *fcnm = "__cmt_doubleCouplePercentage\0";
    double M_devi[9], eigenv_devi[3],
           F, M_iso_percentage, M0, M_DC_percentage, M0_devi, M0_iso, traceM;
    int info;
    const double third = 1.0/3.0;
    const double epsilon = 1.e-13;
    // Initialize
    M_DC_percentage = 0.0;
    *ierr = 0;
    // Compute the magnitude of the isotropic part of the source
    traceM = M6[0] + M6[1] + M6[2];
    M0_iso = fabs(1.0/3.0*traceM);
    // Compute the deviatoric moment tensor (Jost and Herrman Eqn 24)
    M_devi[0] = M6[0] - third*traceM; // mxx
    M_devi[4] = M6[1] - third*traceM; // myy
    M_devi[8] = M6[2] - third*traceM; // mzz
    M_devi[1] = M_devi[3] = M6[3];    // mxy
    M_devi[2] = M_devi[6] = M6[4];    // mxz
    M_devi[5] = M_devi[7] = M6[5];    // myz
    // Compute the eigenvalues (in ascending order).  Use standard QR 
    // over divide and conquer because 3 x 3 is too small.
    info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'N', 'L', 3, M_devi, 3,
                         eigenv_devi);
    if (info != 0){
        log_errorF("%s: Error computing eigenvalues\n", fcnm);
        *ierr = 1;
        return M_DC_percentage;
    }
    // Scalar moment of deviatoric (Bowers and Hudson 1999) 
    // Note the identity eig(A) - 1/3*trace(A) = eig(A - 1/3*trace(A))
    M0_devi = fmax( fmax(fabs(eigenv_devi[0]),
                         fabs(eigenv_devi[1])),
                         fabs(eigenv_devi[2]) );
    // F as in Jost and Herrmann (defined right before Eqn 35)
    if (M0_devi < epsilon){
        F = 0.5;
    }else{
        F =-eigenv_devi[0]/eigenv_devi[2];
    }
    // Scalar moment according to Bowers and Hudson (1999)
    M0 = M0_iso + M0_devi;
    M_iso_percentage = M0_iso/M0*100.0;
    M_DC_percentage = (1.0 - 2.0*fabs(F))*(1.0 - M_iso_percentage/100.0)*100.0;
    return M_DC_percentage;
}
