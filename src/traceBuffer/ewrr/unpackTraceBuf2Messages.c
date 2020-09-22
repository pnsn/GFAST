#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/sorting/sorting.h"

#include "iscl/time/time.h"

static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp);
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp);
static int fastUnpack(const int npts, const int lswap,
                      const int type,
                      const char *__restrict__ msg,
                      int *__restrict__ resp);

/*!
 * @brief Unpacks the tracebuf2 messages read from the ring and returns
 *        the concatenated data for the desired SNCL's in the tb2Data struct
 *
 * @param[in] nRead          number of traces read off the ring
 * @param[in] msgs           tracebuf2 messages read off ring.  the i'th message
 *                           is given by (i-1)*MAX_TRACEBUF_SIZ for
 *                           i=1,2,...,nRead
 *
 * @param[in,out] tb2Data    on input contains the desired SNCL's whose data 
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
    struct tb2Data_struct *tb2Data)
{
    char *msg, netw[64], stat[64], chan[64], loc[64];
    TRACE2_HEADER traceHeader;
    //long *longData;
    //short *shortData;
    double *times, dt;
    int *nsamps;
    int *imap, *imapPtr, *imsg, *iperm, *kpts, *nmsg, *resp,
        dtype, i, i1, i2, ierr, im, indx, ir, k, kndx, l,
        lswap, nchunks, nReadPtr, nsamp0, npts, nsort;
    const int maxpts = MAX_TRACEBUF_SIZ/16; // MAX_TRACEBUF_SIZ/sizeof(int16_t)
    const bool clearSNCL = false;

    //char *msg_logos = (char *)malloc(nRead * 15 * sizeof(char));
    char **msg_logos = malloc(nRead * sizeof(char*));
    char logo[15];

    //------------------------------------------------------------------------//
    //
    // Check the tb2data was initialized
    if (!tb2Data->linit)
    {
        LOG_ERRMSG("%s", "tb2Data never initialized");
        return -1;
    }
    // Nothing to do
    if (tb2Data->ntraces == 0){return 0;}
    if (nRead == 0){return 0;}
//printf("%d\n", nRead);
    // Set the workspace
    msg   = memory_calloc8c(MAX_TRACEBUF_SIZ);
    imap  = memory_calloc32i(nRead+1);
    imsg  = memory_calloc32i(nRead);
    iperm = memory_calloc32i(nRead);
    kpts  = memory_calloc32i(tb2Data->ntraces);
    nmsg  = memory_calloc32i(tb2Data->ntraces);
    imapPtr = memory_calloc32i(nRead + 1); // worst case size
    times = memory_calloc64f(nRead);
    resp  = memory_calloc32i(maxpts);
    nsamps= memory_calloc32i(nRead);
    for (i=0; i<nRead+1; i++){imap[i] = tb2Data->ntraces + 1;}
    // Loop on waveforms and get workspace count
LOG_MSG("== [unpackTraceBuf t0:%f Zero Loop over SCNLs ntraces=%d nRead=%d]", ISCL_time_timeStamp(), tb2Data->ntraces, nRead);
    char *nn = NULL;
    char *ss = NULL;
    char *cc = NULL;
    char *ll = NULL;

    for (i=0; i<nRead; i++)
    {
        indx = i*MAX_TRACEBUF_SIZ;
        memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
        memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));

        msg_logos[i] = (char *)malloc(15);
        //char *msg_logos[i] = (char *)malloc(15 * sizeof(char));
        sprintf(msg_logos[i], "%s.%s.%s.%s",
                traceHeader.net, traceHeader.sta, traceHeader.chan, traceHeader.loc);
        times[i] = traceHeader.starttime;
        nsamps[i]= traceHeader.nsamp;
        //kpts[k] = kpts[k] + traceHeader.nsamp;
        //puts(&msg_logos[i]);
    }
/*
    for (i=0; i<nRead; i++)
    {
        printf("Logo:%s\n", msg_logos[i]);
    }
    printf("That's all folks!\n");
    exit(0);
*/

LOG_MSG("== [unpackTraceBuf t0:%f Xirst Loop over SCNLs ntraces=%d nRead=%d]", ISCL_time_timeStamp(), tb2Data->ntraces, nRead);
    for (k=0; k<tb2Data->ntraces; k++)
    {
        for (i=0; i<nRead; i++)
        {
          int j = 1;
          /*
            strcpy(logo, &msg_logos[i]);
            nn = strtok(logo, ".");
            ss = strtok(NULL, ".");
            cc = strtok(NULL, ".");
            ll = strtok(NULL, ".");
          */
        } // Loop on messages read
    } // Loop on waveforms

