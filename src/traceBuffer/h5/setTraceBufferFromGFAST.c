#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"

int traceBuffer_h5_setTraceBufferFromGFAST(
    const double bufflen,
    struct GFAST_data_struct gps_data,
    struct h5traceBuffer_struct *traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_setTraceBufferFromGFAST\0";
    char temp[1024];
    int i, j, k;
    // Count the number of traces
    traceBuffer->ntraces = 0;
    for (k=0; k<gps_data.stream_length; k++)
    {
        if (gps_data.data[k].lskip_pgd &&
            gps_data.data[k].lskip_cmt && 
            gps_data.data[k].lskip_ff)
        {
            continue;
        }
        traceBuffer->ntraces = traceBuffer->ntraces + 3; // Z, N, E
    }
    if (traceBuffer->ntraces < 1)
    {
        log_errorF("%s: Error no traces to read!\n", fcnm);
        return -1;
    }
    i = 0;
    traceBuffer->traces = calloc(traceBuffer->ntraces,
                                 sizeof(struct h5trace_struct));
    for (k=0; k<gps_data.stream_length; k++)
    {
        if (gps_data.data[k].lskip_pgd &&
            gps_data.data[k].lskip_cmt &&  
            gps_data.data[k].lskip_ff)
        {
            continue;
        }
        for (j=0; j<3; j++)
        {
            memset(temp, 0, sizeof(temp));
            strcpy(temp, "/Data/\0");
            strcat(temp, gps_data.data[k].netw);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].stnm);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].chan[j]);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].loc);
            traceBuffer->traces[i].idest = k*3 + j;
            traceBuffer->traces[i].groupName
                = (char *)calloc(strlen(temp)+1, sizeof(char)); 
            strcpy(traceBuffer->traces[i].groupName, temp);
            traceBuffer->traces[i].dt = gps_data.data[k].dt;
            traceBuffer->traces[i].maxpts  
                = (int) (bufflen/traceBuffer->traces[i].dt + 0.5) + 1;
            i = i + 1; 
        }
    }
    return 0;
}
