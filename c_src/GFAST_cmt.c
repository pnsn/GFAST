#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast.h"

#define n10 10

int GFAST_moment_tensor(int l1, int *signal_ptr,
                        double *sta_lat, double *sta_lon,
                        double *n, double *e, double *u,
                        double SA_lat, double SA_lon, double SA_dep,
                        double *epidist)
{
    const char *fcnm = "GFAST_moment_tensor\0"; 
    double *U, *G, *azi, *azims, *backazi, *xrs, *yrs, *zrs,
           NN[n10], EE[n10], UU[n10],
           E, N, r, t, x1, x2, y1, y2;
    int efftime, i, ierr, indx, iwarn, k, ldg;
    const double pi180i = 180.0/M_PI;
    //------------------------------------------------------------------------//
    //
    // Reality check on sizes
    if (l1 < 1){
        printf("%s: Error no stations at which to invert\n", fcnm);
    }
    xrs = (double *)calloc(l1, sizeof(double));
    yrs = (double *)calloc(l1, sizeof(double));
    zrs = (double *)calloc(l1, sizeof(double));
    azi = (double *)calloc(l1, sizeof(double));
    azims = (double *)calloc(l1, sizeof(double));
    backazi = (double *)calloc(l1, sizeof(double));
    ldg = 3*l1;
    U = (double *)calloc(ldg, sizeof(double));
    G = (double *)calloc(ldg*6, sizeof(double));
    // Compute distances, azimuths, and relative offsets in (x,y,z) coordinates
    for (i=0; i<l1; i++){
        // Convert source/station lat/lons to UTMs
y2 = x2 = x1 = y1 = 0.0;
printf("need a lat/lon to UTM routine!\n");
getchar();
        // Compute azimuth and back-azimuth 
        azi[i] = 90.0 - pi180i*atan2(x2-x1, y2-y1);
        if (azi[i] < 0.0){
            azi[i] = azi[i] + 360.0;
        }
        if (azi[i] > 360.0){
            azi[i] = azi[i] - 360.0;
        }
        if (azi[i] < 180.0){
            backazi[i] = azi[i] + 180.0;
        }
        if (azi[i] > 180.0){
            backazi[i] = azi[i] - 180.0;
        }
        if (azi[i] == 180.0){
            backazi[i] = 0.0;
        }
        // Relative source receiver location in (x,y,z) coordinates
        xrs[i] = x1 - x2;
        yrs[i] = y1 - y2;
        zrs[i] = SA_dep*1000.0;
        azims[i] = 90.0 - azi[i] + 180.0;
    } // Loop on stations
    // Extract the time series
    for (i=0; i<l1; i++){
        efftime = ceil(epidist[i]/2.0/1000.0);
        if (efftime < 290){
            indx = signal_ptr[i] + efftime;
        }else{
            indx = signal_ptr[i] + 290;
        }
        for (k=0; k<n10; k++){
            NN[k] = n[indx]; //[i,efftime:efftime+10]
            EE[k] = e[indx]; //[i,efftime:efftime+10]
            UU[k] = u[indx]; //[i,efftime:efftime+10]
            indx = indx + 1;
        }
        N = numpy_nanmean(n10, NN, &iwarn);
        E = numpy_nanmean(n10, EE, &iwarn);
        // Rotate
        rotate_NE_RT(1, &N, &E, backazi[i], &r, &t);
        // Fill the obervation array 
        indx = 3*i;
        U[indx  ] = r;
        U[indx+1] = t; 
        U[indx+2] = numpy_nanmean(n10, UU, &iwarn); 
    }
    // Compute the corresponding Green's functions
    ierr = GFAST_CMTgreenF(l1, ldg, 
                           xrs, yrs, zrs, azims,
                           G); 
    if (ierr != 0){
        printf("%s: Error computing Green's functions\n", fcnm);
        return -1;
    }
printf("%s: finish here\n", fcnm);
getchar();
    // Solve the least squares problem via the SVD

    // Compute the scalar moment

    // Compute the variance reduction 

    // Compute the nodal planes
 
    // Free memory
    free(xrs);
    free(yrs);
    free(zrs);
    free(azi);
    free(azims);
    free(backazi);
    free(U);
    free(G);
    return 0;
}
