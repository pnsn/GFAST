#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

static int update_dataSet(const hid_t groupID,
                          const char *dataSetName, 
                          int i1, int i2, const int npts,
                          const double *__restrict__ data);

/*!
 * @brief Sets the data in the HDF5 file from the data on the tb2Data 
 *        buffer.  If any data is late then it will be filled with NaN's
 *        in the HDF5 file.
 *
 * @param[in] currentTime    current time (UTC seconds since epoch) to which to
 *                           update the HDF5 data buffers. 
 * @param[in] tb2Data        holds the tracebuffer2 data to be written to 
 *                           disk.  the SNCLs on tb2Data should correspond
 *                           to the SNCLs on h5TraceBuffer and the data should
 *                           be in temporal order. 
 * @param[in] h5traceBuffer  holds the HDF5 group names for each trace and
 *                           HDF5 file details
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 */
int traceBuffer_h5_setData(const double currentTime,
                           struct tb2Data_struct tb2Data,
                           struct h5traceBuffer_struct h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_setData\0";
    char dataBuffer1[64], dataBuffer2[64], dtBuffer[64];
    double *dwork;
    int maxpts, nwork;
    int *map, c1, c2, chunk, i, i1, i2, ierr, j1, j2,
        k, k1, kupd, nchunks, npupd;
    double dt, gain, tmax, ts1, ts1Upd, ts1Use, ts2, ts2Use;
    bool *lhaveData;
    hid_t groupID;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Require both items are set
    if (!tb2Data.linit || !h5traceBuffer.linit)
    {
        if (!tb2Data.linit){log_errorF("%s: Error tb2Data not set\n", fcnm);}
        if (!h5traceBuffer.linit)
        {
            log_errorF("%s: h5traceBuffer not set\n", fcnm);
        }
        return -1;
    }
    // Nothing to do
    if (h5traceBuffer.ntraces < 1)
    {
        return 0;
    }
    // Make a map from the HDF5 trace buffer to the tracebuffer2 data
    map = ISCL_array_set__int(h5traceBuffer.ntraces, -1, &ierr);
    lhaveData = ISCL_memory_calloc__bool(h5traceBuffer.ntraces);
    for (i=0; i<h5traceBuffer.ntraces; i++)
    {
        // Take an educated guess
        if (i < tb2Data.ntraces)
        {
            if ((strcasecmp(h5traceBuffer.traces[i].netw,
                                  tb2Data.traces[i].netw) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].stnm,
                                  tb2Data.traces[i].stnm) == 0) && 
                (strcasecmp(h5traceBuffer.traces[i].chan,
                                  tb2Data.traces[i].chan) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].loc,
                                  tb2Data.traces[i].loc)  == 0))
            {
                map[i] = i;
                if (tb2Data.traces[i].npts > 0)
                {
                    lhaveData[i] = true; 
                }
                goto NEXT_TRACE; 
            }
        }
        // Hunt through the tracebuffers
        for (k=0; k<tb2Data.ntraces; k++)
        {
            if ((strcasecmp(h5traceBuffer.traces[i].netw,
                                  tb2Data.traces[k].netw) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].stnm,
                                  tb2Data.traces[k].stnm) == 0) &&  
                (strcasecmp(h5traceBuffer.traces[i].chan,
                                  tb2Data.traces[k].chan) == 0) &&
                (strcasecmp(h5traceBuffer.traces[i].loc,
                                  tb2Data.traces[k].loc)  == 0)) 
            {   
                map[i] = k;
                if (tb2Data.traces[i].npts > 0)
                {
                    lhaveData[i] = true;
                }
                goto NEXT_TRACE;
            }
        }
