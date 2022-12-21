#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
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
    double *gain, *work, dt, ts1, ts2;
    int i1, i2, ibeg, idt, iend, ierr, j1, j2, k, k1, k2,
        maxpts, ncopy, ntraces;
    int debug = 0;
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
        if (debug) {
            LOG_DEBUGMSG("getData: ntraces: %d", ntraces);
        }
        if (ntraces == 0){continue;}
        // Open + read the data and attributes for this dataset 
        groupID = H5Gopen2(h5traceBuffer->fileID,
                           h5traceBuffer->dtGroupName[idt], H5P_DEFAULT);
        gain = memory_calloc64f(ntraces);
        work = traceBuffer_h5_readData(groupID, ntraces,
                                       &maxpts, &dt, &ts1, &ts2, gain, &ierr);
//LOG_MSG("                Request: t1:%f - t2:%f", t1, t2);
//LOG_MSG("h5 is buffered to hold: ts1:%f -ts2:%f", ts1, ts2);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error reading data");
            return -1;
        }
        // This concludes the fileIO
        status = H5Gclose(groupID);
        // Check what i just read
        if (status < 0)
        {
            LOG_ERRMSG("Error closing group %s",
                       h5traceBuffer->dtGroupName[idt]);
            return -1;
        }
        if (dt <= 0.0)
        {
            LOG_ERRMSG("Invalid sampling period %f", dt);
            return -1;
        } 
        if (maxpts < 1)
        {
            LOG_ERRMSG("Invalid number of points %d", maxpts);
            return -1; 
        }
        // Check what I just read
        if (t1 > ts2)
        {
            LOG_ERRMSG("%s", "Start time is too new");
            return -1;
        }
        if (t2 < ts1)
        {
            LOG_ERRMSG("%s", "End time is too old");
            return -1;
        }
        if (t2 > ts1 + (double) (maxpts - 1)*dt)
        {
            LOG_ERRMSG("%s", "Looking for data that's too current");
            return -1;
        }
        if (t1 < ts1)
        {
// printf("getData: start time is too old\n");
            LOG_ERRMSG("%s", "Error start time is too old");
            //return -1;
        }
        // Indices in chunk to read from
        i1 = (int) ((t1 - ts1)/dt + 0.5);
        i2 = (int) ((t2 - ts1)/dt + 0.5);
        i1 = MAX(i1, 0);          //if (i1 < 0){i1 = 0;}
        i2 = MIN(i2, maxpts - 1); //if (i2 > maxpts - 1){i2 = maxpts - 1;}
        // Indices to set (if i were more clever i could make this span a
        // superset of the times in memory - but then i'd fill it with NaNs
        // and risk messing up the offset computation - particularly at the
        // beginning where we have to know the position at the origin time)
        j1 = i1;
        j2 = i2;
        j1 = MAX(j1, 0);          //if (j1 < 0){j1 = 0;} 
        j2 = MIN(j2, maxpts - 1); //if (j2 > maxpts - 1){j2 = maxpts - 1;}
        if (i2 - i1 > j2 - j1)
        {
            LOG_ERRMSG("%s", "Error source array larger than dest array");
            return -1;
        }
        ncopy = (int) ((t2 - t1)/dt + 0.5) + 1;
        if (debug) {
            LOG_DEBUGMSG("getData: i1:%d, i2:%d, j1:%d, j2:%d, k1:%d, k2:%d, ncopy:%d, t1:%f, t2:%f, maxpts:%d",
                i1, i2, j1, j2, k1, k2, ncopy, t1, t2, maxpts);
        }
        /*
        ncopy = (int) ((t2 - t1)/dt + 0.5) + 1;
        j1 = 0;
        j2 = j1 + ncopy;
        */
        // Get the data onto the buffers
        for (k=k1; k<k2; k++)
        {
            // set info for this trace
            memory_free64f(&h5traceBuffer->traces[k].data);
            h5traceBuffer->traces[k].t1 = t1;
            h5traceBuffer->traces[k].ncopy = ncopy;
            h5traceBuffer->traces[k].gain = gain[k-k1];
            // Set the data to NaN's
            h5traceBuffer->traces[k].data
                = array_set64f(ncopy, (double) NAN, &ierr);
            // copy it
            ibeg = h5traceBuffer->traces[k].traceNumber*maxpts + i1;
            iend = ibeg + ncopy - 1; //i2;
            if (iend - ibeg + 1 != ncopy)
            {
                LOG_ERRMSG("Inconsistent copy size %d %d %d",
                           ibeg, iend, ncopy);
                return -1;
            }
            if (iend >= maxpts*ntraces)
            {
                LOG_ERRMSG("%s", "Critical error - work bounds exceeded");
                return -1;
            } 
            //memcpy(&h5traceBuffer->traces[k].data[j1], &work[ibeg],
            //       (size_t) (ncopy)*sizeof(double)); 
            //ierr = array_copy64f_work(ncopy, &work[ibeg],
            //                          &h5traceBuffer->traces[k].data[j1]);
            ierr = array_copy64f_work(ncopy, &work[ibeg],
                                      &h5traceBuffer->traces[k].data[0]);

            if (debug) {
                LOG_DEBUGMSG("getData: k:%d, %s.%s.%s.%s ibeg:%d iend:%d work[ibeg]:%f work[iend]:%f, traceNumber:%d",
                    k,
                    h5traceBuffer->traces[k].stnm, h5traceBuffer->traces[k].chan,
                    h5traceBuffer->traces[k].netw, h5traceBuffer->traces[k].loc,
                    ibeg, iend, work[ibeg], work[iend], h5traceBuffer->traces[k].traceNumber)
            }
            /*
            LOG_MSG("%s.%s.%s.%s  t1:%f ibeg:%d iend:%d ncopy:%d work[ibeg]:%f work[iend]:%f",
                h5traceBuffer->traces[k].stnm, h5traceBuffer->traces[k].chan,
                h5traceBuffer->traces[k].netw, h5traceBuffer->traces[k].loc,
                h5traceBuffer->traces[k].t1,
                ibeg, iend, ncopy,
                work[ibeg],
                work[iend]);
            */
            /*
                h5traceBuffer->traces[k].data[ibeg+1],
                h5traceBuffer->traces[k].data[iend-1]);
            */

        } // Loop on streams in this group
        // Release temporary memory 
        memory_free64f(&work);
        memory_free64f(&gain);
    } // Loop on sampling period groups
    return 0;
}
