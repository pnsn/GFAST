#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Returns the data on for the stations and channels in the
 *        h5traceBuffer from epochal times t1 to t2
 *
 * @param[in] t1                 epochal start time of traces (UTC seconds)
 * @param[in] t2                 epochal end time of traces (UTC seconds)
 *
 * @param[in,out] h5traceBuffer  on input contains the desired traces (SNCLs)
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
    double *gain, *work, dt, ts1, ts2;
    int i1, i2, ibeg, idt, iend, ierr, j1, j2, k, k1, k2,
        maxpts, ncopy, ntraces;
    hid_t groupID;
    herr_t status;
    //------------------------------------------------------------------------//
    for (idt=0; idt<h5traceBuffer->ndtGroups; idt++)
    {
        gain = NULL;
        work = NULL;
        k1 = h5traceBuffer->dtPtr[idt];
        k2 = h5traceBuffer->dtPtr[idt+1];
        ntraces = k2 - k1;
        if (ntraces == 0){continue;}
        // Open + read the data and attributes for this dataset 
        groupID = H5Gopen2(h5traceBuffer->fileID,
                           h5traceBuffer->dtGroupName[idt], H5P_DEFAULT);
        gain = memory_calloc64f(ntraces);
        work = traceBuffer_h5_readData(groupID, ntraces,
                                       &maxpts, &dt, &ts1, &ts2, gain, &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error reading data\n", fcnm);
            return -1;
        }
        // This concludes the fileIO
        status = H5Gclose(groupID);
        // Check what i just read
        if (status < 0)
        {
            log_errorF("%s: Error closing group %s\n",
                       fcnm, h5traceBuffer->dtGroupName[idt]);
            return -1;
        }
        if (dt <= 0.0)
        {
            log_errorF("%s: Invalid sampling period %f\n", fcnm, dt);
            return -1;
        } 
        if (maxpts < 1)
        {
            log_errorF("%s: Invalid number of points %d\n", fcnm, maxpts);
            return -1; 
        }
        // Check what I just read
        if (t1 > ts2)
        {
            log_errorF("%s: Start time is too new\n", fcnm);
            return -1;
        }
        if (t2 < ts1)
        {
            log_errorF("%s: End time is too old\n", fcnm);
            return -1;
        }
        if (t2 > ts1 + (double) (maxpts - 1)*dt)
        {
            log_errorF("%s: Looking for data that's too current\n", fcnm);
            return -1;
        }
        if (t1 < ts1)
        {
            log_errorF("%s: Error start time is too old\n", fcnm);
            return -1;
        }
        // Indices in chunk to read from
        i1 = (int) ((t1 - ts1)/dt + 0.5);
        i2 = (int) ((t2 - ts1)/dt + 0.5);
        if (i1 < 0){i1 = 0;}
        if (i2 > maxpts - 1){i2 = maxpts - 1;}
        // Indices to set (if i were more clever i could make this span a
        // superset of the times in memory - but then i'd fill it with NaNs
        // and risk messing up the offset computation - particularly at the
        // beginning where we have to know the position at the origin time)
        j1 = i1;
        j2 = i2;
        if (j1 < 0){j1 = 0;} 
        if (j2 > maxpts - 1){j2 = maxpts - 1;}
        if (i2 - i1 > j2 - j1)
        {
            log_errorF("%s: Error source array larger than dest array\n", fcnm);
            return - 1;
        }
        ncopy = (int) ((t2 - t1)/dt + 0.5) + 1;
        // Get the data onto the buffers
        for (k=k1; k<k2; k++)
        {
            // set info for this trace
            memory_free64f(&h5traceBuffer->traces[k].data);
            h5traceBuffer->traces[k].t1 = t1;
            h5traceBuffer->traces[k].ncopy = ncopy;
            h5traceBuffer->traces[k].gain = gain[k-k1];
            h5traceBuffer->traces[k].data
                = ISCL_array_set64f(ncopy, (double) NAN, &ierr);
            // copy it
            ibeg = h5traceBuffer->traces[k].traceNumber*maxpts + i1;
            iend = ibeg + i2;
            if (iend - ibeg + 1 != ncopy)
            {
                log_errorF("%s: Inconsistent copy size %d %d %d\n",
                           fcnm, ibeg, iend, ncopy);
                return -1;
            }
            memcpy(&h5traceBuffer->traces[k].data[j1], &work[ibeg],
                   (size_t) (ncopy)*sizeof(double)); 
        } // Loop on streams in this group
        // Release temporary memory 
        ISCL_memory_free__double(&work);
        ISCL_memory_free__double(&gain);
    } // Loop on sampling period groups
    return 0;
}
/*!
 * @brief Returns the data on for the stations and channels in the
 *        h5traceBuffer from epochal times t1 to t2
 *
 * @param[in] t1                 epochal start time of traces (UTC seconds)
 * @param[in] t2                 epochal end time of traces (UTC seconds)
 *
 * @param[in,out] h5traceBuffer  on input contains the desired traces (SNCLs)
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
/*
int traceBuffer_h5_getData2(const double t1, const double t2,
                            struct h5traceBuffer_struct *h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_getData\0";
    char dataBuffer1[64], dataBuffer2[64];
    double *work, dt, gain, ts1, ts2, ts1Use, ts2Use;
    int i, ierr, ierrAll, i1, i2, j1, j2, maxpts, nc1, nc2, ncopy;
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
    ierrAll = 0;
    status = 0;
    // Loop on the traces
    for (i=0; i<h5traceBuffer->ntraces; i++)
    {
        // Set the trace to a fail
        ISCL_memory_free__double(&h5traceBuffer->traces[i].data);
        h5traceBuffer->traces[i].t1 = 0.0;
        h5traceBuffer->traces[i].ncopy = 0;
        // Open the group for reading 
        groupID = H5Gopen2(h5traceBuffer->fileID,
                           h5traceBuffer->traces[i].groupName, H5P_DEFAULT);
        // Get the scalars describing this dataset
        ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345, (double) NAN,
                                               &maxpts,
                                               &dt, &gain, &ts1, &ts2);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting scalars!\n", fcnm);
            ierrAll = ierrAll + 1;
            continue;
        }
        if (maxpts < 1)
        {
            log_errorF("%s: Error maxpts is wrong %d\n", fcnm, maxpts);
            ierrAll = ierrAll + 1;
            continue;
        }
        if (dt <= 0.0)
        {
            log_errorF("%s: Error dt is wrong %f!\n", fcnm, dt);
            ierrAll = ierrAll + 1;
            continue;
        }
        if (gain == 0.0)
        {
            log_warnF("%s: Division by zero coming up\n", fcnm);
        }
        h5traceBuffer->traces[i].gain = gain;
        if (t1 < ts1 && t1 < ts2)
        {
            log_errorF("%s: Error time is too %16.f %16.f %16.f\n",
                       fcnm, t1, ts1, ts2);
            ierrAll = ierrAll + 1;
            continue;
        }
        if (t2 > fmax(ts1, ts2) + (double) (maxpts - 1)*dt)
        {
            log_errorF("%s: Error stale buffers %16.f %16.f %16.f\n",
                       fcnm, t2, ts1, ts2);
            ierrAll = ierrAll + 1;
            continue;
        }
        ncopy = (int) ((t2 - t1)/dt + 0.5) + 1;
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
                                                           (double) NAN,
                                                           ncopy, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                    ierrAll = ierrAll + 1;
                    continue;
                }
            }
            // Bleeds onto second buffer
            else
            {
                i2 = maxpts - 1;
                j1 = 0;
                j2 = (int) (fmin(maxpts-1, (int) ((t2 - ts2Use)/dt + 0.5)));
                nc1 = i2 - i1 + 1;
                nc2 = j2 - j1 + 1;
                // Get data from [i1:i2] then [j1:j2]
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           i1, i2, 
                                                           dataBuffer1,
                                                           (double) NAN,
                                                           nc1, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                }
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           j1, j2,
                                                           dataBuffer2,
                                                           (double) NAN,
                                                           nc2, &work[nc1]);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                    ierrAll = ierrAll + 1;
                    continue;
                }
            }
            h5traceBuffer->traces[i].data = work;
            h5traceBuffer->traces[i].t1 = ts1Use + (double) i1*dt;
            h5traceBuffer->traces[i].ncopy = ncopy;
        }
        else
        {
            // All on second buffer
            if (t1 >= ts2Use)
            {
                j1 = (int) (fmax(0, (int) ((t1 - ts2Use)/dt + 0.5)));
                j2 = (int) (fmin(maxpts-1, (int) ((t2 - ts2Use)/dt + 0.5)));
                ierr = GFAST_traceBuffer_h5_getDoubleArray(groupID,
                                                           j1, j2,
                                                           dataBuffer2,
                                                           (double) NAN,
                                                           ncopy, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                    ierrAll = ierrAll + 1;
                    continue;
                }
                h5traceBuffer->traces[i].data = work;
                h5traceBuffer->traces[i].t1 = ts2Use + (double) j1*dt;
                h5traceBuffer->traces[i].ncopy = ncopy;
            }
            else
            {
                log_errorF("%s: Invalid data range %16.4f %16.4f %16.4f!\n",
                           fcnm, t1, ts1Use, ts2Use);
            }
        }
        status = status + H5Gclose(groupID);
    } // Loop on traces
    if (status != 0)
    {
        log_errorF("%s: Error accessing data\n", fcnm);
        return -1;
    }
    if (ierrAll != 0)
    {
        log_errorF("%s: %d errors detected when getting data\n",
                   fcnm, ierrAll);
    }
    return ierrAll;
}
*/
