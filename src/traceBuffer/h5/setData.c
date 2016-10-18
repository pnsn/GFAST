#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_hdf5.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*
static int update_dataSet(const hid_t groupID,
                          const char *dataSetName, 
                          int i1, int i2, const int npts,
                          const double *__restrict__ data);
*/

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
    double *dwork, *gains, *work;
    int *map, c1, c2, chunk, i, i1, i2, idt, ierr, ierrAll,
        indx, is, ishift, jndx, k, k1, k2, maxpts, nchunks, ncopy, ntraces;
    double dt, ts1, ts2;
    bool *lhaveData;
    hsize_t dims[2];
    hid_t groupID, dataSet, dataSpace;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Require both items are set
    ierr = 0;
    ierrAll = 0;
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
    // Not finished yet
    if (h5traceBuffer.ndtGroups > 1)
    {
        log_errorF("%s: Multiple dt groups not yet done\n", fcnm);
        return -1;
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
    for (idt=0; idt<h5traceBuffer.ndtGroups; idt++)
    {
        gains = NULL;
        work = NULL;
        k1 = h5traceBuffer.dtPtr[idt];
        k2 = h5traceBuffer.dtPtr[idt+1];
        ntraces = k2 - k1;
        if (ntraces == 0){continue;}
        // Open + read the data and attributes for this dataset 
        groupID = H5Gopen2(h5traceBuffer.fileID,
                           h5traceBuffer.dtGroupName[idt], H5P_DEFAULT);
        gains = ISCL_memory_calloc__double(ntraces);
        work = traceBuffer_h5_readData(groupID, ntraces,
                                       &maxpts, &dt, &ts1, &ts2, gains, &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error reading data\n", fcnm);
            return -1; 
        }
        // Check what i just read
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
        if (currentTime < ts2)
        {
            log_errorF("%s: Update time is less than most recent time %f %f\n",
                       fcnm, currentTime, ts2);
            return -1;
        }
        // Copy the old traces onto the new traces
        dwork = ISCL_array_set__double(maxpts*ntraces, (double) NAN, &ierr);
        ishift = (int) ((currentTime - ts2)/dt + 0.5);
        ncopy = maxpts - ishift;
printf("%d\n", ishift);
        for (k=0; k<ntraces; k++)
        {
            indx = k*maxpts + ishift;
            jndx = k*maxpts;
            ierr = ISCL__array_copy__double(ncopy, &work[indx], &dwork[jndx]);
            if (ierr != 0)
            {
                log_errorF("%s: Error copying trace %d %d\n",
                           fcnm, k+1, ntraces);
                return -1;
            }
        }
        // Update the times
        ts2 = currentTime;
        ts1 = currentTime - (double) (maxpts - 1)*dt;
        // Insert the new data
        for (k=k1; k<k2; k++)
        {
            if (!lhaveData[k]){continue;}
            i = map[k]; // points to tb2trace
            nchunks = tb2Data.traces[i].nchunks;
            c1 = tb2Data.traces[i].chunkPtr[0]; 
            c2 = tb2Data.traces[i].chunkPtr[nchunks];
            if (c2 - c1 <= 0 || c2 - c1 != tb2Data.traces[i].npts)
            {
                log_errorF("%s: npts to update is invalid %d %d %d\n",
                           fcnm, c1, c2, tb2Data.traces[i].npts);
                return -1;
            }
            for (chunk=0; chunk<nchunks; chunk++)
            {
                i1 = tb2Data.traces[i].chunkPtr[chunk];
                i2 = tb2Data.traces[i].chunkPtr[chunk];
                for (is=i1; is<i2; is++)
                {
                    // data expired
                    if (tb2Data.traces[i].times[is] < ts1){continue;}
                    // insert it
                    indx = k*maxpts
                         + (int) ((tb2Data.traces[i].times[is] - ts1)/dt + 0.5);
                    dwork[indx] = (double) tb2Data.traces[i].data[is];
                }
            } // Loop on data chunks 
        } // Loop on waveforms in this group
        // Write the new dataset
        dims[0] = (hsize_t) ntraces;
        dims[1] = (hsize_t) maxpts;
        dataSpace = H5Screate_simple(2, dims, NULL);
        dataSet = H5Dopen(groupID, "Data\0", H5P_DEFAULT); 
        status = H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                          H5P_DEFAULT, work);
        ierr = h5_write_attribute__double("StartTime\0", dataSet,
                                          1, &ts1);
        status = H5Sclose(dataSpace);
        status = H5Dclose(dataSet);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            log_errorF("%s: Error closing group %s\n",
                       fcnm, h5traceBuffer.dtGroupName[idt]);
            return -1; 
        }
        // free memory
        ISCL_memory_free__double(&gains);
        ISCL_memory_free__double(&work); 
        ISCL_memory_free__double(&dwork);
    }
    // Free memory
    ISCL_memory_free__int(&map);
    ISCL_memory_free__bool(&lhaveData);
    return ierrAll;
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
/*
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
*/
