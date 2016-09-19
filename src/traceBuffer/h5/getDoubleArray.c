#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"

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
