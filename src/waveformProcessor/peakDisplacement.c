#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include "gfast.h"

#define PD_MAX_NAN -DBL_MAX

static double __getPeakDisplacement(int npts,
                                    double dt,
                                    double SA_time,
                                    double epoch,
                                    const double *__restrict__ ubuff,
                                    const double *__restrict__ nbuff,
                                    const double *__restrict__ ebuff);
/*!
 * @brief Computes the peak displacement for each GPS dataset with the 
 *        additional requirement that the shear wave has passed through 
 *        the site.
 *
 * @param[in] utm_zone     if not -12345 then this is the desired UTM zone
 *                         in which to compute source and receiver positions.
 *                         otherwise, the UTM zone will be estimated from
 *                         the source location
 * @param[in] svel_window  the shear wave velocity window.  if the site/source
 *                         distance is less than 
 *                         (current_time - SA_time)*svel_window 
 *                         then the site will be excluded
 * @param[in] SA_lat       source hypocentral latitude (degrees) [-90,90]
 * @param[in] SA_lon       source hypocentral longitude (degrees) [0,360]
 * @param[in] SA_dep       source hypocentral depth (km) (this is positive
 *                         down from the free surface)
 * @param[in] SA_time      source epochal (s) origin time (UTC)
 * @param[in] gps_data     contains the most up-to-date precise point 
 *                         positions for each site 
 *
 * @param[inout] pgd_data  on input holds a logical mask if a site is to
 *                         be ignored.
 *                         on output holds the peak ground displacement
 *                         at each site satisfying the S velocity window mask.
 *
 * @param[out] ierr        0 indicates success
 *
 * @result the number of sites at which peak displacement was computed
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_waveformProcessor__peakDisplacement(
    int utm_zone,
    double svel_window,
    double SA_lat,
    double SA_lon,
    double SA_dep,
    double SA_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    int *ierr)
{
    const char *fcnm = "GFAST_waveformProcessor__peakDisplacement\0";
    double currentTime, distance, effectiveHypoDist, peakDisp, x1, x2, y1, y2;
    int k, nsites, zone_loc;
    bool lnorthp;
    //------------------------------------------------------------------------//
    //
    // Error handling
    *ierr = 0;
    nsites = 0;
    if (gps_data.stream_length != pgd_data->nsites)
    {
        log_errorF("%s: Inconsistent structure sizes %d %d\n",
                   fcnm, gps_data.stream_length, pgd_data->nsites); 
        *ierr = 1;
        // For safety cut the inversion off at the knees
        if (pgd_data->nsites > 0)
        {
            for (k=0; k<pgd_data->nsites; k++)
            {
                pgd_data->pd[k] = 0.0;
                pgd_data->lactive[k] = false;
            }
        }
        return nsites;
    }
    // Get the source location
    zone_loc = utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;}
    GFAST_coordtools__ll2utm(SA_lat, SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    // Loop on streams and if they satisfy the S wave mask get their PGD
    for (k=0; k<gps_data.stream_length; k++)
    {
        pgd_data->pd[k] = 0.0; // Null out result
        pgd_data->lactive[k] = false; // Assume site is not active in inversion
        if (pgd_data->lmask[k]){continue;} // Not in inversion
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
        // Apply an S wave window mask so to preclude likely outliers in
        // the ensuing PGD inversion 
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA_time)*svel_window;
        if (distance < effectiveHypoDist)
        {
            // Compute the peak displacement max(norm(u + n + e, 2))
            peakDisp = __getPeakDisplacement(gps_data.data[k].npts,
                                             gps_data.data[k].dt,
                                             SA_time,
                                             gps_data.data[k].epoch,
                                             gps_data.data[k].ubuff,
                                             gps_data.data[k].nbuff,
                                             gps_data.data[k].ebuff);
            // If it isn't a NaN then retain it for processing
            if (peakDisp != NAN)
            {
                pgd_data->pd[k] = peakDisp; // meters
                pgd_data->lactive[k] = true;
                nsites = nsites + 1;
            }
        } // End check on s-wave mask
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
 * @param[in] SA_time          epochal UTC origin time (s)
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
static double __getPeakDisplacement(int npts,
                                    double dt,
                                    double SA_time,
                                    double epoch,
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
    diffT = SA_time - epoch;
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    // Compute the offset
    diffT = SA_time - epoch;
    u0 = ubuff[indx0];
    n0 = nbuff[indx0];
    e0 = ebuff[indx0];
    // Prevent a problem
    if (isnan(u0) || isnan(n0) || isnan(e0))
    {
        return NAN;
    }
    // Compute the maximum peak ground displacement 
    peakDisplacement = PD_MAX_NAN;
    #pragma omp simd reduction(max:peakDisplacement)
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
    if (peakDisplacement == PD_MAX_NAN){peakDisplacement = NAN;}
    return peakDisplacement;
}
