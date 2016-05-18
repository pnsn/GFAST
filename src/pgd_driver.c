#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "gfast.h"

#define DIST_MAX_NAN -(DBL_MAX + 1.0)

double __GFAST_getMaxDistance(int npts, bool lremove_disp0,
                              double dt, double SA_time, 
                              double epoch,
                              double *__restrict__ ubuff,
                              double *__restrict__ nbuff,
                              double *__restrict__ ebuff);

/*!
 * @brief Driver for estimating earthquake magnitude from peak
 *        ground displacement
 *
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
 * @bug The data weights are not yet applied to the data
 *
 */ 
int GFAST_scaling_PGD__driver(
   struct GFAST_pgd_props_struct pgd_props,
   double SA_lat, double SA_lon, double SA_dep,
   struct GFAST_peakDisplacementData_struct pgd_data,
   struct GFAST_pgdResults_struct *pgd)
{
    const char *fcnm = "GFAST_scaling_PGD__driver\0";
    enum pgd_return_enum
    {
        PGD_SUCCESS = 0,           /*!< PGD computation was successful */
        PGD_STRUCT_ERROR = 1,      /*!< PGD structure is invalid */
        PGD_PD_DATA_ERROR = 2,     /*!< PGD data structure invalid */
        PGD_INSUFFICIENT_DATA = 3, /*!< Insufficient data to invert */
        PGD_COMPUTE_ERROR = 4      /*!< A numerical error was encountered */
    };
    double *d, *repi, *staAlt, *utmRecvEasting, *utmRecvNorthing, *wts,
           epiDist, utmSrcEasting, utmSrcNorthing, x1, x2, y1, y2;
    int i, ierr, k, l1, zone_loc;
    bool lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = PGD_SUCCESS;
    d = NULL;
    wts = NULL;
    utmRecvNorthing = NULL;
    utmRecvEasting = NULL;
    staAlt = NULL;
    repi = NULL;
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
        pgd->srcDepths == NULL || pgd->lsiteUsed == NULL)
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
    #pragma omp simd
    for (k=0; k<pgd->nsites; k++)
    {
        pgd->lsiteUsed[k] = true;
    }
    #pragma omp simd
    for (i=0; i<pgd->ndeps; i++)
    {
        pgd->mpgd[i] = 0.0;
        pgd->mpgd_vr[i] = 0.0;
    }
    // Require there is a sufficient amount of data to invert
    l1 = 0;
    for (k=0; k<pgd_data.nsites; k++)
    {
        if (!pgd_data.lactive[k] || pgd_data.wt[k] == 0.0){continue;}
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
    d               = GFAST_memory_calloc__double(l1);
    utmRecvNorthing = GFAST_memory_calloc__double(l1);
    utmRecvEasting  = GFAST_memory_calloc__double(l1);
    staAlt          = GFAST_memory_calloc__double(l1);
    repi            = GFAST_memory_calloc__double(l1);
    wts             = GFAST_memory_calloc__double(l1);
    // Get the source location
    zone_loc = pgd_props.utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Estimate UTM zone from source lon
    GFAST_coordtools__ll2utm(SA_lat, SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    utmSrcNorthing = y1; 
    utmSrcEasting = x1;
    // Loop on the receivers, get distances, and data
    l1 = 0;
    for (k=0; k<pgd_data.nsites; k++)
    {   
        if (!pgd_data.lactive[k] || pgd_data.wt[k] == 0.0){continue;}
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(pgd_data.sta_lat[k],
                                 pgd_data.sta_lon[k],
                                 &y2, &x2,
                                 &lnorthp, &zone_loc);
        // If it isn't a NaN then retain it for processing
        d[l1] = pgd_data.pd[k]*100.0; // convert peak ground displacement to cm
        wts[l1] = pgd_data.wt[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        epiDist = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
        staAlt[l1] = pgd_data.sta_alt[k];
        repi[l1] = epiDist*1.e-3; // convert to kilometers 
        l1 = l1 + 1;
    } // Loop on data
    // Invert!
    if (pgd_props.verbose > 2)
    {   
        log_debugF("%s: Inverting for PGD with %d sites\n", fcnm, l1);
    }   
    ierr = GFAST_scaling_PGD__depthGridSearch(l1, pgd->ndeps,
                                              pgd_props.verbose,
                                              pgd_props.dist_tol,
                                              pgd_props.dist_def,
                                              utmSrcEasting,
                                              utmSrcNorthing,
                                              pgd->srcDepths,
                                              utmRecvEasting,
                                              utmRecvNorthing,
                                              staAlt,
                                              d,
                                              repi,
                                              wts,
                                              pgd->mpgd,
                                              pgd->mpgd_vr);
    if (ierr != 0)
    {   
        if (pgd_props.verbose > 0)
        {
            log_errorF("%s: Error in PGD grid search!\n", fcnm);
        }
        ierr = PGD_COMPUTE_ERROR;
    }
ERROR:;
    GFAST_memory_free__double(&d);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&repi);
    GFAST_memory_free__double(&wts);
    return ierr;
}
//============================================================================//
/*!
 * @brief Peak ground displacement estimation driver routine
 *
 * @param[in] pgd_props  holds properties for the PGD estimation
 * @param[in] SA         holds the hypocentral information (most importantly
 *                       the latitude, longitude, and event time)
 * @param[in] gps_data   holds the GPS data, site locations, and a logical
 *                       mask of whether or not to include a site in the
 *                       inversion 
 *
 * @param[out] pgd       results of the PGD estimation grid-search, 
 *                       the variance reduction at each depth in the grid
 *                       search, and the sites used in the inversion
 *
 * @result 0 indicates success
 *         1 indicates an error on the input PGD structure
 *         2 indicates insusfficient data with cause from input GPS data 
 *         3 indicates insufficient data for inversion
 *         4 indicates an error in computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_scaling_PGD__driver2(struct GFAST_pgd_props_struct pgd_props,
                               struct GFAST_shakeAlert_struct SA,
                               struct GFAST_data_struct gps_data,
                               struct GFAST_pgdResults_struct *pgd)
{
    const char *fcnm = "GFAST_scaling_PGD__driver2\0";
    enum pgd_return_enum
    {
        PGD_SUCCESS = 0,
        PGD_STRUCT_ERROR = 1,
        PGD_GPS_DATA_ERROR = 2,
        PGD_INSUFFICIENT_DATA = 3,
        PGD_COMPUTE_ERROR = 4
    };
    double *d, *repi, *staAlt, *utmRecvEasting, *utmRecvNorthing,
           *wts, *x2, *y2, currentTime, distMax, distance,
           effectiveHypoDist, epiDist,
           utmSrcEasting, utmSrcNorthing, x1, y1;
    int ierr, k, l1, nwork, zone_loc;
    bool lnorthp;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = PGD_SUCCESS;
    x2 = NULL;
    y2 = NULL;
    d = NULL;
    wts = NULL;
    utmRecvNorthing = NULL;
    utmRecvEasting = NULL;
    staAlt = NULL;
    repi = NULL;
    // Reality check
    if (gps_data.stream_length < 1)
    {
        if (pgd_props.verbose > 1)
        {
            ierr = PGD_GPS_DATA_ERROR;
            log_warnF("%s: No GPS streams\n", fcnm);
        }
        goto ERROR;
    }
    if (pgd->ndeps < 1){
        log_errorF("%s: No depths in PGD gridsearch!\n", fcnm);
        ierr = PGD_STRUCT_ERROR;
        goto ERROR;
    }
    // Count the data and get the workspace 
    l1 = 0;
    nwork = 0;
    for (k=0; k<gps_data.stream_length; k++)
    {
        if (gps_data.data[k].lskip_pgd){continue;}
        nwork = fmax(gps_data.data[k].npts, nwork);
        l1 = l1 + 1;
    }
    if (l1 < pgd_props.min_sites)
    {
        if (pgd_props.verbose > 1)
        {
            if (l1 < 1)
            {
                log_warnF("%s: All sites masked in PGD estimation\n", fcnm);
            }
            else
            {
                log_warnF("%s: Too many masked sites to compute PGD\n", fcnm);
            }
        }
        ierr = PGD_GPS_DATA_ERROR;
        goto ERROR;
    }
    if (nwork < 1)
    {
        if (pgd_props.verbose > 1){log_warnF("%s: There is no data\n", fcnm);}
        ierr = PGD_GPS_DATA_ERROR;
        goto ERROR;
    }
    // Initialize
    l1 = 0;
    x2              = GFAST_memory_calloc__double(gps_data.stream_length);
    y2              = GFAST_memory_calloc__double(gps_data.stream_length);
    d               = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvNorthing = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvEasting  = GFAST_memory_calloc__double(gps_data.stream_length);
    staAlt          = GFAST_memory_calloc__double(gps_data.stream_length);
    repi            = GFAST_memory_calloc__double(gps_data.stream_length);
    wts             = GFAST_memory_calloc__double(gps_data.stream_length);
    // Get the source location
    zone_loc = pgd_props.utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;}
    GFAST_coordtools__ll2utm(SA.lat, SA.lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    utmSrcNorthing = y1; 
    utmSrcEasting = x1;
    // Loop on the receivers, get distances, and data
    for (k=0; k<gps_data.stream_length; k++)
    {
        if (gps_data.data[k].lskip_pgd){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(gps_data.data[k].sta_lat,
                                 gps_data.data[k].sta_lon,
                                 &y2[k], &x2[k],
                                 &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up 
        distance = sqrt( pow(x1 - x2[k], 2)
                       + pow(y1 - y2[k], 2)
                       + pow(SA.dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // In a perfect world is there any chance of having data?
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA.time)*pgd_props.window_vel;
        if (distance < effectiveHypoDist)
        {
            // Get the maximum offset
            distMax = __GFAST_getMaxDistance(gps_data.data[k].npts,
                                             pgd_props.lremove_disp0,
                                             gps_data.data[k].dt,
                                             SA.time,
                                             gps_data.data[k].epoch,
                                             gps_data.data[k].ubuff,
                                             gps_data.data[k].nbuff,
                                             gps_data.data[k].ebuff);
            // If it isn't a NaN then retain it for processing
            if (distMax != NAN)
            {
                d[l1] = distMax*100.0; // convert to centimeters
                wts[l1] = 1.0;
                utmRecvNorthing[l1] = y2[k];
                utmRecvEasting[l1] = x2[k];
                epiDist = sqrt(pow(x1 - x2[k], 2) + pow(y1 - y2[k], 2));
                staAlt[l1] = gps_data.data[k].sta_alt;
                repi[l1] = epiDist*1.e-3; // convert to kilometers 
                l1 = l1 + 1;
            }
        }
    }
    // Is there enough data to invert?
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
    // Warn in case hypocenter is outside of grid-search
    if (SA.dep < pgd->srcDepths[0] || SA.dep > pgd->srcDepths[pgd->ndeps-1])
    {
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }
    // Invert!
    if (pgd_props.verbose > 2)
    {
        log_debugF("%s: Inverting for PGD with %d sites\n", fcnm, l1);
    }
    ierr = GFAST_scaling_PGD__depthGridSearch(l1, pgd->ndeps,
                                              pgd_props.verbose,
                                              pgd_props.dist_tol,
                                              pgd_props.dist_def,
                                              utmSrcEasting,
                                              utmSrcNorthing,
                                              pgd->srcDepths,
                                              utmRecvEasting,
                                              utmRecvNorthing,
                                              staAlt,
                                              d,
                                              repi,
                                              wts,
                                              pgd->mpgd,
                                              pgd->mpgd_vr);
    if (ierr != 0)
    {
        if (pgd_props.verbose > 0)
        {
            log_errorF("%s: Error in PGD grid search!\n", fcnm);
        }
        ierr = PGD_COMPUTE_ERROR;
    }
ERROR:;
    GFAST_memory_free__double(&x2);
    GFAST_memory_free__double(&y2);
    GFAST_memory_free__double(&d);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&repi);
    GFAST_memory_free__double(&wts);
    return 0;
}
//============================================================================//
/*!
 * @brief Computes the maximum site distance (offset) from the 
 *        3 channel GPS site data 
 *
 * @param[in] npts             number of points in time series
 * @param[in] lremove_disp0    if true then remove the (u,n,e) position at 
 *                             origin time SA_time.
 * @param[in] dt               sampling period (s) of GPS buffers
 * @param[in] SA_time          epochal UTC origin time (s)
 * @param[in] epoch            epochal UTC start time (s) of GPS traces
 * @param[in] ubuff            vertical position [npts]
 * @param[in] nbuff            north position [npts] 
 * @param[in] ebuff            east position [npts]
 *
 * @result if NAN then an error was encountered in the data
 *         otherwise this is the maximum distance (offset) in the window 
 *         from the origin time to the number of points in the trace
 *
 * @author Ben Baker (ISTI)
 */
double __GFAST_getMaxDistance(int npts, bool lremove_disp0,
                              double dt, double SA_time,
                              double epoch,
                              double *__restrict__ ubuff,
                              double *__restrict__ nbuff,
                              double *__restrict__ ebuff)
{
    double diffT, dist_i, distMax, e0, n0, u0;
    int i, indx0;
    //------------------------------------------------------------------------//
    //
    // Set the initial position
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    diffT = SA_time - epoch;
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    if (lremove_disp0){
        diffT = SA_time - epoch;
        u0 = ubuff[indx0];
        n0 = nbuff[indx0];
        e0 = ebuff[indx0];
        // Prevent a problem
        if (isnan(u0) || isnan(n0) || isnan(e0)){
            return NAN;
        }
    }
    // Compute the distance
    distMax = DIST_MAX_NAN;
    #pragma omp simd reduction(max:distMax)
    for (i=indx0; i<npts; i++){
        dist_i = DIST_MAX_NAN;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) )
        {
            dist_i = sqrt( pow(ubuff[i] - u0, 2)
                         + pow(nbuff[i] - n0, 2)
                         + pow(ebuff[i] - e0, 2));
        }
        distMax = fmax(dist_i, distMax);
    } // Loop on data points
    if (distMax == DIST_MAX_NAN){distMax = NAN;}
    return distMax; 
} 
