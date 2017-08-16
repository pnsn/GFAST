#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

static double traceBuffer_h5_getDoubleScalar(const hid_t groupID,
                                             const char *citem,
                                             const double scalarNaN,
                                             int *ierr);
static int traceBuffer_h5_getIntegerScalar(const hid_t groupID,
                                           const char *citem,
                                           const int scalarNaN,
                                           int *ierr);
/*!
 * @brief Gets the scalar data describing the trace stored in HDF5
 *
 * @param[in] groupID    HDF5 group ID handle
 * @param[in] intNaN     NaN if an integer cannot be found
 * @param[in] doubleNaN  NaN if a double cannot be found
 *
 * @param[out] maxpts    max number of data points per buffer
 * @param[out] dt        sampling period (s)
 * @param[out] gain      channel gain
 * @param[out] ts1       epochal start time of dataBuffer1 (UTC seconds)
 * @param[out] ts2       epochal start time of dataBuffer2 (UTC seconds)
 *
 * @result 0 indicates success
 *
 */
int traceBuffer_h5_getScalars(const hid_t groupID,
                              const int intNaN,
                              const double doubleNaN,
                              int *maxpts,
                              double *dt, double *gain,
                              double *ts1, double *ts2)
{
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
    *gain = traceBuffer_h5_getDoubleScalar(groupID,
                                           "Gain\0",
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
        LOG_ERRMSG("%s", "Error getting scalars");
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Reads a double scalar dataset in the group in the HDF5 file
 *
 * @param[in] groupID     HDF5 groupID handle
 * @param[in] citem       name of scalar dataset
 * @param[in] scalarNaN   default value for double if dataset cannot
 *                        be found or read
 *
 * @result on successful exit this is the desired double number otherwise
 *         it is scalarNaN
 *
 * @author Ben Baker (ISTI)
 *
 */
static double traceBuffer_h5_getDoubleScalar(const hid_t groupID,
                                             const char *citem,
                                             const double scalarNaN,
                                             int *ierr)
{
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
        LOG_ERRMSG("%s", "Getting scalar");
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
 * @brief Reads an integer scalar dataset in the group in the HDF5 file
 *
 * @param[in] groupID     HDF5 groupID handle
 * @param[in] citem       name of scalar dataset
 * @param[in] scalarNaN   default value for integer if dataset cannot
 *                        be found or read
 *
 * @result on successful exit this is the desired integer number otherwise
 *         it is scalarNaN
 *
 * @author Ben Baker (ISTI)
 *
 */
static int traceBuffer_h5_getIntegerScalar(const hid_t groupID,
                                           const char *citem,
                                           const int scalarNaN,
                                           int *ierr)
{
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
        LOG_ERRMSG("%s", "Getting scalar");
        scalar = scalarNaN;
    }
    status += H5Sclose(memSpace);
    status += H5Sclose(dataSpace);
    status += H5Dclose(dataSet);
    if (status != 0){*ierr = 1;}
    return scalar;
}
