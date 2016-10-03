#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
#include "iscl/sorting/sorting.h"

/*!
 * @brief Unpacks the tracebuf2 message and returns the data for
 *        the desired SNCL's in the h5traces struct
 *
 * @param[in] nRead          number of traces read off the ring
 * @param[in] msgs           tracebuf2 messages read off ring.  the i'th message
 *                           is given by (i-1)*MAX_TRACEBUF_SIZ for
 *                           i=1,2,...,nRead
 *
 * @param[in,out] h5traces   on input contains the desired SNCL's whose data 
 *                           will be unpacked from the header (should it be
 *                           present).  
 *                           on output contains the data in the messages for
 *                           the SNCL's defined in this structure (should a 
 *                           match be found).
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_ewrr_unpackTraceBuf2Messages(
    const int nRead,
    const char *msgs,
    struct h5traceBuffer_struct *h5traces)
{
    const char *fcnm = "traceBuffer_ewrr_unpackTraceBuf2Messages\0";
    char *msg, netw[64], stat[64], chan[64], loc[64];
    TRACE2_HEADER traceHeader;
    //long *longData;
    //short *shortData;
    double *times, dt;
    int *imap, *imapPtr, *iperm, *kpts, *npts, i, i1, i2, ierr, im, indx,
         k, nReadPtr, nsamp0;
    bool *lswap;
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
    msg   = ISCL_memory_calloc__char(MAX_TRACEBUF_SIZ);
    imap  = ISCL_memory_calloc__int(nRead+1);
    npts  = ISCL_memory_calloc__int(nRead); 
    iperm = ISCL_memory_calloc__int(nRead);
    kpts  = ISCL_memory_calloc__int(h5traces->ntraces);
    lswap = ISCL_memory_calloc__bool(nRead);
    times = ISCL_memory_calloc__double(nRead);
    for (i=0; i<nRead+1; i++){imap[i] =-1;}
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
            memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
            // Get the bytes in right endianness
            lswap[i] = false;
            nsamp0 = traceHeader.nsamp;
            ierr = WaveMsg2MakeLocal(&traceHeader);
            if (ierr < 0)
            {
                 log_errorF("%s: Error flipping bytes\n", fcnm);
            }
            if (nsamp0 != traceHeader.nsamp){lswap[i] = true;}
            // This is a match - update the 
            if ((strcasecmp(netw, traceHeader.net)  == 0) &&
                (strcasecmp(stat, traceHeader.sta)  == 0) &&
                (strcasecmp(chan, traceHeader.chan) == 0) &&
                (strcasecmp(loc,  traceHeader.loc)  == 0))
            {
                if (imap[i] >= 0)
                {
                    log_errorF("%s: Error multiply mapped station\n", fcnm);
                }
                imap[i] = k;
                npts[i] = traceHeader.nsamp;
                times[i] = traceHeader.starttime; 
                kpts[k] = kpts[k] + npts[i];
                // Verify the sampling periods are consistent
                dt = 1.0/traceHeader.samprate;
                if (fabs(h5traces[k].traces->dt - dt) > 1.e-5)
                {
                    log_errorF("%s: Sampling period mismatch %f %f\n",
                               fcnm, dt, h5traces[k].traces->dt);
                }
                //longData  = (long *)  (msg + sizeof(TRACE2_HEADER));
                //shortData = (short *) (msg + sizeof(TRACE2_HEADER));
                break;
            }
        } // Loop on messages read
    } // Loop on waveforms
    // Argsort the messages to their destinations (SNCLs).  Note, if using
    // intel performance primitives the sort will be stable.  Therefore, if
    // the messages are ordered temporally (more likely case) the unpacking
    // will be faster
    imap[nRead] =-1;
    ierr = ISCL__sorting_argsort__int(nRead, imap, ASCENDING, iperm);
    if (ierr != 0)
    {
        log_errorF("%s: Error sorting messages\n", fcnm);
        return -1;
    }
    // Apply the permutations
    
    // Make a list so that the messages will be unpacked in order of
    // of SNCL matches as to reduce cache conflicts.
    nReadPtr = 0;
    imapPtr = ISCL_memory_calloc__int(nRead + 1); // worst case size
    for (i=0; i<nRead; i++)
    {
        if (imap[iperm[i+1]] != imap[iperm[i]])
        {
            imapPtr[nReadPtr+1] = i + 1;
            nReadPtr = nReadPtr + 1;
        }
    }
/*
    // Now set the workspace
    for (k=0; k<h5traces->ntraces; k++)
    {
        if (kpts[k] > 0)
        {
            h5traces[k].traces->data = ISCL_memory_calloc__double(kpts[k]);
        }
    }
    // Unpack the traces
    for (i=0; i<nReadPtr; i++)
    {
        i1 = imapPtr[i];
        i2 = imapPtr[i+1];
        k = imap[iperm[i]];
        // Loop on the messages for this SNCL
        for (im=i1; im<i2; im++)
        {
 
        }
    }
*/
    // Finally unpack the data

    // Free space
    ISCL_memory_free__char(&msg);
    ISCL_memory_free__int(&imap);
    ISCL_memory_free__int(&npts);
    ISCL_memory_free__int(&iperm);
    ISCL_memory_free__int(&kpts);
    ISCL_memory_free__bool(&lswap);
    ISCL_memory_free__double(&times);
    ISCL_memory_free__int(&imapPtr);
    return 0;
}