LOG_MSG("== [unpackTraceBuf t0:%f Xirst Loop over SCNLs DONE", ISCL_time_timeStamp());

LOG_MSG("== [unpackTraceBuf t0:%f First Loop over SCNLs ntraces=%d nRead=%d]", ISCL_time_timeStamp(), tb2Data->ntraces, nRead);
    for (k=0; k<tb2Data->ntraces; k++)
    {
        // Copy on the SNCL
/*
        memset(netw, 0, sizeof(netw));
        memset(stat, 0, sizeof(stat));
        memset(chan, 0, sizeof(chan));
        memset(loc,  0, sizeof(loc));
        strcpy(netw, tb2Data->traces[k].netw);
        strcpy(stat, tb2Data->traces[k].stnm);
        strcpy(chan, tb2Data->traces[k].chan); 
        strcpy(loc,  tb2Data->traces[k].loc);
*/
        // Loop on the messages and hunt for matching SNCL
        for (i=0; i<nRead; i++)
        {
/*
            indx = i*MAX_TRACEBUF_SIZ;
            memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
            memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
            // Get the bytes in right endianness
            nsamp0 = traceHeader.nsamp;
            ierr = WaveMsg2MakeLocal(&traceHeader);
            if (ierr < 0)
            {
                 LOG_ERRMSG("%s", "Error flipping bytes");
                 return -1;
            }
            // This is a match - update the 
            if ((strcasecmp(netw, traceHeader.net)  == 0) &&
                (strcasecmp(stat, traceHeader.sta)  == 0) &&
                (strcasecmp(chan, traceHeader.chan) == 0) &&
                (strcasecmp(loc,  traceHeader.loc)  == 0))

            if ((strcasecmp(netw, nn)  == 0) &&
                (strcasecmp(stat, ss)  == 0) &&
                (strcasecmp(chan, cc) == 0) &&
                (strcasecmp(loc,  ll)  == 0))

            if ((strcasecmp(tb2Data->traces[k].netw, nn)  == 0) &&
                (strcasecmp(tb2Data->traces[k].stnm, ss)  == 0) &&
                (strcasecmp(tb2Data->traces[k].chan, cc) == 0) &&
                (strcasecmp(tb2Data->traces[k].loc,  ll)  == 0))
            strcpy(logo, &msg_logos[i]);
*/
            strcpy(logo, msg_logos[i]);
            nn = strtok(logo, ".");
            ss = strtok(NULL, ".");
            cc = strtok(NULL, ".");
            ll = strtok(NULL, ".");

            if ((strcmp(tb2Data->traces[k].netw, nn)  == 0) &&
                (strcmp(tb2Data->traces[k].stnm, ss)  == 0) &&
                (strcmp(tb2Data->traces[k].chan, cc) == 0) &&
                (strcmp(tb2Data->traces[k].loc,  ll)  == 0))
            {
                printf("logo=%s matches %s.%s.%s.%s\n",
                        logo, nn, ss, cc, ll);
                if (imap[i] < tb2Data->ntraces + 1)
                {
                    LOG_ERRMSG("%s", "Error multiply mapped wave");
                    return -1;
                }
                imap[i] = k;
                imsg[i] = i;
                //npts = traceHeader.nsamp;
                npts = nsamps[i];
                if (npts < 0 || npts > maxpts)
                {
                    LOG_ERRMSG("Invalid number of points %d %d", npts, maxpts);
                    return -1;
                }
                //times[i] = traceHeader.starttime;
                //kpts[k] = kpts[k] + traceHeader.nsamp;
                kpts[k] = kpts[k] + nsamps[i];
                nmsg[k] = nmsg[k] + 1;
//printf("match %d %d %d %d\n", i, k, nRead, tb2Data->ntraces);
                //longData  = (long *)  (msg + sizeof(TRACE2_HEADER));
                //shortData = (short *) (msg + sizeof(TRACE2_HEADER));
                break;
            }
        } // Loop on messages read
    } // Loop on waveforms
LOG_MSG("== [unpackTraceBuf t0:%f First Loop over SCNLs DONE", ISCL_time_timeStamp());
    // Argsort the messages to their destinations (SNCLs).  Note, if using
    // intel performance primitives the sort will be stable.  Therefore, if
    // the messages are ordered temporally (more likely case) the unpacking
    // will be faster
//printf("%d\n", nRead);
    imap[nRead] =-1;
