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
    int i, nloc;
    pgd->ndeps = pgd_props.ngridSearch_deps;
    pgd->nlats = pgd_props.ngridSearch_lats;
    pgd->nlons = pgd_props.ngridSearch_lons;
    if (pgd->ndeps < 1 || pgd->nlats < 1 || pgd->nlons < 1)
    {
        if (pgd->ndeps < 1)
        {
            log_errorF("%s: No depths in PGD grid search %d\n",
                       fcnm, pgd_props.ngridSearch_deps);
        }
        if (pgd->nlats < 1)
        {
            log_errorF("%s: No lats in PGD grid search %d\n",
                       fcnm, pgd_props.ngridSearch_lats);
        }
        if (pgd->nlats < 1)
        {
            log_errorF("%s: No lons in PGD grid search %d\n",
                       fcnm, pgd_props.ngridSearch_lons);
        }
        return -1;
    }
    pgd->nsites = gps_data.stream_length;
    if (pgd->nsites < pgd_props.min_sites)
    {
        log_errorF("%s: Insufficient number of sites to compute PGD!\n", fcnm);
        return -1;
    }
    // data
    pgd_data->stnm = (char **)calloc((size_t) gps_data.stream_length,
                                     sizeof(char *));
    pgd_data->pd      = memory_calloc64f(gps_data.stream_length);
    pgd_data->wt      = memory_calloc64f(gps_data.stream_length);
    pgd_data->sta_lat = memory_calloc64f(gps_data.stream_length);
    pgd_data->sta_lon = memory_calloc64f(gps_data.stream_length);
    pgd_data->sta_alt = memory_calloc64f(gps_data.stream_length);
    pgd_data->lmask   = memory_calloc8l(gps_data.stream_length);
    pgd_data->lactive = memory_calloc8l(gps_data.stream_length);
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
    nloc = pgd->ndeps*pgd->nlats*pgd->nlons;
    pgd->mpgd       = memory_calloc64f(nloc);
    pgd->mpgd_vr    = memory_calloc64f(nloc);
    pgd->dep_vr_pgd = memory_calloc64f(nloc);
    pgd->srcDepths  = memory_calloc64f(pgd->ndeps);
    pgd->iqr        = memory_calloc64f(nloc);
    pgd->UP         = memory_calloc64f(pgd->nsites*nloc);
    pgd->srdist     = memory_calloc64f(pgd->nsites*nloc);
    pgd->UPinp      = memory_calloc64f(pgd->nsites);
    pgd->lsiteUsed  = memory_calloc8l(pgd->nsites);
    // TODO: fix me and make customizable!
    for (i=0; i<pgd->ndeps; i++)
    {
        pgd->srcDepths[i] = (double) i + 1;
    }
    return 0;
}
