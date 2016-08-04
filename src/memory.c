#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Frees memory associated with peak ground displacement
 *
 * @param[inout] pgd_data     peak ground displacement data structure with
 *                            memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freePGDData(
   struct GFAST_peakDisplacementData_struct *pgd_data)
{
    int i;
    if (pgd_data->stnm != NULL)
    {   
        for (i=0; i<pgd_data->nsites; i++)
        {
            free(pgd_data->stnm[i]);
        }
        free(pgd_data->stnm);
    }
    ISCL_memory_free__double(&pgd_data->pd);
    ISCL_memory_free__double(&pgd_data->wt);
    ISCL_memory_free__double(&pgd_data->sta_lat);
    ISCL_memory_free__double(&pgd_data->sta_lon);
    ISCL_memory_free__double(&pgd_data->sta_alt);
    ISCL_memory_free__bool(&pgd_data->lmask);
    ISCL_memory_free__bool(&pgd_data->lactive);
    memset(pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with the offset data 
 *
 * @param[inout] offset_data     offset data structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeOffsetData(struct GFAST_offsetData_struct *offset_data)
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
 * @brief Frees memory associated with waveform data on a site 
 *
 * @param[inout] data     collocated data structure with memory to be freed 
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeWaveformData(struct GFAST_waveformData_struct *data)
{
    if (data == NULL){return;}
    ISCL_memory_free__double(&data->ubuff);
    ISCL_memory_free__double(&data->nbuff);
    ISCL_memory_free__double(&data->ebuff);
    ISCL_memory_free__double(&data->tbuff);
    memset(data, 0, sizeof(struct GFAST_waveformData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with GPS data structure
 *
 * @param[inout] gps_data    GPS data structure with memory to be freed
 * 
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data)
{
    int k;
    if (gps_data == NULL){return;}
    if (gps_data->stream_length > 0 && gps_data->data != NULL)
    {
        for (k=0; k<gps_data->stream_length; k++)
        {
            GFAST_memory_freeWaveformData(&gps_data->data[k]);
        }
        ISCL_memory_free(gps_data->data);
    }
    memset(gps_data, 0, sizeof(struct GFAST_data_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the PGD results structure
 *
 * @param[inout] pgd       PGD results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freePGDResults(struct GFAST_pgdResults_struct *pgd)
{
    if (pgd == NULL){return;}
    ISCL_memory_free__double(&pgd->mpgd);
    ISCL_memory_free__double(&pgd->mpgd_vr);
    ISCL_memory_free__double(&pgd->iqr75_25);
    ISCL_memory_free__double(&pgd->UP);
    ISCL_memory_free__double(&pgd->UPinp);
    ISCL_memory_free__double(&pgd->srcDepths);
    ISCL_memory_free(pgd->lsiteUsed);
    memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the CMT results structure
 *
 * @param[inout] cmt       CMT results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeCMTResults(struct GFAST_cmtResults_struct *cmt)
{
    if (cmt == NULL){return;}
    ISCL_memory_free__double(&cmt->objfn);
    ISCL_memory_free__double(&cmt->mts);
    ISCL_memory_free__double(&cmt->str1); 
    ISCL_memory_free__double(&cmt->str2);
    ISCL_memory_free__double(&cmt->dip1); 
    ISCL_memory_free__double(&cmt->dip2);
    ISCL_memory_free__double(&cmt->rak1); 
    ISCL_memory_free__double(&cmt->rak2);
    ISCL_memory_free__double(&cmt->Mw);
    ISCL_memory_free__double(&cmt->srcDepths);
    ISCL_memory_free__double(&cmt->EN);
    ISCL_memory_free__double(&cmt->NN);
    ISCL_memory_free__double(&cmt->UN);
    ISCL_memory_free__double(&cmt->Einp);
    ISCL_memory_free__double(&cmt->Ninp);
    ISCL_memory_free__double(&cmt->Uinp);
    ISCL_memory_free__bool(&cmt->lsiteUsed);
    memset(cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the fault plane structure
 *
 * @param[inout] fp       fault plane structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeFaultPlane(struct GFAST_faultPlane_struct *fp)
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
 * @param[inout] ff      finite fault results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeFFResults(struct GFAST_ffResults_struct *ff)
{
    int ifp;
    if (ff == NULL){return;}
    if (ff->nfp < 1){return;}
    for (ifp=0; ifp<ff->nfp; ifp++){
        GFAST_memory_freeFaultPlane(&ff->fp[ifp]);
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
 * @brief Frees memory on the active events structure
 *
 * @param[inout] events     active event list with data to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeEvents(struct GFAST_activeEvents_struct *events)
{
    if (events->nev > 0){
        ISCL_memory_free(events->SA);
    }
    memset(events, 0, sizeof(struct GFAST_activeEvents_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with parameter structure
 *
 * @param[inout] props     GFAST properties structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeProps(struct GFAST_props_struct *props)
{
    memset(props, 0, sizeof(struct GFAST_props_struct));
    return;
}
