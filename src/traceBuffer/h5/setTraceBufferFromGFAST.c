#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Sets the HDF5 traceBuffer from the GFAST structure
 *
 * @param[in] bufflen       max time window in h5traceBuffer TODO: should delete
 * @param[in] gps_data      the GPS streams and basic metadata that will be 
 *                          requested
 *
 * @param[out] traceBuffer  contains a map from the GPS data to the data that 
 *                          is or will be in the H5 file
 *
 * @result 0 indicates success
 *
 */
int traceBuffer_h5_setTraceBufferFromGFAST(
    const double bufflen,
    struct GFAST_data_struct gps_data,
    struct h5traceBuffer_struct *traceBuffer)
{
    const char *fcnm = "traceBuffer_h5_setTraceBufferFromGFAST\0";
    char temp[1024];
    double *dtGroups, dt0, dt;
    int *ndtGroups, i, itn, j, k, ng;
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
    // Set the sampling period group numbers
    dtGroups = (double *)
               calloc((size_t) gps_data.stream_length, sizeof(double));
    ndtGroups = (int *)
                calloc((size_t) gps_data.stream_length, sizeof(int)); 
    ng = 0;
    for (k=0; k<gps_data.stream_length; k++)
    {
        dt = gps_data.data[k].dt;
        for (j=0; j<ng; j++)
        {
            if (fabs(dtGroups[j] - dt) < 1.e-8)
            {
                ndtGroups[k] = j + 1;
                goto NEXT_STATION;
            }
        }
        dtGroups[ng] = dt;
        ndtGroups[k] = ng + 1;
        ng = ng + 1;
NEXT_STATION:;
    }
    // Check
    if (ng < 1)
    {
        log_errorF("%s: Error no dt groups\n", fcnm);
        return -1;
    }
    if (ng > 1)
    {
        log_warnF("%s: Multiple sampling periods is untested\n", fcnm);
        log_warnF("%s: argsort the ndtGroups and fill in order\n", fcnm);
    }
    dt0 = (double) NAN;
    traceBuffer->dtPtr = ISCL_memory_calloc__int(ng + 1);
    traceBuffer->dtGroupName = (char **)
                               calloc((size_t) ng, sizeof(char *));
    traceBuffer->ndtGroups = ng;
    traceBuffer->traces = (struct h5trace_struct *)
                          calloc((size_t) traceBuffer->ntraces,
                                 sizeof(struct h5trace_struct));
    ng = 0;
    i = 0;
    itn = 0;
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
            traceBuffer->traces[i].dtGroupNumber = ndtGroups[k];

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
            sprintf(temp, "/Data/SamplingPeriodGroup_%d/",
                    traceBuffer->traces[i].dtGroupNumber);
            //strcat(temp, gps_data.data[k].netw);
            //strcat(temp, ".\0");
            //strcat(temp, gps_data.data[k].stnm);
            //strcat(temp, ".\0");
            //strcat(temp, gps_data.data[k].chan[j]);
            //strcat(temp, ".\0");
            //strcat(temp, gps_data.data[k].loc);
            traceBuffer->traces[i].idest = k*3 + j;
            traceBuffer->traces[i].groupName
                = (char *)calloc(strlen(temp)+1, sizeof(char));
            strcpy(traceBuffer->traces[i].groupName, temp);
            if (isnan(dt0))
            {
                traceBuffer->dtGroupName[ng] = (char *)calloc(64, sizeof(char)); 
                sprintf(traceBuffer->dtGroupName[ng],
                        "/Data/SamplingPeriodGroup_%d/", ng+1); 
                dt0 = gps_data.data[k].dt;
                itn = 0;
                ng = ng + 1;
            }
            if (fabs(dt0 - gps_data.data[k].dt)/dt0 > 1.e-10)
            {
                log_errorF("%s: Error can't yet handle multirate data\n",
                           fcnm);
                return -1;
            }
            traceBuffer->traces[i].traceNumber = itn;
            traceBuffer->traces[i].dt = gps_data.data[k].dt;
            traceBuffer->traces[i].slat = gps_data.data[k].sta_lat;
            traceBuffer->traces[i].slon = gps_data.data[k].sta_lon;
            traceBuffer->traces[i].selev = gps_data.data[k].sta_alt;
            traceBuffer->traces[i].maxpts
                = (int) (bufflen/traceBuffer->traces[i].dt + 0.5) + 1;
            traceBuffer->traces[i].dt = gps_data.data[k].dt;
            traceBuffer->traces[i].gain = gps_data.data[k].gain[j];
            i = i + 1; 
            itn = itn + 1;
        }
    }
    traceBuffer->dtPtr[ng] = traceBuffer->ntraces; 
    // free space
    free(dtGroups);
    free(ndtGroups);
    return 0;
}
