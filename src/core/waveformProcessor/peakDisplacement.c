#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include "gfast_core.h"

#define PD_MAX_NAN -DBL_MAX
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

static double __getPeakDisplacement(const int npts,
                                    const double dt,
                                    const double ev_time,
                                    const double epoch,
                                    const double *__restrict__ ubuff,
                                    const double *__restrict__ nbuff,
                                    const double *__restrict__ ebuff);
/*!
 * @brief Computes the peak displacement for each GPS precise point position
 *        data stream with the additional requirement that the shear wave
 *        has passed through the site.
 *
 * @param[in] utm_zone      if not -12345 then this is the desired UTM zone
 *                          in which to compute source and receiver positions.
 *                          otherwise, the UTM zone will be estimated from
 *                          the source location
 * @param[in] svel_window   the shear wave velocity used in data windowing
 *                          (km/s).  if the site/source distance is less than
 *                           (current_time - ev_time)*svel_window 
 *                          then the site will be excluded
 * @param[in] ev_lat        source hypocentral latitude (degrees) [-90,90]
 * @param[in] ev_lon        source hypocentral longitude (degrees) [0,360]
 * @param[in] ev_dep        source hypocentral depth (km) (this is positive
 *                          down from the free surface)
 * @param[in] ev_time       source origin time in seconds since epoch (UTC)
 * @param[in] gps_data      contains the most up-to-date precise point 
 *                          positions for each site 
 *
 * @param[in,out] pgd_data  on input holds a logical mask if a site is to
 *                          be ignored.
 *                          on output holds the peak ground displacement
 *                          at each site satisfying the S velocity window mask.
 *                          in this instance all sites with data are given
 *                          data weights of unity and all sites without
 *                          data are given data weights of zero.
 *
 * @param[out] ierr         0 indicates success
 *
 * @result the number of sites at which peak displacement was computed
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int core_waveformProcessor_peakDisplacement(
    const int utm_zone,
    const double svel_window,
    const double ev_lat,
    const double ev_lon,
    const double ev_dep,
    const double ev_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    int *ierr)
{
    double currentTime, distance, effectiveHypoDist, epoch,
           peakDisp, x1, x2, y1, y2;
    int k, nsites, zone_loc;
    int i;
    bool lnorthp;
    //------------------------------------------------------------------------//
    //
    // Error handling
    *ierr = 0;
    nsites = 0;
    if (gps_data.stream_length != pgd_data->nsites)
    {
        LOG_ERRMSG("Inconsistent structure sizes %d %d",
                   gps_data.stream_length, pgd_data->nsites); 
        *ierr = 1;
        // For safety cut the inversion off at the knees
        if (pgd_data->nsites > 0)
        {
            for (k=0; k<pgd_data->nsites; k++)
            {
                pgd_data->pd[k] = 0.0;
                pgd_data->wt[k] = 0.0;
                pgd_data->lactive[k] = false;
            }
        }
        return nsites;
    }
    // Get the source location
    zone_loc = utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Get UTM zone from source lat/lon
    GFAST_core_coordtools_ll2utm(ev_lat, ev_lon,
                                 &y1, &x1,
                                 &lnorthp, &zone_loc);
//LOG_MSG("peakDisp: utm_zone=%d ev_lat:%f ev_lon:%f x1:%f y1:%f\n",
         //utm_zone, ev_lat, ev_lon, x1, y1);
    // Loop on streams and if they satisfy the S wave mask get their PGD
    for (k=0; k<gps_data.stream_length; k++)
    {
        // Make sure I have the latest/greatest site location 
        pgd_data->sta_lat[k] = gps_data.data[k].sta_lat;
        pgd_data->sta_lon[k] = gps_data.data[k].sta_lon; 
        pgd_data->sta_alt[k] = gps_data.data[k].sta_alt;
        // Null out result
        pgd_data->pd[k] = 0.0; // Null out result
        pgd_data->wt[k] = 0.0; // Assume no weight
        pgd_data->lactive[k] = false; // Assume site is not active in inversion
        if (pgd_data->lmask[k]){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_core_coordtools_ll2utm(gps_data.data[k].sta_lat,
                                     gps_data.data[k].sta_lon,
                                     &y2, &x2,
                                     &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2, 2)
                       + pow(y1 - y2, 2)
                       + pow(ev_dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // Apply an S wave window mask to preclude likely outliers in
        // the ensuing PGD inversion 
        epoch = gps_data.data[k].tbuff[0]; //gps_data.data[k].epoch;
        currentTime = epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - ev_time)*svel_window;
LOG_MSG("time:%f effHypoDst:%.1f %s.%s.%s.%s <%.3f, %.3f> dist:%.1f",
         currentTime, effectiveHypoDist,
         gps_data.data[k].stnm, gps_data.data[k].chan[0],
         gps_data.data[k].netw, gps_data.data[k].loc,
         gps_data.data[k].sta_lat, gps_data.data[k].sta_lon, distance);
//LOG_MSG("peakDisp: x1:%f x2:%f (x1-x2):%f y1:%f y2:%f (y1-y2):%f\n",
         //x1, x2, (x1-x2), y1, y2, (y1-y2));
/*
         if (strcmp(gps_data.data[k].stnm, "CONZ") == 0) {
            for (i=0; i<gps_data.data[k].npts; i++) {
              LOG_MSG("<MTH> time:%f CONZ i=%2d ubuf=%f nbuf=%f ebuf=%f", currentTime, i,
                      gps_data.data[k].ubuff[i],
                      gps_data.data[k].nbuff[i],
                      gps_data.data[k].ebuff[i]);
            }
         }
*/

