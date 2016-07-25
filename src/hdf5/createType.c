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
herr_t GFAST_HDF5__createType__peakDisplacementData(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__peakDisplacementData\0";
    hid_t dataType, vlenCData, vlenDData, vlenIData, string64Type;
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
    H5Tset_size(string64Type, 64);
    vlenCData = H5Tvlen_create(string64Type); 
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_peakDisplacementData_struct));
    ierr += H5Tinsert(dataType, "stationName\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, stnm),
                      vlenCData);
    ierr += H5Tinsert(dataType, "peakDisplacement\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, pd),
                      vlenDData);
    ierr += H5Tinsert(dataType, "weight\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, wt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLatitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lat),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLongitude\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_lon),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteElevation\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, sta_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "isMasked\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lmask),
                      vlenIData);
    ierr += H5Tinsert(dataType, "isActive\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, lactive),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_peakDisplacementData_struct, nsites),
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
        log_errorF("%s: Failed to create pgd data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenCData);
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(string64Type);
    ierr += H5Tclose(dataType);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the PGD results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t GFAST_HDF5__createType__pgdResults(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__pgdResults\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "pgdResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_pgdResults_struct));
    ierr += H5Tinsert(dataType, "Magnitude\0",
                      HOFFSET(struct h5_pgdResults_struct, mpgd),
                      vlenDData);
    ierr += H5Tinsert(dataType, "ObjectiveFunction\0",
                      HOFFSET(struct h5_pgdResults_struct, mpgd_vr),
                      vlenDData);
    ierr += H5Tinsert(dataType, "PGDEstimates\0",
                      HOFFSET(struct h5_pgdResults_struct, UP),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceDepth\0",
                      HOFFSET(struct h5_pgdResults_struct, srcDepths),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteUsed\0",
                      HOFFSET(struct h5_pgdResults_struct, lsiteUsed),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfGridsearchDepths\0",
                      HOFFSET(struct h5_pgdResults_struct, ndeps),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_pgdResults_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "pgdResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create pgd results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    return ierr;
}
//============================================================================//
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
herr_t GFAST_HDF5__createType__offsetData(hid_t group_id)
{
    const char *fcnm = "GFAST_HDF5__createType__offsetData\0";
    hid_t dataType, vlenCData, vlenDData, vlenIData, string64Type;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "offsetDataStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string64Type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string64Type, 64);
    vlenCData = H5Tvlen_create(string64Type);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_offsetData_struct));
    ierr += H5Tinsert(dataType, "stationName\0",
                      HOFFSET(struct h5_offsetData_struct, stnm),
                      vlenCData);
    ierr += H5Tinsert(dataType, "verticalOffset\0",
                      HOFFSET(struct h5_offsetData_struct, ubuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northOffset\0",
                      HOFFSET(struct h5_offsetData_struct, nbuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastOffset\0",
                      HOFFSET(struct h5_offsetData_struct, ebuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "verticalWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wtu),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wtn),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastWeight\0",
                      HOFFSET(struct h5_offsetData_struct, wte),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLatitude\0",
                      HOFFSET(struct h5_offsetData_struct, sta_lat),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteLongitude\0",
                      HOFFSET(struct h5_offsetData_struct, sta_lon),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteElevation\0",
                      HOFFSET(struct h5_offsetData_struct, sta_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "isMasked\0",
                      HOFFSET(struct h5_offsetData_struct, lmask),
                      vlenIData);
    ierr += H5Tinsert(dataType, "isActive\0",
                      HOFFSET(struct h5_offsetData_struct, lactive),
                      vlenIData);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_offsetData_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "offsetDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create offset data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    return ierr;
}
