#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Free the fault plane structure
 *
 * @param[in,out] fp       fault plane structure with memory to be freed
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_ff_finalizeFaultPlane(struct GFAST_faultPlane_struct *fp)
{
    if (fp == NULL){return;}
    memory_free64f(&fp->lon_vtx);
    memory_free64f(&fp->lat_vtx);
    memory_free64f(&fp->dep_vtx);
    memory_free64f(&fp->fault_xutm);
    memory_free64f(&fp->fault_yutm);
    memory_free64f(&fp->fault_alt);
    memory_free64f(&fp->strike);
    memory_free64f(&fp->dip);
    memory_free64f(&fp->length);
    memory_free64f(&fp->width);
    memory_free64f(&fp->sslip);
    memory_free64f(&fp->dslip);
    memory_free64f(&fp->sslip_unc);
    memory_free64f(&fp->dslip_unc);
    memory_free64f(&fp->EN);
    memory_free64f(&fp->NN);
    memory_free64f(&fp->UN);
    memory_free64f(&fp->Einp);
    memory_free64f(&fp->Ninp);
    memory_free64f(&fp->Uinp);
    memory_free32i(&fp->fault_ptr);
    memset(fp, 0, sizeof(struct GFAST_faultPlane_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the finite fault results structure
 *
 * @param[in,out] ff      finite fault results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void core_ff_finalizeResults(struct GFAST_ffResults_struct *ff)
{
    int ifp;
    if (ff == NULL){return;}
    if (ff->nfp < 1){return;}
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        core_ff_finalizeFaultPlane(&ff->fp[ifp]);
    }
    if (ff->fp != NULL){free(ff->fp);}
    memory_free64f(&ff->vr);
    memory_free64f(&ff->Mw);
    memory_free64f(&ff->str);
    memory_free64f(&ff->dip);
    memset(ff, 0, sizeof(struct GFAST_ffResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with the offset data 
 *
 * @param[in,out] offset_data     offset data structure with memory to be freed
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_ff_finalizeOffsetData(struct GFAST_offsetData_struct *offset_data)
{
    int i;
    if (offset_data->stnm != NULL)
    {
        for (i=0; i<offset_data->nsites; i++)
        {
            free(offset_data->stnm[i]);
        }
        free(offset_data->stnm);
    }
    memory_free64f(&offset_data->ubuff);
    memory_free64f(&offset_data->nbuff);
    memory_free64f(&offset_data->ebuff);
    memory_free64f(&offset_data->wtu);
    memory_free64f(&offset_data->wtn);
    memory_free64f(&offset_data->wte);
    memory_free64f(&offset_data->sta_lat);
    memory_free64f(&offset_data->sta_lon);
    memory_free64f(&offset_data->sta_alt);
    memory_free8l(&offset_data->lmask);
    memory_free8l(&offset_data->lactive);
    memset(offset_data, 0, sizeof(struct GFAST_offsetData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Convenience function for freeing the finite fault structure and
 *        the data
 *
 * @param[out] ff_props       nulled out finite fault properties
 * @param[out] offset_data    offset data structure with memory to be freed
 * @param[out] ff             finite fault results structure with memory
 *                            to be freed
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_ff_finalize(struct GFAST_ff_props_struct *ff_props,
                      struct GFAST_offsetData_struct *offset_data,
                      struct GFAST_ffResults_struct *ff)
{
    core_properties_finalizeFFProperties(ff_props);
    core_ff_finalizeOffsetData(offset_data);
    core_ff_finalizeResults(ff);
    return;
}

