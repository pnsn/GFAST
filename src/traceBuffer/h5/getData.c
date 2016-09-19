#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
/*!
 * @brief Returns the data on for the stations and channels in the
 *        h5traceBuffer from epochal times t1 to t2
 *
 * @param[in] t1                 epochal start time of traces (UTC seconds)
 * @param[in] t2                 epochal end time of traces (UTC seconds)
 *
 * @param[inout] h5traceBuffer   on input contains the desired traces (SNCLs)
 *                               to query the HDF5 file.
 *                               on successful exit contains the traces for
 *                               each SNCL from times t1 to t2.  any unknown
 *                               data points must be detected by the user as
 *                               they were defined in the HDF5 write step.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int traceBuffer_h5_getData(const double t1, const double t2,
                           struct h5traceBuffer_struct *h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_getData\0";
    char dataBuffer1[64], dataBuffer2[64];
    double *work, dt, ts1, ts2, ts1Use, ts2Use;
    int i, ierr, i1, i2, j1, j2, maxpts, nc1, nc2, ncopy;
    hid_t groupID;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Do the times make sense?
    if (t2 < t1)
    {
        log_errorF("%s: Invalid input times\n", fcnm);
        return -1;
    }
    status = 0;
    // Loop on the traces
    for (i=0; i<h5traceBuffer->ntraces; i++)
    {
        // Open the group for reading 
        groupID = H5Gopen2(h5traceBuffer->fileID,
                           h5traceBuffer->traces[i].groupName, H5P_DEFAULT);
        // Get the scalars describing this dataset
        ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345, -NAN,
                                               &maxpts,
                                               &dt, &ts1, &ts2);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting scalars!\n", fcnm);
            continue;
        }
        if (dt <= 0.0)
        {
            log_errorF("%s: Error dt is wrong %f!\n", fcnm);
            continue;
        }
        if (t1 < ts1 && t1 < ts2)
        {
            log_errorF("%s: Error time is too %16.f %16.f %16.f\n",
                       fcnm, t1, ts1, ts2);
            continue;
        }
        if (t2 > fmax(ts1, ts2) + (double) (maxpts - 1)*dt)
        {
            log_errorF("%s: Error stale buffers %16.f %16.f %16.f\n",
                       fcnm, t2, ts1, ts2);
            continue;
        }
        ncopy = (int) ((t2 - t1)/dt + 0.5) + 1;
        ISCL_memory_free__double(&h5traceBuffer->traces[i].data);
        work = ISCL_memory_alloc__double(ncopy);
        // Set the databuffers and names
        memset(dataBuffer1, 0, sizeof(dataBuffer1));
        memset(dataBuffer2, 0, sizeof(dataBuffer2));
        if (ts1 < ts2)
        {
            ts1Use = ts1;
            ts2Use = ts2;
            strcpy(dataBuffer1, "dataBuffer1\0");
            strcpy(dataBuffer2, "dataBuffer2\0");
        }
        else
        {
            ts1Use = ts2;
            ts2Use = ts1;
            strcpy(dataBuffer1, "dataBuffer2\0");
            strcpy(dataBuffer2, "dataBuffer1\0");
        }
        // First buffer
        if (t1 < ts2Use)
        {
            i1 = (int) ((t1 - ts1Use)/dt + 0.5);
            i2 = maxpts - 1;
            // All on first buffer
            if (t2 < ts2Use)
            {
                i2 = (int) ((t2 - ts1Use)/dt + 0.5);
                // Get data from [i1:i2]
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           i1, i2,
                                                           dataBuffer1,
                                                           NAN,
                                                           ncopy, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                }
            }
            // Bleeds onto second buffer
            else
            {
                i2 = maxpts - 1;
                j1 = 0;
                j2 = fmin(maxpts-1, (int) ((t2 - ts2Use)/dt + 0.5));
                nc1 = i2 - i1 + 1;
                nc2 = j2 - j1 + 1;
                // Get data from [i1:i2] then [j1:j2]
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           i1, i2, 
                                                           dataBuffer1,
                                                           NAN,
                                                           nc1, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                }
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           j1, j2,
                                                           dataBuffer2,
                                                           NAN,
                                                           nc2, &work[nc1]);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                }
            }
            h5traceBuffer->traces[i].data = work;
            h5traceBuffer->traces[i].t1 = ts1 + (double) i1*dt;
            h5traceBuffer->traces[i].ncopy = ncopy;
        }
        else
        {
            log_errorF("%s: Invalid data range %16.4f %16.4f %16.4f!\n",
                       fcnm, t1, ts1Use, ts2Use);
        }
        status = status + H5Gclose(groupID);
    } // Loop on traces
    if (status != 0)
    {
        log_errorF("%s: Error accessing data\n", fcnm);
        return -1;
    }
    return 0;
}