LOG_MSG("<MTH> ev_time:%f epoch:%f", ev_time, epoch);

        if (distance < effectiveHypoDist)
        {
            // Compute the peak displacement max(norm(u + n + e, 2))
            peakDisp = __getPeakDisplacement(gps_data.data[k].npts,
                                             gps_data.data[k].dt,
                                             ev_time,
                                             epoch,
                                             gps_data.data[k].ubuff,
                                             gps_data.data[k].nbuff,
                                             gps_data.data[k].ebuff);
            if (isnan(peakDisp))
            {
LOG_MSG("time:%f %s.%s.%s.%s Got peakDisp = nan ubuf=%f nbuf=%f ebuf=%f",
         currentTime,
         gps_data.data[k].stnm, gps_data.data[k].chan[0],
         gps_data.data[k].netw, gps_data.data[k].loc,
         gps_data.data[k].ubuff[gps_data.data[k].npts-1],
         gps_data.data[k].nbuff[gps_data.data[k].npts-1],
         gps_data.data[k].ebuff[gps_data.data[k].npts-1]);
            }
            else
            {
LOG_MSG("time:%f %s.%s.%s.%s Got peakDisp=%f ubuf=%f nbuf=%f ebuf=%f",
         currentTime,
         gps_data.data[k].stnm, gps_data.data[k].chan[0],
         gps_data.data[k].netw, gps_data.data[k].loc,
         peakDisp,
         gps_data.data[k].ubuff[gps_data.data[k].npts-1],
         gps_data.data[k].nbuff[gps_data.data[k].npts-1],
         gps_data.data[k].ebuff[gps_data.data[k].npts-1]);
            }

            // If it isn't a NaN then retain it for processing
            if (!isnan(peakDisp))
            {
                pgd_data->pd[k] = peakDisp; // meters
                pgd_data->wt[k] = 1.0;
                pgd_data->lactive[k] = true;
                nsites = nsites + 1;
            }
        } // End check on S-wave mask
    } // Loop on data streams
    return nsites;
}
//============================================================================//
/*!
 * @brief Waveform processor to estimate the peak displacement observed
 *        on a 3 channel GPS stream where the peak displacement at any
 *        sample is Euclidean norm of it's displacement.
 *
 * @param[in] npts             number of points in time series
 * @param[in] dt               sampling period (s) of GPS buffers
 * @param[in] ev_time          epochal UTC origin time (s)
 * @param[in] epoch            epochal UTC start time (s) of GPS traces
 * @param[in] ubuff            vertical position [npts]
 * @param[in] nbuff            north position [npts]
 * @param[in] ebuff            east position [npts]
 *
 * @result the peak displacement observed on a trace.  this has the same
 *         units as ubuff, nbuff, and ebuff.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
static double __getPeakDisplacement(const int npts,
                                    const double dt,
                                    const double ev_time,
                                    const double epoch,
                                    const double *__restrict__ ubuff,
                                    const double *__restrict__ nbuff,
                                    const double *__restrict__ ebuff)
{
    double diffT, peakDisplacement_i, peakDisplacement, e0, n0, u0;
    int i, indx0;
    //------------------------------------------------------------------------//
    //
    // Set the initial position
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    diffT = ev_time - epoch;
    indx0 = MAX(0, (int) (diffT/dt + 0.5));
    indx0 = MIN(npts-1, indx0);
    // Compute the offset
    u0 = ubuff[indx0];
    n0 = nbuff[indx0];
    e0 = ebuff[indx0];
    // Prevent a problem
    //LOG_MSG("diffT=%f indx0=%d npts=%d u0=%f n0=%f e0=%f Final:u=%f n=%f e=%f", 
             //diffT, indx0, npts, u0, n0, e0, ubuff[npts-1], nbuff[npts-1], ebuff[npts-1]);
    LOG_MSG("diffT=%f indx0=%d npts=%d", diffT, indx0, npts);
    if (isnan(u0) || isnan(n0) || isnan(e0))
    {
    LOG_MSG("%s", "u0 || n0 || e0 is NaN!");
    }

    if (isnan(u0) || isnan(n0) || isnan(e0))
    {
      LOG_MSG("Returning NAN instead of calculating epoch:%f diffT=%f indx0=%d",
          epoch, diffT, indx0)
        return (double) NAN;
    }
    // Compute the maximum peak ground displacement 
    peakDisplacement = PD_MAX_NAN;
    for (i=indx0; i<npts; i++)
    {
        peakDisplacement_i = PD_MAX_NAN;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) )
        {
            peakDisplacement_i = sqrt( pow(ubuff[i] - u0, 2)
                                     + pow(nbuff[i] - n0, 2)
                                     + pow(ebuff[i] - e0, 2));
        }
        peakDisplacement = fmax(peakDisplacement_i, peakDisplacement);
    } // Loop on data points
    if (fabs(peakDisplacement - PD_MAX_NAN)/fabs(PD_MAX_NAN) < 1.e-10)
    {
         peakDisplacement = (double) NAN;
    }
    return peakDisplacement;
}
