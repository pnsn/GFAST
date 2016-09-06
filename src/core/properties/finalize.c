#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"

//============================================================================//
/*!
 * @brief Frees/nulls ou thte GFAST PGD properties structure
 *
 * @param[out] pgd_props   the freed/nulled out GFAST PGD properties structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_properties_finalize__pgdProperties(
   struct GFAST_pgd_props_struct *pgd_props)
{
    memset(pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees/nulls out the GFAST CMT properties structure
 *
 * @param[out] cmt_props   the freed/nulled out GFAST CMT properties structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_properties_finalize__cmtProperties(
   struct GFAST_cmt_props_struct *cmt_props)
{
    memset(cmt_props, 0, sizeof(struct GFAST_cmt_props_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees/nulls out the GFAST finite fault properties
 *
 * @param[out] ff_props  the freed/nulled out GFAST finite fault properties
 *                       structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_properties_finalize__ffProperties(
    struct GFAST_ff_props_struct *ff_props)
{
    memset(ff_props, 0, sizeof(struct GFAST_ff_props_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees/nulls out the GFAST activeMQ properties
 *
 * @param[out] activeMQ_props    the freed/nulled out GFAST activeMQ properties
 *                               structure
 *
 * @author Ben Baker (ISTI)
 *
 */ 
void core_properties_finalize__activeMQProperties(
    struct GFAST_activeMQ_struct *activeMQ_props)
{
    memset(activeMQ_props, 0, sizeof(struct GFAST_activeMQ_struct));
    return; 
}
//============================================================================//
/*!
 * @brief Frees/nulls out the GFAST properties structure
 *
 * @param[out] props     the freed/nulled out GFAST properties structure
 *
 * @author Ben Baker (ISTI)
 *
 */
void core_properties_finalize(struct GFAST_props_struct *props)
{
    GFAST_core_properties_finalize__pgdProperties(&props->pgd_props);
    GFAST_core_properties_finalize__cmtProperties(&props->cmt_props);
    GFAST_core_properties_finalize__ffProperties(&props->ff_props);
    GFAST_core_properties_finalize__activeMQProperties(&props->activeMQ_props);
    memset(props, 0, sizeof(struct GFAST_props_struct)); 
    return;
}
