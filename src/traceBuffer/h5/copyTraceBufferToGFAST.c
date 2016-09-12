#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

static void copyTrace(const int npts,
                      const double *__restrict__ origin,
                      const int ndest, 
                      double *__restrict__ dest)
{
    int i;
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
    return;
}

int traceBuffer_h5_copyTraceBufferToGFAST(
    struct h5traceBuffer_struct *traceBuffer,
    struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "traceBuffer_h5_copyTraceBufferToGFAST\0";
    double dt;
    int i, j, k, l;
    // Copy the data back
    for (i=0; i<traceBuffer->ntraces; i++)
    {
        j = fmod(traceBuffer->traces[i].idest, 3);
        k = (traceBuffer->traces[i].idest - j)/3;
        if (traceBuffer->traces[i].ncopy > gps_data->data[k].maxpts)
        {
            log_errorF("%s: Invalid copy size\n", fcnm);
            continue;
        }
        dt = traceBuffer->traces[i].dt;
        if (j == 0)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            copyTrace(gps_data->data[k].npts,
                      traceBuffer->traces[i].data, 
                      gps_data->data[k].maxpts,
                      gps_data->data[k].ubuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
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
            copyTrace(gps_data->data[k].npts,
                      traceBuffer->traces[i].data, 
                      gps_data->data[k].maxpts,
                      gps_data->data[k].nbuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
        else if (j == 2)
        {
            gps_data->data[k].npts = traceBuffer->traces[i].ncopy;
            copyTrace(gps_data->data[k].npts,
                      traceBuffer->traces[i].data, 
                      gps_data->data[k].maxpts,
                      gps_data->data[k].ebuff);
            ISCL_memory_free__double(&traceBuffer->traces[i].data);
            //gps_data->data[k].epoch = traceBuffer->traces[i].t1;
        }
//    printf("%d %d\n", k, j);
    }
    return 0;
}
