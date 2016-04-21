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
bool __GFAST_getAvgDisplacement(int npts, bool lremove_disp0,
                                double dt, double SA_time,
                                double cmt_window_avg,
                                double epoch,
                                double *__restrict__ ubuff,
                                double *__restrict__ nbuff,
                                double *__restrict__ ebuff,
                                double *uAvg, double *nAvg, double *eAvg);

int GFAST_CMT__driver(struct GFAST_props_struct props,
                      struct GFAST_shakeAlert_struct SA,
                      struct GFAST_data_struct gps_data,
                      struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_cmt__driver\0";
    enum cmt_return_enum
    {
        CMT_SUCCESS = 0,
        CMT_STRUCT_ERROR = 1,
        CMT_GPS_DATA_ERROR = 2,
        CMT_INSUFFICIENT_DATA = 3,
        CMT_COMPUTE_ERROR = 4
    };
    double *utmRecvEasting, *utmRecvNorthing, *staAlt, *x2, *y2,
           *eAvgDisp, *nAvgDisp, *uAvgDisp, 
           currentTime, distance, eAvg, effectiveHypoDist, nAvg,
           uAvg, utmSrcEasting, utmSrcNorthing, x1, y1;
    int ierr, k, l1, nwork, zone_loc;
    bool lnorthp, luse;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = CMT_SUCCESS;
    x2 = NULL;
    y2 = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    staAlt = NULL;
    uAvgDisp = NULL;
    nAvgDisp = NULL;
    eAvgDisp = NULL;
    // Reality check
    if (gps_data.stream_length < 1){
        if (props.verbose > 1){
            ierr = CMT_GPS_DATA_ERROR;
            log_warnF("%s: No GPS streams\n", fcnm);
            goto ERROR;
        }
    }
    if (cmt->ndeps < 1){ 
        log_errorF("%s: No depths in CMT gridsearch!\n", fcnm);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Count the data and get the workspace 
    l1 = 0;
    nwork = 0;
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_cmt){continue;}
        nwork = fmax(gps_data.data[k].npts, nwork);
        l1 = l1 + 1;
    }
    if (l1 < props.cmt_min_sites){
        if (props.verbose > 1){
            if (l1 < 1){
                log_warnF("%s: All sites masked in CMT estimation\n", fcnm);
            }else{
                log_warnF("%s: Too many masked sites to compute CMT\n", fcnm);
            }
        }
        ierr = CMT_GPS_DATA_ERROR;
        goto ERROR;
    }
    if (nwork < 1){
        if (props.verbose > 1){
            log_warnF("%s: There is no data\n", fcnm);
        }
        ierr = CMT_GPS_DATA_ERROR;
        goto ERROR;
    }
    // Initialize
    l1 = 0;
    x2 = GFAST_memory_calloc__double(gps_data.stream_length);
    y2 = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvNorthing = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvEasting  = GFAST_memory_calloc__double(gps_data.stream_length);
    staAlt = GFAST_memory_calloc__double(gps_data.stream_length);
    uAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    nAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    eAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    // Get the source location
    if (props.utm_zone ==-12345){
        zone_loc =-1;
    }else{
        zone_loc = props.utm_zone;
    }   
    GFAST_coordtools_ll2utm_ori(SA.lat, SA.lon,
                                &y1, &x1,
                                &lnorthp, &zone_loc);
    utmSrcNorthing = y1; 
    utmSrcEasting = x1;
    // Loop on the receivers, get distances, and data
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_cmt){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools_ll2utm_ori(gps_data.data[k].sta_lat,
                                    gps_data.data[k].sta_lon,
                                    &y2[k], &x2[k],
                                    &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2[k], 2)
                       + pow(y1 - y2[k], 2)
                       + pow(SA.dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // In a perfect world is there any chance of having data?
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        // In a perfect world is there any chance of having data?
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA.time)*props.cmt_window_vel;
        if (distance < effectiveHypoDist){
            luse = __GFAST_getAvgDisplacement(gps_data.data[k].npts,
                                              props.lremove_disp0,
                                              gps_data.data[k].dt,
                                              SA.time,
                                              props.cmt_window_avg,
                                              gps_data.data[k].epoch,
                                              gps_data.data[k].ubuff,
                                              gps_data.data[k].nbuff,
                                              gps_data.data[k].ebuff,
                                              &uAvg, &nAvg, &eAvg);
            if (luse){
                uAvgDisp[l1] = uAvg;
                nAvgDisp[l1] = nAvg;
                eAvgDisp[l1] = eAvg;
                utmRecvNorthing[l1] = y2[k]; 
                utmRecvEasting[l1] = x2[k]; 
                staAlt[l1] = gps_data.data[k].sta_alt;
                l1 = l1 + 1;
            }
        }
    } // Loop on data streams
    if (l1 < props.cmt_min_sites){
        if (props.verbose > 1){
            log_warnF("%s: Insufficient data to invert\n", fcnm);
        }
        ierr = CMT_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Warn in case hypocenter is outside of grid-search
    if (SA.dep < cmt->srcDepths[0] || SA.dep > cmt->srcDepths[cmt->ndeps-1]){
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }   
    // Invert!
    if (props.verbose > 2){ 
        log_debugF("%s: Inverting for PGD with %d sites\n", fcnm, l1);
    }
    ierr = GFAST_CMT__depthGridSearch(l1, cmt->ndeps,
                                      props.verbose,
                                      props.ldeviatoric_cmt,
                                      utmSrcEasting,
                                      utmSrcNorthing,
                                      cmt->srcDepths,
                                      utmRecvEasting,
                                      utmRecvNorthing,
                                      staAlt,
                                      nAvgDisp,
                                      eAvgDisp,
                                      uAvgDisp,
                                      cmt->mts,
                                      cmt->cmt_vr);
    if (ierr != 0){
        log_errorF("%s: Error in CMT gridsearch!\n", fcnm);
        goto ERROR;
    }
ERROR:;
    GFAST_memory_free__double(&x2);
    GFAST_memory_free__double(&y2);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&uAvgDisp);
    GFAST_memory_free__double(&nAvgDisp);
    GFAST_memory_free__double(&eAvgDisp);
    return ierr;
}
//============================================================================//
/*!
 * @brief Computes the maximum site distance (offset) from the 
 *        3 channel GPS site data 
 *
 * @param[in] npts             number of points in time series
 * @param[in] lremove_disp0    if true then remove the (u,n,e) position at 
 *                             origin time SA_time.
 * @param[in] dt               sampling period (s) of GPS buffers
 * @param[in] SA_time          epochal UTC origin time (s)
 * @param[in] cmt_window_avg   time from end of acquisition which the data
 *                             is to be averaged (s)
 * @param[in] epoch            epochal UTC start time (s) of GPS traces
 * @param[in] ubuff            vertical position [npts]
 * @param[in] nbuff            north position [npts] 
 * @param[in] ebuff            east position [npts]
 *
 * @param[out] uAvg            average displacement on the vertical channel
 * @param[out] nAvg            average displacement on the north channel
 * @param[out] eAvg            average displacement on the east channel
 *
 * @result if true then the data is usable in the inversion otherwise
 *         this site should be skipped.
 *
 * @author Ben Baker (ISTI)
 */
