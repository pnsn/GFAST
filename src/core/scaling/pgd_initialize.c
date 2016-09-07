#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
/*!
 * @brief Allocates space for the PGD grid search and its data structure
 *
 * @param[in] pgd_props  holds the PGD parameters
 * @param[in] gps_data   holds the site stream length
 *
 * @param[out] pgd       has space allocated for the PGD depth gridsearch
 * @param[out] pgd_data  holds space for peak ground dispaclement data
 *                       as well as requisite site information
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI) 
 */
int core_scaling_pgd_initialize(struct GFAST_pgd_props_struct pgd_props,
                                struct GFAST_data_struct gps_data,
                                struct GFAST_pgdResults_struct *pgd,
                                struct GFAST_peakDisplacementData_struct *pgd_data)
{
    const char *fcnm = "core_scaling_pgd_initialize\0";
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
    pgd_data->pd = ISCL_memory_calloc__double(gps_data.stream_length);
    pgd_data->wt = ISCL_memory_calloc__double(gps_data.stream_length);
    pgd_data->sta_lat = ISCL_memory_calloc__double(gps_data.stream_length);
    pgd_data->sta_lon = ISCL_memory_calloc__double(gps_data.stream_length);
    pgd_data->sta_alt = ISCL_memory_calloc__double(gps_data.stream_length);
    pgd_data->lmask   = ISCL_memory_calloc__bool(gps_data.stream_length);
    pgd_data->lactive = ISCL_memory_calloc__bool(gps_data.stream_length);
    pgd_data->nsites = gps_data.stream_length;
    for (i=0; i<pgd_data->nsites; i++)
    {
        pgd_data->sta_lat[i] = gps_data.data[i].sta_lat;
        pgd_data->sta_lon[i] = gps_data.data[i].sta_lon; 
        pgd_data->sta_alt[i] = gps_data.data[i].sta_alt;
        pgd_data->stnm[i] = (char *)calloc(64, sizeof(char));
        strcpy(pgd_data->stnm[i], gps_data.data[i].netw);
        strcat(pgd_data->stnm[i], ".\0");
        strcat(pgd_data->stnm[i], gps_data.data[i].stnm);
        strcat(pgd_data->stnm[i], ".\0");
        strncpy(pgd_data->stnm[i], gps_data.data[i].chan[0], 2); 
        strcat(pgd_data->stnm[i], "?.\0");
        if (strlen(gps_data.data[i].loc) > 0)
        {
            strcat(pgd_data->stnm[i], gps_data.data[i].loc);
        }
        if (gps_data.data[i].lskip_pgd){pgd_data->lmask[i] = true;}
    }
    pgd->mpgd = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->mpgd_vr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->dep_vr_pgd = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->srcDepths = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->iqr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->UP = ISCL_memory_calloc__double(pgd->nsites*pgd->ndeps);
    pgd->srdist = ISCL_memory_calloc__double(pgd->nsites*pgd->ndeps);
    pgd->UPinp = ISCL_memory_calloc__double(pgd->nsites);
    pgd->lsiteUsed = ISCL_memory_calloc__bool(pgd->nsites);
    // TODO: fix me and make customizable!
    for (i=0; i<pgd->ndeps; i++)
    {
        pgd->srcDepths[i] = (double) i + 1;
    }
    return 0;
}
