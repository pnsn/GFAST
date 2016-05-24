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
static int __verify_ff_structs(struct GFAST_offsetData_struct ff_data,
                               struct GFAST_ffResults_struct *ff);
/*!
 * @brief Drives the finite fault fault plane grid search inversion 
 */
int GFAST_FF__driver(struct GFAST_ff_props_struct ff_props,
                     double SA_lat, double SA_lon, double SA_dep,
                     struct GFAST_offsetData_struct ff_data,
                     struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "GFAST_FF__driver\0";
    double *dip, *dslip, *dslip_unc, *eOffset, *EN, *eWts,
           *fault_xutm, *fault_yutm, *fault_alt, *length,
           *Mw, *nOffset, *NN, *nWts, *sslip, *sslip_unc, *staAlt,
           *strike, *uOffset, *utmRecvEasting, *utmRecvNorthing,
           *UN, *uWts, *vr, *width,
           wte, wtn, wtu, x1, x2, y1, y2;
    int i, ierr, ierr1, if_off, ifp, io_off, k, l1, l2,
        ndip, nfp, nstr, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = FF_SUCCESS;
    staAlt = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    eOffset = NULL;
    nOffset = NULL;
    uOffset = NULL;
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
    // Verify the input data structures
    ierr = __verify_ff_structs(ff_data, ff);
    if (ierr != 0)
    {
        log_errorF("%s: Error failed to verify data structures\n", fcnm);
        goto ERROR;
    }
    // Initialize the result
    nfp = ff->nfp;
    nstr = ff->fp[0].nstr;
    ndip = ff->fp[0].ndip;
    l2 = nstr*ndip;
    ff->preferred_fault_plane = 0;
    for (ifp=0; ifp<nfp; ifp++)
    {
        if_off = ifp*l2;
        // Faults plane info
        #pragma omp simd
        for (i=0; i<l2; i++)
        {
            ff->fp[ifp].fault_xutm[i] = 0.0;
            ff->fp[ifp].fault_yutm[i] = 0.0;
            ff->fp[ifp].fault_alt[i] = 0.0;
            ff->fp[ifp].strike[i] = 0.0;
            ff->fp[ifp].dip[i] = 0.0;
            ff->fp[ifp].length[i] = 0.0;
            ff->fp[ifp].width[i] = 0.0;
            ff->fp[ifp].sslip[i] = 0.0;
            ff->fp[ifp].dslip[i] = 0.0;
            ff->fp[ifp].sslip_unc[i] = 0.0;
            ff->fp[ifp].dslip_unc[i] = 0.0;
        }
        // Plotting information
        #pragma omp simd
        for (i=0; i<4*l2; i++)
        {
            ff->fp[ifp].lon_vtx[i] = 0.0;
            ff->fp[ifp].lat_vtx[i] = 0.0;
            ff->fp[ifp].dep_vtx[i] = 0.0;
        }
        // Observations
        #pragma omp simd
        for (i=0; i<ff->fp[ifp].maxobs; i++)
        {
            ff->fp[ifp].EN[i] = 0.0;
            ff->fp[ifp].NN[i] = 0.0;
            ff->fp[ifp].UN[i] = 0.0;
            ff->fp[ifp].Einp[i] = 0.0;
            ff->fp[ifp].Ninp[i] = 0.0;
            ff->fp[ifp].Uinp[i] = 0.0;
        }
        ff->fp[ifp].nsites_used = 0;
        ff->Mw[ifp] = 0.0;
        ff->vr[ifp] = 0.0;
    } // Loop on fault planes 
    // Require there is a sufficient amount of data to invert
    luse = GFAST_memory_calloc__bool(ff_data.nsites);
    l1 = 0;
    for (k=0; k<ff_data.nsites; k++)
    {
        wtu = ff_data.wtu[k];
        wtn = ff_data.wtn[k];
        wte = ff_data.wte[k];
        if (!ff_data.lactive[k] || fmax(wtu, fmax(wtn, wte)) <= 0.0)
        {
             continue;
        }
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < ff_props.min_sites)
    {
        if (ff_props.verbose > 1)
        {
            log_warnF("%s: Insufficient data to invert %d < %d\n",
                      fcnm, l1, ff_props.min_sites);
        }
        ierr = FF_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Set space
    uOffset = GFAST_memory_calloc__double(l1);
    nOffset = GFAST_memory_calloc__double(l1);
    eOffset = GFAST_memory_calloc__double(l1);
    utmRecvEasting  = GFAST_memory_calloc__double(l1);
    utmRecvNorthing = GFAST_memory_calloc__double(l1);
    staAlt = GFAST_memory_calloc__double(l1);
    uWts = GFAST_memory_calloc__double(l1);
    nWts = GFAST_memory_calloc__double(l1);
    eWts = GFAST_memory_calloc__double(l1);
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
    // Get the source location
    zone_loc = ff_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    GFAST_coordtools__ll2utm(SA_lat, SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    // Get cartesian positions and observations onto local arrays
    l1 = 0;
    for (k=0; k<ff_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(ff_data.sta_lat[k],
                                 ff_data.sta_lon[k],
                                 &y2, &x2,
                                 &lnorthp, &zone_loc);
        // Copy the pertinent data
        uOffset[l1] = ff_data.ubuff[k];
        nOffset[l1] = ff_data.nbuff[k];
        eOffset[l1] = ff_data.ebuff[k];
        uWts[l1] = ff_data.wtu[k];
        nWts[l1] = ff_data.wtn[k];
        eWts[l1] = ff_data.wte[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = ff_data.sta_alt[k];
        l1 = l1 + 1;
    }
    //------------------------------Fault Plane Mesher------------------------//
    // Mesh the fault planes remembering the event hypocenter and strike/dip
    // information were defined in the calling routine
    ierr = 0;
    if (ff_props.verbose > 2)
    {
        log_debugF("%s: Meshing fault plane...\n", fcnm);
    }
#ifdef PARALLEL_FF
    #pragma omp parallel for \
     private(ierr1, ifp) \
     shared(fcnm, ff, ff_props, zone_loc) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ierr1 = GFAST_FF__meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
                                         ff_props.flen_pct,
                                         ff_props.fwid_pct,
                                         ff->SA_mag, ff->str[ifp], ff->dip[ifp],
                                         ff->fp[ifp].nstr, ff->fp[ifp].ndip,
                                         zone_loc, ff_props.verbose,
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
        if (ierr1 != 0)
        {
            log_errorF("%s: Error meshing fault plane\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
    } // Loop on fault planes
    if (ierr != 0)
    {
        log_errorF("%s: Error meshing fault planes!\n", fcnm);
        goto ERROR;
    }
    //------------------------------ Inversion -------------------------------//
    // Map the fault planes to the meshes arrays
    for (ifp=0; ifp<nfp; ifp++)
    {   
        if_off = ifp*l2;
        #pragma omp simd
        for (i=0; i<l2; i++)
        {
            fault_xutm[if_off+i] = ff->fp[ifp].fault_xutm[i];
            fault_yutm[if_off+i] = ff->fp[ifp].fault_yutm[i];
            fault_alt[if_off+i]  = ff->fp[ifp].fault_alt[i];
            length[if_off+i]     = ff->fp[ifp].length[i];
            width[if_off+i]      = ff->fp[ifp].width[i]; 
            strike[if_off+i]     = ff->fp[ifp].strike[i];
            dip[if_off+i]        = ff->fp[ifp].dip[i];
        }
    }
    // Let user know an inversion is about to happen 
    if (ff_props.verbose > 2)
    {   
        log_debugF("%s: Inverting for slip on %d planes with %d sites\n",
                   fcnm, nfp, l1);
    }
    // Perform the finite fault inversion
    ierr = GFAST_FF__faultPlaneGridSearch(l1, l2,
                                          nstr, ndip, nfp,
                                          ff_props.verbose,
                                          nOffset, eOffset, uOffset,
                                          nWts, eWts, uWts,
                                          utmRecvEasting, utmRecvNorthing,
                                          staAlt,
                                          fault_xutm, fault_yutm, fault_alt,
                                          length, width,
                                          strike, dip,
                                          sslip, dslip,
                                          Mw, vr,
                                          NN, EN, UN,
                                          sslip_unc, dslip_unc);
    if (ierr != 0)
    {
        log_errorF("%s: Error performing finite fault grid search\n", fcnm);
        goto ERROR;
    }
    //----------------------------Extract the Results-------------------------//
    // Unpack results onto struture and choose a preferred plane
    ff->preferred_fault_plane = 0;
    for (ifp=0; ifp<nfp; ifp++)
    {
        if_off = ifp*l2;
        io_off = ifp*l1;
        #pragma omp simd
        for (i=0; i<l2; i++)
        {
            ff->fp[ifp].sslip[i] = sslip[if_off+i];
            ff->fp[ifp].dslip[i] = dslip[if_off+i];
            ff->fp[ifp].sslip_unc[i] = sslip[if_off+i];
            ff->fp[ifp].dslip_unc[i] = dslip[if_off+i];
        }
        // Observations
        i = 0;
        for (k=0; k<ff->fp[ifp].maxobs; k++)
        {
            if (luse[k])
            {
                ff->fp[ifp].EN[k] = EN[io_off+i];
                ff->fp[ifp].NN[k] = NN[io_off+i];
                ff->fp[ifp].UN[k] = UN[io_off+i];
                ff->fp[ifp].Einp[k] = eOffset[i];
                ff->fp[ifp].Ninp[k] = nOffset[i];
                ff->fp[ifp].Uinp[k] = uOffset[i];
                i = i + 1;
            }
        }
        ff->fp[ifp].nsites_used = i;
        ff->Mw[ifp] = Mw[ifp]; // moment magnitude 
        ff->vr[ifp] = vr[ifp]; // variance reduction
        // Preferred fault plane has greatest variance reduction
        if (ff->vr[ifp] > ff->vr[ff->preferred_fault_plane])
        {
            ff->preferred_fault_plane = ifp;
        }
    } // Loop on fault planes
ERROR:;
    GFAST_memory_free__bool(&luse);
    GFAST_memory_free__double(&uOffset);
    GFAST_memory_free__double(&nOffset);
    GFAST_memory_free__double(&eOffset);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&nWts);
    GFAST_memory_free__double(&eWts);
    GFAST_memory_free__double(&uWts);
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
 * @brief Utility function for verifying input data structures
 *
 * @param[in] ff_data    ff_data structure to verify
 * @param[in] ff         ff results structure to verify
 *
 * @result FF_SUCCESS indicates input structures are ready for use
 *
 * @author Ben Baker (ISTI)
 *
 */
static int __verify_ff_structs(struct GFAST_offsetData_struct ff_data,
                               struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "__verify_ff_structs\0";
    int ierr, ifp;
    ierr = FF_SUCCESS;
    if (ff_data.nsites < 1)
    {
        ierr = FF_OS_DATA_ERROR;
        log_errorF("%s: No peak displacement data\n", fcnm);
        goto ERROR;
    }
    // Verify the output data structures
    if (ff->nfp < 1)
    {
        log_errorF("%s: No fault planes in fault plane gridsearch!\n", fcnm);
        ierr = FF_STRUCT_ERROR;
        goto ERROR;
    }
    if (ff->fp == NULL || ff->vr == NULL || ff->Mw == NULL ||
        ff->str == NULL || ff->dip == NULL)
    {
        if (ff->fp == NULL)
        {
            log_errorF("%s: Error fault plane is NULL\n", fcnm);
        }
        if (ff->vr == NULL)
        {
            log_errorF("%s: Error ff->vr is NULL\n", fcnm);
        }
        if (ff->Mw == NULL)
        {
            log_errorF("%s: Error ff->Mw is NULL\n", fcnm);
        }
        if (ff->str == NULL)
        {
            log_errorF("%s: Error ff->str is NULL\n", fcnm);
        }
        if (ff->dip == NULL)
        {
            log_errorF("%s: Error ff->dip is NULL\n", fcnm);
        }
        ierr = FF_STRUCT_ERROR;
        goto ERROR;
    }
    // Loop on fault planes and check each fp structure
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        if (ff->fp[ifp].nstr != ff->fp[0].nstr)
        {
            log_errorF("%s: Error inconsistent number of strike patches\n",
                       fcnm);
            ierr = FF_STRUCT_ERROR;
        }
        if (ff->fp[ifp].ndip != ff->fp[0].ndip)
        {
            log_errorF("%s: Error inconsistent number of dip patches\n", fcnm);
            ierr = FF_STRUCT_ERROR;       
        }
        if (ff->fp[ifp].lon_vtx == NULL ||
            ff->fp[ifp].lat_vtx == NULL ||
            ff->fp[ifp].dep_vtx == NULL ||
            ff->fp[ifp].fault_xutm == NULL ||
            ff->fp[ifp].fault_yutm == NULL ||
            ff->fp[ifp].fault_alt == NULL ||
            ff->fp[ifp].strike == NULL ||
            ff->fp[ifp].dip == NULL ||
            ff->fp[ifp].length == NULL ||
            ff->fp[ifp].width == NULL ||
            ff->fp[ifp].sslip == NULL ||
            ff->fp[ifp].dslip == NULL ||
            ff->fp[ifp].sslip_unc == NULL ||
            ff->fp[ifp].dslip_unc == NULL)
        {
            if (ff->fp[ifp].lon_vtx == NULL)
            {
                log_errorF("%s: Error lon_vtx null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].lat_vtx == NULL)
            {
                log_errorF("%s: Error lat_vtx null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].dep_vtx == NULL)
            {
                log_errorF("%s: Error dep_vtx null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].fault_xutm == NULL)
            {
                log_errorF("%s: Error fault_xutm null on ifp %d\n",
                            fcnm, ifp+1);
            }
            if (ff->fp[ifp].fault_yutm == NULL)
            {
                log_errorF("%s: Error fault_xutm null on ifp %d\n",
                            fcnm, ifp+1);
            }
            if (ff->fp[ifp].fault_alt == NULL)
            {
                log_errorF("%s: Error fault_alt null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].strike == NULL)
            {
                log_errorF("%s: Error strike null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].dip == NULL)
            {
                log_errorF("%s: Error dip null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].length == NULL)
            {
                log_errorF("%s: Error length null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].width == NULL)
            {
                log_errorF("%s: Error width null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].sslip == NULL)
            {
                log_errorF("%s: Error sslip null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].dslip == NULL)
            {
                log_errorF("%s: Error dslip null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].sslip_unc == NULL)
            {
                log_errorF("%s: Error sslip_unc null on ifp %d\n", fcnm, ifp+1);
            }
            if (ff->fp[ifp].dslip_unc == NULL)
            {
                log_errorF("%s: Error dslip_unc null on ifp %d\n", fcnm, ifp+1);
            }
            log_errorF("%s: Error fp %d is invalid\n", fcnm, ifp+1);
            ierr = FF_STRUCT_ERROR;
            goto ERROR;
        }
    } // Loop on fault planes
    // Avoid a segfault
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        if (ff->fp[ifp].maxobs != ff_data.nsites)
        {
            log_errorF("%s: nsites on ff and ff_data differs %d %d\n",
                       fcnm, ff->fp[ifp].maxobs, ff_data.nsites);
            ierr = FF_STRUCT_ERROR;
            goto ERROR;
        }
    }
ERROR:;
    return ierr;
}
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
int GFAST_FF__driver2(struct GFAST_ff_props_struct props,
                      struct GFAST_shakeAlert_struct SA,
                      struct GFAST_data_struct gps_data,
                      struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "GFAST_FF__driver2\0";
    enum ff_return2_enum
    {   
        FF_SUCCESS = 0,
        FF_STRUCT_ERROR = 1,
        FF_GPS_DATA_ERROR = 2,
        FF_INSUFFICIENT_DATA = 3,
        FF_COMPUTE_ERROR = 4,
        FF_MEMORY_ERROR = 5
    };
    double *dip, *dslip, *dslip_unc, *eAvgDisp, *EN, *eWts,
           *fault_xutm, *fault_yutm, *fault_alt, *length,
           *Mw, *nAvgDisp, *NN, *nWts, *sslip, *sslip_unc, *staAlt,
           *strike, *uAvgDisp, *utmRecvEasting, *utmRecvNorthing,
           *UN, *uWts, *vr, *width, currentTime, distance, eAvg,
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
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
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
    if (gps_data.stream_length < 1)
    {
        if (props.verbose > 1)
        {
            ierr = FF_GPS_DATA_ERROR;
            log_warnF("%s: No GPS streams\n", fcnm);
        }
        goto ERROR;
    }
    if (ff->nfp < 1)
    {
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
    if (l1 < props.min_sites)
    {
        if (props.verbose > 1)
        {
            if (l1 < 1)
            {
                log_warnF("%s: All sites masked in FF estimation\n", fcnm);
            }
            else
            {
                log_warnF("%s: Too many masked sites to compute FF\n", fcnm);
            }
        }
        ierr = FF_GPS_DATA_ERROR;
        goto ERROR;
    }   
    if (nwork < 1)
    {
        if (props.verbose > 1)
        {
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
    uWts = GFAST_memory_calloc__double(gps_data.stream_length);
    nWts = GFAST_memory_calloc__double(gps_data.stream_length);
    eWts = GFAST_memory_calloc__double(gps_data.stream_length);
    // Get the source location
    zone_loc = props.utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Get UTM zone from source lat/lon
    GFAST_coordtools__ll2utm(ff->SA_lat, ff->SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k=0; k<gps_data.stream_length; k++)
    {
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
        effectiveHypoDist = (currentTime - SA.time)*props.window_vel;
        if (distance < effectiveHypoDist)
        {
            luse = __GFAST_FF__getAvgDisplacement(gps_data.data[k].npts,
                                                  true,
                                                  gps_data.data[k].dt,
                                                  SA.time,
                                                  props.window_avg,
                                                  gps_data.data[k].epoch,
                                                  gps_data.data[k].ubuff,
                                                  gps_data.data[k].nbuff,
                                                  gps_data.data[k].ebuff,
                                                  &uAvg, &nAvg, &eAvg);
            if (luse)
            {
                uAvgDisp[l1] = uAvg;
                nAvgDisp[l1] = nAvg;
                eAvgDisp[l1] = eAvg;
                uWts[l1] = 1.0;
                nWts[l1] = 1.0;
                eWts[l1] = 1.0;
                utmRecvNorthing[l1] = y2;
                utmRecvEasting[l1] = x2;
                staAlt[l1] = gps_data.data[k].sta_alt;
                l1 = l1 + 1;
            }
        }
    } // Loop on data streams
    if (l1 < props.min_sites){
        if (props.verbose > 1){
            log_warnF("%s: Insufficient data to invert\n", fcnm);
        }
        ierr = FF_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // An inversion is going to hapen 
    if (props.verbose > 2)
    {
        log_debugF("%s: Performing finite fault inversion with %d sites\n",
                   fcnm, l1);
    }
    nfp = ff->nfp;
    nstr = props.nstr;
    ndip = props.ndip;
    l2 = nstr*ndip;
    // Mesh the fault planes remembering the event hypocenter and strike/dip
    // information were defined in the calling routine
    ierr = 0;
    if (props.verbose > 2)
    {
        log_debugF("%s: Meshing fault plane...\n", fcnm);
    }
#ifdef PARALLEL_FF
    #pragma omp parallel for \
     private(ierr1, ifp) \
     shared(fcnm, ff, props) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ierr1 = GFAST_FF__meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
                                         props.flen_pct,
                                         props.fwid_pct,
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
        if (ierr1 != 0)
        {
            log_errorF("%s: Error meshing fault plane\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
    } // Loop on fault planes
    if (ierr != 0)
    {
        log_errorF("%s: Error meshing fault planes!\n", fcnm);
        goto ERROR;
    }
    // Save the number of sites used in the inversion
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
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
    for (ifp=0; ifp<nfp; ifp++)
    {
        if_off = ifp*l2;
        #pragma omp simd
        for (i=0; i<l2; i++)
        {
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
                                          nWts, eWts, uWts,
                                          utmRecvEasting, utmRecvNorthing,
                                          staAlt,
                                          fault_xutm, fault_yutm, fault_alt,
                                          length, width,
                                          strike, dip,
                                          sslip, dslip,
                                          Mw, vr,
                                          NN, EN, UN,
                                          sslip_unc, dslip_unc);
    if (ierr != 0)
    {
        log_errorF("%s: Error performing finite fault grid search\n", fcnm);
    }
    else
    {
        // Unpack results onto struture and choose a preferred plane
        ff->preferred_fault_plane = 0;
        for (ifp=0; ifp<nfp; ifp++)
        {
            if_off = ifp*l2;
            io_off = ifp*l1;
            #pragma omp simd
            for (i=0; i<l2; i++)
            {
                ff->fp[ifp].sslip[i] = sslip[if_off+i]; 
                ff->fp[ifp].dslip[i] = dslip[if_off+i];
            }
            if (ff->fp[ifp].sslip_unc)
            {
                cblas_dcopy(l2, &sslip_unc[if_off], 1,
                                ff->fp[ifp].sslip_unc, 1);
            } 
            if (ff->fp[ifp].dslip_unc)
            {
                cblas_dcopy(l2, &dslip_unc[if_off], 1,
                                ff->fp[ifp].dslip_unc, 1);
            }
            // Observations
            #pragma omp simd
            for (i=0; i<l1; i++)
            {
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
            if (ff->vr[ifp] > ff->vr[ff->preferred_fault_plane])
            {
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
    GFAST_memory_free__double(&nWts);
    GFAST_memory_free__double(&eWts);
    GFAST_memory_free__double(&uWts);
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
