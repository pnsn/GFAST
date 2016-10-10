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

static int udpate_dataSet(const hid_t groupID,
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
    double *dwork;
    int maxpts, nwork;
    int *map, i, ierr, k;
    double dt, gain, ts1, ts2;
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
        dwork = NULL;
        // Open the group for reading/writing
        groupID = H5Gopen2(h5traceBuffer.fileID,
                           h5traceBuffer.traces[i].groupName, H5P_DEFAULT);
        // Get the scalars describing this dataset
        ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345, (double) NAN,
                                               &maxpts,
                                               &dt, &gain, &ts1, &ts2);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting scalars!\n", fcnm);
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
        // Get the current time
      
        // Push the data
        if (lhaveData[k])
        {
            nwork = (int) ((currentTime - tb2Data.traces[k].times[0])
                          /h5traceBuffer.traces[i].dt + 0.5) + 1;
            dwork = ISCL_array_set__double(nwork, (double) NAN, &ierr);


            ISCL_memory_free__double(&dwork);
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
static int udpate_dataSet(const hid_t groupID,
                          const char *dataSetName,  
                          int i1, int i2, const int npts,
                          const double *__restrict__ data)
{
    const char *fcnm = "udpate_dataSet\0";
    hid_t dataSetID, dataSpace, memSpace;
    herr_t status;
    hsize_t count[1], chunkDims[1], dims[1], offset[1];
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
    if (H5Lexists(groupID, dataSetName, H5P_DEFAULT != 1))
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
    chunkDims[0] = dims[0];
    memSpace = H5Screate_simple(rank, chunkDims, NULL); 
    // Select HDF5 chunk
    status = 0;
    offset[0] = (hsize_t) i1;
    count[0] = (hsize_t) npts;
    status = H5Sselect_hyperslab(dataSpace, H5S_SELECT_SET, offset, NULL,
                                 count, NULL);
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
