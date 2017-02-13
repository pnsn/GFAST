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
 * @brief Reads the blocked data /Data in the Data group
 *
 * @param[in] groupID    handle for HDF5 group with data
 * @param[in] ntraces    number of traces I'm expecting to read
 *
 * @param[out] maxpts    number of points in block
 * @param[out] dt        sampling period (s)
 * @param[out] ts1       epochal start time (UTC seconds) of data chunk
 * @param[out] ts2       epochal end time (UTC seconds) of data chunk
 * @param[out] gain      gain for each trace [ntraces]
 * @param[out] ierr      0 indicates success
 *
 * @result data chunk [ntraces x maxpts] with leading dimension maxpts
 *
 * @author Ben Baker
 *
 */
double *traceBuffer_h5_readData(const hid_t groupID,
                                const int ntraces,
                                int *maxpts,
                                double *dt, double *ts1, double *ts2,
                                double *gain, int *ierr)
{
    const char *fcnm = "traceBuffer_h5_readData\0";
    double *work;
    hid_t attribute, dataSet, dataSpace, memSpace;
    hsize_t dims[2];
    herr_t status;
    int ntracesIn, nwork, rankIn;
    const int rank = 2;
    //------------------------------------------------------------------------//
    *ierr = 0;
    work = NULL;
    if (gain == NULL)
    {
        log_errorF("%s: Error gain cannot be NULL\n", fcnm);
        *ierr = 1;
        return work;
    }
    dataSet = H5Dopen2(groupID, "Data\0", H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    rankIn = H5Sget_simple_extent_ndims(dataSpace);
    if (rankIn != 2)
    {
        log_errorF("%s: Invalid number of dimensions %d\n", fcnm, rankIn);
        *ierr = 1;
        return work;
    }
    status = H5Sget_simple_extent_dims(dataSpace, dims, NULL);
    nwork = (int) (dims[0]*dims[1]);
    memSpace = H5Screate_simple(rank, dims, NULL);
    work = memory_calloc64f(nwork);
    status = H5Dread(dataSet, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                     H5P_DEFAULT, work);
    if (status < 0)
    {
        log_errorF("%s: Error loading data\n", fcnm);
        *ierr = 1;
        return work;
    }
    // Pick off the attributes while the dataset is open
    attribute = H5Aopen(dataSet, "SamplingPeriod\0", H5P_DEFAULT);
    status = H5Aread(attribute, H5T_NATIVE_DOUBLE, dt);
    status = H5Aclose(attribute);
    attribute = H5Aopen(dataSet, "StartTime\0", H5P_DEFAULT);
    status = H5Aread(attribute, H5T_NATIVE_DOUBLE, ts1);
    status = H5Aclose(attribute);
    attribute = H5Aopen(dataSet, "NumberOfTraces\0", H5P_DEFAULT);
    status = H5Aread(attribute, H5T_NATIVE_INT, &ntracesIn);
    if (ntracesIn != ntraces)
    {
        log_errorF("%s: Inconsistent number of traces %d %d\n",
                   fcnm, ntracesIn, ntraces);
        *ierr = 1;
        return work;
    }
    status = H5Aclose(attribute);
    attribute = H5Aopen(dataSet, "NumberOfPoints\0", H5P_DEFAULT);
    status = H5Aread(attribute, H5T_NATIVE_INT, maxpts);
    status = H5Aclose(attribute);
    *ts2 = *ts1 + (double) (*maxpts - 1)*(*dt);
    // close Data dataset 
    status  = H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);
    if (status < 0)
    {
        log_errorF("%s: Error closing data dataset\n", fcnm);
        *ierr = 1;
        return work;
    }
    // Likewise get the gain for each channel
    dataSet = H5Dopen2(groupID, "Gain\0", H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    dims[0] = (hsize_t) ntracesIn;
    memSpace = H5Screate_simple(1, dims, NULL);
    status = H5Dread(dataSet, H5T_NATIVE_DOUBLE, memSpace, dataSpace,
                     H5P_DEFAULT, gain);
    if (status < 0)
    {
        log_errorF("%s: Error loading gain\n", fcnm);
        *ierr = 1;
        return work;
    }
    // close Gain dataset
    status  = H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);
    if (status < 0)
    {
        log_errorF("%s: Error closing data dataset\n", fcnm);
        *ierr = 1;
        return work;
    }
    return work;
}
