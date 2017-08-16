#include <stdio.h>
#include <stdlib.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

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
                                    const int scalar)
{
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
        LOG_ERRMSG("%s", "Error citem must be defined");
        return -1;
    }
    if (H5Lexists(groupID, citem, H5P_DEFAULT) == 1){return 0;} // Nothing to do
    scalars[0] = scalar;
    // Update an existing scalar
    if (H5Lexists(groupID, citem, H5P_DEFAULT) == 1)
    {
         dataSetID = H5Dopen(groupID, citem, H5P_DEFAULT);
         dataSpace = H5Dget_space(dataSetID);
    }
    // Make a brand new scalar
    else
    {
        dataSpace = H5Screate_simple(rank, dims, NULL);
        dataSetID = H5Dcreate2(groupID, citem, H5T_NATIVE_INT,
                               dataSpace,
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }
    status = H5Dwrite(dataSetID, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, scalars);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Failed to write dataset");
        ierr = 1;
    }
    status  = H5Sclose(dataSpace);
    status += H5Dclose(dataSetID);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Failed to close dataspace or dataset");
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
        LOG_ERRMSG("%s", "Error citem must be defined");
        return -1;
    }
    scalars[0] = scalar;
    // Update an existing scalar
    if (H5Lexists(groupID, citem, H5P_DEFAULT) == 1)
    {
         dataSetID = H5Dopen(groupID, citem, H5P_DEFAULT);
         dataSpace = H5Dget_space(dataSetID);
    }
    // Make a brand new scalar
    else
    {
         dataSpace = H5Screate_simple(rank, dims, NULL);
         dataSetID = H5Dcreate2(groupID, citem, H5T_NATIVE_DOUBLE,
                                dataSpace, 
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }
    status = H5Dwrite(dataSetID, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, scalars);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Failed to write dataset");
        ierr = 1;
    }
    status  = H5Sclose(dataSpace);
    status += H5Dclose(dataSetID);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Failed to close dataspace or dataset");
        ierr = 1;
    }
    return ierr;
}
