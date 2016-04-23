#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <stdbool.h>
#include <omp.h>
#include <lapacke.h>
#include "gfast.h"

bool __GFAST_CMT__getAvgDisplacement(int npts, bool lremove_disp0,
                                     double dt, double SA_time,
                                     double cmt_window_avg,
                                     double epoch,
                                     double *__restrict__ ubuff,
                                     double *__restrict__ nbuff,
                                     double *__restrict__ ebuff,
                                     double *uAvg, double *nAvg, double *eAvg);

/*!
 * @brief Drives the CMT estimation.
 *
 * @param[in] props      properties for CMT grid search inversion
 * @param[in] SA         shakeAlert structure with hypocentral information
 * @param[in] gps_data   GPS data to invert
 *
 * @param[inout] cmt     on input contains the depths for the grid search
 *                       on output contains the corresponding variance
 *                       reduction, moment tensors, nodal planes at each
 *                       depth in the CMT grid search, and optimal depth
 *                       index.
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_CMT__driver(struct GFAST_props_struct props,
                      struct GFAST_shakeAlert_struct SA,
                      struct GFAST_data_struct gps_data,
                      struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_CMT__driver\0";
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
        }
        goto ERROR;
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
            luse = __GFAST_CMT__getAvgDisplacement(gps_data.data[k].npts,
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
        log_debugF("%s: Inverting for CMT with %d sites\n", fcnm, l1);
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
                                      cmt->objfn,
                                      cmt->mts,
                                      cmt->str1, cmt->str2,
                                      cmt->dip1, cmt->dip2,
                                      cmt->rak1, cmt->rak2,
                                      cmt->Mw);
    if (ierr != 0){
        log_errorF("%s: Error in CMT gridsearch!\n", fcnm);
        goto ERROR;
    }else{
        cmt->opt_indx = numpy_argmin(cmt->ndeps, cmt->objfn);
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
bool __GFAST_CMT__getAvgDisplacement(int npts, bool lremove_disp0,
                                     double dt, double SA_time,
                                     double cmt_window_avg,
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
    indx0 = npts - (int) (cmt_window_avg/dt + 0.5);
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
