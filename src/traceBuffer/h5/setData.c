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
 * @param[in] h5traceBuffer   contains the data and streams to be updated in the
 *                            HDF5 file
 *
 * @result 0 indicates success
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
    // Never initialized
    if (!h5traceBuffer->linit)
    {
        log_errorF("%s: Error h5traceBuffer not initialized\n", fcnm);
        return -1;
    }
    // No data
    if (h5traceBuffer->ntraces == 0){return 0;}
    // Nothing to update
    lnoData = true;
    for (i=0; i<h5traceBuffer->ntraces; i++)
    {
        if (h5traceBuffer->traces[i].ncopy > 0)
        {
            lnoData = false;
            break;
        } 
    }
    if (lnoData){return 0;}
    // Get the min and max times to copy
    ierr = 0;
    ierrAll = 0;
    dt0 = (double) NAN;
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
        ierr = GFAST_traceBuffer_h5_getScalars(groupID, -12345, (double) NAN,
                                               &maxpts,
                                               &dtH5, &ts1, &ts2);
        if (fabs(dtH5 - dt)/dtH5 > 1.e-10)
        {
            log_errorF("%s: Error saved sampling periods from data %f %f\n",
                       fcnm, dt, dtH5);
            ierrAll = ierrAll + 1;
        }
        if (isnan(dt0)){dt0 = dt;}
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
int udpate_dataSet(const hid_t groupID,
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
