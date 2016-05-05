#include <stdio.h>
#include <stdlib.h>
#include "gfast.h"

/*!
 * @brief Allocates space for the finite fault inversion.  It is assumed
 *        here that only two planes are inverted on. 
 *
 * @param[in] props      holds the FF parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] ff        has space allocated for the FF inversion
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_FF__init(struct GFAST_props_struct props,
                   struct GFAST_data_struct gps_data,
                   struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "GFAST_FF__init\0";
    int ifp, maxobs, nstr_ndip;
    maxobs = gps_data.stream_length;
    if (maxobs < 1){
        log_errorF("%s: There will be no data in FF inversion!\n", fcnm);
        return -1;
    }
    nstr_ndip = props.ff_ndip*props.ff_nstr;
    if (nstr_ndip < 1){
        log_errorF("%s: Error not fault patches!\n", fcnm);
        return -1;
    }
    ff->preferred_fault_plane = 0;
    ff->nfp = props.ff_nfp;
    ff->vr = GFAST_memory_calloc__double(ff->nfp);
    ff->Mw = GFAST_memory_calloc__double(ff->nfp);
    ff->str = GFAST_memory_calloc__double(ff->nfp);
    ff->dip = GFAST_memory_calloc__double(ff->nfp);
    ff->fp = (struct GFAST_faultPlane_struct *)
             calloc(ff->nfp, sizeof(struct GFAST_faultPlane_struct));
    for (ifp=0; ifp<ff->nfp; ifp++){
        ff->fp[ifp].maxobs = maxobs;
        ff->fp[ifp].nsites_used = 0;
        ff->fp[ifp].nstr = props.ff_nstr;
        ff->fp[ifp].ndip = props.ff_ndip;
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
    
