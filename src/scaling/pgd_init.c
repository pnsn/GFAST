#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h> 
#include "gfast.h"
/*!
 * @brief Allocates space for the PGD grid search
 *
 * @param[in] props      holds the PGD parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] pgd       has space allocated for the PGD depth gridsearch
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI) 
 */
int GFAST_scaling_PGD__init(struct GFAST_props_struct props,
                            struct GFAST_data_struct gps_data,
                            struct GFAST_pgdResults_struct *pgd)
{
    const char *fcnm = "GFAST_scaling_PGD__init\0";
    int i;
    pgd->ndeps = props.pgd_ngridSearch_deps;
    if (pgd->ndeps < 1){
        log_errorF("%s: No depths in PGD grid search\n", fcnm);
        return -1;
    }
    pgd->nsites = gps_data.stream_length;
    if (pgd->nsites < props.pgd_min_sites){
        log_errorF("%s: Insufficient number of sites to compute PGD!\n", fcnm);
        return -1;
    }
    pgd->mpgd = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->mpgd_vr = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->srcDepths = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->lsiteUsed = GFAST_memory_calloc__bool(pgd->nsites);
    // TODO: fix me and make customizable!
    for (i=0; i<pgd->ndeps; i++){
        pgd->srcDepths[i] = (double) i + 1;
    }
    return 0;
}
