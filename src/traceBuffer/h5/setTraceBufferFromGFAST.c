#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
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
    char temp[1024];
    double *dtGroups, dt0, dt;
    int *ndtGroups, i, itn, j, k, ng;
    int debug = 0;
    if (debug) {
        LOG_DEBUGMSG("%s", "Entering traceBuffer_h5_setTraceBufferFromGFAST");
    }
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
        traceBuffer->ntraces = traceBuffer->ntraces + 6; // Z, N, E, 3, 2, 1
    }
    if (traceBuffer->ntraces < 1)
    {
        LOG_ERRMSG("%s", "Error no traces to read!");
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
        LOG_ERRMSG("%s", "Error no dt groups");
        return -1;
    }
    if (ng > 1)
    {
        LOG_WARNMSG("%s", "Multiple sampling periods is untested");
        LOG_WARNMSG("%s", "argsort the ndtGroups and fill in order");
    }
    dt0 = (double) NAN;
    traceBuffer->dtPtr = memory_calloc32i(ng + 1);
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
        // Loop for number of channels (z, n, e, 3, 2, 1)
        for (j = 0; j < 6; j++)
        {
            memset(traceBuffer->traces[i].netw, 0,
                   sizeof(traceBuffer->traces[i].netw));
            memset(traceBuffer->traces[i].stnm, 0,
                   sizeof(traceBuffer->traces[i].stnm));
            memset(traceBuffer->traces[i].chan, 0,
                   sizeof(traceBuffer->traces[i].chan));
            memset(traceBuffer->traces[i].loc, 0,
                   sizeof(traceBuffer->traces[i].loc));
            strcpy(traceBuffer->traces[i].netw, gps_data.data[k].netw);
            strcpy(traceBuffer->traces[i].stnm, gps_data.data[k].stnm); 
            strcpy(traceBuffer->traces[i].chan, gps_data.data[k].chan[j]);
            strcpy(traceBuffer->traces[i].loc,  gps_data.data[k].loc);
            traceBuffer->traces[i].dtGroupNumber = ndtGroups[k];
            traceBuffer->traces[i].idest = k * 6 + j;

            if (debug) {
                LOG_DEBUGMSG("setH5: Check traceBuffer->traces[%d].chan = %s",
                    i, traceBuffer->traces[i].chan)
            }

            memset(temp, 0, sizeof(temp));
            strcpy(temp, "/MetaData/\0");
            strcat(temp, gps_data.data[k].netw);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].stnm);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].chan[j]);
            strcat(temp, ".\0");
            strcat(temp, gps_data.data[k].loc);
            traceBuffer->traces[i].metaGroupName
                = (char *)calloc(strlen(temp)+1, sizeof(char));
            strcpy(traceBuffer->traces[i].metaGroupName, temp);

            memset(temp, 0, sizeof(temp)); 
            sprintf(temp, "/Data/SamplingPeriodGroup_%d/",
                    traceBuffer->traces[i].dtGroupNumber);
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
                LOG_ERRMSG("%s", "Error can't yet handle multirate data");
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

            if (debug) {
                LOG_DEBUGMSG("setH5: Made trace %d, groupName: %s, dtGroupNumber: %d, idest: %d",
                    i,
                    traceBuffer->traces[i].groupName,
                    traceBuffer->traces[i].dtGroupNumber,
                    traceBuffer->traces[i].idest)
            }

            i = i + 1; 
            itn = itn + 1;
        }
    }
    traceBuffer->dtPtr[ng] = traceBuffer->ntraces; 

    if (debug) {
        LOG_DEBUGMSG("setH5: ng: %d, ndtGroups: %d, ntraces: %d",
            ng,
            traceBuffer->ndtGroups,
            traceBuffer->ntraces)
    }

    // free space
    free(dtGroups);
    free(ndtGroups);
    return 0;
}