NEXT_TRACE:;
    } 
    // Push the current H5 archive to the current time and, if possible, add
    // the new data
    for (k=0; k<h5traceBuffer.ntraces; k++)
    {
        dt = 0.0; 
        maxpts = 0;
        ts1Use = (double) NAN;
        ts2Use = (double) NAN;
        memset(dataBuffer1, 0, sizeof(dataBuffer1));
        memset(dataBuffer2, 0, sizeof(dataBuffer2));
        dwork = NULL;
        // Open the group for reading/writing
        groupID = H5Gopen2(h5traceBuffer.fileID,
                           h5traceBuffer.traces[k].groupName, H5P_DEFAULT);
        // Loop in case I have to do an update - in which case ts1 and ts2
        // would be reset
        for (kupd=0; kupd<2; kupd++)
        {
            // Get the scalars describing this dataset
            ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345,
                                                   (double) NAN,
                                                   &maxpts,
                                                   &dt, &gain, &ts1, &ts2);
            if (ierr != 0)
            {
                log_errorF("%s: Error getting scalars %s!\n",
                           fcnm, h5traceBuffer.traces[i].groupName);
                return -1;
            }
            if (dt <= 0.0 || fabs(gain) < 1.e-15 || maxpts < 1)
            {
                if (dt <= 0.0)
                {
                    log_errorF("%s: Sampling period is invalid\n", fcnm);
                }
                if (fabs(gain) < 1.e-15)
                {
                    log_errorF("%s: Trace division by zero coming\n", fcnm);
                }
                if (maxpts < 1)
                {
                    log_errorF("%s: The buffers are empty\n", fcnm);
                }
                goto CLOSE_GROUP;
            }
            // Require the time makes sense
            if (currentTime < ts1 && currentTime < ts2)
            {
                log_errorF("%s: Packet is too old to be updated\n", fcnm);
                goto CLOSE_GROUP;
            } 
            // Set the databuffers and names
            memset(dataBuffer1, 0, sizeof(dataBuffer1));
            memset(dataBuffer2, 0, sizeof(dataBuffer2));
            if (ts1 < ts2)
            {
                ts1Use = ts1;
                ts2Use = ts2;
                strcpy(dtBuffer, "Buffer1StartTime\0");
                strcpy(dataBuffer1, "dataBuffer1\0");
                strcpy(dataBuffer2, "dataBuffer2\0");
            }
            else
            {
                ts1Use = ts2;
                ts2Use = ts1;
                strcpy(dtBuffer, "Buffer2StartTime\0");
                strcpy(dataBuffer1, "dataBuffer2\0");
                strcpy(dataBuffer2, "dataBuffer1\0");
            }
            // Need to do a push - pop the oldest dataset
            tmax = ts2Use + dt*(double) (maxpts - 1);
            if (currentTime > tmax)
            {
                // Update the times
                ts1Upd = ts2Use + (double) maxpts*dt;
                traceBuffer_h5_setDoubleScalar(groupID,
                                               dtBuffer,
                                               ts1Upd);
                //dwork = ISCL_array_set__double(maxpts, (double) k, &ierr);
                dwork = ISCL_array_set__double(maxpts, (double) NAN, &ierr);
                ierr = traceBuffer_h5_setDoubleScalar(groupID, dtBuffer,
                                                      ts1Upd);
                if (ierr != 0)
                {
                    log_errorF("%s: Error updating start time %s %s\n",
                               fcnm, h5traceBuffer.traces[k].groupName,
                               dtBuffer);
                }
                ierr = update_dataSet(groupID, dataBuffer1, 0, maxpts-1,
                                      maxpts, dwork);
                ISCL_memory_free__double(&dwork);
                if (ierr != 0)
                {
                    log_errorF("%s: Error setting NaN data %s %d\n",
                               fcnm, h5traceBuffer.traces[k].groupName,
                               dataBuffer1);
                    goto CLOSE_GROUP;
                }
            }
            // no push - done with this loop
            else
            {
                break;
            }
        } // Loop on start times
        // verify i've succeeded in getting my times straight
        if (isnan(ts1Use) || isnan(ts2Use) || fabs(dt) < 1.e-14)
        {
            log_errorF("%s: Failed to set start times\n", fcnm);
            goto CLOSE_GROUP;
        }
        // Update the data
        if (lhaveData[k])
        {
            // Get the response as a double onto data 
            i = map[k]; // points to tb2trace
            nchunks = tb2Data.traces[i].nchunks;
            c1 = tb2Data.traces[i].chunkPtr[0]; 
            c2 = tb2Data.traces[i].chunkPtr[nchunks];
            if (c2 - c1 <= 0 || c2 - c1 != tb2Data.traces[i].npts)
            {
                log_errorF("%s: npts to update is invalid %d %d %d\n",
                           fcnm, c1, c2, tb2Data.traces[i].npts);
                goto CLOSE_GROUP;
            }
            dwork = ISCL_memory_calloc__double(tb2Data.traces[i].npts);
            #pragma omp simd
            for (i1=0; i1<tb2Data.traces[i].npts; i1++)
            {
                dwork[i1] = (double) tb2Data.traces[i].data[i1];
                dwork[i1] = (double) tb2Data.traces[i].times[i1];
            }
            // Loop on the distinct messages
            for (chunk=0; chunk<nchunks; chunk++)
            {
                c1 = tb2Data.traces[i].chunkPtr[chunk];
                c2 = tb2Data.traces[i].chunkPtr[chunk+1] - 1;
                i1 = (int) ((tb2Data.traces[i].times[c1] - ts2Use)/dt + 0.5);
                i2 = (int) ((tb2Data.traces[i].times[c2] - ts2Use)/dt + 0.5);
                npupd = c2 - c1 + 1;
                if (npupd <= 0)
                {
                    log_errorF("%s: no points to update\n", fcnm);
                    goto CLOSE_GROUP;
                }
                // This isn't plausible
                if (i1 >= maxpts || i2 >= maxpts)
                {
                    log_errorF("%s: %d or %d exceeds space %d\n",
                               fcnm, i1, i2, maxpts);
                    goto CLOSE_GROUP;
                }
                if (i1 > i2)
                {
                    log_errorF("%s: Data is out of order %d %d\n",
                               fcnm, i1, i2);
                    goto CLOSE_GROUP;
                }
                // Data is too old to use
                if (tb2Data.traces[i].times[c2] < ts1Use)
                {
                    log_warnF("%s: Data is too old - skipping\n", fcnm);
                    continue;
                }
                // Update is entirely in second buffer
                if (i1 >= 0)
                {
                    nwork = i2 - i1 + 1;
                    ierr = update_dataSet(groupID, dataBuffer2, i1, i2,
                                          nwork, &dwork[c1]);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Failed current update\n", fcnm);
                        goto CLOSE_GROUP;
                    }
                }
                // Update starts in previous buffer and carries into this one
                else if (i1 < 0 && i2 >= 0)
                {
                    // Update is pretty old - but parts of it are valid
                    j1 = (int) ((tb2Data.traces[i].times[c1]-ts1Use)/dt + 0.5);
                    j2 = maxpts - 1;
                    k1 = 0;
                    if (j1 < 0)
                    {
                        j1 = 0;
                        k1 = c1 - j1; //- a negaitve is a positive
                        log_warnF("%s: This isn't checked", fcnm);
                    }
                    nwork = j2 - j1 + 1;
                    ierr = update_dataSet(groupID, dataBuffer1, j1, j2,
                                          nwork, &dwork[k1]);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Failed overlap update 1\n", fcnm);
                        goto CLOSE_GROUP;
                    }
                    j1 = 0;
                    j2 = (int) ((tb2Data.traces[i].times[c2]-ts2Use)/dt + 0.5);
                    k1 = k1 + nwork;
                    nwork = j2 - j1 + 1;
                    ierr = update_dataSet(groupID, dataBuffer2, j1, j2,
                                          nwork, &dwork[k1]);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Failed overlap update 1\n", fcnm);
                        goto CLOSE_GROUP;
                    }
                }
                // Update is entirely in previous buffer
                else if (i1 < 0 && i2 < 0)
                {
                    j1 = (int) ((tb2Data.traces[i].times[c1]-ts1Use)/dt + 0.5);
                    j2 = (int) ((tb2Data.traces[i].times[c1]-ts1Use)/dt + 0.5);
                    k1 = c1;
                    if (j1 < 0)
                    {
                        j1 = 0;
                        k1 = c1 - j1; //- a negative is a positive
                        log_warnF("%s: This isn't checked either\n", fcnm);
                    }
                    nwork = j2 - j1 + 1;
                    ierr = update_dataSet(groupID, dataBuffer1, j1, j2,
                                          nwork, &dwork[k1]);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Failed previous update %d %d %d\n",
                                   fcnm, j1, j2, nwork);
                        goto CLOSE_GROUP;
                    }
                }
                // Unclassified case
                else
                {
                    log_errorF("%s: Unclassified case %d %d\n", fcnm, i1, i2);
                }
            } // Loop on messages 
            ISCL_memory_free__double(&dwork);
