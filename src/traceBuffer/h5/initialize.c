#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "gfast_hdf5.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/time/time.h"
#include "fileutils.h"
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
    FILE *fp;
    double *work; 
    char **traceOut, **traces, h5name[PATH_MAX], cwork[512], temp[64];
    herr_t status;
    hsize_t dims[2];
    hid_t dataSet, dataSpace, groupID, properties;
    double *dts, *selevs, *slats, *slons, tbeg, t1;
    int i, ierr, j, k, maxpts, ndtGroups, ntraces;
    size_t blockSize;
    const bool lsave = true;;
    // Make sure there is data
    if (h5traceBuffer->traces == NULL || h5traceBuffer->ntraces < 1)
    {
        LOG_ERRMSG("%s", "Input traces do not exist");
        return -1;
    }
    // Set the filename
    ierr = traceBuffer_h5_setFileName(h5dir, h5file, h5name);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error setting the HDF5 filename");
        return -1;
    }
    // In this instance the file is simply opened for reading
    if (job == 1)
    {
        // If scratch file was saved then remove it
        if (!cfileexists(h5name))
        {
            LOG_ERRMSG("Error HDF5 file does %s not exist!", h5name);
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
                LOG_ERRMSG("%s", "Error setting properties list");
                return -1; 
            }
        }
        h5traceBuffer->fileID = H5Fopen(h5name, H5F_ACC_RDONLY, properties);
        status = H5Pclose(properties);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error closing the properties list");
            return -1;
        }
        // Verify the group is there
        for (i=0; i<h5traceBuffer->ndtGroups; i++)
        {
            if (H5Lexists(h5traceBuffer->fileID,
                          h5traceBuffer->dtGroupName[i], //traces[i].groupName,
                          H5P_DEFAULT) != 1)
            {
                LOG_ERRMSG("Error couldn't find group: %s", 
                           h5traceBuffer->dtGroupName[i]);
                return -1;
            }
        }
        // Get the metaData and make a permutation
        traces = h5_read_array__string("/MetaData/TraceNames\0",
                                       h5traceBuffer->fileID, &ntraces, &ierr);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Couldn't read traces");
            return -1;
        }
        if (ntraces != h5traceBuffer->ntraces)
        {
            LOG_ERRMSG("%s", "metadata size inconsistency");
            return -1;
        }
        // Match those SNCLs
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            memset(temp, 0, sizeof(temp));
            strcpy(temp, h5traceBuffer->traces[i].netw);
            strcat(temp, ".\0");
            strcat(temp, h5traceBuffer->traces[i].stnm);
            strcat(temp, ".\0");
            strcat(temp, h5traceBuffer->traces[i].chan);
            strcat(temp, ".\0");
            strcat(temp, h5traceBuffer->traces[i].loc);
            for (j=0; j<h5traceBuffer->ntraces; j++)
            {
                if (strcasecmp(traces[j], temp) == 0)
                {
                    h5traceBuffer->traces[i].traceNumber = j;
                    goto FOUND_TRACE;
                }
            }
            LOG_ERRMSG("%s", "Failed to find trace!");
            return -1;
