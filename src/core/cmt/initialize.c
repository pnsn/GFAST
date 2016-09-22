#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Allocates space for the CMT grid search 
 *
 * @param[in] props      holds the CMT parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] cmt       has space allocated for the CMT depth gridsearch
 * @param[out] cmt_data  has space sufficient to carry the offset data
 *                       and weights.  also, contains an initial data
 *                       mask.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @bug Can't accept arbitrary source depths from input file
 *
 */
int core_cmt_initialize(struct GFAST_cmt_props_struct props,
                        struct GFAST_data_struct gps_data,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_offsetData_struct *cmt_data)
{
    const char *fcnm = "core_cmt_initialize\0";
    int i;
    //------------------------------------------------------------------------//
    cmt->ndeps = props.ngridSearch_deps;
    if (cmt->ndeps < 1)
    {
        log_errorF("%s: No depths in CMT grid search %d\n",
                   fcnm, props.ngridSearch_deps);
        return -1;
    }
    cmt->nsites = gps_data.stream_length;
    if (gps_data.stream_length < 1)
    {
        log_errorF("%s: Error insufficient data to estimate CMT %d\n",
                   fcnm, gps_data.stream_length);
    }
    // data
    cmt_data->stnm = (char **)calloc(gps_data.stream_length, sizeof(char *));
    cmt_data->ubuff   = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->nbuff   = ISCL_memory_calloc__double(gps_data.stream_length); 
    cmt_data->ebuff   = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->wtu     = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->wtn     = ISCL_memory_calloc__double(gps_data.stream_length); 
    cmt_data->wte     = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->sta_lat = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->sta_lon = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->sta_alt = ISCL_memory_calloc__double(gps_data.stream_length);
    cmt_data->lmask   = ISCL_memory_calloc__bool(gps_data.stream_length);
    cmt_data->lactive = ISCL_memory_calloc__bool(gps_data.stream_length);
    cmt_data->nsites = gps_data.stream_length;
    for (i=0; i<cmt_data->nsites; i++)
    {
        cmt_data->sta_lat[i] = gps_data.data[i].sta_lat;
        cmt_data->sta_lon[i] = gps_data.data[i].sta_lon;
        cmt_data->sta_alt[i] = gps_data.data[i].sta_alt;
        cmt_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(cmt_data->stnm[i], gps_data.data[i].netw);
        strcat(cmt_data->stnm[i], ".\0");
        strcat(cmt_data->stnm[i], gps_data.data[i].stnm);
        strcat(cmt_data->stnm[i], ".\0");
        strncpy(cmt_data->stnm[i], gps_data.data[i].chan[0], 2);
        strcat(cmt_data->stnm[i], "?.\0");
        if (strlen(gps_data.data[i].loc) > 0)
        {
            strcat(cmt_data->stnm[i], gps_data.data[i].loc);
        } 
        if (gps_data.data[i].lskip_cmt){cmt_data->lmask[i] = true;}
    }
    // cmt structure
    cmt->l2        = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->pct_dc    = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->objfn     = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->mts       = ISCL_memory_calloc__double(6*cmt->ndeps);
    cmt->str1      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->str2      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->dip1      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->dip2      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->rak1      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->rak2      = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->Mw        = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->srcDepths = ISCL_memory_calloc__double(cmt->ndeps);
    cmt->EN        = ISCL_memory_calloc__double(cmt->ndeps*cmt->nsites);
    cmt->NN        = ISCL_memory_calloc__double(cmt->ndeps*cmt->nsites);
    cmt->UN        = ISCL_memory_calloc__double(cmt->ndeps*cmt->nsites);
    cmt->Einp      = ISCL_memory_calloc__double(cmt->nsites);
    cmt->Ninp      = ISCL_memory_calloc__double(cmt->nsites);
    cmt->Uinp      = ISCL_memory_calloc__double(cmt->nsites);
    cmt->lsiteUsed = ISCL_memory_calloc__bool(cmt->nsites);
    /* TODO fix me */
    for (i=0; i<cmt->ndeps; i++)
    {
        cmt->srcDepths[i] = (double) (i + 1);
    }
    return 0;
}