//LOG_MSG("== [unpackTraceBuf t0:%f call sorting_argsort32i_work]", ISCL_time_timeStamp());
    ierr = sorting_argsort32i_work(nRead, imap, SORT_ASCENDING, iperm);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error sorting messages");
        return -1;
    }
    // Apply the permutations
    ierr = sorting_applyPermutation32i_work(nRead, iperm, imap,  imap);
    ierr = sorting_applyPermutation32i_work(nRead, iperm, imsg,  imsg);
    ierr = sorting_applyPermutation64f_work(nRead, iperm, times, times);
    // Make a list so that the messages will be unpacked in order of
    // of SNCL matches as to reduce cache conflicts.
    nReadPtr = 0;
LOG_MSG("== [unpackTraceBuf t0:%f Second loop over ring msgs. nRead=%d]", ISCL_time_timeStamp(), nRead);
    for (i=0; i<nRead; i++)
    {
//printf("%d\n", imap[i]);
        if (imap[i] == tb2Data->ntraces + 1){break;} // Out of things to do 
        // update next station
        if (imap[i+1] != imap[i])
        {
            imapPtr[nReadPtr+1] = i + 1;
            i1 = imapPtr[nReadPtr];
            i2 = imapPtr[nReadPtr+1];
            // Do a partial sort based on start times
            nsort = i2 - i1;
            if (nsort > 1)
            {
//printf("sorting %d %d\n", i1, i2);
                // Verify sort is necessary (benefit of stable sort) 
                if (!sorting_issorted64f(nsort, &times[i1],
                                              SORT_ASCENDING, &ierr))
                {
                    ierr = sorting_argsort64f_work(nsort, &times[i1],
                                                        SORT_ASCENDING, iperm);
                    if (ierr != 0)
                    {
                        LOG_ERRMSG("%s", "Failed partial sort");
                        return -1;
                    }
                    sorting_applyPermutation32i_work(nsort, iperm,
                                                     &imap[i1],
                                                     &imap[i1]);
                    sorting_applyPermutation32i_work(nsort, iperm,
                                                     &imsg[i1],
                                                     &imsg[i1]);
                    sorting_applyPermutation64f_work(nsort, iperm,
                                                     &times[i1],
                                                     &times[i1]);
                }
            }
            else if (nsort == 0)
            {
                LOG_ERRMSG("%s", "Counting error");
                return -1;
            }
            nReadPtr = nReadPtr + 1;
        }
    //free(&msg_logos[i]);
    }
LOG_MSG("== [unpackTraceBuf t0:%f Second loop over ring msgs DONE]", ISCL_time_timeStamp());
    // Now set the workspace
    for (k=0; k<tb2Data->ntraces; k++)
    {
        traceBfufer_ewrr_freetb2Trace(clearSNCL, &tb2Data->traces[k]);
        if (kpts[k] > 0)
        {
            tb2Data->traces[k].data  = memory_calloc32i(kpts[k]);
            tb2Data->traces[k].times = memory_calloc64f(kpts[k]);
            tb2Data->traces[k].chunkPtr = memory_calloc32i(nmsg[k]+1);
            tb2Data->traces[k].npts = kpts[k];
        }
    }
//printf("nReadPtr: %d\n", nReadPtr);
    // Unpack the traces
