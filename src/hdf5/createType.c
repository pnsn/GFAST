#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_hdf5.h"
#include "iscl/log/log.h"

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
herr_t hdf5_createType__peakDisplacementData(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__peakDisplacementData\0";
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
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlenCData);
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(string64Type);
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
herr_t hdf5_createType__pgdResults(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__pgdResults\0";
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
    ierr += H5Tinsert(dataType, "PGDVarianceReduction\0",
                      HOFFSET(struct h5_pgdResults_struct, mpgd_vr),
                      vlenDData);
    ierr += H5Tinsert(dataType, "PGDVarianceReduction_ScaledByIQR\0",
                      HOFFSET(struct h5_pgdResults_struct, dep_vr_pgd),
                      vlenDData); 
    ierr += H5Tinsert(dataType, "PGDEstimates\0",
                      HOFFSET(struct h5_pgdResults_struct, UP),
                      vlenDData);
    ierr += H5Tinsert(dataType, "PGDObservations\0",
                      HOFFSET(struct h5_pgdResults_struct, UPinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceDepth\0",
                      HOFFSET(struct h5_pgdResults_struct, srcDepths),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceReceiverDistance\0",
                      HOFFSET(struct h5_pgdResults_struct, srdist),
                      vlenDData);
    ierr += H5Tinsert(dataType, "interQuartileRange\0",
                      HOFFSET(struct h5_pgdResults_struct, iqr),
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
    ierr += H5Tclose(dataType);
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
herr_t hdf5_createType__offsetData(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__offsetData\0";
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
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlenCData);
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(string64Type);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the hypocenter data type
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__hypocenter(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__hypocenter\0";
    hid_t dataType, string128Type;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "hypocenterStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string128Type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string128Type, 128);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_hypocenter_struct));
    ierr += H5Tinsert(dataType, "eventName\0",
                      HOFFSET(struct h5_hypocenter_struct, eventid),
                      string128Type);
    ierr += H5Tinsert(dataType, "latitude\0",
                      HOFFSET(struct h5_hypocenter_struct, lat),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "longitude\0",
                      HOFFSET(struct h5_hypocenter_struct, lon),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "depth\0",
                      HOFFSET(struct h5_hypocenter_struct, dep),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "magnitude\0",
                      HOFFSET(struct h5_hypocenter_struct, mag),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "originTime\0",
                      HOFFSET(struct h5_hypocenter_struct, time),
                      H5T_NATIVE_DOUBLE);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "hypocenterStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create offset data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(string128Type);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the CMT results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__cmtResults(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__cmtResults\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "cmtResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_cmtResults_struct));
    ierr += H5Tinsert(dataType, "L2norm\0",
                      HOFFSET(struct h5_cmtResults_struct, l2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "percentDoubleCouple\0",
                      HOFFSET(struct h5_cmtResults_struct, pct_dc),
                      vlenDData);
    ierr += H5Tinsert(dataType, "ObjectiveFunction\0",
                      HOFFSET(struct h5_cmtResults_struct, objfn),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentTensors\0",
                      HOFFSET(struct h5_cmtResults_struct, mts),
                      vlenDData);
    ierr += H5Tinsert(dataType, "strikeFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, str1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "dipFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, dip1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "rakeFaultPlane1\0",
                      HOFFSET(struct h5_cmtResults_struct, rak1),
                      vlenDData);
    ierr += H5Tinsert(dataType, "strikeFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, str2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "dipFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, dip2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "rakeFaultPlane2\0",
                      HOFFSET(struct h5_cmtResults_struct, rak2),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentMagnitudes\0",
                      HOFFSET(struct h5_cmtResults_struct, Mw),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceDepths\0",
                      HOFFSET(struct h5_cmtResults_struct, srcDepths),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, EN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, NN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upEstimates\0",
                      HOFFSET(struct h5_cmtResults_struct, UN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Einp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Ninp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upObservedOffset\0",
                      HOFFSET(struct h5_cmtResults_struct, Uinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "siteUsed\0",
                      HOFFSET(struct h5_cmtResults_struct, lsiteUsed),
                      vlenIData);

    ierr += H5Tinsert(dataType, "optimumIndex\0",
                      HOFFSET(struct h5_cmtResults_struct, opt_indx),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfGridsearchDepths\0",
                      HOFFSET(struct h5_cmtResults_struct, ndeps),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_cmtResults_struct, nsites),
                      H5T_NATIVE_INT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to pack type\n", fcnm);
        return ierr;
    }
    // Commit it
    ierr = H5Tcommit(group_id, "cmtResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create cmt results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the finite fault fault plane structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__faultPlane(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__faultPlane\0";
    hid_t dataType, vlenDData, vlenIData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "faultPlaneStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    vlenIData = H5Tvlen_create(H5T_NATIVE_INT);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_faultPlane_struct));
    ierr += H5Tinsert(dataType, "longitudeVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, lon_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "latitudeVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, lat_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "depthVertices\0",
                      HOFFSET(struct h5_faultPlane_struct, dep_vtx),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchEastingUTM\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_xutm),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchNorthingUTM\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_yutm),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPatchDepth\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_alt),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, strike),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultDip\0",
                      HOFFSET(struct h5_faultPlane_struct, dip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultLength\0",
                      HOFFSET(struct h5_faultPlane_struct, length),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultWidth\0",
                      HOFFSET(struct h5_faultPlane_struct, width),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, sslip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongDip\0",
                      HOFFSET(struct h5_faultPlane_struct, dslip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongStrikeUncertainty\0",
                      HOFFSET(struct h5_faultPlane_struct, sslip_unc),
                      vlenDData);
    ierr += H5Tinsert(dataType, "slipAlongDipUncertainty\0",
                      HOFFSET(struct h5_faultPlane_struct, dslip_unc),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, EN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, NN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upEstimateOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, UN),
                      vlenDData);
    ierr += H5Tinsert(dataType, "eastObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Einp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "northObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Ninp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "upObservedOffset\0",
                      HOFFSET(struct h5_faultPlane_struct, Uinp),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPointerStructure\0",
                      HOFFSET(struct h5_faultPlane_struct, fault_ptr),
                      vlenIData);
    ierr += H5Tinsert(dataType, "maxObservations\0",
                      HOFFSET(struct h5_faultPlane_struct, maxobs),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfSitesUsed\0",
                      HOFFSET(struct h5_faultPlane_struct, nsites_used),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaltPatchesAlongStrike\0",
                      HOFFSET(struct h5_faultPlane_struct, nstr),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaultPatchesAlongDip\0",
                      HOFFSET(struct h5_faultPlane_struct, ndip),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "faultPlaneStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create fault plane structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlenIData);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the finite fault results structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__ffResults(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__ffResults\0";
    hid_t dataType, faultType, vlenDData, vlenFault;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //
    // Nothing to do
    if (H5Lexists(group_id, "finiteFaultResultsStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    if (H5Lexists(group_id, "faultPlaneStructure\0", H5P_DEFAULT) == 0)
    {
        log_warnF("%s: Making fault plane structure\n", fcnm);
        ierr = GFAST_hdf5_createType__faultPlane(group_id);
        if (ierr != 0)
        {
            log_errorF("%s: ERror making fault plane structure\n", fcnm);
            return ierr;
        }
    }
    faultType = H5Topen(group_id, "faultPlaneStructure\0", H5P_DEFAULT);
    vlenFault = H5Tvlen_create(faultType);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_ffResults_struct));
    ierr += H5Tinsert(dataType, "faultPlanes\0",
                      HOFFSET(struct h5_ffResults_struct, fp),
                      vlenFault);
    ierr += H5Tinsert(dataType, "varianceReduction\0",
                      HOFFSET(struct h5_ffResults_struct, vr),
                      vlenDData);
    ierr += H5Tinsert(dataType, "momentMagnitude\0",
                      HOFFSET(struct h5_ffResults_struct, Mw),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPlaneStrikes\0",
                      HOFFSET(struct h5_ffResults_struct, str),
                      vlenDData);
    ierr += H5Tinsert(dataType, "faultPlaneDips\0",
                      HOFFSET(struct h5_ffResults_struct, dip),
                      vlenDData);
    ierr += H5Tinsert(dataType, "sourceLatitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_lat),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "sourceLongitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_lon),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "sourceDepth\0",
                      HOFFSET(struct h5_ffResults_struct, SA_dep),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "sourceMagnitude\0",
                      HOFFSET(struct h5_ffResults_struct, SA_mag),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "preferredFaultPlane\0",
                      HOFFSET(struct h5_ffResults_struct,
                              preferred_fault_plane),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfFaultPlanes\0",
                      HOFFSET(struct h5_ffResults_struct, nfp),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "finiteFaultResultsStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create ff results structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(faultType);
    ierr += H5Tclose(vlenFault);
    ierr += H5Tclose(vlenDData);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the three component GPS data structure 
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__waveform3CData(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__waveform3CData\0";
    hid_t dataType, string64Type, vlenCData, vlenDData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //  
    // Nothing to do
    if (H5Lexists(group_id, "waveform3CDataStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    // String data type 
    string64Type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string64Type, 64);
    vlenCData = H5Tvlen_create(string64Type);
    vlenDData = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_waveform3CData_struct));
    ierr += H5Tinsert(dataType, "Network\0",
                      HOFFSET(struct h5_waveform3CData_struct, netw),
                      vlenCData);
    ierr += H5Tinsert(dataType, "Station\0", 
                      HOFFSET(struct h5_waveform3CData_struct, stnm),
                      vlenCData);
    ierr += H5Tinsert(dataType, "Channels\0",
                      HOFFSET(struct h5_waveform3CData_struct, chan),
                      vlenCData);
    ierr += H5Tinsert(dataType, "Location\0",
                      HOFFSET(struct h5_waveform3CData_struct, loc),
                      vlenCData);
    ierr += H5Tinsert(dataType, "UpPrecisePointPosition\0",
                      HOFFSET(struct h5_waveform3CData_struct, ubuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "NorthPrecisePointPosition\0",
                      HOFFSET(struct h5_waveform3CData_struct, nbuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "EastPrecisePointPosition\0",
                      HOFFSET(struct h5_waveform3CData_struct, ebuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "EpochalTimes\0",
                      HOFFSET(struct h5_waveform3CData_struct, tbuff),
                      vlenDData);
    ierr += H5Tinsert(dataType, "gain\0",
                      HOFFSET(struct h5_waveform3CData_struct, gain),
                      vlenDData);
    ierr += H5Tinsert(dataType, "samplingPeriod\0",
                      HOFFSET(struct h5_waveform3CData_struct, dt),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "stationLatitude\0",
                      HOFFSET(struct h5_waveform3CData_struct, sta_lat),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "stationLongitude\0",
                      HOFFSET(struct h5_waveform3CData_struct, sta_lon),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "stationElevation\0",
                      HOFFSET(struct h5_waveform3CData_struct, sta_alt),
                      H5T_NATIVE_DOUBLE);
    ierr += H5Tinsert(dataType, "maxNumberOfPoints\0",
                      HOFFSET(struct h5_waveform3CData_struct, maxpts),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "numberOfPoints\0",
                      HOFFSET(struct h5_waveform3CData_struct, npts),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "lskipPGD\0",
                      HOFFSET(struct h5_waveform3CData_struct, lskip_pgd),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "lskipCMT\0",
                      HOFFSET(struct h5_waveform3CData_struct, lskip_cmt),
                      H5T_NATIVE_INT);
    ierr += H5Tinsert(dataType, "lskipFF\0",
                      HOFFSET(struct h5_waveform3CData_struct, lskip_ff),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "waveform3CDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create 3C data structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlenCData);
    ierr += H5Tclose(vlenDData);
    ierr += H5Tclose(string64Type);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the GFAST data structure
 *
 * @param[in] group_id    HDF5 group_id handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t hdf5_createType__gpsData(hid_t group_id)
{
    const char *fcnm = "hdf5_createType__gpsData\0";
    hid_t dataType, threeCdataType, vlen3CData;
    herr_t ierr = 0;
    //------------------------------------------------------------------------//
    //  
    // Nothing to do
    if (H5Lexists(group_id, "gpsDataStructure\0", H5P_DEFAULT) != 0)
    {
        return ierr;
    }
    if (H5Lexists(group_id, "waveform3CDataStructure\0", H5P_DEFAULT) == 0)
    {
        log_warnF("%s: Making 3C data structure\n", fcnm);
        ierr = GFAST_hdf5_createType__waveform3CData(group_id);
        if (ierr != 0)
        {
            log_errorF("%s: ERror making 3C data structure\n", fcnm);
            return ierr;
        }
    }
    threeCdataType = H5Topen(group_id, "waveform3CDataStructure\0",
                             H5P_DEFAULT);
    vlen3CData = H5Tvlen_create(threeCdataType);
    // Build the data structure
    dataType = H5Tcreate(H5T_COMPOUND,
                         sizeof(struct h5_gpsData_struct));
    ierr += H5Tinsert(dataType, "threeComponentDataStreams\0",
                      HOFFSET(struct h5_gpsData_struct, data),
                      vlen3CData);
    ierr += H5Tinsert(dataType, "numberOfSites\0",
                      HOFFSET(struct h5_gpsData_struct, stream_length),
                      H5T_NATIVE_INT);
    // Commit it
    ierr = H5Tcommit(group_id, "gpsDataStructure\0", dataType,
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (ierr != 0)
    {
        log_errorF("%s: Failed to create gpsData structure\n", fcnm);
        return ierr;
    }
    ierr += H5Tclose(dataType);
    ierr += H5Tclose(vlen3CData);
    ierr += H5Tclose(threeCdataType);
    return ierr;
}
