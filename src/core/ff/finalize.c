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
void core_ff_finalize__faultPlane(struct GFAST_faultPlane_struct *fp)
{
    if (fp == NULL){return;}
    ISCL_memory_free__double(&fp->lon_vtx);
    ISCL_memory_free__double(&fp->lat_vtx);
    ISCL_memory_free__double(&fp->dep_vtx);
    ISCL_memory_free__double(&fp->fault_xutm);
    ISCL_memory_free__double(&fp->fault_yutm);
    ISCL_memory_free__double(&fp->fault_alt);
    ISCL_memory_free__double(&fp->strike);
    ISCL_memory_free__double(&fp->dip);
    ISCL_memory_free__double(&fp->length);
    ISCL_memory_free__double(&fp->width);
    ISCL_memory_free__double(&fp->sslip);
    ISCL_memory_free__double(&fp->dslip);
    ISCL_memory_free__double(&fp->sslip_unc);
    ISCL_memory_free__double(&fp->dslip_unc);
    ISCL_memory_free__double(&fp->EN);
    ISCL_memory_free__double(&fp->NN);
    ISCL_memory_free__double(&fp->UN);
    ISCL_memory_free__double(&fp->Einp);
    ISCL_memory_free__double(&fp->Ninp);
    ISCL_memory_free__double(&fp->Uinp);
    ISCL_memory_free__int(&fp->fault_ptr);
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
void core_ff_finalize__ffResults(struct GFAST_ffResults_struct *ff)
{
    int ifp;
    if (ff == NULL){return;}
    if (ff->nfp < 1){return;}
    for (ifp=0; ifp<ff->nfp; ifp++)
    {
        core_ff_finalize__faultPlane(&ff->fp[ifp]);
    }
    ISCL_memory_free(ff->fp);
    ISCL_memory_free__double(&ff->vr);
    ISCL_memory_free__double(&ff->Mw);
    ISCL_memory_free__double(&ff->str);
    ISCL_memory_free__double(&ff->dip);
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
void core_ff_finalize__offsetData(struct GFAST_offsetData_struct *offset_data)
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
    ISCL_memory_free__double(&offset_data->ubuff);
    ISCL_memory_free__double(&offset_data->nbuff);
    ISCL_memory_free__double(&offset_data->ebuff);
    ISCL_memory_free__double(&offset_data->wtu);
    ISCL_memory_free__double(&offset_data->wtn);
    ISCL_memory_free__double(&offset_data->wte);
    ISCL_memory_free__double(&offset_data->sta_lat);
    ISCL_memory_free__double(&offset_data->sta_lon);
    ISCL_memory_free__double(&offset_data->sta_alt);
    ISCL_memory_free__bool(&offset_data->lmask);
    ISCL_memory_free__bool(&offset_data->lactive);
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
    core_properties_finalize__ffProperties(ff_props);
    core_ff_finalize__offsetData(offset_data);
    core_ff_finalize__ffResults(ff);
    return;
}

