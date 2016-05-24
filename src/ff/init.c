#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"

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
int GFAST_FF__init(struct GFAST_ff_props_struct props,
                   struct GFAST_data_struct gps_data,
                   struct GFAST_ffResults_struct *ff,
                   struct GFAST_offsetData_struct *ff_data)
{
    const char *fcnm = "GFAST_FF__init\0";
    int i, ifp, maxobs, nstr_ndip;
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
    ff_data->stnm = (char **)calloc(gps_data.stream_length, sizeof(char *));
    ff_data->ubuff   = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->nbuff   = GFAST_memory_calloc__double(gps_data.stream_length); 
    ff_data->ebuff   = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->wtu     = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->wtn     = GFAST_memory_calloc__double(gps_data.stream_length); 
    ff_data->wte     = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_lat = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_lon = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_alt = GFAST_memory_calloc__double(gps_data.stream_length);
    ff_data->lmask   = GFAST_memory_calloc__bool(gps_data.stream_length);
    ff_data->lactive = GFAST_memory_calloc__bool(gps_data.stream_length);
    ff_data->nsites = gps_data.stream_length;
    for (i=0; i<ff_data->nsites; i++)
    {
        ff_data->sta_lat[i] = gps_data.data[i].sta_lat;
        ff_data->sta_lon[i] = gps_data.data[i].sta_lon;
        ff_data->sta_alt[i] = gps_data.data[i].sta_alt;
        ff_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(ff_data->stnm[i], gps_data.data[i].site);
        if (gps_data.data[i].lskip_ff){ff_data->lmask[i] = true;}
    }
    // ff inversion structure
    ff->preferred_fault_plane = 0;
    ff->nfp = props.nfp;
    ff->vr = GFAST_memory_calloc__double(ff->nfp);
    ff->Mw = GFAST_memory_calloc__double(ff->nfp);
    ff->str = GFAST_memory_calloc__double(ff->nfp);
    ff->dip = GFAST_memory_calloc__double(ff->nfp);
    ff->fp = (struct GFAST_faultPlane_struct *)
             calloc(ff->nfp, sizeof(struct GFAST_faultPlane_struct));
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ff->fp[ifp].maxobs = maxobs;
        ff->fp[ifp].nsites_used = 0;
        ff->fp[ifp].nstr = props.nstr;
        ff->fp[ifp].ndip = props.ndip;
        ff->fp[ifp].lon_vtx = GFAST_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].lat_vtx = GFAST_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].dep_vtx = GFAST_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].fault_xutm = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].fault_yutm = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].fault_alt = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].strike = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dip = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].length = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].width = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].sslip = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dslip = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].sslip_unc = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dslip_unc = GFAST_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].EN = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].NN = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].UN = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].Einp = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].Ninp = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].Uinp = GFAST_memory_calloc__double(maxobs);
        ff->fp[ifp].fault_ptr = GFAST_memory_calloc__int(nstr_ndip + 1);
    }
    return 0;
} 
    
