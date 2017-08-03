#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

int pgd_inversion_test(void);
int pgd_inversion_test2(void);

static bool lequal(double a, double b, double tol)
{
    if (a == 0.0 && b == 0.0){return true;}
    if (fabs(a - b)/fabs(a + b) > tol){return false;}
    return true;
}

int pgd_inversion_test(void)
{
    const char *fcnm = "pgd_inversion_test\0";
    int verbose = 4;
    double dist_tol = 6.0;
    double disp_def = 0.01;
    double IQR, M, VR;
    double SA_lat = 47.19;
    double SA_lon =-122.66;
    double SA_dep = 57.0;
    // stations: bamf, cabl, chzz, eliz, holb, neah, p058, p159
    int l1 = 9;
    double wts[9];
    double stla[9] = {  48.83532872,  42.83609887,  45.48651503,
                        49.87305293,  50.64035267,  41.90232489,
                        48.29785467,  40.87630594,  40.50478709};
    double stlo[9] = {-125.13510527,-124.56334629,-123.97812400,
                      -127.12266484,-128.13499899,-120.30283244,
                      -124.62490719,-124.07537043,-124.28278289};
    double staAlt[9] = {0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0};
    double utmRecvNorthing[9], utmRecvEasting[9], Uest[9*1];
    double d[9] = {4, 7.1, 8,  3, 1, 9,  6, 5.5, 6.5};
    double srdist[9];
    double SA_xutm, SA_yutm, utmSrcEasting, utmSrcNorthing, xutm, yutm;
    int zone = 10;
    const int ndeps = 1;
    bool lnorth;
    int i, ierr;
    ierr = 0;
    // Compute source location
    GFAST_core_coordtools_ll2utm(SA_lat, SA_lon,
                                 &SA_yutm, &SA_xutm,
                                 &lnorth, &zone);
    utmSrcNorthing = SA_yutm;
    utmSrcEasting = SA_xutm;
    // Compute station locations 
    for (i=0; i<l1; i++)
    {
        GFAST_core_coordtools_ll2utm(stla[i], stlo[i],
                                     &yutm, &xutm,
                                     &lnorth, &zone);
        utmRecvNorthing[i] = yutm;
        utmRecvEasting[i] = xutm;
        wts[i] = 1.0;
    }
    // Compute scaling
    ierr = GFAST_core_scaling_pgd_depthGridSearch(l1, ndeps,
                                              verbose,
                                              dist_tol,
                                              disp_def,
                                              utmSrcEasting,
                                              utmSrcNorthing,
                                              &SA_dep,
                                              utmRecvEasting,
                                              utmRecvNorthing,
                                              staAlt,
                                              d, wts, srdist,
                                              &M, &VR, &IQR, Uest);
    if (ierr != 0)
    {
        log_errorF("%s: Error computing scaling\n", fcnm);
        return -1;
    }
    if (fabs(M - 6.6379784) > 1.e-4)
    {
        printf("%s: Failed to compute M %f\n", fcnm, M);
        return -1;
    } 
/*
    if (fabs(VR - 6.1681204583368228) > 1.e-6){
        printf("%s: Failed to compute VR\n", fcnm);
        return -1;
    }
*/
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}

static int read_results(const char *filenm,
                        struct GFAST_pgd_props_struct *pgd_props,
                        struct GFAST_peakDisplacementData_struct *pgd_data,
                        struct GFAST_pgdResults_struct *pgd,
                        double *SA_lat, double *SA_lon, double *SA_dep)
{
    FILE *infl;
    char cline[128];
    int i, ierr;
    //------------------------------------------------------------------------//
    ierr = 1;
    infl = fopen(filenm, "r");
    // line 1
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;} 
    sscanf(cline, "%d %d %d %d %lf %lf %lf\n",
           &pgd_data->nsites, &pgd_props->ngridSearch_deps,
           &pgd_props->utm_zone, &pgd_props->min_sites, 
           &pgd_props->dist_tol, &pgd_props->disp_def,
           &pgd_props->window_vel);
    // line 2
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf\n", SA_lat, SA_lon, SA_dep);
    // pgd data
    pgd_data->pd = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->wt = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_lat = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_lon = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_alt = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->lmask   = ISCL_memory_calloc__bool(pgd_data->nsites);
    pgd_data->lactive = ISCL_memory_calloc__bool(pgd_data->nsites);
    for (i=0; i<pgd_data->nsites; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf\n",
               &pgd_data->sta_lat[i], &pgd_data->sta_lon[i],
               &pgd_data->sta_alt[i], &pgd_data->pd[i]);
        pgd_data->wt[i] = 1.0;
        pgd_data->lactive[i] = true;
    }
    // Results + depths in grid search
    pgd->nsites = pgd_data->nsites;
    pgd->ndeps = pgd_props->ngridSearch_deps;
    pgd->nlats = 1;
    pgd->nlons = 1;
    pgd->mpgd    = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->mpgd_vr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->dep_vr_pgd = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->iqr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->UP = ISCL_memory_calloc__double(pgd->ndeps*pgd->nsites);
    pgd->UPinp = ISCL_memory_calloc__double(pgd->nsites);
    pgd->srcDepths = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->srdist = ISCL_memory_calloc__double(pgd->ndeps*pgd->nsites);
    pgd->lsiteUsed = ISCL_memory_calloc__bool(pgd->nsites);
    // (a) depths
    for (i=0; i<pgd->ndeps; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf\n", &pgd->srcDepths[i]);
    }
    // (b) results
    for (i=0; i<pgd->ndeps; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf\n",
               &pgd->mpgd[i], &pgd->mpgd_vr[i], &pgd->iqr[i]);
    }
    fclose(infl);
    ierr = 0;
