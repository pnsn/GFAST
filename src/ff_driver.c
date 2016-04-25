#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <lapacke.h>
#include <cblas.h>
#include "gfast.h"

bool __GFAST_FF__getAvgDisplacement(int npts, bool lremove_disp0,
                                    double dt, double SA_time,
                                    double ff_window_avg,
                                    double epoch,
                                    double *__restrict__ ubuff,
                                    double *__restrict__ nbuff,
                                    double *__restrict__ ebuff,
                                    double *uAvg, double *nAvg, double *eAvg);

/*!
 * @brief Perform the finite fault inversion on input nodal planes.
 *
 * @param[in] props       holds the finite fault inversion properties
 * @param[in] SA          hypocentral information (lat, lon, depth, time)
 * @param[in] gps_data    GPS data to invert
 *
 * @param[inout] ff       on input contains space for the search on fault planes
 *                        on output contains the corresponding fault plane mesh,
 *                        the corresponding slip on each fault patch for each
 *                        fault plane, the observed and estimate data, and 
 *                        the optimal fault plane index
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_FF__driver(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct SA,
                     struct GFAST_data_struct gps_data,
                     struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "GFAST_FF__driver\0";
    enum ff_return_enum
    {   
        FF_SUCCESS = 0,
        FF_STRUCT_ERROR = 1,
        FF_GPS_DATA_ERROR = 2,
        FF_INSUFFICIENT_DATA = 3,
        FF_COMPUTE_ERROR = 4,
        FF_MEMORY_ERROR = 5
    };
    double *G2, *eAvgDisp, *nAvgDisp, *S, *staAlt, *T,
           *uAvgDisp, *utmRecvEasting, *utmRecvNorthing,
           *UD, *UP, *xrs, *yrs, *zrs,
           currentTime, distance, eAvg, effectiveHypoDist,
           lampred, M0, nAvg, res, st, uAvg, x1, x2, xden, xnum, y1, y2;
    int i, ierr, ierr1, ifp, ij, j, k, l1, l2, mrowsG, mrowsG2, mrowsT,
        ncolsG, ncolsG2, ncolsT, ndip, ng, ng2, nstr, nt, nwork, zone_loc;
    bool lnorthp, luse;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = FF_SUCCESS;
    G2 = NULL;
    UD = NULL;
    UP = NULL;
    T  = NULL;
    S  = NULL;
    staAlt = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    eAvgDisp = NULL;
    nAvgDisp = NULL;
    uAvgDisp = NULL;
    xrs = NULL;
    yrs = NULL;
    zrs = NULL;
    // Reality checks
    if (gps_data.stream_length < 1){ 
        if (props.verbose > 1){ 
            ierr = FF_GPS_DATA_ERROR;
            log_warnF("%s: No GPS streams\n", fcnm);
        }
        goto ERROR;
    }
    if (ff->nfp < 1){
        ierr = FF_STRUCT_ERROR;
        log_errorF("%s: Error no nodal planes\n", fcnm);
        goto ERROR;
    }
    // Count the data and get the workspace 
    l1 = 0;
    nwork = 0;
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_ff){continue;}
        nwork = fmax(gps_data.data[k].npts, nwork);
        l1 = l1 + 1;
    }
    if (l1 < props.ff_min_sites){
        if (props.verbose > 1){ 
            if (l1 < 1){ 
                log_warnF("%s: All sites masked in FF estimation\n", fcnm);
            }else{
                log_warnF("%s: Too many masked sites to compute FF\n", fcnm);
            }
        }
        ierr = FF_GPS_DATA_ERROR;
        goto ERROR;
    }   
    if (nwork < 1){ 
        if (props.verbose > 1){ 
            log_warnF("%s: There is no data\n", fcnm);
        }
        ierr = FF_GPS_DATA_ERROR;
        goto ERROR;
    }
    // Set the data and position arrays
    uAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    nAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    eAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvEasting  = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvNorthing = GFAST_memory_calloc__double(gps_data.stream_length);
    staAlt = GFAST_memory_calloc__double(gps_data.stream_length);
    // Get the source location
    if (props.utm_zone ==-12345){
        zone_loc =-1;
    }else{
        zone_loc = props.utm_zone;
    }
    GFAST_coordtools_ll2utm_ori(ff->SA_lat, ff->SA_lon,
                                &y1, &x1,
                                &lnorthp, &zone_loc);
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_ff){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools_ll2utm_ori(gps_data.data[k].sta_lat,
                                    gps_data.data[k].sta_lon,
                                    &y2, &x2,
                                    &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2, 2)
                       + pow(y1 - y2, 2)
                       + pow(SA.dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // In a perfect world is there any chance of having data?
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA.time)*props.ff_window_vel;
        if (distance < effectiveHypoDist){
            luse = __GFAST_FF__getAvgDisplacement(gps_data.data[k].npts,
                                                  props.lremove_disp0,
                                                  gps_data.data[k].dt,
                                                  SA.time,
                                                  props.ff_window_avg,
                                                  gps_data.data[k].epoch,
                                                  gps_data.data[k].ubuff,
                                                  gps_data.data[k].nbuff,
                                                  gps_data.data[k].ebuff,
                                                  &uAvg, &nAvg, &eAvg);
            if (luse){
                uAvgDisp[l1] = uAvg;
                nAvgDisp[l1] = nAvg;
                eAvgDisp[l1] = eAvg;
                utmRecvNorthing[l1] = y2;
                utmRecvEasting[l1] = x2;
                staAlt[l1] = gps_data.data[k].sta_alt;
                l1 = l1 + 1;
            }
        }
    } // Loop on data streams
    if (l1 < props.ff_min_sites){
        if (props.verbose > 1){
            log_warnF("%s: Insufficient data to invert\n", fcnm);
        }
        ierr = FF_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Invert!
    if (props.verbose > 2){ 
        log_debugF("%s: Performing finite fault inversion with %d sites\n",
                   fcnm, l1);
    }
    nstr = props.ff_nstr;
    ndip = props.ff_ndip;
    l2 = nstr*ndip;
    xrs = GFAST_memory_calloc__double(l1*l2);
    yrs = GFAST_memory_calloc__double(l1*l2);
    zrs = GFAST_memory_calloc__double(l1*l2);
    // Compute sizes of matrices in G2 S = UP where G2 = [G; T]
    mrowsG = 3*l1;
    ncolsG = 2*l2;
    mrowsT = 2*l2 + 2*(2*nstr + 2*(ndip - 2));
    mrowsT = 2*ndip*nstr + 2*(2*nstr+2*(ndip - 2));
    ncolsT = 2*l2;
    mrowsG2 = mrowsG + mrowsT;
    ncolsG2 = ncolsG;
    nt = mrowsT*ncolsT;
    ng = mrowsG*ncolsG;
    ng2 = mrowsG2*ncolsG2;
    G2 = GFAST_memory_calloc__double(ng2);
    UD = GFAST_memory_calloc__double(mrowsG2);
    UP = GFAST_memory_calloc__double(mrowsG2);
    T  = GFAST_memory_calloc__double(nt);
    S  = GFAST_memory_calloc__double(ncolsG2);
    if (G2 == NULL || UD == NULL || UP == NULL || T == NULL || S == NULL){
        if (G2 == NULL){log_errorF("%s: Error setting space for G2\n", fcnm);}
        if (UD == NULL){log_errorF("%s: Error setting space for UP\n", fcnm);}
        if (UP == NULL){log_errorF("%s: Error setting space for UP\n", fcnm);}
        if (T  == NULL){log_errorF("%s: Error setting space for T\n",  fcnm);}
        if (S  == NULL){log_errorF("%s: Error setting space for S\n",  fcnm);}
        ierr = FF_MEMORY_ERROR;
        goto ERROR;
    }
    // Set the hypocenter information
/*
    ff->SA_lat = SA.lat;
    ff->SA_lon = SA.lon;
    ff->SA_dep = SA.dep;
*/
    ff->preferred_fault_plane =-1;
    for (ifp=0; ifp<ff->nfp; ifp++){ff->fp[ifp].nsites_used = l1;}
    // Set the RHS
    ierr = GFAST_FF__setRHS(l1, props.verbose,
                            nAvgDisp, eAvgDisp, uAvgDisp, UD);
    if (ierr != 0){
        log_errorF("%s: Error setting right hand side\n", fcnm);
        ierr = FF_COMPUTE_ERROR;
        goto ERROR; 
    }
    // Loop on the fault planes
