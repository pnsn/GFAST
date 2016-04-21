#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "gfast.h"
/*!
 * @brief Allocates space for the CMT grid search 
 *
 * @param[in] props      holds the CMT parameters
 * @param[in] gps_data   holds the site stream length
 * @param[out] cmt       has space allocated for the CMT depth gridsearch
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_CMT__init(struct GFAST_props_struct props,
                    struct GFAST_data_struct gps_data,
                    struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_CMT__init\0";
    int i;
    cmt->ndeps = props.cmt_ngridSearch_deps;
    if (cmt->ndeps < 1){
        log_errorF("%s: No depths in CMT grid search %d\n",
                   fcnm, props.cmt_ngridSearch_deps);
        return -1;
    }
    cmt->cmt_vr    = GFAST_memory_calloc__double(cmt->ndeps);
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
    for (i=0; i<cmt->ndeps; i++){
        cmt->srcDepths[i] = (double) (i + 1);
    }
    return 0;
}
