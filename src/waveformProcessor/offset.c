#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"

static bool __getAverageOffset(int npts,
                               double dt, 
                               double SA_time,
                               double swave_time,
                               double epoch,
                               const double *__restrict__ ubuff,
                               const double *__restrict__ nbuff,
                               const double *__restrict__ ebuff,
                               double *uOffset,
                               double *nOffset,
                               double *eOffset);

/*!
 * @brief Estimates the average offset for each GPS precise point positiion
 *        data stream with the additional requirement that the shear wave
 *        has passed through through the site.
 *
 * @note To perform the averaging it is presumed the shear wave has passed
 *       through the station then the average offset is estimated from
 *       data beginning at the shear wave arrival and ending at the last
 *       sample seen at the station.  The approximate shear wave arrival
 *       time is estimated by presuming a shear wave velocity (svel_window,
 *       \f$ v_{s} \f$) and an origin time \f$ t_{0} \f$ then computing the
 *       source receiver distance \f$ d_{s,r} \f$ and applying
 *       \f$ t_{s} = t_0 + \frac{ d_{s,r} }{ v_{s} } \f$. 
 *       Because the inversion is for static offsets a conservative
 *       strategy to avoid modeling the dynamic response is to lower
 *       the shear velocity.
 *         
 *
 * @param[in] utm_zone        if not -12345 then this is the desired UTM zone
 *                            in which to compute source and receiver positions.
 *                            otherwise, the UTM zone will be estimated from
 *                            the source location
 * @param[in] svel_window     the shear wave velocity used in data windowing
 *                            (km/s).  if the the site/source distance is
 *                            less than
 *                              (current_time - SA_time)*svel_window 
 *                            then the site will be excluded
 * @param[in] SA_lat          source hypocentral latitude (degrees) [-90,90]
 * @param[in] SA_lon          source hypocentral longitude (degrees) [0,360]
 * @param[in] SA_dep          source hypocentral depth (km) (this is positive
 *                            down from the free surface)
 * @param[in] SA_time         source origin time in seconds since epoch (UTC)
 * @param[in] gps_data        contains the most up-to-date precise point
 *                            positions for each site
 *
 * @param[inout] offset_data   on input holds a logical mask if a site is to
 *                             be ignored.
 *                             on output holds the average offset at each 
 *                             site satisfying the S velocity window mask.
 *                             in this case, all sites with data are given
 *                             data weights of unity and all sites without
 *                             data are given weights of zero.
 *
 * @param[out] ierr        0 indicates success
 *
 * @result the number of sites at which peak displacement was computed
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int GFAST_waveformProcessor__offset(
    int utm_zone,
    double svel_window,
    double SA_lat,
    double SA_lon,
    double SA_dep,
    double SA_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_offsetData_struct *offset_data,
    int *ierr)
{
    const char *fcnm = "GFAST_waveformProcessor__offset\0";
    double currentTime, distance, effectiveHypoDist, eOffset, nOffset,
           swave_time, uOffset, x1, x2, y1, y2;
    int k, nsites, zone_loc;
    bool lnorthp, luse;
    //------------------------------------------------------------------------//
    //
    // Error handling
    *ierr = 0;
    nsites = 0;
    if (gps_data.stream_length != offset_data->nsites)
    {
        log_errorF("%s: Inconsistent structure sizes %d %d\n",
                   fcnm, gps_data.stream_length, offset_data->nsites);
        *ierr = 1;
        // For safety cut the inversion off at the knees
        if (offset_data->nsites > 0)
        {
            for (k=0; k<offset_data->nsites; k++)
            {
                offset_data->ubuff[k] = 0.0;
                offset_data->nbuff[k] = 0.0;
                offset_data->ebuff[k] = 0.0;
                offset_data->wtu[k] = 0.0;
                offset_data->wtn[k] = 0.0;
                offset_data->wte[k] = 0.0;
                offset_data->lactive[k] = false;
            }
        }
        return nsites;
    }
    // Get the source location
    zone_loc = utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Get UTM zone from source lat/lon
    GFAST_coordtools__ll2utm(SA_lat, SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    // Loop on streams and if they satisfy the S wave mask get their offsets 
    for (k=0; k<gps_data.stream_length; k++)
    {
        // Make sure I have the latest/greatest site location 
        offset_data->sta_lat[k] = gps_data.data[k].sta_lat;
        offset_data->sta_lon[k] = gps_data.data[k].sta_lon; 
        offset_data->sta_alt[k] = gps_data.data[k].sta_alt;
        // Null out result
        offset_data->ubuff[k] = 0.0; // Null out result
        offset_data->nbuff[k] = 0.0;
        offset_data->ebuff[k] = 0.0;
        offset_data->wtu[k] = 0.0; // Assume no weight
        offset_data->wtn[k] = 0.0;
        offset_data->wte[k] = 0.0; 
        offset_data->lactive[k] = false;
        if (offset_data->lmask[k]){continue;}
        //if (gps_data.data[k].lskip_cmt){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(gps_data.data[k].sta_lat,
                                 gps_data.data[k].sta_lon,
                                 &y2, &x2,
                                 &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2, 2)
                       + pow(y1 - y2, 2)
                       + pow(SA_dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // Apply an S wave window mask to preclude likely outliers in the
        // ensuing CMT/finite fault inversions
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA_time)*svel_window;
        if (distance < effectiveHypoDist)
        {
            swave_time = SA_time + distance/svel_window;
            // Compute the average offset beginning after the S wave mask
            luse = __getAverageOffset(gps_data.data[k].npts,
                                      gps_data.data[k].dt, 
                                      SA_time,
                                      swave_time,
                                      gps_data.data[k].epoch,
                                      gps_data.data[k].ubuff,
                                      gps_data.data[k].nbuff,
                                      gps_data.data[k].ebuff,
                                      &uOffset,
                                      &nOffset,
                                      &eOffset);
            // Only use average offset if it isn't all NaN's
            if (luse)
            {
                offset_data->ubuff[k] = uOffset; // meters
                offset_data->nbuff[k] = nOffset;
                offset_data->ebuff[k] = eOffset;
                offset_data->wtu[k] = 1.0;
                offset_data->wtn[k] = 1.0;
                offset_data->wte[k] = 1.0;
                offset_data->lactive[k] = true;
                nsites = nsites + 1;
            } // End check on whether or not to use average offset
        } // End check on S-wave mask
    } // Loop on data streams
    return nsites; 
}
//============================================================================//
/*!
 * @brief Computes the average offset on all components beginning at the
 *        S wave time and running to the end of the observed
 *
 * @param[in] npts        number of points in time series
 * @param[in] dt          sampling period (s) of time series
 * @param[in] SA_time     epoch (s) of origin time (UTC)
 * @param[in] swave_time  epoch (s) of s wave arrival (UTC)
 * @param[in] epoch       epoch (s) of trace start time (UTC)
 * @param[in] ubuff       vertical precise point position data [npts]
 * @param[in] nbuff       north precise point position data [npts]
 * @param[in] ebuff       east precise point position data [npts]
 *
 * @param[out] uOffset    offset in vertical position
 * @param[out] noffset    offset in north position
 * @param[out] eOffset    offset in east position
 *
 * @result if true then uOffset, nOffset, and eOffset are not NaN's and can
 *         be used in the inversion
 *
 * @author Ben Baker (ISTI)
 *
 */