bool __GFAST_getAvgDisplacement(int npts, bool lremove_disp0,
                                double dt, double SA_time,
                                double cmt_window_avg,
                                double epoch,
                                double *__restrict__ ubuff,
                                double *__restrict__ nbuff,
                                double *__restrict__ ebuff,
                                double *uAvg, double *nAvg, double *eAvg)
{
    double diffT, disp_i, de, dn, du, e0, n0, u0, eAvgNan, nAvgNan, uAvgNan;
    int i, iavg, iavg1, indx0, npavg;
    bool luse;
    //------------------------------------------------------------------------//
    //
    // Initialize result 
    *uAvg = NAN;
    *nAvg = NAN;
    *eAvg = NAN;
    // Set the initial position
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    diffT = SA_time - epoch;
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    if (lremove_disp0){
        diffT = SA_time - epoch;
        u0 = ubuff[indx0];
        n0 = nbuff[indx0];
        e0 = ebuff[indx0];
        // Prevent a problem
        if (isnan(u0) || isnan(n0) || isnan(e0)){
            return NAN;
        }
    }
    // Set the start index
    indx0 = npts - (int) (cmt_window_avg/dt + 0.5);
    indx0 = fmax(0, indx0);
    diffT = epoch;
    npavg = npts - indx0 + 1;
    uAvgNan = 0.0;
    nAvgNan = 0.0;
    eAvgNan = 0.0; 
    iavg = 0; 
    // Compute the average over the window
    #pragma omp simd reduction(+:iavg, eAvgNan, nAvgNan, uAvgNan)
    for (i=indx0; i<npts; i++){
        luse = false;
        du = 0.0;
        dn = 0.0;
        de = 0.0; 
        iavg1 = 0;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) &&
            ubuff[i] >-999.0 && nbuff[i] >-999.0 && ebuff[i] >-999.0)
        {
            luse = true;
        }
        if (luse){du = ubuff[i] - u0;}
        if (luse){dn = nbuff[i] - n0;}
        if (luse){de = ebuff[i] - e0;}
        if (luse){iavg1 = 1;}
        uAvgNan = uAvgNan + du;
        nAvgNan = nAvgNan + dn;
        eAvgNan = eAvgNan + de;
        iavg = iavg + iavg1;
    } // Loop on data points
    // Average it if possible
    if (iavg > 0){
        *uAvg = uAvgNan/(double) iavg;
        *nAvg = nAvgNan/(double) iavg;
        *eAvg = eAvgNan/(double) iavg;
        luse = true;
    }else{
        luse = false;
    }
    return luse;
}
//============================================================================//
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
int GFAST_CMT__depthGridSearchFail(int l1, int ndeps, int verbose,
                               double time_window,
                               bool deviatoric,
                               bool weight_dc,
                               int zone_in, int *signal_ptr,
                               double utmSrcEasting,
                               double utmSrcNorthing,
                               double *__restrict__ utmRecvEasting,
                               double *__restrict__ utmRecvNorthing,
                               double *__restrict__ staAlt,
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
    xrs = GFAST_memory_calloc__double(l1);     // X relative position
    yrs = GFAST_memory_calloc__double(l1);     // Y relative position
    zrs = GFAST_memory_calloc__double(l1);     // Z relative position
    azims = GFAST_memory_calloc__double(l1);   // Azimuths (Cartesian)
    backazi = GFAST_memory_calloc__double(l1); // Backazimuth
    UP = GFAST_memory_calloc__double(m);       // Estimates UP = G*S
    U = GFAST_memory_calloc__double(m);        // Data G*S = U
    G = GFAST_memory_calloc__double(ldg*ncol); // Forward modeling matrix G
/*
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
*/
    // Compute distances, azimuths, and relative offsets in (x,y,z) coordinates
    #pragma omp simd
    for (i=0; i<l1; i++){
/*
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
*/
        xrs[i] = utmSrcEasting  - utmRecvEasting[i];
        yrs[i] = utmSrcNorthing - utmRecvNorthing[i];
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
/*
    #pragma omp parallel for \
     firstprivate(deviatoric, G, l1, ldg, m, ncol, ndeps, nrhs, verbose, sqrt22, UP, weight_dc, zrs), \
     private(dc_pct, i, idep, ierr1, Mo, rank, rcond, Swork), \
     shared(azims, fcnm, Mw, S, SA_dep, U, VR, xrs, yrs), \
     reduction(+:ierr, iwarn) default(none)
*/
    for (idep=0; idep<ndeps; idep++){
        // Fill the depths 
        #pragma omp simd
        for (i=0; i<l1; i++){
            zrs[i] = SA_dep[idep]*1000.0 + staAlt[idep];
        } 
        // Compute the corresponding Green's functions
/*
        ierr1 = GFAST_CMT__setForwardModel(l1, deviatoric,
                                           xrs, yrs, zrs, azims,
                                           G);
*/
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
