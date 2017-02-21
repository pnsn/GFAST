#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "gfast_hdf5.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"

/*!
 * @brief Initializes the archive for the given event
 *
 * @param[in] adir           HDF5 archive directory.  if NULL then the archive
 *                           will be in the current working directory
 * @param[in] evid           event ID
 * @param[in] propfilename   name of GFAST properties file
 *
 * @result 0 indicates success
 *
 */
int hdf5_initialize(const char *adir,
                    const char *evid,
                    const char *propfilename)
{
    FILE *ifl;
    const char *fcnm = "hdf5_initialize\0";
    char *bufout[1];
    char fname[PATH_MAX], *buffer;
    hid_t fileID, groupID;
    int ierr;
    size_t lsize, nread;
    //------------------------------------------------------------------------//
    //
    // Set the filename
    ierr = 0;
    ierr = GFAST_hdf5_setFileName(adir, evid, fname);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting filename\n", fcnm);
        return -1;
    }
    // If file exists then let user know it is about to be deleted
    if (os_path_isfile(fname))
    {
        log_warnF("%s: H5 archive file %s will be overwritten\n", fcnm, fname);
    }
    // Set the filename and open it
    fileID = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    // Create a directory for the types and write them 
    ierr = ierr + h5_create_group(fileID, "/DataStructures\0");
    groupID = H5Gopen2(fileID, "/DataStructures\0", H5P_DEFAULT);
    ierr = ierr + GFAST_hdf5_createType__peakDisplacementData(groupID);
    ierr = ierr + GFAST_hdf5_createType__pgdResults(groupID);
    ierr = ierr + GFAST_hdf5_createType__cmtResults(groupID);
    ierr = ierr + GFAST_hdf5_createType__faultPlane(groupID);
    ierr = ierr + GFAST_hdf5_createType__ffResults(groupID);
    ierr = ierr + GFAST_hdf5_createType__hypocenter(groupID);
    ierr = ierr + GFAST_hdf5_createType__offsetData(groupID);
    ierr = ierr + GFAST_hdf5_createType__waveform3CData(groupID);
    ierr = ierr + GFAST_hdf5_createType__gpsData(groupID);
    ierr = ierr + H5Gclose(groupID);
    // Save the ini file
    ierr = ierr + h5_create_group(fileID, "/InitializationFile\0");
    if (os_path_isfile(propfilename))
    {
        ifl = fopen(propfilename, "rb");
        fseek(ifl, 0L, SEEK_END);
        lsize = (size_t) (ftell(ifl));
        rewind(ifl);
        buffer = (char *)calloc(1, (lsize+1)*sizeof(char));
        nread = fread(buffer, lsize, 1, ifl);
        if (nread < 1)
        {
            log_errorF("%s: Failed to read text file!\n", fcnm);
        }
        fclose(ifl);
        if (nread > 0 && buffer != NULL)
        {
            bufout[0] = buffer;
            ierr = ierr + h5_write_array__chars("/InitializationFile/IniFile\0",
                                                fileID, 1, bufout);
        }
        free(buffer);
    }
    // Create the directory which will hold the history
    ierr = ierr + h5_create_group(fileID, "/GFAST_History\0");

    // Create the directory which will hold the summary XML messages
    ierr = ierr + h5_create_group(fileID, "/Summary\0");

    // Close the archive
    ierr = ierr + H5Fclose(fileID); 
    return ierr;
}