#ifdef __PARALLEL_FF
    #pragma omp parallel for \
     firstprivate(G2, S, T, UP, xrs, yrs, zrs) \
     private(i, ierr1, ifp, ij, j, lampred, M0, res, st, xden, xnum) \
     shared(fcnm, ff, l1, l2, mrowsG2, ncolsG2, mrowsG, ng, ng2, nt, props, staAlt, UD, utmRecvEasting, utmRecvNorthing) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++){
        // Null out G2 and regularizer
        memset(G2, 0, ng2*sizeof(double));
        memset(T, 0, nt*sizeof(double)); 
        // Mesh the fault plane
        ierr1 = GFAST_FF__meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
                                         props.ff_flen_pct,
                                         props.ff_fwid_pct,
                                         ff->SA_mag, ff->str[ifp], ff->dip[ifp],
                                         ff->fp[ifp].nstr, ff->fp[ifp].ndip,
                                         props.utm_zone, props.verbose,
                                         ff->fp[ifp].fault_ptr,
                                         ff->fp[ifp].lat_vtx,
                                         ff->fp[ifp].lon_vtx,
                                         ff->fp[ifp].dep_vtx,
                                         ff->fp[ifp].fault_xutm,
                                         ff->fp[ifp].fault_yutm,
                                         ff->fp[ifp].fault_alt,
                                         ff->fp[ifp].strike,
                                         ff->fp[ifp].dip,
                                         ff->fp[ifp].length,
                                         ff->fp[ifp].width);
        if (ierr1 != 0){
            log_errorF("%s: Error meshing fault plane\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the site/fault patch offsets
        for (i=0; i<l2; i++){
            for (j=0; j<l1; j++){
                ij = l1*i + j;
                xrs[ij] = utmRecvEasting[j]  - ff->fp[ifp].fault_xutm[i];
                yrs[ij] = utmRecvNorthing[j] - ff->fp[ifp].fault_yutm[i];
                zrs[ij] = ff->fp[ifp].fault_alt[i]*1.e3 + staAlt[j];
            }
        }
        // Compute the forward modeling matrix (which is in row major format)
        ierr1 = GFAST_FF__setForwardModel__okadagreenF(l1, l2,
                                                       xrs, yrs, zrs, 
                                                       ff->fp[ifp].strike,
                                                       ff->fp[ifp].dip,
                                                       ff->fp[ifp].width,
                                                       ff->fp[ifp].length,
                                                       G2);
        if (ierr1 != 0){
            log_errorF("%s: Error setting forward model\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Set the regularizer
        ierr1 = GFAST_FF__setRegularizer(l2,
                                         ff->fp[ifp].nstr,
                                         ff->fp[ifp].ndip,
                                         nt,
                                         ff->fp[ifp].width,
                                         ff->fp[ifp].length,
                                         T);
        if (ierr1 != 0){
            log_errorF("%s: Error setting regulizer\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute scale factor for regularizer
        lampred = 1.0/pow( (double) l2*2.0, 2);
        lampred = lampred/(cblas_dasum(ng, G2, 1)/(double) ng);
        lampred = lampred/4.0*ff->fp[ifp].length[0]*ff->fp[ifp].width[0]/1.e6;

/*
int count = 0;
double tij;
for (i=0; i<mrowsT; i++){
for (j=0; j<ncolsT; j++){
tij = T[i*ncolsT + j];
if (fabs(tij) > 0.0){
count = count + 1;
printf("%d %d %d %e\n", count, i, j, T[i*ncolsT + j]);
}
}
}
printf("%f\n", lampred);
getchar();
*/
        // Append lampred*T to G2
        cblas_daxpy(nt, lampred, T, 1, &G2[ng], 1);
        // Solve the least squares problem
        ierr1 = numpy_lstsq__qr(LAPACK_ROW_MAJOR,
                                mrowsG2, ncolsG2, 1, G2, UD,
                                S);
        if (ierr1 != 0){
            log_errorF("%s: Error solving least squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the forward problem 
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    mrowsG2, ncolsG2, 1.0, G2, ncolsG2, S, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
        #pragma omp simd reduction(+:xnum, xden)
        for (i=0; i<mrowsG; i++){
            res = UP[i] - UD[i];
            xnum = xnum + res*res;
            xden = xden + UD[i]*UD[i];
        }
        ff->vr[ifp] = (1.0 - xnum/xden)*100.0;
//printf("%f\n", ff->vr[ifp]);
        // Extract the estimates
        #pragma omp simd
        for (i=0; i<l1; i++){
            ff->fp[ifp].EN[i] = UP[3*i+0];
            ff->fp[ifp].NN[i] = UP[3*i+1];
            ff->fp[ifp].UN[i] = UP[3*i+2];
            ff->fp[ifp].Einp[i] = UD[3*i+0];
            ff->fp[ifp].Ninp[i] = UD[3*i+1];
            ff->fp[ifp].Uinp[i] = UD[3*i+2]; 
         }
         // Extract the slip
         for (i=0; i<l2; i++){
             ff->fp[ifp].sslip[i] = S[2*i+0];
             ff->fp[ifp].dslip[i] = S[2*i+1];
//printf("%f %f\n", S[2*i+0], S[2*i+1]);
         }
//getchar();
         // Compute the magnitude
         M0 = 0.0;
         #pragma omp simd reduction(+:M0)
         for (i=0; i<l2; i++){
             st = sqrt( pow(ff->fp[ifp].sslip[i], 2)
                      + pow(ff->fp[ifp].dslip[i], 2) );
             M0 = M0 + 3.e10*st*ff->fp[ifp].length[i]*ff->fp[ifp].width[i];

         }
         ff->Mw[ifp] = 0.0;
         if (M0 > 0.0){ff->Mw[ifp] = (log10(M0*1.e7) - 16.1)/1.5;}
//printf("%f\n", ff->Mw[ifp]);
         //  Set the number of observations
         ff->fp[ifp].nsites_used = l1;
    } // Loop on fault planes
    if (ierr != 0){
        log_errorF("%s: There were errors detected in the inversion\n", fcnm);
        ierr = FF_COMPUTE_ERROR;
    }else{
/*
        // Choose a preferred plane
        ff->preferred_plane = 0;
        for (ifp=1; ifp<props.nfp; ifp++){
            if (ff->vr[i] < ff->VR[ff->preferred_plane]){
                ff->preferred_plane = ifp;
            }
        }
*/
    }
ERROR:;
    GFAST_memory_free__double(&G2);
    GFAST_memory_free__double(&UD);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&T);
    GFAST_memory_free__double(&S);
    GFAST_memory_free__double(&xrs);
    GFAST_memory_free__double(&yrs);
    GFAST_memory_free__double(&zrs);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&eAvgDisp);
    GFAST_memory_free__double(&nAvgDisp);
    GFAST_memory_free__double(&uAvgDisp);
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
 * @param[in] ff_window_avg    time from end of acquisition which the data
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
 *
 */
bool __GFAST_FF__getAvgDisplacement(int npts, bool lremove_disp0,
                                    double dt, double SA_time,
                                    double ff_window_avg,
                                    double epoch,
                                    double *__restrict__ ubuff,
                                    double *__restrict__ nbuff,
                                    double *__restrict__ ebuff,
                                    double *uAvg, double *nAvg, double *eAvg)
{
    double diffT, de, dn, du, e0, n0, u0, eAvgNan, nAvgNan, uAvgNan;
    int i, iavg, iavg1, indx0;
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
    indx0 = npts - (int) (ff_window_avg/dt + 0.5);
    indx0 = fmax(0, indx0);
    diffT = epoch;
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
