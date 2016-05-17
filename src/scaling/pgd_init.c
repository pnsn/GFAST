#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief Allocates space for the PGD grid search
 *
 * @param[in] pgd_props  holds the PGD parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] pgd       has space allocated for the PGD depth gridsearch
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI) 
 */
int GFAST_scaling_PGD__init(struct GFAST_pgd_props_struct pgd_props,
                            struct GFAST_data_struct gps_data,
                            struct GFAST_pgdResults_struct *pgd,
                            struct GFAST_peakDisplacementData_struct *pgd_data)
{
    const char *fcnm = "GFAST_scaling_PGD__init\0";
    int i;
    pgd->ndeps = pgd_props.ngridSearch_deps;
    if (pgd->ndeps < 1)
    {
        log_errorF("%s: No depths in PGD grid search\n", fcnm);
        return -1;
    }
    pgd->nsites = gps_data.stream_length;
    if (pgd->nsites < pgd_props.min_sites)
    {
        log_errorF("%s: Insufficient number of sites to compute PGD!\n", fcnm);
        return -1;
    }
    // data
    pgd_data->stnm = (char **)calloc(gps_data.stream_length, sizeof(char *));
    pgd_data->pd = GFAST_memory_calloc__double(gps_data.stream_length);
    pgd_data->wt = GFAST_memory_calloc__double(gps_data.stream_length);
    pgd_data->lmask   = GFAST_memory_calloc__bool(gps_data.stream_length);
    pgd_data->lactive = GFAST_memory_calloc__bool(gps_data.stream_length);
    pgd_data->nsites = gps_data.stream_length;
    for (i=0; i<pgd_data->nsites; i++)
    {
        pgd_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(pgd_data->stnm[i], gps_data.data[i].site);
        if (gps_data.data[i].lskip_pgd){pgd_data->lmask[i] = true;}
    }
    pgd->mpgd = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->mpgd_vr = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->srcDepths = GFAST_memory_calloc__double(pgd->ndeps);
    pgd->lsiteUsed = GFAST_memory_calloc__bool(pgd->nsites);
    // TODO: fix me and make customizable!
    for (i=0; i<pgd->ndeps; i++)
    {
        pgd->srcDepths[i] = (double) i + 1;
    }
    return 0;
}
