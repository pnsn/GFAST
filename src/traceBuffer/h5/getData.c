#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
/*
double traceBuffer_h5_getData__fromH5(hid_t groupID)
{
    const char *app = {"/Buffer1StartTime\0",
                       "/Buffer2StartTime\0",
                       "/MaxNumberOfPoints\0",
                       "/SamplingPeriod\0"};
} 
*/

int traceBuffer_h5_getDoubleArray(const hid_t groupID,
                                  const int i1, const int i2,
                                  const char *citem,
                                  const double traceNaN, 
                                  const int nwork,
                                  double *__restrict__ work)
{
    const char *fcnm = "traceBuffer_h5_getDoubleArray\0";
    int i, ierr, ncopy, rank;
    hid_t dataSet, dataSpace, memSpace;
    hsize_t count[1], dims[1], offset[1];
    herr_t status;
    //------------------------------------------------------------------------// 
    ierr = 0;
    ncopy = i2 - i1 + 1;
    if (ncopy < 1 || work == NULL || ncopy > nwork)
    {
        if (ncopy < 1){log_errorF("%s: No data to copy!\n", fcnm);}
        if (work == NULL){log_errorF("%s: Destination is NULL!\n", fcnm);}
        if (ncopy > nwork)
        {
            log_errorF("%s: Insufficient workspace!\n", fcnm); 
        }
        return -1;
    }
    offset[0] = i1;
    count[0] = ncopy;
    dataSet = H5Dopen(groupID, citem, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    rank = H5Sget_simple_extent_ndims(dataSpace);
    if (rank != 1)
    {
        log_errorF("%s: Invalid rank for array data\n", fcnm);
        return -1;
    }
    status = H5Sget_simple_extent_dims(dataSpace, dims, NULL);
    if (status < 0)
    {
        log_errorF("%s: Failed getting dimension!\n", fcnm);
        return -1;
    }
    memSpace = H5Screate_simple(rank, dims, NULL);
    status = H5Sselect_hyperslab(dataSpace, H5S_SELECT_SET, offset, 
                                 NULL, count, NULL);
    offset[0] = i1;
    count[0] = ncopy;
    status = H5Sselect_hyperslab(memSpace, H5S_SELECT_SET, offset, 
                                 NULL, count, NULL);
    status = H5Dread(dataSet, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                     H5P_DEFAULT, work);
    status += H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);    
    if (status != 0)
    {
        log_errorF("%s: Error reading dataset!\n", fcnm);
        ierr = 1;
        for (i=0; i<ncopy; i++)
        {
            work[i] = traceNaN;
        }
    }
    return ierr;
}

/*!
 * @brief Gets a double precision scalar dataset in the group in the HDF5 file
 */
double traceBuffer_h5_getDoubleScalar(const hid_t groupID,
                                      const char *citem,
                                      const double scalarNaN,
                                      int *ierr)
{
    const char *fcnm = "traceBuffer_h5_getDoubleScalar\0";
    double scalar;
    hid_t dataSet, dataSpace, memSpace;
    herr_t status;
    const hsize_t dims[1] = {1};
    *ierr = 0;
    status = 0; 
    scalar = scalarNaN;
    dataSet = H5Dopen(groupID, citem, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    memSpace = H5Screate_simple(1, dims, NULL);
    status = H5Dread(dataSet, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                     H5P_DEFAULT, &scalar);
    if (status != 0)
    {
        log_errorF("%s: Getting scalar\n", fcnm);
        scalar = scalarNaN;
    }
    status += H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);
    if (status != 0){*ierr = 1;}
    return scalar;
}
//============================================================================//
/*!
 * @brief Gets an integer scalar dataset in the group in the HDF5 file
 */
