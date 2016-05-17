#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief Allocates space for the CMT grid search 
 *
 * @param[in] props      holds the CMT parameters
 * @param[in] gps_data   holds the site stream length
 * @param[out] cmt       has space allocated for the CMT depth gridsearch
 * @param[out] cmt_data  has space sufficient to carry the offset data
 *                       and weights.  also, contains an initial data
 *                       mask.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_CMT__init(struct GFAST_props_struct props,
                    struct GFAST_data_struct gps_data,
                    struct GFAST_cmtResults_struct *cmt,
                    struct GFAST_offsetData_struct *cmt_data)
{
    const char *fcnm = "GFAST_CMT__init\0";
    int i;
    cmt->ndeps = props.cmt_ngridSearch_deps;
    if (cmt->ndeps < 1)
    {
        log_errorF("%s: No depths in CMT grid search %d\n",
                   fcnm, props.cmt_ngridSearch_deps);
        return -1;
    }
    if (gps_data.stream_length < 1)
    {
        log_errorF("%s: Error insufficient data to estimate CMT %d\n",
                   fcnm, gps_data.stream_length);
    }
    // data
    cmt_data->stnm = (char **)calloc(gps_data.stream_length, sizeof(char *));
    cmt_data->ubuff = GFAST_memory_calloc__double(gps_data.stream_length);
    cmt_data->nbuff = GFAST_memory_calloc__double(gps_data.stream_length); 
    cmt_data->ebuff = GFAST_memory_calloc__double(gps_data.stream_length);
    cmt_data->wtu   = GFAST_memory_calloc__double(gps_data.stream_length);
    cmt_data->wtn   = GFAST_memory_calloc__double(gps_data.stream_length); 
    cmt_data->wte   = GFAST_memory_calloc__double(gps_data.stream_length);
    cmt_data->lmask   = GFAST_memory_calloc__bool(gps_data.stream_length);
    cmt_data->lactive = GFAST_memory_calloc__bool(gps_data.stream_length);
    cmt_data->nsites = gps_data.stream_length;
    for (i=0; i<cmt_data->nsites; i++)
    {
        cmt_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(cmt_data->stnm[i], gps_data.data[i].site);
        if (gps_data.data[i].lskip_cmt){cmt_data->lmask[i] = true;}
    }
    // cmt structure
    cmt->objfn     = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->mts       = GFAST_memory_calloc__double(6*cmt->ndeps);
    cmt->str1      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->str2      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->dip1      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->dip2      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->rak1      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->rak2      = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->Mw        = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->srcDepths = GFAST_memory_calloc__double(cmt->ndeps);
    /* TODO fix me */
    for (i=0; i<cmt->ndeps; i++)
    {
        cmt->srcDepths[i] = (double) (i + 1);
    }
    return 0;
}
