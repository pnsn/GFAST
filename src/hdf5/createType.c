#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Creates the peak displacement data type
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5_createType__peakDisplacementData(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5_createType__peakDisplacementData\0";
    hid_t dataType, string64Type;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "peakDisplacementDataStructure\0",
                  H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string64Type = H5Tcopy(H5T_C_S1);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_peakDisplacementData_struct));
    ierr += H5Tinsert(dataType, "stationName\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, stnm),
                      string64Type);
    ierr += H5Tinsert(dataType, "peakDisplacement\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, pd),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "weight\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, wt),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "siteLatitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lat),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "siteLongitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lon),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "siteElevation\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_alt),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "isMasked\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lmask),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "isActive\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lactive),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "peakDisplacementDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create pd data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(string64Type);
    ierr += H5Tclose(dataType);
    return ierr;
}