static bool __getAverageOffset(int npts,
                               double dt,
                               double SA_time,
                               double swave_time,
                               double epoch,
                               const double *__restrict__ ubuff,
                               const double *__restrict__ nbuff,
                               const double *__restrict__ ebuff,
                               double *uOffset,
                               double *nOffset,
                               double *eOffset)
{
    const char *fcnm = "__getAverageOffset\0";
    double diffT, de, dn, du, e0, n0, u0, eOffsetNan, nOffsetNan, uOffsetNan;
    int i, iavg, iavg1, indx0;
    bool luse;
    //------------------------------------------------------------------------//
    //
    // Initialize result 
    *uOffset = NAN;
    *nOffset = NAN;
    *eOffset = NAN;
    luse = false;
    // This is a bad input
    if (SA_time > swave_time)
    {
        log_errorF("%s: event origin time exceeds S wave arrival\n", fcnm);
        return luse;
    }
    // This might compromise the offset
    if (epoch > SA_time)
    {
        log_warnF("%s: Warning trace start-time is after event origint ime\n",
                  fcnm);
    }
    // Set the initial position
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    // Estimate the origin time index
    diffT = SA_time - epoch;
    if (diffT < 0.0){return luse;} // This will be a disaster
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    u0 = ubuff[indx0];
    n0 = nbuff[indx0];
    e0 = ebuff[indx0];
    // Prevent a nonsensical difference
    if (isnan(u0) || isnan(n0) || isnan(e0)){return luse;}
    // Estimate the S wave arrival time index
    diffT = swave_time - epoch;
    if (diffT < 0.0){return luse;}
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    // Compute the average from the S wave arrival to the end of the data
    uOffsetNan = 0.0;
    nOffsetNan = 0.0;
    eOffsetNan = 0.0;
    iavg = 0;
    // Compute the average over the window
    #pragma omp simd reduction(+:iavg, eOffsetNan, nOffsetNan, uOffsetNan)
    for (i=indx0; i<npts; i++)
    {
        luse = false;
        du = 0.0;
        dn = 0.0;
        de = 0.0;
        iavg1 = 0;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) &&
            ubuff[i] >-999.0 && nbuff[i] >-999.0 && ebuff[i] >-999.0)
        {
            luse = true;
        }
        if (luse){du = ubuff[i] - u0;}
        if (luse){dn = nbuff[i] - n0;}
        if (luse){de = ebuff[i] - e0;}
        if (luse){iavg1 = 1;}
        uOffsetNan = uOffsetNan + du;
        nOffsetNan = nOffsetNan + dn;
        eOffsetNan = eOffsetNan + de;
        iavg = iavg + iavg1;
    } // Loop on data points
    // There's data - average it and use this result
    if (iavg > 0)
    {
        *uOffset = uOffsetNan/(double) iavg;
        *nOffset = nOffsetNan/(double) iavg;
        *eOffset = eOffsetNan/(double) iavg;
        luse = true;
    }
    return luse;
}
