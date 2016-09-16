#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <hdf5.h>
#include "gfast_hdf5.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"
/*!
 * @brief Initializes the current directory for this GFAST iteration.
 *
 * @param[in] adir     archive directory.  if NULL this will be the current
 *                     working directory.
 * @param[in] evid     event ID
 *
 * @result if -1 an error occurred.
 *         otherwise, this is the is iteration number for writing GFAST's
 *         history
 *
 */
int hdf5_update__getIteration(const char *adir,
                              const char *evid,
                              const double epoch)
{
    const char *fcnm = "hdf5_update__getIteration\0";
    const char *group_root = "/GFAST_History\0";
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX], iterGroup[256];
    hid_t fileID, groupID;
    int ierr, k;
    //------------------------------------------------------------------------//
    //
    // Open the old HDF5 file 
    ierr = GFAST_hdf5_setFileName(adir, evid, h5fl);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    k = h5_n_group_members(group_root, fileID) + 1;
    memset(iterGroup, 0, sizeof(iterGroup));
    sprintf(iterGroup, "%s_%d", item_root, k);
    if (h5_item_exists(fileID, iterGroup))
    {
        log_errorF("%s: Error group shouldn't exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    ierr = h5_create_group(fileID, iterGroup);
    // Write the epochal time as an attribute
    groupID = H5Gopen2(fileID, iterGroup, H5P_DEFAULT);
    ierr = h5_write_attribute__double("epoch\0", groupID, 1, &epoch);
    ierr = H5Gclose(groupID);
    ierr = h5_close(fileID);
    // Add the epochal time
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
int hdf5_update__hypocenter(const char *adir,
                            const char *evid,
                            const int h5k,
                            struct GFAST_shakeAlert_struct hypo)
{
    const char *fcnm = "hdf5_update__hypocenter\0";
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
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Verify iteration group exists 
    memset(hypoGroup, 0, sizeof(hypoGroup));
    sprintf(hypoGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, hypoGroup))
    {
        log_errorF("%s: Error group should exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing
    groupID = H5Gopen2(fileID, hypoGroup, H5P_DEFAULT);
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copy__hypocenter(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing hypocenter\n", fcnm);
    }
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}

//============================================================================//
/*!
 * @brief Writes the next iteration of the PGD estimation for the given event
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_update__pgd(const char *adir,
                     const char *evid,
                     const int h5k,
                     struct GFAST_peakDisplacementData_struct pgd_data,
                     struct GFAST_pgdResults_struct pgd)
{
    const char *fcnm = "hdf5_update__pgd\0";
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
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(pgdGroup, 0, sizeof(pgdGroup));
    sprintf(pgdGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, pgdGroup))
    {
        log_errorF("%s: Error group should exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing
    groupID = H5Gopen2(fileID, pgdGroup, H5P_DEFAULT); 
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copy__peakDisplacementData(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing PGD data\n", fcnm);
    }
    // Copy and write the results 
    ierr = ierr +  GFAST_hdf5_copy__pgdResults(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing PGD results\n", fcnm);
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        log_errorF("%s: Error closing HDF5 data items\n", fcnm);
    } 
    // Free the space
    ierr = GFAST_hdf5_memory__freePGDData(&h5_pgd_data);
    ierr = GFAST_hdf5_memory__freePGDResults(&h5_pgd);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_update__cmt(const char *adir,
                     const char *evid,
                     const int h5k,
                     struct GFAST_offsetData_struct cmt_data,
                     struct GFAST_cmtResults_struct cmt)
{
    const char *fcnm = "hdf5_update__cmt\0";
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
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(cmtGroup, 0, sizeof(cmtGroup));
    sprintf(cmtGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, cmtGroup))
    {
        log_errorF("%s: Error group should exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, cmtGroup, H5P_DEFAULT);
    // Copy and write the data
    ierr = ierr + GFAST_hdf5_copy__offsetData(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing CMT data\n", fcnm);
    }
    // Copy and write the results
    ierr = ierr +  GFAST_hdf5_copy__cmtResults(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing Greens functions\n", fcnm);
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {   
        log_errorF("%s: Error closing HDF5 data items\n", fcnm);
    }
    // Free the space
    ierr = GFAST_hdf5_memory__freeOffsetData(&h5_cmt_data);
    ierr = GFAST_hdf5_memory__freeCMTResults(&h5_cmt);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_update__ff(const char *adir,
                    const char *evid,
                    const int h5k,
                    struct GFAST_offsetData_struct ff_data,
                    struct GFAST_ffResults_struct ff)
{
    const char *fcnm = "hdf5_update__ff\0";
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
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1; 
    }   
    if (!os_path_isfile(h5fl))
    {   
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1; 
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(ffGroup, 0, sizeof(ffGroup));
    sprintf(ffGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, ffGroup))
    {
        log_errorF("%s: Error group should exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, ffGroup, H5P_DEFAULT);
    // Copy and write the results
    ierr = ierr + GFAST_hdf5_copy__ffResults(COPY_DATA_TO_H5,
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
        log_errorF("%s: Error writing Greens functions\n", fcnm);
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        log_errorF("%s: Error closing HDF5 data items\n", fcnm);
    }
    // Write the faults individually for h5py
    dataType = H5Topen(groupID, "/DataStructures/faultPlaneStructure\0",
                       H5P_DEFAULT);
    dataSpace = H5Screate_simple(rank, dimInfo, NULL);
    /* TODO: this is a kludge for h5py - retry with newer version */
    for (i=0; i<ff.nfp; i++)
    {
        memset(dataName, 0, sizeof(dataName));
        sprintf(dataName, "faultPlane_%d", i+1);
        dataSet = H5Dcreate(groupID, dataName, dataType,
                            dataSpace,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        GFAST_hdf5_copy__faultPlane(COPY_DATA_TO_H5, &ff.fp[i], &h5_fp);
        ierr = H5Dwrite(dataSet, dataType, H5S_ALL, H5S_ALL,
                        H5P_DEFAULT, &h5_fp);
        ierr = hdf5_memory__freeFaultPlane(&h5_fp);
    }
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {   
        log_errorF("%s: Error closing HDF5 data items\n", fcnm);
    }
    // Free the space
    ierr = GFAST_hdf5_memory__freeFFResults(&h5_ff);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
//============================================================================//
int hdf5_update__gpsData(const char *adir,
                         const char *evid,
                         const int h5k,
                         struct GFAST_data_struct data)
{
    const char *fcnm = "hdf5_update__gpsData\0";
    const char *item_root = "/GFAST_History/Iteration\0";
    char h5fl[PATH_MAX], dataName[256];
    struct h5_gpsData_struct h5_gpsData;
    //struct h5_waveform3CData_struct h5_3cdata;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char gpsGroup[256];
    int i, ierr;
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
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    if (!os_path_isfile(h5fl))
    {
        log_errorF("%s: Error file %s does not exist!\n", fcnm);
        return -1;
    }
    fileID = h5_open_rdwt(h5fl);
    // Have HDF5 count the group members as to compute the iteration number 
    memset(gpsGroup, 0, sizeof(gpsGroup));
    sprintf(gpsGroup, "%s_%d", item_root, h5k);
    if (!h5_item_exists(fileID, gpsGroup))
    {
        log_errorF("%s: Error group should exist\n", fcnm);
        ierr = h5_close(fileID);
        return ierr;
    }
    // Open the group for writing 
    groupID = H5Gopen2(fileID, gpsGroup, H5P_DEFAULT);
    // Copy and write the results
    ierr = hdf5_copy__gpsData(COPY_DATA_TO_H5,
                              &data, &h5_gpsData);
    if (ierr != 0)
    {
        log_errorF("%s: Error copying GPS data!\n", fcnm);
        h5_close(fileID);
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
        log_errorF("%s: Error writing Greens functions\n", fcnm);
    }
    ierr = H5Dclose(dataSet);
    ierr = ierr + H5Sclose(dataSpace);
    ierr = ierr + H5Tclose(dataType);
    if (ierr != 0)
    {
        log_errorF("%s: Error closing HDF5 data items\n", fcnm);
    }

    ierr = h5_close(fileID);
    return ierr;
}
