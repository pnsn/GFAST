#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <hdf5.h>

static hid_t traceFileID;
static double nanDefault;
static bool linit;

/*
struct h5BufferData_struct
{
    hvl_t data1;
    hvl_t data2;
    double t0Data1;
    double t0Data2;
    int npData1;
    int npData2;
    int maxPts;
    double dt;
}
*/
/*!
 * @brief Initializes the buffered memory using HDF5.  If in playback mode
 *        this will simply open the file for reading.
 *
 * @param[in] scratchDir        name of scratch directory.  if the file is
 *                              to be kept in memory and not saved then this
 *                              is not required.  if saving the file or using
 *                              harddisk as a buffer and this is NULL then the
 *                              current directory will be used.
 * @param[in] scratchName       scratch filename.  if the file is to be kept
 *                              in memory and not saved then this need not be
 *                              set.  if saving the file or using harddisk
 *                              as a buffer and this is NULL then it will default
 *                              to traceBuffers.h5. 
 *
 * @param[in] linMemory      
 * @param[in] lsaveWhenClosed   if true then when the file is closed the 
 *                              HDF5 memory buffer will be written to disk.
 * @param[in] bufferSize        memory size (bytes) which HDF5 will set aside
 *                              if keeping the file in memory
 * @param[in] nanValue          if a value for a time does not exist this will
 *                              be its default value.
 *
 * @result 0 indicates success
 *
 */
int traceBuffer_hdf5_initialize(const char *scratchDir,
                                const char *scratchName,
                                const bool linMemory,
                                const bool lsaveWhenClosed,
                                const size_t bufferSize,
                                const double nanValue)
{
    hid_t faplistID;
    faplistID = H5Pcreate(H5P_FILE_ACCESS);
    // Reading the file from disk
    if (job == 1)
    {
        status = H5Pset_fapl_core(faplist_id, blockSize, lsaveWhenClosed);
    }
    else
    {
        traceFileID = H5Fopen("traceBuffers.h5", H5F_ACC_TRUNC, faplistID);
    }
    return 0;
}

size_t buffer_estimateSize(const int nwaves,
                           const double timeSaved,
                           const double *__restrict__ dt,
                           int *ierr)
{
    size_t bufferSize;
    int i; 
    *ierr = 0;
    bufferSize = 0;
    if (nwaves < 1 || timeSaved <= 0.0 || dt == NULL)
    {
        *ierr = 1;
        return bufferSize;
    }
    for (i=0; i<nwaves; i++)
    {
        if (dt[i] <= 0.0)
        {
            *ierr = 1;
            break;
        }
        np = 4*64*sizeof(char);     // SNCLs
        np = np + 4*sizeof(int);    // points for size of buffer
        np = np + 4*sizeof(double); // start times and sampling periods
        npTrace = (int) (timeSave/dt[i] + 0.5);
        np = np + 2*(int) (timeSaved/dt[i] + 0.5)*sizeof(double);
        np = (int) (double) np*1.05; // pad some extra space
        bufferSize = bufferSize + np;
    }
    return bufferSize; 
}

int buffer_makeDataStructures( )
{

    return 0;
}

int buffer_addData(const char *netw,
                   const char *stat,
                   const char *chan,
                   const char *loc,
                   const double t0, const int npts, const double *__restrict__ values)
{
    for (i=0; i<nwaves; i++)
    {
        "/Waveform/\0";
    }
    return 0;
}

int buffer_writeData(const int npts, const double t0, int npts, double *vals )
{
    // Get the dataset name

    // Open the dataset for writing

    // Who is younger
    lflip = false;
    if (h5trace.t0Data1 > h5trace.t0Data2){lflip = true;}
    // This is normal order -> insert here
    if (t0 >= h5trace.t0Data1 && t0 < h5trace.t1Data1)
    {
        i1 = int ((t0 - h5trace.t0Data1)/dt + 0.5);
        i2 = i1 + npts;
        i2max = fmin(i2, h5trace.maxpts);
        ncopy = i2 - i1 + 1;
        memcpy(&h5trace.data1.p[i1], vals, ncopy*sizeof(double));
        ndif = i2max - i2;
        if (ndif > 0)
        {
            memcpy(&h5trace.data2.p[i2], vals, ndif*sizeof(double));
        }
        // Update the time stamps?
        if (ndif > 0 && h5trace.t0Data1 < h5trace.t0data2)
        {
            h5trace.t0data2 = h5trace.t1data1 + h5trace.dt;
            h5traee.t1data2 = h5trace.t0data1 + h5trace.dt*(h5trace.maxpts - 1);
        }
    }
    else if (t0 >= h5trace.t0Data1 && t2 < h5trace.t1Data2)
    {

    }
}

int buffer_finalize()
{
    status = (int) H5Fclose(tracefileID);
    return ierr;
}