int traceBuffer_h5_getIntegerScalar(const hid_t groupID,
                                    const char *citem,
                                    const int scalarNaN,
                                    int *ierr)
{
    const char *fcnm = "traceBuffer_h5_getIntegerScalar\0";
    int scalar;
    hid_t dataSet, dataSpace, memSpace;
    herr_t status;
    const hsize_t dims[1] = {1};
    *ierr = 0;
    status = 0;
    scalar = scalarNaN;
    dataSet = H5Dopen(groupID, citem, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    memSpace = H5Screate_simple(1, dims, NULL);
    status = H5Dread(dataSet, H5T_NATIVE_INT, memSpace, dataSpace,
                     H5P_DEFAULT, &scalar);
    if (status != 0)
    {
        log_errorF("%s: Getting scalar\n", fcnm);
        scalar = scalarNaN;
    }
    status += H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);
    if (status != 0){*ierr = 1;}
    return scalar;
}

int traceBuffer_h5_getScalars(const hid_t groupID,
                              const int intNaN,
                              const double doubleNaN,
                              int *maxpts,
                              double *dt, double *ts1, double *ts2)
{
    const char *fcnm = "traceBuffer_h5_getScalars\0";
    int ierr, ierr1;
    ierr = 0;
    *maxpts = traceBuffer_h5_getIntegerScalar(groupID,
                                              "MaxNumberOfPoints\0",
                                              intNaN, &ierr1);
    ierr = ierr + ierr1;
    *dt  = traceBuffer_h5_getDoubleScalar(groupID,
                                          "SamplingPeriod\0",
                                          doubleNaN, &ierr1);
    ierr = ierr + ierr1;
    *ts1 = traceBuffer_h5_getDoubleScalar(groupID,
                                          "Buffer1StartTime\0",
                                          doubleNaN, &ierr1);
    ierr = ierr + ierr1;
    *ts2 = traceBuffer_h5_getDoubleScalar(groupID,
                                          "Buffer2StartTime\0",
                                          doubleNaN, &ierr1);
    ierr = ierr + ierr1;
    if (ierr != 0)
    {
        log_errorF("%s: Error getting scalars\n", fcnm);
    }
    return ierr;
}


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
        ierr = traceBuffer_h5_getScalars(groupID, -12345, -NAN,
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
        if (t2 > ts1 && t2 > ts2)
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
                ierr = traceBuffer_h5_getDoubleArray(groupID,
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
                ierr = traceBuffer_h5_getDoubleArray(groupID,
                                                     i1, i2, 
                                                     dataBuffer1,
                                                     NAN,
                                                     nc1, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error getting array\n", fcnm);
                }
                ierr = traceBuffer_h5_getDoubleArray(groupID,
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
/*
        // Get the data in order
        if (ts1 < ts2)
        {
            // First buffer
            if (t1 < ts2)
            {
                i1 = (int) ((t1 - ts1)/dt + 0.5);
                i2 = maxpts - 1;
                // All on first buffer
                if (t2 < ts2)
                {
                    i2 = (int) ((t2 - ts1)/dt + 0.5);
                    // Get data from [i1:i2]
                    ierr = traceBuffer_h5_getDoubleArray(groupID,
                                                         i1, i2,
                                                         "dataBuffer1\0",
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
                    j2 = fmin(maxpts-1, (int) ((t2 - ts2)/dt + 0.5));
                    nc1 = i2 - i1 + 1;
                    nc2 = j2 - j1 + 1;
                    // Get data from [i1:i2] then [j1:j2]
                    ierr = traceBuffer_h5_getDoubleArray(groupID,
                                                         i1, i2, 
                                                         "dataBuffer1\0",
                                                         NAN,
                                                         nc1, work);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Error getting array\n", fcnm);
                    }
                    ierr = traceBuffer_h5_getDoubleArray(groupID,
                                                         j1, j2,
                                                         "dataBuffer2\0",
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
        }
printf("%s not done!\n", fcnm);
ierr = 1;
return ierr;
        }
        //h5traceBuffer->data = (double *)calloc(npcopy, sizeof(double));
//printf("%d\n", ncopy);
*/
        status = status + H5Gclose(groupID);
//getchar();
    }
    if (status != 0)
    {
        log_errorF("%s: Error accessing data\n", fcnm);
    }
    return 0;
}
