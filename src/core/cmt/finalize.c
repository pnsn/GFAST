#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Free the CMT results structure
 *
 * @param[in,out] cmt       CMT results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void core_cmt_finalize__cmtResults(struct GFAST_cmtResults_struct *cmt)
{
    if (cmt == NULL){return;}
    ISCL_memory_free__double(&cmt->l2);
    ISCL_memory_free__double(&cmt->pct_dc);
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
 * @brief Frees memory associated with the offset data 
 *
 * @param[in,out] offset_data     offset data structure with memory to be freed
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_cmt_finalize__offsetData(struct GFAST_offsetData_struct *offset_data)
{
    GFAST_core_ff_finalize__offsetData(offset_data);
    return;
}
//============================================================================//
/*!
 * @brief Convenience function for freeing the CMT data structures and data
 *
 * @param[out] cmt_props      nulled out CMT properties
 * @param[out] offset_data    offset data structure with memory to be freed
 * @param[out] cmt            cmt results structure with memory to be freed
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_cmt_finalize(struct GFAST_cmt_props_struct *cmt_props,
                       struct GFAST_offsetData_struct *offset_data,
                       struct GFAST_cmtResults_struct *cmt)
{
    core_properties_finalize__cmtProperties(cmt_props);
    core_cmt_finalize__offsetData(offset_data);
    core_cmt_finalize__cmtResults(cmt);
    return;
}

