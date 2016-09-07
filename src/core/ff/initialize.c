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
    ff_data->stnm = (char **)calloc(gps_data.stream_length, sizeof(char *));
    ff_data->ubuff   = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->nbuff   = ISCL_memory_calloc__double(gps_data.stream_length); 
    ff_data->ebuff   = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->wtu     = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->wtn     = ISCL_memory_calloc__double(gps_data.stream_length); 
    ff_data->wte     = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_lat = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_lon = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->sta_alt = ISCL_memory_calloc__double(gps_data.stream_length);
    ff_data->lmask   = ISCL_memory_calloc__bool(gps_data.stream_length);
    ff_data->lactive = ISCL_memory_calloc__bool(gps_data.stream_length);
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
    ff->vr = ISCL_memory_calloc__double(ff->nfp);
    ff->Mw = ISCL_memory_calloc__double(ff->nfp);
    ff->str = ISCL_memory_calloc__double(ff->nfp);
    ff->dip = ISCL_memory_calloc__double(ff->nfp);
    ff->fp = (struct GFAST_faultPlane_struct *)
             calloc(ff->nfp, sizeof(struct GFAST_faultPlane_struct));
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        ff->fp[ifp].maxobs = maxobs;
        ff->fp[ifp].nsites_used = 0;
        ff->fp[ifp].nstr = props.nstr;
        ff->fp[ifp].ndip = props.ndip;
        ff->fp[ifp].lon_vtx = ISCL_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].lat_vtx = ISCL_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].dep_vtx = ISCL_memory_calloc__double(4*nstr_ndip);
        ff->fp[ifp].fault_xutm = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].fault_yutm = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].fault_alt = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].strike = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dip = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].length = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].width = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].sslip = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dslip = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].sslip_unc = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].dslip_unc = ISCL_memory_calloc__double(nstr_ndip);
        ff->fp[ifp].EN = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].NN = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].UN = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].Einp = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].Ninp = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].Uinp = ISCL_memory_calloc__double(maxobs);
        ff->fp[ifp].fault_ptr = ISCL_memory_calloc__int(nstr_ndip + 1);
    }
    return 0;
} 
    
