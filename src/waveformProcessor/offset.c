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

    return 0;
}
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
    // Estimate the S wave arrival time
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
