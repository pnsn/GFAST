#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

static int copyTrace(const int npts,
                     const double *__restrict__ origin,
                     const int ndest, 
                     double *__restrict__ dest);

/*!
 * @brief Copies the HDF5 trace buffer data onto the GPS data structure
 *
 * @param[in] traceBuffer    contains data from HDF5 file to copy onto
 *                           the GPS data structure.  Notice, the trace
 *                           buffer is appropriately queried for times
 *                           [t1, t2]
 *
 * @param[inout] gps_data    on input contains the requisite space 
 *                           to store the data from the traceBuffer.
 *                           on output contains the data from [t1, t2]
 *                           and fills all excess points (up to maxpts)
 *                           with NAN.
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker
 */
int traceBuffer_h5_copyTraceBufferToGFAST(
    struct h5traceBuffer_struct *traceBuffer,
    struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "traceBuffer_h5_copyTraceBufferToGFAST\0";
    double dt;
    int i, ierr, ierr1, j, k, l;
    bool *ltInit;
    ierr = 0;
    if (traceBuffer->ntraces < 1){return ierr;} // Nothing to do
    if (fmod(traceBuffer->ntraces, 3) != 0)
    {
        log_warnF("%s: Expecting multiple of 3 traces\n", fcnm);
    }
    ltInit = ISCL_memory_calloc__bool(fmax(traceBuffer->ntraces/3, 1)); 
    // Copy the data back
    for (i=0; i<traceBuffer->ntraces; i++)
    {
        j = fmod(traceBuffer->traces[i].idest, 3);
        k = (traceBuffer->traces[i].idest - j)/3;
        if (traceBuffer->traces[i].ncopy > gps_data->data[k].maxpts)
        {
            log_errorF("%s: Invalid copy size\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        dt = traceBuffer->traces[i].dt;
        if (j == 0)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            ierr1 = copyTrace(gps_data->data[k].npts,
                              traceBuffer->traces[i].data, 
                              gps_data->data[k].maxpts,
                              gps_data->data[k].ubuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error copying ubuff\n", fcnm);
                ierr = ierr + 1;
            }
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
            #pragma omp simd
            for (l=0; l<gps_data->data[k].npts; l++)
            {
                gps_data->data[k].tbuff[l] = traceBuffer->traces[i].t1 + l*dt;
            }
        }
        else if (j == 1)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            ierr1 = copyTrace(gps_data->data[k].npts,
                              traceBuffer->traces[i].data, 
                              gps_data->data[k].maxpts,
                              gps_data->data[k].nbuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error copying nbuff\n", fcnm);
                ierr = ierr + 1;
            }
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
        else if (j == 2)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            ierr1 = copyTrace(gps_data->data[k].npts,
                              traceBuffer->traces[i].data, 
                              gps_data->data[k].maxpts,
                              gps_data->data[k].ebuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error copying ebuff\n", fcnm);
                ierr = ierr + 1;
            }
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
//    printf("%d %d\n", k, j);
    }
    ISCL_memory_free__bool(&ltInit);
    return ierr;
}
//============================================================================//
/*!
 * @brief Convenience function for copying origin to destination and
 *        filling any leftover poitns with NANs.
 *
 * @param[in] npts     number of points on origin to copy
 * @param[in] origin   origin array to copy to dest [npts]
 * @param[in] ndest    number of points in dest (>= npts)
 *
 * @param[out] dest    the first npts contains the data from the origin
 *                     buffer and final [npts:ndest] is filled with NaNs
 *
 * @result 0 indicates successs
 *
 * @author Ben Baker (ISTI)
 *
 */
static int copyTrace(const int npts,
                     const double *__restrict__ origin,
                     const int ndest,
                     double *__restrict__ dest)
{
    const char *fcnm = "copyTrace\0";
    int i, ierr;//, nc;
    ierr = 0;
    if (npts > ndest)
    {
        log_errorF("%s: npts > ndest %d %d\n", fcnm, npts, ndest);
        return 1;
    }
/*
    ierr = ISCL__array_copy__double(npts, origin, dest);
    nc = npts - ndest + 1;
    if (nc > 0)
    {
        ierr = ISCL__array_set__double(nc, NAN, &dest[npts]);
    }
*/
    if (ndest > 0 && dest == NULL)
    {
        log_errorF("%s: dest is NULL\n", fcnm);
        return 1;
    }
    if (npts > 0 && origin == NULL)
    {
        log_errorF("%s: origin is NULL\n", fcnm);
        return 1;
    }
    #pragma omp simd
    for (i=0; i<npts; i++)
    {
        dest[i] = origin[i];
    }
    #pragma omp simd
    for (i=npts; i<ndest; i++)
    {
        dest[i] = NAN;
    }
    return ierr;
}
