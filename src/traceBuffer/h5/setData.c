#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <hdf5.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Writes the data on the h5traceBuffer to the HDF5 archive
 *
 *
 */
int traceBuffer_h5_setData(struct h5traceBuffer_struct *h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_setData\0";
    double dt, dt0, dtH5, t1, t2, tmaxH5, tmaxH50, tmaxIn, ts1,ts1Min, 
           ts2, ts2Max;
    int i, ierr, ierrAll, maxpts, npts;
    bool lnoData;
    hid_t groupID;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Error checking
    if (h5traceBuffer->ntraces == 0){return 0;} // Nothing to update
    // Get the min and max times to copy
    ierr = 0;
    ierrAll = 0;
    dt0 =-DBL_MAX;
    ts1Min = DBL_MAX;
    ts2Max =-DBL_MAX;
    tmaxH50 =-DBL_MAX;
    tmaxIn =-DBL_MAX;
    tmaxH5 =-DBL_MAX;
    lnoData = true;
    for (i=0; i<h5traceBuffer->ntraces; i++)
    {
        // Make sure there is data worth considering
        npts = h5traceBuffer->traces[i].ncopy;
        if (npts < 0 || h5traceBuffer->traces[i].data == NULL){continue;}
        lnoData = false;
        // Get the max time on the trace
        dt = h5traceBuffer->traces[i].dt;
        t1 = h5traceBuffer->traces[i].t1;
        t2 = t1 + (double) (npts - 1)*dt;
        tmaxIn = fmax(t2, tmaxIn);
        // Get the same information from the HDF5 buffer
        groupID =  H5Gopen2(h5traceBuffer->fileID,
                            h5traceBuffer->traces[i].groupName, H5P_DEFAULT);
        ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345, -NAN,
                                               &maxpts,
                                               &dtH5, &ts1, &ts2);
        if (fabs(dtH5 - dt)/dtH5 > 1.e-10)
        {
            log_errorF("%s: Error saved sampling periods from data %f %f\n",
                       fcnm, dt, dtH5);
            ierrAll = ierrAll + 1;
        }
        if (dt0 ==-DBL_MAX){dt0 = dt;}
        if (fabs(dt - dt0)/dt > 1.e-10)
        {
            log_errorF("%s: Need constant sampling period in data\n", fcnm);
            return -1;
        } 
        ts1Min = fmin(ts1Min, fmin(ts1, ts2));
        ts2Max = fmax(ts2Max, fmax(ts1, ts2));
        tmaxH50 = fmax(ts1, ts2) + (double) (maxpts - 1)*dtH5;
        tmaxH5 = fmax(tmaxH5, tmaxH50);
        if (tmaxH50 >-DBL_MAX)
        {
            if ((tmaxH50 - tmaxH5)/tmaxH5 > 1.e-10)
            {
                log_warnF("%s: All buffers should be consistent %16.f %16.f\n",
                          fcnm, tmaxH5, tmaxH50);
            }
        }
        tmaxH50 = tmaxH5;
        status = H5Gclose(groupID);
        if (status != 0)
        {
            log_errorF("%s: Error closing the group\n", fcnm);
            ierrAll = ierrAll + 1;
        }
    } // Loop on traces
    if (ierrAll != 0)
    {
        log_errorF("%s: Error in HDF5 IO\n", fcnm);
        return -1;
    }
    if (lnoData)
    {
        log_warnF("%s: No data provided\n", fcnm);
        return 0;
    }
    // Not updating because data is too latent 
    if (tmaxIn < ts1Min)
    {
        log_warnF("%s: Skipping update - data is too latent\n", fcnm);
        return 0;
    }
    // Update the buffers to the current time
    for (i=0; i<h5traceBuffer->ntraces; i++)
    {
        // Get the data from the group
 
    }
    return ierr;
}
