#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"
#include "iscl/sorting/sorting.h"

static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         double *__restrict__ resp);
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         double *__restrict__ resp);
static int fastUnpack(const int npts, const int lswap,
                      const int type,
                      const char *__restrict__ msg,
                      double *__restrict__ resp);

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
    double *resp, *times, dt;
    int *imap, *imapPtr, *imsg, *iperm, *jperm, *kpts, *npts,
        dtype, i, i1, i2, ierr, im, indx, ir, k, lswap, nReadPtr, nsamp0, nsort;
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
    imsg  = ISCL_memory_calloc__int(nRead);
    npts  = ISCL_memory_calloc__int(nRead); 
    iperm = ISCL_memory_calloc__int(nRead);
    jperm = ISCL_memory_calloc__int(nRead);
    kpts  = ISCL_memory_calloc__int(h5traces->ntraces);
    imapPtr = ISCL_memory_calloc__int(nRead + 1); // worst case size
    times = ISCL_memory_calloc__double(nRead);
    resp  = ISCL_memory_calloc__double(MAX_TRACEBUF_SIZ/8);
    for (i=0; i<nRead+1; i++){imap[i] = h5traces->ntraces+1;}
    // Loop on waveforms and get workspace count
    for (k=0; k<h5traces->ntraces; k++)
    {
        // Copy on the SNCL
        memset(netw, 0, sizeof(netw));
        memset(stat, 0, sizeof(stat));
        memset(chan, 0, sizeof(chan));
        memset(loc,  0, sizeof(loc));
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
            nsamp0 = traceHeader.nsamp;
            ierr = WaveMsg2MakeLocal(&traceHeader);
            if (ierr < 0)
            {
                 log_errorF("%s: Error flipping bytes\n", fcnm);
            }
            // This is a match - update the 
            if ((strcasecmp(netw, traceHeader.net)  == 0) &&
                (strcasecmp(stat, traceHeader.sta)  == 0) &&
                (strcasecmp(chan, traceHeader.chan) == 0) &&
                (strcasecmp(loc,  traceHeader.loc)  == 0))
            {
                if (imap[i] > h5traces->ntraces)
                {
                    log_errorF("%s: Error multiply mapped wave\n", fcnm);
                }
                imap[i] = k;
                imsg[i] = i;
                npts[i] = traceHeader.nsamp;
                times[i] = traceHeader.starttime;
                kpts[k] = kpts[k] + traceHeader.nsamp;
                // Verify the sampling periods are consistent
                dt = 1.0/traceHeader.samprate;
printf("match %d %d %d %d\n", i, k, nRead, h5traces->ntraces);
                if (fabs(h5traces->traces[k].dt - dt) > 1.e-5)
                {
                    log_errorF("%s: Sampling period mismatch %f %f\n",
                               fcnm, dt, h5traces->traces[k].dt);
                }
                //longData  = (long *)  (msg + sizeof(TRACE2_HEADER));
                //shortData = (short *) (msg + sizeof(TRACE2_HEADER));
                break;
            }
        } // Loop on messages read
    } // Loop on waveforms
