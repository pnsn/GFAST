#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

int traceBuffer_ewrr_unpackTraceBuf2Messages(
    const int nRead,
    const char *msgs,
    struct h5traceBuffer_struct *h5traces)
{
    const char *fcnm = "traceBuffer_ewrr_unpackTraceBuf2Messages\0";
    char *msg, netw[64], stat[64], chan[64], loc[64];
    TRACE2_HEADER *traceHeader;
    long *longData;
    short *shortData;
    int i, ierr, indx, k, nsamp0;
    bool lswap;
    //------------------------------------------------------------------------//
    //
    // Check the h5traces was initialized
    if (!h5traces->linit)
    {
        log_errorF("%s: h5traces never initialized\n", fcnm);
        return -1;
    } 
    // Nothing to do
    if (h5traces->ntraces == 0){return 0;}
    if (nRead == 0){return 0;}
    // Set the workspace
    msg = ISCL_memory_calloc__char(MAX_TRACEBUF_SIZ);
    // Loop on waveforms and get workspace count
    for (k=0; k<h5traces->ntraces; k++)
    {
        // Copy on the SNCL
        strcpy(netw, h5traces->traces[k].netw);
        strcpy(stat, h5traces->traces[k].stnm);
        strcpy(chan, h5traces->traces[k].chan); 
        strcpy(loc,  h5traces->traces[k].loc);
        // Loop on the messages and hunt for matching SNCL
        for (i=0; i<nRead; i++)
        {
            indx = i*MAX_TRACEBUF_SIZ;
            memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
            traceHeader = (TRACE2_HEADER *) msg;
            // Get the bytes in right endianness
            lswap = false;
            nsamp0 = traceHeader->nsamp;
            ierr = WaveMsg2MakeLocal(traceHeader);
            if (ierr < 0)
            {
                 log_errorF("%s: Error flipping bytes\n", fcnm);
            }
            if (nsamp0 != traceHeader->nsamp){lswap = true;}
            if ((strcasecmp(netw, traceHeader->net)  == 0) &&
                (strcasecmp(stat, traceHeader->sta)  == 0) &&
                (strcasecmp(chan, traceHeader->chan) == 0) &&
                (strcasecmp(loc,  traceHeader->loc)  == 0))
            {
                longData  = (long *)  (msg + sizeof(TRACE2_HEADER));
                shortData = (short *) (msg + sizeof(TRACE2_HEADER));
                break;
            }
        } // Loop on messages read
    } // Loop on waveforms
    ISCL_memory_free__char(&msg);
    return 0;
}
