#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Allocates space for the finite fault inversion.  It is assumed
 *        here that only two planes are inverted on. 
 *
 * @param[in] props      holds the FF parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] ff        has space allocated for the FF inversion
 * @param[out] ff_data   holds space for the offset data to be used
 *                       in the FF inversion as well as requisite
 *                       site information
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_ff_initialize(struct GFAST_ff_props_struct props,
                       struct GFAST_data_struct gps_data,
                       struct GFAST_ffResults_struct *ff,
                       struct GFAST_offsetData_struct *ff_data)
{
    const char *fcnm = "core_ff_initialize\0";
    int i, ifp, maxobs, nstr_ndip;
    //------------------------------------------------------------------------//
    maxobs = gps_data.stream_length;
    if (maxobs < 1)
    {
        log_errorF("%s: There will be no data in FF inversion!\n", fcnm);
        return -1;
    }
    nstr_ndip = props.ndip*props.nstr;
    if (nstr_ndip < 1)
    {
        log_errorF("%s: Error not fault patches!\n", fcnm);
        return -1;
    }
    // data
    ff_data->stnm = (char **)calloc((size_t) (gps_data.stream_length),
                                    sizeof(char *));
    ff_data->ubuff   = memory_calloc64f(gps_data.stream_length);
    ff_data->nbuff   = memory_calloc64f(gps_data.stream_length); 
    ff_data->ebuff   = memory_calloc64f(gps_data.stream_length);
    ff_data->wtu     = memory_calloc64f(gps_data.stream_length);
    ff_data->wtn     = memory_calloc64f(gps_data.stream_length); 
    ff_data->wte     = memory_calloc64f(gps_data.stream_length);
    ff_data->sta_lat = memory_calloc64f(gps_data.stream_length);
    ff_data->sta_lon = memory_calloc64f(gps_data.stream_length);
    ff_data->sta_alt = memory_calloc64f(gps_data.stream_length);
    ff_data->lmask   = memory_calloc8l(gps_data.stream_length);
    ff_data->lactive = memory_calloc8l(gps_data.stream_length);
    ff_data->nsites = gps_data.stream_length;
    for (i=0; i<ff_data->nsites; i++)
    {
        ff_data->sta_lat[i] = gps_data.data[i].sta_lat;
        ff_data->sta_lon[i] = gps_data.data[i].sta_lon;
        ff_data->sta_alt[i] = gps_data.data[i].sta_alt;
        ff_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(ff_data->stnm[i], gps_data.data[i].netw);
        strcat(ff_data->stnm[i], ".\0");
        strcat(ff_data->stnm[i], gps_data.data[i].stnm);
        strcat(ff_data->stnm[i], ".\0");
        strncpy(ff_data->stnm[i], gps_data.data[i].chan[0], 2);
        strcat(ff_data->stnm[i], "?.\0");
        if (strlen(gps_data.data[i].loc) > 0)
        {
            strcat(ff_data->stnm[i], gps_data.data[i].loc);
        }
        if (gps_data.data[i].lskip_ff){ff_data->lmask[i] = true;}
    }
    // ff inversion structure
    ff->preferred_fault_plane = 0;
    ff->nfp = props.nfp;
    ff->vr = memory_calloc64f(ff->nfp);
    ff->Mw = memory_calloc64f(ff->nfp);
    ff->str = memory_calloc64f(ff->nfp);
    ff->dip = memory_calloc64f(ff->nfp);
    ff->fp = (struct GFAST_faultPlane_struct *)
             calloc((size_t) ff->nfp,
                    sizeof(struct GFAST_faultPlane_struct));
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ff->fp[ifp].maxobs = maxobs;
        ff->fp[ifp].nsites_used = 0;
        ff->fp[ifp].nstr = props.nstr;
        ff->fp[ifp].ndip = props.ndip;
        ff->fp[ifp].lon_vtx    = memory_calloc64f(4*nstr_ndip);
        ff->fp[ifp].lat_vtx    = memory_calloc64f(4*nstr_ndip);
        ff->fp[ifp].dep_vtx    = memory_calloc64f(4*nstr_ndip);
        ff->fp[ifp].fault_xutm = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].fault_yutm = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].fault_alt  = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].strike     = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].dip        = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].length     = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].width      = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].sslip      = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].dslip      = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].sslip_unc  = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].dslip_unc  = memory_calloc64f(nstr_ndip);
        ff->fp[ifp].EN         = memory_calloc64f(maxobs);
        ff->fp[ifp].NN         = memory_calloc64f(maxobs);
        ff->fp[ifp].UN         = memory_calloc64f(maxobs);
        ff->fp[ifp].Einp       = memory_calloc64f(maxobs);
        ff->fp[ifp].Ninp       = memory_calloc64f(maxobs);
        ff->fp[ifp].Uinp       = memory_calloc64f(maxobs);
        ff->fp[ifp].fault_ptr  = memory_calloc32i(nstr_ndip + 1);
    }
    return 0;
} 
    
