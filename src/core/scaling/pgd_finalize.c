#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Releases the memory on the PGD data data structure
 *
 * @param[out] pgd_data    freed PGD data data structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_scaling_pgd_finalize__pgdData(
     struct GFAST_peakDisplacementData_struct *pgd_data)
{
    int i;
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
    return;
}
//============================================================================//
/*!
 * @brief Releases the memory on the PGD result structure
 *
 * @param[out] pgd         freed PGD results data structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_scaling_pgd_finalize__pgdResults(
    struct GFAST_pgdResults_struct *pgd)
{
    ISCL_memory_free__double(&pgd->mpgd);
    ISCL_memory_free__double(&pgd->mpgd_vr);
    ISCL_memory_free__double(&pgd->dep_vr_pgd);
    ISCL_memory_free__double(&pgd->iqr);
    ISCL_memory_free__double(&pgd->UP);
    ISCL_memory_free__double(&pgd->UPinp);
    ISCL_memory_free__double(&pgd->srcDepths);
    ISCL_memory_free__double(&pgd->srdist);
    ISCL_memory_free(pgd->lsiteUsed);
    memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    return;
}
//============================================================================//
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
    GFAST_core_properties_finalize__pgdProperties(pgd_props);
    GFAST_core_scaling_pgd_finalize__pgdData(pgd_data);
    GFAST_core_scaling_pgd_finalize__pgdResults(pgd);
    return;
}
