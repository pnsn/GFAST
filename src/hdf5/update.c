#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "gfast_hdf5.h"
#include "gfast_core.h"
#include "iscl/os/os.h"
/*!
 * @brief Initializes the current directory for this GFAST iteration.
 *
 * @param[in] adir     Archive directory.  If NULL then the current working
 *                     directory will be used.
 * @param[in] evid     Event ID.
 *
 * @retval If -1 an error occurred.
 * @retval Otherwise, this is the is iteration number for writing GFAST's
 *         history.
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_updateGetIteration(const char *adir,
                            const char *evid,
                            const double epoch)
{
    const char *group_root = "/GFAST_History\0";
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX], iterGroup[256];
    hid_t fileID, groupID;
    herr_t status;
    int ierr, k;
    //------------------------------------------------------------------------//
    //
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting filename");
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    k = h5_n_group_members(group_root, fileID) + 1;
    memset(iterGroup, 0, sizeof(iterGroup));
    sprintf(iterGroup, "%s_%d", item_root, k);
    if (h5_item_exists(fileID, iterGroup))
    {
        LOG_ERRMSG("%s", "Error group shouldn't exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    ierr = h5_create_group(fileID, iterGroup);
    // Write the epochal time as an attribute
    groupID = H5Gopen2(fileID, iterGroup, H5P_DEFAULT);
    status = h5_write_attribute__double("epoch\0", groupID, 1, &epoch);
    if (status < 0)
    {
        LOG_ERRMSG("%s", "Error writing attribute");
    }
    status = H5Gclose(groupID);
    status = h5_close(fileID);
    return k;
}
//============================================================================//
/*!
 * @brief Writes the triggering hypocenter for the next iteration of GFAST
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_updateHypocenter(const char *adir,
                          const char *evid,
                          const int h5k,
                          struct GFAST_shakeAlert_struct hypo)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX];
    struct h5_hypocenter_struct h5_hypo;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char hypoGroup[256];
    int ierr;
    hsize_t dimInfo[1] = {1};
    const int rank = {1};
    //------------------------------------------------------------------------//
    //
    // Initialize
    memset(&h5_hypo, 0, sizeof(struct h5_hypocenter_struct));
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting filename");
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Verify iteration group exists 
    memset(hypoGroup, 0, sizeof(hypoGroup));
    sprintf(hypoGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, hypoGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing
    groupID = H5Gopen2(fileID, hypoGroup, H5P_DEFAULT);
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copyHypocenter(COPY_DATA_TO_H5,
                                            &hypo,
                                            &h5_hypo);
    dataType = H5Topen(groupID, "/DataStructures/hypocenterStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "triggeringHypocenter\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_hypo);
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing hypocenter");
    }
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes the next iteration of the PGD estimation for the given event.
 *
 * @param[in] adir      Archive directory.  If NULL then this will use the
 *                      current working directory.
 * @param[in] evid      Event ID. 
 * @param[in] h5k       Iteration number.  This can be obtained from: 
 *                      hdf5_updateGetIteration.
 * @param[in] pgd_data  Peak ground displacement data used in this iteration
 *                      to archive.
 * @param[in] pgd       PGD results from this iteration to archive.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_updatePGD(const char *adir,
                   const char *evid,
                   const int h5k,
                   struct GFAST_peakDisplacementData_struct pgd_data,
                   struct GFAST_pgdResults_struct pgd)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX];
    struct h5_peakDisplacementData_struct h5_pgd_data;
    struct h5_pgdResults_struct h5_pgd;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char pgdGroup[256];
    int ierr;
    hsize_t dimInfo[1] = {1};
    const int rank = {1};
    //------------------------------------------------------------------------//
    //
    // Initialize
    memset(&h5_pgd_data, 0, sizeof(struct h5_peakDisplacementData_struct));
    memset(&h5_pgd, 0, sizeof(struct h5_pgdResults_struct));
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting filename");
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(pgdGroup, 0, 256*sizeof(char));
    sprintf(pgdGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, pgdGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing
    groupID = H5Gopen2(fileID, pgdGroup, H5P_DEFAULT); 
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copyPeakDisplacementData(COPY_DATA_TO_H5,
                                                      &pgd_data,
                                                      &h5_pgd_data);
    dataType = H5Topen(groupID,
                       "/DataStructures/peakDisplacementDataStructure\0",
                        H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "pgdData\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_pgd_data);
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing PGD data");
    }
    // Copy and write the results 
    ierr = ierr +  GFAST_hdf5_copyPGDResults(COPY_DATA_TO_H5,
                                             &pgd, &h5_pgd);
    dataType = H5Topen(groupID, "/DataStructures/pgdResultsStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "pgdResults\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_pgd);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing PGD results");
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error closing HDF5 data items");
    } 
    // Free the space
    ierr = GFAST_hdf5_memory_freePGDData(&h5_pgd_data);
    ierr = GFAST_hdf5_memory_freePGDResults(&h5_pgd);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes the next iteration of the CMT estimation for the given event.
 *
 * @param[in] adir      Archive directory.  If NULL then this will use the
 *                      current working directory.
 * @param[in] evid      Event ID. 
 * @param[in] h5k       Iteration number.  This can be obtained from: 
 *                      hdf5_updateGetIteration.
 * @param[in] cmt_data  Offset data used in this iteration to archive.
 * @param[in] cmt       CMT results from this iteration to archive.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_updateCMT(const char *adir,
                   const char *evid,
                   const int h5k,
                   struct GFAST_offsetData_struct cmt_data,
                   struct GFAST_cmtResults_struct cmt)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX];
    struct h5_offsetData_struct h5_cmt_data;
    struct h5_cmtResults_struct h5_cmt;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char cmtGroup[256];
    int ierr;
    hsize_t dimInfo[1] = {1};
    const int rank = {1};
    //------------------------------------------------------------------------//
    //
    // Initialize
    memset(&h5_cmt_data, 0, sizeof(struct h5_offsetData_struct));
    memset(&h5_cmt, 0, sizeof(struct h5_cmtResults_struct));
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting filename");
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(cmtGroup, 0, 256*sizeof(char));
    sprintf(cmtGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, cmtGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, cmtGroup, H5P_DEFAULT);
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copyOffsetData(COPY_DATA_TO_H5,
                                            &cmt_data,
                                            &h5_cmt_data);
    dataType = H5Topen(groupID, "/DataStructures/offsetDataStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "cmtData\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_cmt_data);
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error writing CMT data");
    }
    // Copy and write the results
    ierr = ierr +  GFAST_hdf5_copyCMTResults(COPY_DATA_TO_H5,
                                             &cmt, &h5_cmt);
    dataType = H5Topen(groupID, "/DataStructures/cmtResultsStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "cmtResults\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_cmt); 
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error writing Greens functions");
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error closing HDF5 data items");
    }
    // Free the space
    ierr = GFAST_hdf5_memory_freeOffsetData(&h5_cmt_data);
    ierr = GFAST_hdf5_memory_freeCMTResults(&h5_cmt);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_updateXMLMessage(const char *adir,
                          const char *evid,
                          const int h5k,
                          const char *messageName, char *message)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX], msgGroup[256];
    hid_t fileID, groupID;
    int ierr;
    // There's nothing to do
    if (message == NULL){return 0;}
    if (strlen(message) < 1)
    {
        LOG_ERRMSG("%s", "Message is empty");
        return -1;
    }
    if (messageName == NULL)
    {
        LOG_ERRMSG("%s", "Message name cannot be NULL");
        return -1;
    }
    if (strlen(messageName) < 1)
    {
        LOG_ERRMSG("%s", "Message name cannot be blank");
        return -1;
    }
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error setting filename");
        return -1; 
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!\n", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(msgGroup, 0, 256*sizeof(char));
    sprintf(msgGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, msgGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, msgGroup, H5P_DEFAULT);
    if (h5_item_exists(groupID, messageName))
    {
        LOG_ERRMSG("%s exists; skipping", messageName);
        ierr = 1;
        goto ERROR;
    }
    ierr = h5_write_array__chars(messageName, groupID, 1, &message);
    if (ierr != 0)
    {
        LOG_ERRMSG("Error writing %s", messageName);
        ierr =-1;
    }
    // Close the group and file
ERROR:;
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_updateFF(const char *adir,
                  const char *evid,
                  const int h5k,
                  struct GFAST_ffResults_struct ff)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX], dataName[256];
    struct h5_offsetData_struct h5_ff_data;
    struct h5_ffResults_struct h5_ff;
    struct h5_faultPlane_struct h5_fp;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char ffGroup[256];
    int i, ierr;
    hsize_t dimInfo[1] = {1};
    const int rank = {1};
    //------------------------------------------------------------------------//
    //  
    // Initialize
    memset(&h5_ff_data, 0, sizeof(struct h5_offsetData_struct));
    memset(&h5_ff, 0, sizeof(struct h5_ffResults_struct));
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error setting filename");
        return -1; 
    }   
    if (!os_path_isfile(h5fl))
    {   
        LOG_ERRMSG("Error file %s does not exist!\n", h5fl);
        return -1; 
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(ffGroup, 0, 256*sizeof(char));
    sprintf(ffGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, ffGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, ffGroup, H5P_DEFAULT);
    // Copy and write the results
    ierr = ierr + GFAST_hdf5_copyFFResults(COPY_DATA_TO_H5,
                                           &ff, &h5_ff);
    dataType = H5Topen(groupID, "/DataStructures/finiteFaultResultsStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "finiteFaultResults\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_ff);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing Greens functions");
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error closing HDF5 data items");
    }
    // Write the faults individually for h5py
    dataType = H5Topen(groupID, "/DataStructures/faultPlaneStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    /* TODO: this is a kludge for h5py - retry with newer version */
    for (i=0; i<ff.nfp; i++)
    {
        memset(dataName, 0, 256*sizeof(char));
        sprintf(dataName, "faultPlane_%d", i+1);
        dataSet = H5Dcreate(groupID, dataName, dataType,
                            dataSpace,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        GFAST_hdf5_copyFaultPlane(COPY_DATA_TO_H5, &ff.fp[i], &h5_fp);
        ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                        H5P_DEFAULT, &h5_fp);
        ierr = hdf5_memory_freeFaultPlane(&h5_fp);
    }
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s", "Error closing HDF5 data items");
    }
    // Free the space
    ierr = GFAST_hdf5_memory_freeFFResults(&h5_ff);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_update_gpsData(const char *adir,
                        const char *evid,
                        const int h5k,
                        struct GFAST_data_struct data)
{
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX];
    struct h5_gpsData_struct h5_gpsData;
    //struct h5_waveform3CData_struct h5_3cdata;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char gpsGroup[256];
    int ierr;
    hsize_t dimInfo[1] = {1};
    const int rank = {1};
    //------------------------------------------------------------------------//
    //  
    // Initialize
    memset(&h5_gpsData, 0, sizeof(struct h5_gpsData_struct));
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting filename");
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        LOG_ERRMSG("Error file %s does not exist!", h5fl);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(gpsGroup, 0, 256*sizeof(char));
    sprintf(gpsGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, gpsGroup))
    {
        LOG_ERRMSG("%s", "Error group should exist");
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, gpsGroup, H5P_DEFAULT);
    // Copy and write the results
    ierr = hdf5_copyGPSData(COPY_DATA_TO_H5,
                            &data, &h5_gpsData);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error copying GPS data!");
        h5_close(fileID);
        return -1;
    }
    if (!h5_item_exists(groupID, "/DataStructures/gpsDataStructure\0"))
    {
        LOG_ERRMSG("%s", "Error gps data type does not exist");
        ierr = h5_close(fileID);
        return -1;
    }
    dataType = H5Topen(groupID, "/DataStructures/gpsDataStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    dataSet   = H5Dcreate(groupID, "gpsData\0", dataType,
                          dataSpace,
                          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, &h5_gpsData);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing Greens functions");
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    ierr = ierr + hdf5_memory_freeGPSData(&h5_gpsData);
    ierr = ierr + H5Gclose(groupID);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error closing HDF5 data items");
    }
    ierr = h5_close(fileID);
    return ierr;
}