LOG_MSG("== [unpackTraceBuf t0:%f Third loop over nReadPtr mapping]", ISCL_time_timeStamp());
    for (ir=0; ir<nReadPtr; ir++)
    {
        i1 = imapPtr[ir];
        i2 = imapPtr[ir+1];
        k = imap[i1];
        kndx = 0;
//printf("%d %d %d %d %d %d\n", tb2Data->ntraces, k, ir, i1, i2, kpts[k]);
        // Loop on the messages for this SNCL
        for (im=i1; im<i2; im++)
        {
            i = imsg[im];
            if (i < 0 || i >= nRead)
            {
                LOG_ERRMSG("Invalid message number %d", i);
                continue;
            }
            indx = i*MAX_TRACEBUF_SIZ;
            memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
            memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
            nsamp0 = traceHeader.nsamp;
            ierr = WaveMsg2MakeLocal(&traceHeader);
            if (ierr < 0)
            {
                 LOG_ERRMSG("%s", "Error flipping bytes");
            }
            dtype = 4;
            //if (strcasecmp(traceHeader.datatype, "s2\0") == 0 ||
                //strcasecmp(traceHeader.datatype, "i2\0") == 0)
            if (strcmp(traceHeader.datatype, "s2\0") == 0 ||
                strcmp(traceHeader.datatype, "i2\0") == 0)
            {
                dtype = 2;
            }
            lswap = 0;
            if (nsamp0 != traceHeader.nsamp){lswap = 1;}
            npts = traceHeader.nsamp;
            ierr = fastUnpack(npts, lswap, dtype, &msgs[indx], resp);
            if (ierr != 0)
            {
                LOG_ERRMSG("%s", "Error unpacking data");
            }
            // Update the points
            dt = 1.0/traceHeader.samprate;
            tb2Data->traces[k].dt = dt;
            // Is a new chunk beginning?
            if (im > i1)
            {
                if (fabs( (tb2Data->traces[k].times[kndx] + dt)
                        - traceHeader.starttime ) < 1.e-6)
                {
                    tb2Data->traces[k].nchunks = tb2Data->traces[k].nchunks + 1;
                    nchunks = tb2Data->traces[k].nchunks;
                    tb2Data->traces[k].chunkPtr[nchunks] = kndx + 1;
                }
            }
            // Update the points
#ifdef _OPENMP
            #pragma omp simd
#endif
            for (l=0; l<npts; l++)
            {
                tb2Data->traces[k].data[kndx+l] = resp[l];
                tb2Data->traces[k].times[kndx+l] = traceHeader.starttime
                                                 + (double) l*dt;
            }
            kndx = kndx + npts; 
// printf("%16.8f %s %s %s %s %d %f\n", traceHeader.starttime,
//                                traceHeader.net, traceHeader.sta,
//                                traceHeader.chan, traceHeader.loc,
//                                traceHeader.nsamp, (double) resp[0]/1000000); 
        } // Loop on messages for this SNCL
        // Special case for one message
        if (i2 - i1 == 1 && kpts[k] > 0)
        {
            tb2Data->traces[k].nchunks = 1;
            tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kpts[k];
        }
        // Reality check
        if (kndx != kpts[k])
        {
            LOG_ERRMSG("Lost count %d %d", kndx, kpts[k]);
            return -1;
        }
        if (tb2Data->traces[k].nchunks > 0)
        {
            nchunks = tb2Data->traces[k].nchunks;
            if (tb2Data->traces[k].chunkPtr[nchunks] != tb2Data->traces[k].npts)
            {
                LOG_ERRMSG("Inconsistent number of points %d %d",
                           tb2Data->traces[k].chunkPtr[nchunks],
                           tb2Data->traces[k].npts);
                return -1;
            }
        }
    } // Loop on pointers
LOG_MSG("== [unpackTraceBuf t0:%f Third loop over nReadPtr mapping DONE]", ISCL_time_timeStamp());

    // Free space
    memory_free8c(&msg);
    memory_free32i(&imap);
    memory_free32i(&imsg);
    memory_free32i(&iperm);
    memory_free32i(&kpts);
    memory_free32i(&nmsg);
    memory_free32i(&resp);
    memory_free64f(&times);
    memory_free32i(&imapPtr);
    memory_free32i(&nsamps);
    free(msg_logos);
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
 * @bug This doesn't vectorize.
 *
 */
static void fastUnpackI4(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c4[4];
    int i, i4;
    if (lswap == 0)
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[0] = msg[ioff+4*i+0];
            c4[1] = msg[ioff+4*i+1]; 
            c4[2] = msg[ioff+4*i+2]; 
            c4[3] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (int) i4; 
        }
    }
    else
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c4[3] = msg[ioff+4*i+0];
            c4[2] = msg[ioff+4*i+1]; 
            c4[1] = msg[ioff+4*i+2]; 
            c4[0] = msg[ioff+4*i+3];
            memcpy(&i4, c4, sizeof(int));
            resp[i] = (int) i4;
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
 * @bug This doesn't vectorize.
 *
 */
static void fastUnpackI2(const int npts, const int lswap,
                         const char *__restrict__ msg,
                         int *__restrict__ resp)
{
    const int ioff = (int) (sizeof(TRACE2_HEADER));
    char c2[2];
    int i;
    short i2;
    if (lswap == 0)
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[0] = msg[ioff+2*i+0];
            c2[1] = msg[ioff+2*i+1];
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (int) i2;
        }
    }
    else
    {
        //#pragma omp simd
        for (i=0; i<npts; i++)
        {
            c2[1] = msg[ioff+2*i+0];
            c2[0] = msg[ioff+2*i+1]; 
            memcpy(&i2, c2, sizeof(short));
            resp[i] = (int) i2;
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
                      int *__restrict__ resp)
{
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
        LOG_ERRMSG("%s", "Invalid type");
        return -1;
    }
    return 0;
}