printf("here\n");
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
    ierr = ISCL__sorting_applyPermutation__int(nRead,    iperm, imap,  imap);
    ierr = ISCL__sorting_applyPermutation__int(nRead,    iperm, imsg,  imsg);
    ierr = ISCL__sorting_applyPermutation__int(nRead,    iperm, npts,  npts);
    ierr = ISCL__sorting_applyPermutation__double(nRead, iperm, times, times);
    // Make a list so that the messages will be unpacked in order of
    // of SNCL matches as to reduce cache conflicts.
    nReadPtr = 0;
    for (i=0; i<nRead; i++)
    {
        // update next station
        if (imap[i+1] != imap[i])
        {
            imapPtr[nReadPtr+1] = i + 1;
            i1 = imapPtr[nReadPtr];
            i2 = imapPtr[nReadPtr+1];
            // Do a partial sort based on start times
            nsort = i2 - i1 + 1;
            if (nsort > 1)
            {
                // Verify sort is necessary (benefit of stable sort) 
                if (!ISCL_sorting_issorted__double(nsort, &times[i1],
                                                   ASCENDING))
                {
                    ierr = ISCL__sorting_argsort__double(nsort, &times[i1],
                                                         ASCENDING, jperm);
                    if (ierr != 0)
                    {
                        log_errorF("%s: Failed partial sort\n", fcnm);
                        return -1;
                    }
                    ISCL__sorting_applyPermutation__int(nsort, jperm,
                                                        &imap[i1],
                                                        &imap[i1]);
                    ISCL__sorting_applyPermutation__int(nsort, jperm,
                                                        &imsg[i1],
                                                        &imsg[i1]);
                    ISCL__sorting_applyPermutation__int(nsort, jperm,
                                                        &npts[i1],
                                                        &npts[i1]);
                    ISCL__sorting_applyPermutation__double(nsort, jperm,
                                                           &times[i1],
                                                           &times[i1]);
                }
            }
            else if (nsort == 0)
            {
                log_errorF("%s: Counting error\n", fcnm);
                return -1;
            }
            nReadPtr = nReadPtr + 1;
        }
    }
    // Now set the workspace
    for (k=0; k<h5traces->ntraces; k++)
    {
        if (kpts[k] > 0)
        {
            //h5traces->traces[k].data = ISCL_memory_calloc__double(kpts[k]);
        }
    }
    // Unpack the traces
    for (ir=0; ir<nReadPtr; ir++)
    {
        i1 = imapPtr[ir];
        i2 = imapPtr[ir+1];
        k = imap[i1];
        // Loop on the messages for this SNCL
        for (im=i1; im<i2; im++)
        {
            i = imsg[im];
printf("%d %d %d %d\n", i1, i2, imap[im], i); 
            indx = i*MAX_TRACEBUF_SIZ;

            memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
            memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
            nsamp0 = traceHeader.nsamp;
            ierr = WaveMsg2MakeLocal(&traceHeader);
            if (ierr < 0)
            {
                 log_errorF("%s: Error flipping bytes\n", fcnm);
            }
            dtype = 4;
            if (strcasecmp(traceHeader.datatype, "s2\0") == 0 ||
                strcasecmp(traceHeader.datatype, "i2\0") == 0)
            {
                dtype = 2;
            }
            lswap = 0;
            if (nsamp0 != traceHeader.nsamp){lswap = 1;}
            ierr = fastUnpack(npts[i], lswap, dtype, &msgs[indx], resp);
            if (ierr != 0)
            {
                log_errorF("%s: Error unpacking data\n", fcnm);
            }
        }
    }

    // Free space
    ISCL_memory_free__char(&msg);
    ISCL_memory_free__int(&imap);
    ISCL_memory_free__int(&imsg);
    ISCL_memory_free__int(&npts);
    ISCL_memory_free__int(&iperm);
    ISCL_memory_free__int(&jperm);
    ISCL_memory_free__int(&kpts);
    ISCL_memory_free__double(&times);
    ISCL_memory_free__double(&resp);
    ISCL_memory_free__int(&imapPtr);
    return 0;
}
//============================================================================//
/*!
 * @brief Unpacks a 4 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         double *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c4[4];
    int i, i4;
    if (lswap == 0)
    {
        #pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[0] = msg[ioff+4*i+0];
            c4[1] = msg[ioff+4*i+1]; 
            c4[2] = msg[ioff+4*i+2]; 
            c4[3] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (double) i4; 
        }
    }
    else
    {
        #pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[3] = msg[ioff+4*i+0];
            c4[2] = msg[ioff+4*i+1]; 
            c4[1] = msg[ioff+4*i+2]; 
            c4[0] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (double) i4;
        }
    }
    return;
}
//============================================================================//
/*!
 * @brief Unpacks a 2 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         double *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c2[2];
    int i;
    short i2;
    if (lswap == 0)
    {
        #pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[0] = msg[ioff+2*i+0];
            c2[1] = msg[ioff+2*i+1];
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (double) i2;
        }
    }
    else
    {
        #pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[1] = msg[ioff+2*i+0];
            c2[0] = msg[ioff+2*i+1]; 
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (double) i2;
        }
    }   
    return;
}
//============================================================================//
/*!
 * @brief Unpacks a 2 or 4 byte integer character data
 *
 * @param[in] npts    number of points to unpack
 * @param[in] lswap   if 0 then do not byte swap the data.
 *                    if 1 then do byte swap the data.
 * @param[in] dtype   if 4 then the data is 4 bytes.
 *                    if 2 then the data is 2 bytes.
 * @param[in] msg     tracebuf2 message to unpack [MAX_TRACEBUF_SIZ]
 *
 * @param[out] resp   response data [npts]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
static int fastUnpack(const int npts, const int lswap,
                      const int dtype,
                      const char *__restrict__ msg,
                      double *__restrict__ resp)
{
    const char *fcnm = "fastUnpack\0";
    if (npts < 1){return 0;} // Nothing to do
    if (dtype == 4)
    {
        fastUnpackI4(npts, lswap, msg, resp);
    }
    else if (dtype == 2)
    {
        fastUnpackI2(npts, lswap, msg, resp);
    }
    else
    {
        log_errorF("%s: Invalid type\n", fcnm);
        return -1;
    }
    return 0;
}
