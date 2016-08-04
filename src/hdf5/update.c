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
    hid_t fileID;
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
    ierr = h5_close(fileID);
    return k;
}
//============================================================================//
/*
 * @brief Writes the next iteration of the PGD estimation fro the given event
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
    // Open the requisite HDF5 data structures

    // Copy the data and the results
    //ierr = ierr + GFAST_HDF5__copy__peakDisplacementData(COPY_DATA_TO_H5,
    //                                                  &pgd_data, &h5_pgd_data);
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
    char h5fl[PATH_MAX];
    struct h5_offsetData_struct h5_ff_data;
    struct h5_ffResults_struct h5_ff;
    hid_t dataSet, dataSpace, dataType, fileID, groupID;
    char ffGroup[256];
    int ierr;
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
    // Free the space
    ierr = GFAST_hdf5_memory__freeFFResults(&h5_ff);
    // Close the group and file
    ierr = ierr + H5Gclose(groupID);
    ierr = h5_close(fileID);
    return ierr;
}