ERROR:;
    return ierr;
}
//============================================================================//
/*!
 * @brief Benchmark against the final result in Brendan's python scripts
 */
int pgd_inversion_test2(void)
{
    const char *fcnm = "pgd_inversion_test2\0";
    const char *filenm = "files/final_pgd.maule.txt\0";
    struct GFAST_pgd_props_struct pgd_props;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd_ref, pgd;
    double SA_lat, SA_lon, SA_dep;
    const double tol = 1.e-4;
    int i, ierr;
    memset(&pgd_props, 0, sizeof(pgd_props));
    memset(&pgd_data, 0, sizeof(pgd_data));
    memset(&pgd_ref, 0, sizeof(pgd_ref));
    memset(&pgd, 0, sizeof(pgd));
    ierr = read_results(filenm,
                        &pgd_props,
                        &pgd_data,
                        &pgd_ref,
                        &SA_lat, &SA_lon, &SA_dep);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading input file\n", fcnm);
        return EXIT_FAILURE;
    }
    // Set space
    pgd.nsites = pgd_ref.nsites;
    pgd.ndeps = pgd_ref.ndeps;
    pgd.nlats = 1;
    pgd.nlons = 1;
    pgd.mpgd    = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.mpgd_vr = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.dep_vr_pgd = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.iqr = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.UP = ISCL_memory_calloc__double(pgd.ndeps*pgd.nsites);
    pgd.UPinp = ISCL_memory_calloc__double(pgd.nsites);
    pgd.srcDepths = ISCL_memory_calloc__double(pgd.ndeps);
    pgd.srdist = ISCL_memory_calloc__double(pgd.ndeps*pgd.nsites);
    pgd.lsiteUsed = ISCL_memory_calloc__bool(pgd.nsites);
    for (i=0; i<pgd.ndeps; i++)
    {
        pgd.srcDepths[i] = pgd_ref.srcDepths[i];
    }
    ierr = eewUtils_drivePGD(pgd_props,
                             SA_lat, SA_lon, SA_dep,
                             pgd_data, &pgd);
    if (ierr != PGD_SUCCESS)
    {
        log_errorF("%s: Error computing PGD!\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<pgd.ndeps; i++)
    {
        if (!lequal(pgd.mpgd[i], pgd_ref.mpgd[i], tol))
        {
            log_errorF("%s: Error mpgd is wrong %f %f %f\n", fcnm,
                       pgd.srcDepths[i], pgd.mpgd[i], pgd_ref.mpgd[i]);
            return EXIT_FAILURE;
        }
        if (!lequal(pgd.mpgd_vr[i], pgd_ref.mpgd_vr[i], tol))
        {
            log_errorF("%s: Error mpgd_vr is wrong %f %f %f\n", fcnm,
                       pgd.srcDepths[i], pgd.mpgd_vr[i], pgd_ref.mpgd_vr[i]);
            return EXIT_FAILURE;
        }
        if (!lequal(pgd.iqr[i], pgd_ref.iqr[i], tol))
        {
            log_errorF("%s: Error iqr is wrong %f %f %f\n", fcnm,
                       pgd.srcDepths[i], pgd.iqr[i],
                       pgd_ref.iqr[i]);
            return EXIT_FAILURE;
        }
    }
    // Clean up
    core_scaling_pgd_finalizeData(&pgd_data);
    core_scaling_pgd_finalizeResults(&pgd);
    core_scaling_pgd_finalizeResults(&pgd_ref);
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
