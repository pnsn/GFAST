#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
    double dt0;
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
    dt0 = (double) NAN;
    traceBuffer->traces = (struct h5trace_struct *)
                          calloc((unsigned long) traceBuffer->ntraces,
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
            memset(traceBuffer->traces[i].netw, 0,
                   sizeof(traceBuffer->traces[i].netw));
            memset(traceBuffer->traces[i].stnm, 0,
                   sizeof(traceBuffer->traces[i].stnm));
            memset(traceBuffer->traces[i].chan, 0,
                   sizeof(traceBuffer->traces[i].chan[j]));
            memset(traceBuffer->traces[i].loc, 0,
                   sizeof(traceBuffer->traces[i].loc));
            strcpy(traceBuffer->traces[i].netw, gps_data.data[k].netw);
            strcpy(traceBuffer->traces[i].stnm, gps_data.data[k].stnm); 
            strcpy(traceBuffer->traces[i].chan, gps_data.data[k].chan[j]);
            strcpy(traceBuffer->traces[i].loc,  gps_data.data[k].loc);

            memset(temp, 0, sizeof(temp));
            strcpy(temp, "/MetaData/\0");
            strcat(temp, gps_data.data[k].netw);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].stnm);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].chan[j]);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].loc);
            traceBuffer->traces[i].idest = k*3 + j;
            traceBuffer->traces[i].metaGroupName
                = (char *)calloc(strlen(temp)+1, sizeof(char));
            strcpy(traceBuffer->traces[i].metaGroupName, temp);

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
            if (isnan(dt0)){dt0 = gps_data.data[k].dt;}
            if (fabs(dt0 - gps_data.data[k].dt)/dt0 > 1.e-10)
            {
                log_errorF("%s: Error can't yet handle multirate data\n",
                           fcnm);
                return -1;
            }
            traceBuffer->traces[i].dt = gps_data.data[k].dt;
            traceBuffer->traces[i].slat = gps_data.data[k].sta_lat;
            traceBuffer->traces[i].slon = gps_data.data[k].sta_lon;
            traceBuffer->traces[i].selev = gps_data.data[k].sta_alt;
            traceBuffer->traces[i].maxpts
                = (int) (bufflen/traceBuffer->traces[i].dt + 0.5) + 1;
            traceBuffer->traces[i].dt = gps_data.data[k].dt;
            i = i + 1; 
        }
    }
    return 0;
}