/*
            nwork = (int) ((currentTime - tb2Data.traces[k].times[0])
                          /h5traceBuffer.traces[i].dt + 0.5) + 1;
            dwork = ISCL_array_set__double(nwork, (double) NAN, &ierr);
            ISCL_memory_free__double(&dwork);
*/
        }
        // Close the group
CLOSE_GROUP:;
        status = H5Gclose(groupID);
        if (status < 0)
        {
            log_errorF("%s: Error closing group\n", fcnm);
            continue;
        }
    }
    // Free memory
    ISCL_memory_free__int(&map);
    ISCL_memory_free__bool(&lhaveData);
    return 0;
}
//============================================================================//
/*!
 * @brief Updates the subset of data in the existing dataSetName
 *        to dataSet[i1:i2] inclusive
 *
 * @param[in] groupID      HDF5 group handle containing dataSetID
 * @param[in] dataSetName  null terminated name of HDF5 dataset
 * @param[in] i1           first index in HDF5 (C numbered)
 * @param[in] i2           last index in HDF5 to write data (C numbered)
 * @param[in] npts         number of data points to write (should = i2 - i1 + 1)
 * @param[in] data         dataset to write [npts]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
static int update_dataSet(const hid_t groupID,
                          const char *dataSetName,  
                          int i1, int i2, const int npts,
                          const double *__restrict__ data)
{
    const char *fcnm = "update_dataSet\0";
    hid_t dataSetID, dataSpace, memSpace;
    herr_t status;
    hsize_t block[1], count[1], dims[1], offset[1], stride[1];
    const int rank = 1; // Data is 1 dimensional
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (npts == 0){return 0;}
    // Check the inputs
    if (i2 < i1 || npts != i2 - i1 + 1 || data == NULL)
    {
        if (i2 < i1){log_errorF("%s: Error i2 < i1!\n", fcnm);}
        if (npts != i2 - i1 + 1)
        {
            log_errorF("%s: Error npts != i2 - i1 + 1\n", fcnm);
        }
        if (data == NULL){log_errorF("%s: data is NULL\n", fcnm);}
        return -1;
    }
    if (H5Lexists(groupID, dataSetName, H5P_DEFAULT) != 1)
    {
        log_errorF("%s: Dataset %s does not exist\n", fcnm, dataSetName); 
        return -1;
    }
    // Open the dataspace
    dataSetID = H5Dopen(groupID, dataSetName, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSetID);
    if (H5Sget_simple_extent_ndims(dataSpace) != rank)
    {
        log_errorF("%s: Invalid rank\n", fcnm);
        status =-1;
        goto ERROR1;
    }
    status = H5Sget_simple_extent_dims(dataSpace, dims, NULL);
    if (dims[0] < (hsize_t) npts)
    {
        log_errorF("%s: Too many points to write %d %d!\n",
                   fcnm, npts, (int) dims[0]);
        status =-1;
        goto ERROR1;
    }
    if (dims[0] < (hsize_t) (i1 + npts))
    {
        log_errorF("%s: Trying to write past end of data %d %d %d!\n",
                   fcnm, i1, npts, (int) dims[0]);
        status =-1;
        goto ERROR1;
    }
    dims[0] = (hsize_t) npts;
    memSpace = H5Screate_simple(rank, dims, NULL); 
    // Select HDF5 chunk
    status = 0;
    offset[0] = (hsize_t) i1;
    stride[0] = 1;
    count[0] = (hsize_t) npts;
    block[0] = 1;
    status = H5Sselect_hyperslab(dataSpace, H5S_SELECT_SET, offset, stride,
                                 count, block);
    if (status < 0)
    {
        log_errorF("%s: Error selecting hyperslab\n", fcnm);
        status =-1;
        goto ERROR2;
    }
    // Write the data to that space
    status = H5Dwrite(dataSetID, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                      H5P_DEFAULT, data);
    if (status < 0)
    {
        log_errorF("%s: Error writing data\n", fcnm);
        status =-1;
        goto ERROR2;
    }
ERROR2:;
    status = H5Sclose(memSpace);
ERROR1:;
    status = H5Sclose(dataSpace);
    status = H5Dclose(dataSetID);
    return status;
}
