#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "gfast_hdf5.h"
#include "iscl/array/array.h"
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
    double *dwork, *gains, *work;
    int *map, c1, c2, chunk, i, i1, i2, idt, ierr, ierrAll,
        indx, is, ishift, jndx, k, k1, k2, maxpts, nchunks, ncopy, ntraces;
    double dt, ts1, ts2;
    bool *lhaveData;
    hsize_t dims[2];
    hid_t attribute, groupID, dataSet, dataSpace;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Require both items are set
    ierr = 0;
    ierrAll = 0;
    if (!tb2Data.linit || !h5traceBuffer.linit)
    {
        if (!tb2Data.linit){LOG_ERRMSG("%s", "Error tb2Data not set");}
        if (!h5traceBuffer.linit){LOG_ERRMSG("%s", "h5traceBuffer not set");}
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
        LOG_ERRMSG("%s", "Multiple dt groups not yet done");
        return -1;
    }
    // Make a map from the HDF5 trace buffer to the tracebuffer2 data
    map = array_set32i(h5traceBuffer.ntraces, -1, &ierr);
    lhaveData = memory_calloc8l(h5traceBuffer.ntraces);
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
        gains = memory_calloc64f(ntraces);
        work = traceBuffer_h5_readData(groupID, ntraces,
                                       &maxpts, &dt, &ts1, &ts2, gains, &ierr);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error reading data");
            return -1; 
        }
        // Check what i just read
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
        if (currentTime < ts2)
        {
            LOG_ERRMSG("Update time is less than most recent time %f %f",
                       currentTime, ts2);
            return -1;
        }
        // Copy the old traces onto the new traces
        //dwork = array_set64f(maxpts*ntraces, (double) NAN, &ierr);
        // MTH: quick hack to prevent mem leak on line 205
        dwork = array_set64f(2*maxpts*ntraces, (double) NAN, &ierr);
        ishift = (int) ((currentTime - ts2)/dt + 0.5);
        ncopy = maxpts - ishift;
printf("ishift=%d\n", ishift);
LOG_DEBUGMSG("currentTime:%f - ts2:%f = ishift=%d", currentTime, ts2, ishift);
        for (k=0; k<ntraces; k++)
        {
            indx = k*maxpts + ishift;
            jndx = k*maxpts;
            ierr = array_copy64f_work(ncopy, &work[indx], &dwork[jndx]);
            if (ierr != 0)
            {
                LOG_ERRMSG("Error copying trace %d %d", k+1, ntraces);
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
                LOG_ERRMSG("npts to update is invalid %d %d %d",
                           c1, c2, tb2Data.traces[i].npts);
                return -1;
            }
            for (chunk=0; chunk<nchunks; chunk++)
            {
                i1 = tb2Data.traces[i].chunkPtr[chunk];
                i2 = tb2Data.traces[i].chunkPtr[chunk+1];
                for (is=i1; is<i2; is++)
                {
                    // data expired
                    if (tb2Data.traces[i].times[is] < ts1){continue;}
                    // insert it
                    indx = k*maxpts
                         + (int) ((tb2Data.traces[i].times[is] - ts1)/dt + 0.5);
                    //printf("k=%d indx=%d set dwork[indx]\n", k, indx);
                    dwork[indx] = (double) tb2Data.traces[i].data[is];
                    //LOG_DEBUGMSG("i:%d is:%d time:%f insert dwork[%d]=%f", 
                             //i, is, tb2Data.traces[i].times[is], indx, dwork[indx]);
                             //
                    LOG_DEBUGMSG("   Insert tb2Data %s.%s.%s.%s t:%f (npts:%d) (int) data:%d into dwork",
                                  tb2Data.traces[i].stnm, tb2Data.traces[i].chan,
                                  tb2Data.traces[i].netw, tb2Data.traces[i].loc,
                                  tb2Data.traces[i].times[is],
                                  tb2Data.traces[i].npts, tb2Data.traces[i].data[is]);
                    /*
                    LOG_DEBUGMSG("%s.%s.%s.%s t:%f (t-ts1):%f k:%d k*maxpts:%d indx:%d",
                                  tb2Data.traces[i].stnm, tb2Data.traces[i].chan,
                                  tb2Data.traces[i].netw, tb2Data.traces[i].loc,
                                  tb2Data.traces[i].times[is], (tb2Data.traces[i].times[is] - ts1),
                                  k, (k*maxpts), indx);

                    LOG_DEBUGMSG("   Insert tb2Data %s.%s.%s.%s t:%f data:%d into dwork[indx=%d]",
                                  tb2Data.traces[i].stnm, tb2Data.traces[i].chan,
                                  tb2Data.traces[i].netw, tb2Data.traces[i].loc,
                                  tb2Data.traces[i].times[is], tb2Data.traces[i].data[is], indx);
                    */
                }
            } // Loop on data chunks 
        } // Loop on waveforms in this group
        // Write the new dataset
        dims[0] = (hsize_t) ntraces;
        dims[1] = (hsize_t) maxpts;
        dataSpace = H5Screate_simple(2, dims, NULL);
        dataSet = H5Dopen(groupID, "Data\0", H5P_DEFAULT); 
        status = H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                          H5P_DEFAULT, dwork);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error writing data chunk");
            return -1;
        }
        attribute = H5Aopen(dataSet, "StartTime\0", H5P_DEFAULT);
        status = H5Awrite(attribute, H5T_NATIVE_DOUBLE, &ts1);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error updating start time");
            return -1;
        }
        status = H5Aclose(attribute);
        status = H5Sclose(dataSpace);
        status = H5Dclose(dataSet);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            LOG_ERRMSG("Error closing group %s",
                       h5traceBuffer.dtGroupName[idt]);
            return -1; 
        }
        // free memory
        memory_free64f(&gains);
        memory_free64f(&work); 
        memory_free64f(&dwork);
    }
    // Free memory
    memory_free32i(&map);
    memory_free8l(&lhaveData);
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
        if (i2 < i1){LOG_ERRMSG("%s", "Error i2 < i1!");}
        if (npts != i2 - i1 + 1)
        {
            LOG_ERRMSG("%s", "Error npts != i2 - i1 + 1");
        }
        if (data == NULL){LOG_ERRMSG("%s", "data is NULL");}
        return -1;
    }
    if (H5Lexists(groupID, dataSetName, H5P_DEFAULT) != 1)
    {
        LOG_ERRMSG("Dataset %s does not exist", dataSetName); 
        return -1;
    }
    // Open the dataspace
    dataSetID = H5Dopen(groupID, dataSetName, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSetID);
    if (H5Sget_simple_extent_ndims(dataSpace) != rank)
    {
        LOG_ERRMSG("%s", "Invalid rank");
        status =-1;
        goto ERROR1;
    }
    status = H5Sget_simple_extent_dims(dataSpace, dims, NULL);
    if (dims[0] < (hsize_t) npts)
    {
        LOG_ERRMSG("Too many points to write %d %d!",
                    npts, (int) dims[0]);
        status =-1;
        goto ERROR1;
    }
    if (dims[0] < (hsize_t) (i1 + npts))
    {
        LOG_ERRMSG("Trying to write past end of data %d %d %d!\n",
                   i1, npts, (int) dims[0]);
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
        LOG_ERRMSG("%s", "Error selecting hyperslab");
        status =-1;
        goto ERROR2;
    }
    // Write the data to that space
    status = H5Dwrite(dataSetID, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                      H5P_DEFAULT, data);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Error writing data");
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