FOUND_TRACE:;
        }
        if (ntraces > 0)
        {
            free(traces[0]);
            free(traces);
        }
    }
    // Otherwise the file size must be estimated and opened for writing
    else
    {
        // Set the time to now
        tbeg = (double) ((long) (time_timeStamp())); 
        // If scratch file was saved then remove it 
        if (cfileexists(h5name))
        {
            LOG_WARNMSG("Deleting file %s", h5name);
        }
        // Space estimate
        maxpts = 0;
        for (i=0; i<h5traceBuffer->ntraces; i++)
        {
            maxpts = (int) (fmax(h5traceBuffer->traces[i].maxpts, maxpts));
            if (maxpts <= 0)
            {
                if (maxpts < 0)
                {
                    LOG_WARNMSG("maxpts = %d is negative - setting to 0",
                                maxpts);
                    maxpts = 0;
                    h5traceBuffer->traces[i].maxpts = maxpts;
                }
                else
                {
                    LOG_WARNMSG("maxpts is 0 for trace %d", i+1);
                }
            }
        }
        if (maxpts == 0)
        {
            LOG_ERRMSG("%s", "There's no data in the buffers");
            return -1;
        }
        blockSize = (size_t)
                    (h5traceBuffer->ntraces*(8*maxpts + 4*8 + 64 + 2*8 + 2*4));
        // add a little extra
        blockSize = (size_t) ((double) (blockSize*1.1 + 0.5));
        properties = H5Pcreate(H5P_FILE_ACCESS); 
        if (linMemory)
        {
            status = H5Pset_fapl_core(properties, blockSize, lsave);
            if (status < 0)
            {
                LOG_ERRMSG("%s", "Error setting properties list");
                return -1;
            }
        }
        h5traceBuffer->fileID = H5Fcreate(h5name, H5F_ACC_TRUNC,
                                          H5P_DEFAULT, properties);
        status = H5Pclose(properties);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error closing the properties list");
            return -1; 
        }
        // Make the root groups
        groupID = H5Gcreate2(h5traceBuffer->fileID,
                             "/Data\0",
                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error creating /Data group");
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
            LOG_ERRMSG("%s", "Error no sampling period groups");
            return -1;
        }
        if (ndtGroups > 1)
        {
            LOG_WARNMSG("%s", "Multiple dt not tested");
            return -1;
        }
        for (i=0; i<ndtGroups; i++)
        {
            k = h5traceBuffer->dtPtr[i];
            memset(cwork, 0, sizeof(cwork));
            sprintf(cwork, "/Data/SamplingPeriodGroup_%d", i+1);
            groupID = H5Gcreate2(h5traceBuffer->fileID,
                                 cwork, 
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            h5_write_attribute__double("SamplingPeriod\0", groupID,
                                       1, &h5traceBuffer->traces[k].dt);
            status = H5Gclose(groupID);
        }
        groupID = H5Gcreate2(h5traceBuffer->fileID,
                             "/MetaData\0",
                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        status = H5Gclose(groupID);
        if (status < 0)
        {
            LOG_ERRMSG("%s", "Error creating /MetaData group");
            return -1;
        }
        // Make the data
        for (i=0; i<h5traceBuffer->ndtGroups; i++)
        {
            k = h5traceBuffer->dtPtr[i];
            ntraces = h5traceBuffer->dtPtr[i+1] - h5traceBuffer->dtPtr[i];
            memset(cwork, 0, sizeof(cwork));
            sprintf(cwork, "/Data/SamplingPeriodGroup_%d/Data", i+1);
            work = array_set64f(h5traceBuffer->ntraces*maxpts,
                                (double) NAN, &ierr);
            dims[0] = (hsize_t) h5traceBuffer->ntraces;
            dims[1] = (hsize_t) maxpts;
            dataSpace = H5Screate_simple(2, dims, NULL);
            dataSet = H5Dcreate2(h5traceBuffer->fileID, cwork,
                                 H5T_NATIVE_DOUBLE, dataSpace,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                              H5P_DEFAULT, work);
            ierr = h5_write_attribute__double("SamplingPeriod\0", dataSet,
                                              1, &h5traceBuffer->traces[k].dt);
            t1 = tbeg - (double) (maxpts - 1)*h5traceBuffer->traces[k].dt;
            ierr = h5_write_attribute__double("StartTime\0", dataSet,
                                              1, &t1);
            ierr = h5_write_attribute__int("NumberOfPoints\0", dataSet,
                                           1, &maxpts);
            ierr = h5_write_attribute__int("NumberOfTraces\0", dataSet,
                                           1, &ntraces);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            // Make the gain
            dims[0] = (hsize_t) ntraces;
            for (k=h5traceBuffer->dtPtr[i]; k<h5traceBuffer->dtPtr[i+1]; k++)
            {
                work[k] = h5traceBuffer->traces[k].gain;
            }
            dims[0] = (hsize_t) ntraces;
            dataSpace = H5Screate_simple(1, dims, NULL);
            dataSet = H5Dcreate2(h5traceBuffer->fileID,
                                 "/Data/SamplingPeriodGroup_1/Gain\0",
                                 H5T_NATIVE_DOUBLE, dataSpace,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                              H5P_DEFAULT, work);
            status = H5Sclose(dataSpace);
            status = H5Dclose(dataSet);
            // Make the metadata
            slats  = memory_calloc64f(ntraces);
            slons  = memory_calloc64f(ntraces);
            selevs = memory_calloc64f(ntraces);
            dts    = memory_calloc64f(ntraces);
            traceOut = (char **) calloc((size_t) ntraces, sizeof(char *));
            for (k=h5traceBuffer->dtPtr[i]; k<h5traceBuffer->dtPtr[i+1]; k++)
            {
                j = k - h5traceBuffer->dtPtr[i];
                h5traceBuffer->traces[k].traceNumber = j;
                slats[j] = h5traceBuffer->traces[k].slat;
                slons[j] = h5traceBuffer->traces[k].slon;
                selevs[j] = h5traceBuffer->traces[k].selev;
                dts[j] = h5traceBuffer->traces[k].dt;
                traceOut[j] = (char *) calloc(64, sizeof(char));
                strcpy(traceOut[j], h5traceBuffer->traces[k].netw); 
                strcat(traceOut[j], ".\0");
                strcat(traceOut[j], h5traceBuffer->traces[k].stnm);
                strcat(traceOut[j], ".\0");
                strcat(traceOut[j], h5traceBuffer->traces[k].chan);
                strcat(traceOut[j], ".\0");
                strcat(traceOut[j], h5traceBuffer->traces[k].loc);
            }
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, "/MetaData/SamplingPeriods\0");
            ierr = h5_write_array__double(cwork, h5traceBuffer->fileID,
                                          ntraces, dts); 
            if (ierr != 0)
            {
                LOG_ERRMSG("%s", "Error writing dts");
                return -1;
            }
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, "/MetaData/StationElevations\0");
            ierr = h5_write_array__double(cwork, h5traceBuffer->fileID,
                                          ntraces, selevs);
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, "/MetaData/StationLatitudes\0");
            ierr = h5_write_array__double(cwork, h5traceBuffer->fileID,
                                          ntraces, slats);
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, "/MetaData/StationLongitudes\0");
            ierr = h5_write_array__double(cwork, h5traceBuffer->fileID,
                                          ntraces, slons);
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, "/MetaData/TraceNames\0");
            ierr = h5_write_array__chars(cwork, h5traceBuffer->fileID,
                                         ntraces, traceOut);
            // Free workspace
            memory_free64f(&dts);
            memory_free64f(&selevs);
            memory_free64f(&slons);
            memory_free64f(&slats);
            memory_free64f(&work);
            for (j=0; j<ntraces; j++)
            {
                free(traceOut[j]);
            }
            free(traceOut);
        }
    }
    h5traceBuffer->linit = true;
    return 0;
}
