#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"
/*!
 * @brief Releases the memory on the PGD data structures
 *
 * @param[out] pgd_props   nulled out PGD properties structure
 * @param[out] pgd_data    freed PGD data data structure
 * @param[out] pgd         freed PGD results data structure
 *
 * @author Ben Baker (ISTI)
 *
 */ 
void core_scaling_pgd_finalize(
    struct GFAST_pgd_props_struct *pgd_props,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct GFAST_pgdResults_struct *pgd)
{
    int i;
    // Destroy the input parameters
    memset(pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));
    // Free the PGD data data
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
    // Free the results data structure
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
