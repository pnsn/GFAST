#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "gfast_eewUtils.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Driver for estimating earthquake magnitude from peak
 *        ground displacement
 *
 * @param[in] pgd_props  PGD inversion parameters
 * @param[in] SA_lat     event latitude (degrees) [-90,90]
 * @param[in] SA_lon     event longitude (degrees) [0,360]
 * @param[in] SA_dep     event depth (km)
 * @param[in] pgd_data   data structure holding the peak ground displacement
 *                       data, weights, and logical mask indicating site is
 *                       a candidate for inversion
 *
 * @param[out] pgd       results of the PGD estimation grid-search, 
 *                       the variance reduction at each depth in the grid
 *                       search, and the sites used in the inversion
 *
 * @result 0 indicates success
 *         1 indicates an error on the input pgd structure
 *         2 indicates error in input pgd_data structure
 *         3 indicates insufficient data for inversion
 *         4 indicates an error in computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */ 
int eewUtils_drivePGD(const struct GFAST_pgd_props_struct pgd_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_peakDisplacementData_struct pgd_data,
                      struct GFAST_pgdResults_struct *pgd)
{
    const char *fcnm = "eewUtils_drivePGD\0";
    double *d, *srdist, *staAlt, *Uest, *utmRecvEasting, *utmRecvNorthing, *wts,
           iqrMin, utmSrcEasting, utmSrcNorthing, x1, x2, y1, y2;
    int i, idep, ierr, j, k, l1, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = PGD_SUCCESS;
    d = NULL;
    wts = NULL;
    utmRecvNorthing = NULL;
    utmRecvEasting = NULL;
    staAlt = NULL;
    Uest = NULL;
    srdist = NULL;
    luse = NULL;
    // Verify the input data structure makes sense
    if (pgd_data.nsites < 1)
    {
        ierr = PGD_PD_DATA_ERROR;
        if (pgd_props.verbose > 1)
        {
            log_warnF("%s: No peak displacement data\n", fcnm);
        }
        goto ERROR;
    }
    if (pgd_data.pd == NULL || pgd_data.wt == NULL ||
        pgd_data.lmask == NULL || pgd_data.lactive == NULL)
    {
        ierr = PGD_PD_DATA_ERROR;
        if (pgd_data.pd == NULL)
        {
            log_errorF("%s: pgd_data.pd is NULL\n", fcnm);
        }
        if (pgd_data.wt == NULL)
        {
            log_errorF("%s: pgd_data.wt is NULL\n", fcnm);
        }
        if (pgd_data.lactive == NULL)
        {
            log_errorF("%s: pgd_data.lactive is NULL\n", fcnm);
        }
        if (pgd_data.lmask == NULL)
        {
            log_errorF("%s: pgd_data.lmask is NULL\n", fcnm);
        }
        goto ERROR;
    }
    // Verify the output data structures 
    if (pgd->ndeps < 1)
    {
        log_errorF("%s: No depths in PGD gridsearch!\n", fcnm);
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    if (pgd->mpgd == NULL || pgd->mpgd_vr == NULL ||
        pgd->srcDepths == NULL || pgd->UP == NULL ||
        pgd->UPinp == NULL || pgd->srdist == NULL ||
        pgd->lsiteUsed == NULL)
    {
        if (pgd->mpgd == NULL)
        {
            log_errorF("%s: pgd->mpgd is NULL\n", fcnm);
        }
        if (pgd->mpgd_vr == NULL)
        {
            log_errorF("%s: pgd->mpgd_vr is NULL\n", fcnm);
        }
        if (pgd->srcDepths == NULL)
        {
            log_errorF("%s: pgd->srcDepths is NULL\n", fcnm);
        }
        if (pgd->UP == NULL)
        {
            log_errorF("%s: pgd->UP is NULL\n", fcnm);
        }
        if (pgd->UPinp == NULL)
        {
            log_errorF("%s: pgd->UPinp is NULL\n", fcnm);
        }
        if (pgd->srdist == NULL)
        {
            log_errorF("%s: pgd->srdist is NULL\n", fcnm);
        }
        if (pgd->lsiteUsed == NULL)
        {
            log_errorF("%s: pgd->lsiteUsed is NULL\n", fcnm);
        }
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    // Avoid a segfault
    if (pgd->nsites != pgd_data.nsites)
    {
        log_errorF("%s: nsites on pgd and pgd_data differs %d %d\n",
                   fcnm, pgd->nsites, pgd_data.nsites);
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    // Warn in case hypocenter is outside of grid-search
    if (pgd_props.verbose > 1 &&
        (SA_dep < pgd->srcDepths[0] || SA_dep > pgd->srcDepths[pgd->ndeps-1]))
    {
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }
    // Null out results
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (k=0; k<pgd->nsites; k++)
    {
        pgd->UPinp[k] = 0.0;
        pgd->lsiteUsed[k] = true;
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<pgd->ndeps; i++)
    {
        pgd->mpgd[i] = 0.0;
        pgd->mpgd_vr[i] = 0.0;
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<pgd->ndeps*pgd->nsites; i++)
    {
        pgd->UP[i] = 0.0;
    }
    // Require there is a sufficient amount of data to invert
    luse = memory_calloc8l(pgd_data.nsites);
    l1 = 0;
    for (k=0; k<pgd_data.nsites; k++)
    {
        if (!pgd_data.lactive[k] || pgd_data.wt[k] <= 0.0){continue;}
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < pgd_props.min_sites)
    {
        if (pgd_props.verbose > 1)
        {
            log_warnF("%s: Insufficient data to invert %d < %d\n",
                      fcnm, l1, pgd_props.min_sites);
        }
        ierr = PGD_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Allocate space
    d               = memory_calloc64f(l1);
    utmRecvNorthing = memory_calloc64f(l1);
    utmRecvEasting  = memory_calloc64f(l1);
    staAlt          = memory_calloc64f(l1);
    wts             = memory_calloc64f(l1);
    Uest            = memory_calloc64f(l1*pgd->ndeps);
    srdist          = memory_calloc64f(l1*pgd->ndeps);
    // Get the source location
    zone_loc = pgd_props.utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Estimate UTM zone from source lon
    core_coordtools_ll2utm(SA_lat, SA_lon,
                           &y1, &x1,
                           &lnorthp, &zone_loc);
    utmSrcNorthing = y1; 
    utmSrcEasting = x1;
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k=0; k<pgd_data.nsites; k++)
    {   
        if (!pgd_data.lactive[k] || pgd_data.wt[k] <= 0.0){continue;}
        // Get the recevier UTM
        core_coordtools_ll2utm(pgd_data.sta_lat[k],
                               pgd_data.sta_lon[k],
                               &y2, &x2,
                               &lnorthp, &zone_loc);
        // Copy information to data structures for grid search
        d[l1] = pgd_data.pd[k]*100.0; // convert peak ground displacement to cm
        wts[l1] = pgd_data.wt[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = pgd_data.sta_alt[k];
        l1 = l1 + 1;
    } // Loop on data
    // Invert!
    if (pgd_props.verbose > 2)
    {   
        log_debugF("%s: Inverting for PGD with %d sites\n", fcnm, l1);
    }   
    ierr = core_scaling_pgd_depthGridSearch(l1, pgd->ndeps,
                                            pgd_props.verbose,
                                            pgd_props.dist_tol,
                                            pgd_props.disp_def,
                                            utmSrcEasting,
                                            utmSrcNorthing,
                                            pgd->srcDepths,
                                            utmRecvEasting,
                                            utmRecvNorthing,
                                            staAlt,
                                            d,
                                            wts,
                                            srdist,
                                            pgd->mpgd,
                                            pgd->mpgd_vr,
                                            pgd->iqr,
                                            Uest);
    if (ierr != 0)
    {   
        if (pgd_props.verbose > 0)
        {
            log_errorF("%s: Error in PGD grid search!\n", fcnm);
        }
        ierr = PGD_COMPUTE_ERROR;
    }
    // Extract observations
    k = 0;
    for (i=0; i<pgd->nsites; i++)
    {
        pgd->lsiteUsed[i] = luse[i];
        if (!luse[i]){continue;}
        pgd->UPinp[i] = d[k];
        k = k + 1;
    }
    iqrMin = array_min64f(pgd->ndeps, pgd->iqr, &ierr);
    // Extract the estimates and compute weighted objective function
    for (idep=0; idep<pgd->ndeps; idep++)
    {
        pgd->dep_vr_pgd[idep] = pgd->mpgd[idep]*iqrMin/pgd->iqr[idep];
        j = 0;
        for (i=0; i<pgd->nsites; i++)
        {
            pgd->UP[idep*pgd->nsites+i] = 0.0;
            pgd->srdist[idep*pgd->nsites+i] = 0.0;
            if (luse[i])
            {
                pgd->UP[idep*pgd->nsites+i] = Uest[idep*l1+j];
                pgd->srdist[idep*pgd->nsites+i] = srdist[idep*l1+j];
                j = j + 1;
            }
        }
    }
ERROR:;
    memory_free64f(&d);
    memory_free64f(&utmRecvNorthing);
    memory_free64f(&utmRecvEasting);
    memory_free64f(&staAlt);
    memory_free64f(&wts);
    memory_free64f(&Uest);
    memory_free64f(&srdist);
    memory_free8l(&luse);
    return ierr;
}

