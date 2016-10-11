#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_hdf5.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"
/*!
 * @brief Initializes the HDF5 file for archiving the acquisition or
 *        reading in playback mode. 
 *
 * @param[in] job                If job = 1 then the file will be opened as
 *                               read only.
 *                               If job = 2 then the file will be opened as
 *                               read/write.
 * @param[in] linMemory          If true then keep the opened file in memory.
 * @param[in] h5dir              directory where HDF5 file exists
 * @param[in] h5file             name of the HDF5 file in the directory
 *
 * @param[in,out] h5traceBuffer  On input, contains the names of the HDF5
 *                               dataset names.
 *                               On output, contains the HDF5 file handle
 *                               from which we'll read and write.
 *
 * @result 0 indicates success
 * 
 */
int traceBuffer_h5_initialize(const int job,
                              const bool linMemory,
                              const char *h5dir,
                              const char *h5file,
                              struct h5traceBuffer_struct *h5traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_initialize\0";
    FILE *fp;
    double *work; 
    char h5name[PATH_MAX], cwork[512];
    herr_t status;
    hid_t groupID, properties;
    double dt, gain, slat, selev, slon, tbeg, t1, t2;
    int i, ierr, k, maxpts, ndtGroups;
    size_t blockSize;
    const bool lsave = true;;
    // Make sure there is data
    if (h5traceBuffer->traces == NULL || h5traceBuffer->ntraces < 1)
    {
        log_errorF("%s: Input traces do not exist\n", fcnm);
        return -1;
    }
    // Set the filename
    ierr = traceBuffer_h5_setFileName(h5dir, h5file, h5name);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting the HDF5 filename\n", fcnm);
        return -1;
    }
    // In this instance the file is simply opened for reading
    if (job == 1)
    {
        // If scratch file was saved then remove it
        if (!os_path_isfile(h5name))
        {
            log_errorF("%s: Error HDF5 file does %s not exist!\n",
                       fcnm, h5name);
            return -1;
        }
        // Get the size of the file
        fp = fopen(h5name, "rb\0");
        fseek(fp, 0L, SEEK_END);
        blockSize = (size_t) (ftell(fp));
        fclose(fp);
        // Open the file
        properties = H5Pcreate(H5P_FILE_ACCESS);
        if (linMemory)
        {
            status = H5Pset_fapl_core(properties, blockSize, false);
            if (status < 0)
            {
                log_errorF("%s: Error setting properties list\n", fcnm);
                return -1; 
            }
        }
        h5traceBuffer->fileID = H5Fopen(h5name, H5F_ACC_RDONLY, properties);
        status = H5Pclose(properties);
        if (status < 0)
        {
            log_errorF("%s: Error closing the properties list\n", fcnm);
            return -1;
        }
        // Verify the group is there
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            if (H5Lexists(h5traceBuffer->fileID,
                          h5traceBuffer->traces[i].groupName,
                          H5P_DEFAULT) != 1)
            {
                log_errorF("%s: Error couldn't find group: %s\n", fcnm,
                           h5traceBuffer->traces[i].groupName);
                return -1;
            }
        }
    }
    // Otherwise the file size must be estimated and opened for writing
    else
    {
        // Set the time to now
        tbeg = (double) ((long) (ISCL_time_timeStamp())); 
        // If scratch file was saved then remove it 
        if (os_path_isfile(h5name))
        {
            log_warnF("%s: Deleting file %s\n", fcnm, h5name);
        }
        blockSize = 0;
        // Space estimate
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            maxpts = h5traceBuffer->traces[i].maxpts;
            if (maxpts <= 0)
            {
                if (maxpts < 0)
                {
                    log_warnF("%s: maxpts is negative - setting to 0\n", fcnm);
                    maxpts = 0;
                    h5traceBuffer->traces[i].maxpts = maxpts;
                }
                else
                {
                    log_warnF("%s: maxpts is 0 for trace %d\n", fcnm, i+1);
                }
            }
            blockSize = blockSize + 8*2*(size_t) maxpts + 8*3 + 4;
        }
        // add a little extra
        blockSize = (size_t) ((double) (blockSize*1.1 + 0.5));
        properties = H5Pcreate(H5P_FILE_ACCESS); 
        if (linMemory)
        {
            status = H5Pset_fapl_core(properties, blockSize, lsave);
            if (status < 0)
            {
                log_errorF("%s: Error setting properties list\n", fcnm);
                return -1;
            }
        }
        h5traceBuffer->fileID = H5Fcreate(h5name, H5F_ACC_TRUNC,
                                          H5P_DEFAULT, properties);
        status = H5Pclose(properties);
        if (status < 0)
        {
            log_errorF("%s: Error closing the properties list\n", fcnm);
            return -1; 
        }
        // Make the root groups
        groupID = H5Gcreate2(h5traceBuffer->fileID,
                             "/Data\0",
                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            log_errorF("%s: Error creating /Data group\n", fcnm);
            return -1;
        }
        ndtGroups = 0;
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            if (h5traceBuffer->traces[i].dtGroupNumber > ndtGroups)
            {
                ndtGroups = h5traceBuffer->traces[i].dtGroupNumber;
            }
        }
        if (ndtGroups <= 0)
        {
            log_errorF("%s: Error no sampling period groups\n", fcnm);
            return -1;
        }
        for (i=0; i<ndtGroups; i++)
        {
            memset(cwork, 0, sizeof(work));
            sprintf(cwork, "/Data/SamplingPeriodGroup_%d", i+1);
            groupID = H5Gcreate2(h5traceBuffer->fileID,
                                 cwork, 
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Gclose(groupID);
        } 
        groupID = H5Gcreate2(h5traceBuffer->fileID,
                             "/MetaData\0",
                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            log_errorF("%s: Error creating /MetaData group\n", fcnm);
            return -1;
        }
        // Make the data groups
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            // MetaData
            groupID = H5Gcreate2(h5traceBuffer->fileID,
                                 h5traceBuffer->traces[i].metaGroupName,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            dt = h5traceBuffer->traces[i].dt;
            slat = h5traceBuffer->traces[i].slat;
            slon = h5traceBuffer->traces[i].slon; 
            selev = h5traceBuffer->traces[i].selev;
            ierr = 0;
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "StationLatitude\0", slat);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "StationLongitude\0", slon);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "StationElevation\0", selev);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "SamplingPeriod\0", dt);
            ierr += traceBuffer_h5_setIntegerScalar(groupID,
                                        "SamplingPeriodGroupNumber\0",
                                        h5traceBuffer->traces[i].dtGroupNumber);
            if (ierr != 0)
            {
                log_errorF("%s: Error writing scalar metadata\n", fcnm);
                return -1;
            }
            status = H5Gclose(groupID);
            if (status < 0)
            {
                log_errorF("%s: Error creating group %s\n",
                           fcnm, h5traceBuffer->traces[i].metaGroupName);
                return -1;
            }
            // Data
            groupID = H5Gcreate2(h5traceBuffer->fileID,
                                 h5traceBuffer->traces[i].groupName,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            // Set the basic data.  Set the first buffer to the
            // past so that latent data can be added during startup
            maxpts = h5traceBuffer->traces[i].maxpts;
            t1 = tbeg - (double) maxpts*dt;
            t2 = tbeg;
            if (maxpts > 0)
            {
                work = ISCL_memory_calloc__double(maxpts);
                for (k=0; k<maxpts; k++)
                {
                    work[k] = (double) NAN;
                }
                ierr = 0;
                ierr += h5_write_array__double("dataBuffer1\0", groupID,
                                               maxpts, work);
                ierr += h5_write_array__double("dataBuffer2\0", groupID,
                                               maxpts, work);
                if (ierr != 0)
                {
                    log_errorF("%s: Error writing databuffers\n", fcnm);
                    return -1;
                }
                ISCL_memory_free__double(&work);
            }
            gain = h5traceBuffer->traces[i].gain;
            ierr = 0;
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "Buffer1StartTime\0", t1);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "Buffer2StartTime\0", t2);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "Gain\0", gain);
            ierr += traceBuffer_h5_setDoubleScalar(groupID,
                                                   "SamplingPeriod\0", dt);
            ierr += traceBuffer_h5_setIntegerScalar(groupID,
                                                    "MaxNumberOfPoints\0",
                                                    maxpts);
            if (ierr != 0)
            {
                log_errorF("%s: Error writing scalar data\n", fcnm);
                return -1;
            }
            status = H5Gclose(groupID);
            if (status < 0)
            {
                log_errorF("%s: Error creating group %s\n",
                           fcnm, h5traceBuffer->traces[i].groupName);
                return -1;
            }
        }
    }
    h5traceBuffer->linit = true;
    return 0;
}
