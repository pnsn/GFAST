#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cblas.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
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
 * @param[in,out] gps_data   on input contains the requisite space 
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
    double dt, gain;
    int i, ierr, ierr1, j, k, l;
    int ii;
    bool *ltInit;
    ierr = 0;
    if (traceBuffer->ntraces < 1){return ierr;} // Nothing to do
    if (fmod(traceBuffer->ntraces, 3) != 0)
    {
        LOG_WARNMSG("%s", "Expecting multiple of 3 traces");
    }
    ltInit = memory_calloc8l((int) (fmax(traceBuffer->ntraces/3, 1)));
    // Copy the data back
    LOG_MSG("Copy the data back traceBuffer->ntraces=%d", traceBuffer->ntraces);
    for (i=0; i<traceBuffer->ntraces; i++)
    {
        j = (int) (fmod(traceBuffer->traces[i].idest, 3));
        k = (traceBuffer->traces[i].idest - j)/3;
        if (traceBuffer->traces[i].ncopy > gps_data->data[k].maxpts)
        {
            LOG_ERRMSG("%s", "Invalid copy size");
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
            memory_free64f(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                LOG_ERRMSG("%s", "Error copying ubuff");
                ierr = ierr + 1;
            }
            // Apply the gain
            gps_data->data[k].gain[j] = 1.0; // will be applied here
            gain = traceBuffer->traces[i].gain;
            if (fabs(gain) < 1.e-15)
            {
                LOG_ERRMSG("%s", "Division by zero");
                ierr = ierr + 1;
            }
            /*
            for (ii=0; ii<gps_data->data[k].npts; ii++){
              LOG_MSG("Before: %s.%s.%s.%s i:%d (npts=%d) t:%f (dbl) data=%f",
                  gps_data->data[k].stnm, gps_data->data[k].chan[j],
                  gps_data->data[k].netw, gps_data->data[k].loc,
                  ii,
                  gps_data->data[k].npts,
                  gps_data->data[k].tbuff[ii],
                  gps_data->data[k].ubuff[ii]);
            }
            */
            gain = 1.0/gain;
            cblas_dscal(gps_data->data[k].npts, gain,
                        gps_data->data[k].ubuff, 1);

            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
#ifdef _OPENMP
            #pragma omp simd
#endif
            for (l=0; l<gps_data->data[k].npts; l++)
            {
                gps_data->data[k].tbuff[l] = traceBuffer->traces[i].t1 + l*dt;
            }
            for (ii=0; ii<gps_data->data[k].npts; ii++){
              LOG_MSG(" After: %s.%s.%s.%s i:%d (npts=%d) t:%f (dbl) data=%f",
                  gps_data->data[k].stnm, gps_data->data[k].chan[j],
                  gps_data->data[k].netw, gps_data->data[k].loc,
                  ii,
                  gps_data->data[k].npts,
                  gps_data->data[k].tbuff[ii],
                  gps_data->data[k].ubuff[ii]);
            }
            /*
            */
        }
        else if (j == 1)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            ierr1 = copyTrace(gps_data->data[k].npts,
                              traceBuffer->traces[i].data, 
                              gps_data->data[k].maxpts,
                              gps_data->data[k].nbuff);
            memory_free64f(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                LOG_ERRMSG("%s", "Error copying nbuff");
                ierr = ierr + 1;
            }
            // Apply the gain
            gps_data->data[k].gain[j] = 1.0; // will be applied here
            gain = traceBuffer->traces[i].gain;
            if (gain == 0.0)
            {
                LOG_ERRMSG("%s", "Division by zero");
                ierr = ierr + 1;
            }
            gain = 1.0/gain;
            cblas_dscal(gps_data->data[k].npts, gain,
                        gps_data->data[k].nbuff, 1);
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
        else if (j == 2)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            ierr1 = copyTrace(gps_data->data[k].npts,
                              traceBuffer->traces[i].data, 
                              gps_data->data[k].maxpts,
                              gps_data->data[k].ebuff);
            memory_free64f(&traceBuffer->traces[i].data);
            if (ierr1 != 0)
            {
                LOG_ERRMSG("%s", "Error copying ebuff");
                ierr = ierr + 1;
            }
            // Apply the gain
            gps_data->data[k].gain[j] = 1.0; // will be applied here
            gain = traceBuffer->traces[i].gain;
            if (gain == 0.0)
            {
                LOG_ERRMSG("%s", "Division by zero");
                ierr = ierr + 1;
            }
            gain = 1.0/gain;
            cblas_dscal(gps_data->data[k].npts, gain,
                        gps_data->data[k].ebuff, 1);
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
//    printf("%d %d\n", k, j);
    }
    memory_free8l(&ltInit);
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
    int i, ierr;//, nc;
    ierr = 0;
    if (npts > ndest)
    {
        LOG_ERRMSG("npts > ndest %d %d", npts, ndest);
        return 1;
    }
/*
    ierr = array_copy64f_work(npts, origin, dest);
    nc = npts - ndest + 1;
    if (nc > 0)
    {
        ierr = array_set64f_work(nc, NAN, &dest[npts]);
    }
*/
    if (ndest > 0 && dest == NULL)
    {
        LOG_ERRMSG("%s", "dest is NULL");
        return 1;
    }
    if (npts > 0 && origin == NULL)
    {
        LOG_ERRMSG("%s", "origin is NULL");
        return 1;
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<npts; i++)
    {
        LOG_MSG("copyTrace: i=%d origin[i]=%f", i, origin[i]);
        dest[i] = origin[i];
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=npts; i<ndest; i++)
    {
        dest[i] = (double) NAN;
    }
    return ierr;
}
