#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <lapacke.h>
#include <cblas.h>
#include "gfast.h"
#include "iscl/memory/memory.h"

static int __verify_ff_structs(struct GFAST_offsetData_struct ff_data,
                               struct GFAST_ffResults_struct *ff);
/*!
 * @brief Drives the finite fault fault plane grid search inversion 
 *
 * @param[in] ff_props    finite fault inversion parameters
 * @param[in] SA_lat      event latitude (degrees)
 * @param[in] SA_lon      event longitude (degrees) 
 * @param[in] SA_dep      event depth (km).  note, it may be advantageous
 *                        to obtain this from the CMT inversion
 * @param[in] ff_data     offset data for the finite fault inversion
 *
 * @param[inout] ff       on input contains space for the finite fault
 *                        inversion as well as the fault strike and rake
 *                        on all the nodal planes in the inversion.
 *                        on output contains the results of the finite
 *                        fault inversion which includes slip along
 *                        strike and slip down dip on each plane,
 *                        uncertainties, and estimate data and observed data 
 *
 * @result \reval 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
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
    luse = ISCL_memory_calloc__bool(ff_data.nsites);
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
    uOffset = ISCL_memory_calloc__double(l1);
    nOffset = ISCL_memory_calloc__double(l1);
    eOffset = ISCL_memory_calloc__double(l1);
    utmRecvEasting  = ISCL_memory_calloc__double(l1);
    utmRecvNorthing = ISCL_memory_calloc__double(l1);
    staAlt = ISCL_memory_calloc__double(l1);
    uWts = ISCL_memory_calloc__double(l1);
    nWts = ISCL_memory_calloc__double(l1);
    eWts = ISCL_memory_calloc__double(l1);
    fault_xutm = ISCL_memory_calloc__double(l2*nfp);
    fault_yutm = ISCL_memory_calloc__double(l2*nfp);
    fault_alt  = ISCL_memory_calloc__double(l2*nfp);
    length     = ISCL_memory_calloc__double(l2*nfp);
    width      = ISCL_memory_calloc__double(l2*nfp);
    strike     = ISCL_memory_calloc__double(l2*nfp);
    dip        = ISCL_memory_calloc__double(l2*nfp);
    sslip      = ISCL_memory_calloc__double(l2*nfp);
    dslip      = ISCL_memory_calloc__double(l2*nfp);
    Mw         = ISCL_memory_calloc__double(nfp);
    vr         = ISCL_memory_calloc__double(nfp);
    NN         = ISCL_memory_calloc__double(l1*nfp);
    EN         = ISCL_memory_calloc__double(l1*nfp);
    UN         = ISCL_memory_calloc__double(l1*nfp);
    sslip_unc  = ISCL_memory_calloc__double(l2*nfp);
    dslip_unc  = ISCL_memory_calloc__double(l2*nfp);
    // Get the source location
    zone_loc = ff_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    GFAST_core_coordtools_ll2utm(SA_lat, SA_lon,
                                 &y1, &x1,
                                 &lnorthp, &zone_loc);
    // Get cartesian positions and observations onto local arrays
    l1 = 0;
    for (k=0; k<ff_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        GFAST_core_coordtools_ll2utm(ff_data.sta_lat[k],
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
        ierr1 = GFAST_core_ff_meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
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
    ierr = GFAST_core_ff_faultPlaneGridSearch(l1, l2,
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
    ISCL_memory_free__bool(&luse);
    ISCL_memory_free__double(&uOffset);
    ISCL_memory_free__double(&nOffset);
    ISCL_memory_free__double(&eOffset);
    ISCL_memory_free__double(&utmRecvEasting);
    ISCL_memory_free__double(&utmRecvNorthing);
    ISCL_memory_free__double(&staAlt);
    ISCL_memory_free__double(&nWts);
    ISCL_memory_free__double(&eWts);
    ISCL_memory_free__double(&uWts);
    ISCL_memory_free__double(&fault_xutm);
    ISCL_memory_free__double(&fault_yutm);
    ISCL_memory_free__double(&fault_alt);
    ISCL_memory_free__double(&length);
    ISCL_memory_free__double(&width);
    ISCL_memory_free__double(&strike);
    ISCL_memory_free__double(&dip);
    ISCL_memory_free__double(&sslip);
    ISCL_memory_free__double(&dslip);
    ISCL_memory_free__double(&Mw);
    ISCL_memory_free__double(&vr);
    ISCL_memory_free__double(&NN);
    ISCL_memory_free__double(&EN);
    ISCL_memory_free__double(&UN);
    ISCL_memory_free__double(&sslip_unc);
    ISCL_memory_free__double(&dslip_unc);
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
