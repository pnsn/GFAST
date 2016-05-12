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
    double *dip, *dslip, *dslip_unc, *eAvgDisp, *EN,
           *fault_xutm, *fault_yutm, *fault_alt, *length,
           *Mw, *nAvgDisp, *NN, *sslip, *sslip_unc, *staAlt,
           *strike, *uAvgDisp, *utmRecvEasting, *utmRecvNorthing,
           *UN, *vr, *width, currentTime, distance, eAvg,
           effectiveHypoDist, nAvg, uAvg, x1, x2, y1, y2;
    int i, ierr, ierr1, if_off, ifp, io_off, k, l1, l2,
        ndip, nfp, nstr, nwork, zone_loc;
    bool lnorthp, luse;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = FF_SUCCESS;
    staAlt = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    eAvgDisp = NULL;
    nAvgDisp = NULL;
    uAvgDisp = NULL;
    fault_xutm = NULL;
    fault_yutm = NULL;
    fault_alt = NULL;
    length = NULL;
    width = NULL;
    strike = NULL;
    dip = NULL;
    sslip = NULL;
    dslip = NULL;
    Mw = NULL;
    vr = NULL;
    NN = NULL;
    EN = NULL;
    UN = NULL;
    sslip_unc = NULL;
    dslip_unc = NULL;
    ff->preferred_fault_plane =-1;
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
    GFAST_coordtools__ll2utm(ff->SA_lat, ff->SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_ff){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(gps_data.data[k].sta_lat,
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
    // An inversion is going to hapen 
    if (props.verbose > 2){ 
        log_debugF("%s: Performing finite fault inversion with %d sites\n",
                   fcnm, l1);
    }
    nfp = ff->nfp;
    nstr = props.ff_nstr;
    ndip = props.ff_ndip;
    l2 = nstr*ndip;
    // Mesh the fault planes remembering the event hypocenter and strike/dip
    // information were defined in the calling routine
    ierr = 0;
    if (props.verbose > 2){
        log_debugF("%s: Meshing fault plane...\n", fcnm);
    }
#ifdef __PARALLEL_FF
    #pragma omp parallel for \
     private(ierr1, ifp) \
     shared(fcnm, ff, props) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++){
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
    } // Loop on fault planes
    if (ierr != 0){
        log_errorF("%s: Error meshing fault planes!\n", fcnm);
        goto ERROR;
    }
    // Save the number of sites used in the inversion
    for (ifp=0; ifp<ff->nfp; ifp++){
        ff->fp[ifp].nsites_used = l1;
    }
    // Map the fault planes to the meshes
    nfp = ff->nfp;
    fault_xutm = GFAST_memory_calloc__double(l2*nfp);
    fault_yutm = GFAST_memory_calloc__double(l2*nfp);
    fault_alt  = GFAST_memory_calloc__double(l2*nfp);
    length     = GFAST_memory_calloc__double(l2*nfp);
    width      = GFAST_memory_calloc__double(l2*nfp);
    strike     = GFAST_memory_calloc__double(l2*nfp);
    dip        = GFAST_memory_calloc__double(l2*nfp);
    sslip      = GFAST_memory_calloc__double(l2*nfp);
    dslip      = GFAST_memory_calloc__double(l2*nfp);
    Mw         = GFAST_memory_calloc__double(nfp);
    vr         = GFAST_memory_calloc__double(nfp);
    NN         = GFAST_memory_calloc__double(l1*nfp);
    EN         = GFAST_memory_calloc__double(l1*nfp);
    UN         = GFAST_memory_calloc__double(l1*nfp);
    sslip_unc  = GFAST_memory_calloc__double(l2*nfp);
    dslip_unc  = GFAST_memory_calloc__double(l2*nfp);
    for (ifp=0; ifp<nfp; ifp++){
        if_off = ifp*l2;
        #pragma omp simd
        for (i=0; i<l2; i++){
            fault_xutm[if_off+i] = ff->fp[ifp].fault_xutm[i];
            fault_yutm[if_off+i] = ff->fp[ifp].fault_yutm[i];
            fault_alt[if_off+i]  = ff->fp[ifp].fault_alt[i];
            length[if_off+i]     = ff->fp[ifp].length[i];
            width[if_off+i]      = ff->fp[ifp].width[i]; 
            strike[if_off+i]     = ff->fp[ifp].strike[i];
            dip[if_off+i]        = ff->fp[ifp].dip[i];
        }
        ff->fp[ifp].nsites_used = l1; // Save number of sites in inversion 
    }
    // Perform the finite fault inversion
    ierr = GFAST_FF__faultPlaneGridSearch(l1, l2,
                                          nstr, ndip, nfp,
                                          props.verbose,
                                          nAvgDisp, eAvgDisp, uAvgDisp,
                                          utmRecvEasting, utmRecvNorthing,
                                          staAlt,
                                          fault_xutm, fault_yutm, fault_alt,
                                          length, width,
                                          strike, dip,
                                          sslip, dslip,
                                          Mw, vr,
                                          NN, EN, UN,
                                          sslip_unc, dslip_unc);
    if (ierr != 0){
        log_errorF("%s: Error performing finite fault grid search\n", fcnm);
    }else{
        // Unpack results onto struture and choose a preferred plane
        ff->preferred_fault_plane = 0;
        for (ifp=0; ifp<nfp; ifp++){
            if_off = ifp*l2;
            io_off = ifp*l1;
            #pragma omp simd
            for (i=0; i<l2; i++){
                ff->fp[ifp].sslip[i] = sslip[if_off+i]; 
                ff->fp[ifp].dslip[i] = dslip[if_off+i];
            }
            if (ff->fp[ifp].sslip_unc){
                cblas_dcopy(l2, &sslip_unc[if_off], 1,
                                ff->fp[ifp].sslip_unc, 1);
            } 
            if (ff->fp[ifp].dslip_unc){
                cblas_dcopy(l2, &dslip_unc[if_off], 1,
                                ff->fp[ifp].dslip_unc, 1);
            }
            // Observations
            #pragma omp simd
            for (i=0; i<l1; i++){
                ff->fp[ifp].EN[i] = EN[io_off+i];
                ff->fp[ifp].NN[i] = NN[io_off+i];
                ff->fp[ifp].UN[i] = UN[io_off+i];
                ff->fp[ifp].Einp[i] = eAvgDisp[i];
                ff->fp[ifp].Ninp[i] = nAvgDisp[i];
                ff->fp[ifp].Uinp[i] = uAvgDisp[i];
            }
            ff->Mw[ifp] = Mw[ifp]; // moment magnitude 
            ff->vr[ifp] = vr[ifp]; // variance reduction
            // Preferred fault plane has greatest variance reduction
            if (ff->vr[ifp] > ff->vr[ff->preferred_fault_plane]){
                ff->preferred_fault_plane = ifp;
            }
        }
    } // Loop on fault planes
    // Free memory
ERROR:;
    GFAST_memory_free__double(&uAvgDisp);
    GFAST_memory_free__double(&nAvgDisp);
    GFAST_memory_free__double(&eAvgDisp);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&fault_xutm);
    GFAST_memory_free__double(&fault_yutm);
    GFAST_memory_free__double(&fault_alt);
    GFAST_memory_free__double(&length);
    GFAST_memory_free__double(&width);
    GFAST_memory_free__double(&strike);
    GFAST_memory_free__double(&dip);
    GFAST_memory_free__double(&sslip);
    GFAST_memory_free__double(&dslip);
    GFAST_memory_free__double(&Mw);
    GFAST_memory_free__double(&vr);
    GFAST_memory_free__double(&NN);
    GFAST_memory_free__double(&EN);
    GFAST_memory_free__double(&UN);
    GFAST_memory_free__double(&sslip_unc);
    GFAST_memory_free__double(&dslip_unc);
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
