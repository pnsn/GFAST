#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>
#include "iscl/log/log.h"

/*!
 * @brief Sets the integer scalar dataset
 *
 * @param[in] groupID     HDF5 group handle
 * @param[in] citem       NULL terminated dataset name to be written in group
 * @param[in] scalar      scalar data to write to a dataset
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int traceBuffer_h5_setIntegerScalar(const hid_t groupID,
                                    const char *citem,
                                    const double scalar)
{
    const char *fcnm = "traceBuffer_h5_setIntegerScalar\0";
    int scalars[1];
    hid_t dataSetID, dataSpace;
    herr_t status;
    int ierr;
    const hsize_t dims[1] = {1};
    const int rank = 1;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (citem == NULL)
    {
        log_errorF("%s: Error citem must be defined\n", fcnm);
        return -1;
    }
    if (H5Lexists(groupID, citem, H5P_DEFAULT) == 1){return 0;} // Nothing to do
    scalars[0] = scalar;
    dataSpace = H5Screate_simple(rank, dims, NULL);
    dataSetID = H5Dcreate2(groupID, citem, H5T_NATIVE_INT,
                           dataSpace,
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataSetID, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, scalars);
    if (status < 0)
    {
        log_errorF("%s: Failed to write dataset\n", fcnm);
        ierr = 1;
    }
    status  = H5Sclose(dataSpace);
    status += H5Dclose(dataSetID);
    if (status < 0)
    {
        log_errorF("%s: Failed to close dataspace or dataset\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Sets the double scalar dataset
 *
 * @param[in] groupID     HDF5 group handle
 * @param[in] citem       NULL terminated dataset name to be written in group
 * @param[in] scalar      scalar data to write to a dataset
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int traceBuffer_h5_setDoubleScalar(const hid_t groupID,
                                   const char *citem,
                                   const double scalar)
{
    const char *fcnm = "traceBuffer_h5_setDoubleScalar\0";
    double scalars[1];
    hid_t dataSetID, dataSpace;
    herr_t status;
    int ierr;
    const hsize_t dims[1] = {1};
    const int rank = 1;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (citem == NULL)
    {
        log_errorF("%s: Error citem must be defined\n", fcnm);
        return -1;
    }
    if (H5Lexists(groupID, citem, H5P_DEFAULT) == 1){return 0;} // Nothing to do
    scalars[0] = scalar;
    dataSpace = H5Screate_simple(rank, dims, NULL);
    dataSetID = H5Dcreate2(groupID, citem, H5T_NATIVE_DOUBLE,
                           dataSpace, 
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dataSetID, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, scalars);
    if (status < 0)
    {
        log_errorF("%s: Failed to write dataset\n", fcnm);
        ierr = 1;
    }
    status  = H5Sclose(dataSpace);
    status += H5Dclose(dataSetID);
    if (status < 0)
    {
        log_errorF("%s: Failed to close dataspace or dataset\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
