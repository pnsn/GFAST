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
    TRACE2_HEADER  *trh;
    //long *longData;
    //short *shortData;
    double *times, dt;
    int *nsamps;
    int *imap, *imapPtr, *imsg, *iperm, *kpts, *nmsg, *resp,
        dtype, i, i1, i2, ierr, im, indx, ir, k, kndx, l,
        lswap, nchunks, nReadPtr, nsamp0, npts, nsort;
    const int maxpts = MAX_TRACEBUF_SIZ/16; // MAX_TRACEBUF_SIZ/sizeof(int16_t)
    const bool clearSNCL = false;

    //char **msg_logos = (char **)malloc(sizeof(char *) * nRead);
    char buf[15];
    char msg_logos[nRead][15];
    char *logo;
    char *nn = NULL;
    char *ss = NULL;
    char *cc = NULL;
    char *ll = NULL;

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
    logo  = memory_calloc8c(15);

    for (i=0; i<nRead+1; i++){imap[i] = tb2Data->ntraces + 1;}

    // MTH: load up the msg logos once
    for (i=0; i<nRead; i++)
    {
        indx = i*MAX_TRACEBUF_SIZ;
        trh  = (TRACE2_HEADER *) &msgs[indx];
        //msg_logos[i] = (char *)malloc(15);
        sprintf(msg_logos[i], "%s.%s.%s.%s",
                trh->net, trh->sta, trh->chan, trh->loc);
        times[i] = trh->starttime;
        nsamps[i]= trh->nsamp;
        if (strcmp(msg_logos[i], "CI.0001.LYZ.20")==0) {
          printf("unpackTB2: msg_logos[%d]=%s time:%f nsamps:%d nRead:%d\n",
              i, msg_logos[i], times[i], nsamps[i], nRead);
        }
    }

    // Loop on waveforms and get workspace count

LOG_DEBUGMSG("== [unpackTraceBuf t0:%f First Loop over SCNLs ntraces=%d nRead=%d]", ISCL_time_timeStamp(), tb2Data->ntraces, nRead);
    for (k=0; k<tb2Data->ntraces; k++)
    {
        // Copy on the SNCL
        // Loop on the messages and hunt for matching SNCL
        for (i=0; i<nRead; i++)
        {
            memcpy(logo, msg_logos[i], 15);
            //puts(logo);
            nn = strtok(logo, ".");
            ss = strtok(NULL, ".");
            cc = strtok(NULL, ".");
            ll = strtok(NULL, ".");

            if ((strcmp(tb2Data->traces[k].netw, nn)  == 0) &&
                (strcmp(tb2Data->traces[k].stnm, ss)  == 0) &&
                (strcmp(tb2Data->traces[k].chan, cc) == 0) &&
                (strcmp(tb2Data->traces[k].loc,  ll)  == 0))
            {
              /*
                printf("%s.%s.%s.%s matches %s.%s.%s.%s\n",
                        tb2Data->traces[k].netw, tb2Data->traces[k].stnm,
                        tb2Data->traces[k].chan, tb2Data->traces[k].loc, nn, ss, cc, ll);
              */

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
                if (strcmp(tb2Data->traces[k].stnm, "0001")  == 0 && strcmp(tb2Data->traces[k].chan, "LYZ") == 0) {
                  sprintf(buf, "%s.%s.%s.%s", tb2Data->traces[k].netw, tb2Data->traces[k].stnm,
                            tb2Data->traces[k].chan, tb2Data->traces[k].loc);
                  printf("unpackTB2 %s k=%d i=%d imap[%d]=k kpts[k]=%d nmsg[k]=%d\n",
                      buf, k, i, i, imap[i], kpts[k], nmsg[k]);
                }
                break;
            }
        } // Loop on messages read
    } // Loop on waveforms
LOG_DEBUGMSG("== [unpackTraceBuf t0:%f First Loop over SCNLs DONE", ISCL_time_timeStamp());
    // Argsort the messages to their destinations (SNCLs).  Note, if using
    // intel performance primitives the sort will be stable.  Therefore, if
    // the messages are ordered temporally (more likely case) the unpacking
    // will be faster

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
LOG_DEBUGMSG("== [unpackTraceBuf t0:%f Second loop over ring msgs. nRead=%d]", ISCL_time_timeStamp(), nRead);
    for (i=0; i<nRead; i++)
    {
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
    }

    for (i=0; i<nRead; i++)
    {
      k = imap[i];
      if (strcmp(tb2Data->traces[k].stnm, "0001")  == 0 && strcmp(tb2Data->traces[k].chan, "LYZ") == 0)
      {
        printf("unpackTB2 After sort %s k=%d i=%d imap[%d]=k kpts[k]=%d nmsg[k]=%d\n",
                      buf, k, i, i, imap[i], kpts[k], nmsg[k]);
      }
    }

    /*
    for (i=0; i<nRead; i++){
      free(msg_logos[i]);
    }
    free(msg_logos);
    */

LOG_DEBUGMSG("== [unpackTraceBuf t0:%f Second loop over ring msgs DONE]", ISCL_time_timeStamp());
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
LOG_DEBUGMSG("== [unpackTraceBuf t0:%f Third loop over nReadPtr mapping]", ISCL_time_timeStamp());
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
            trh  = (TRACE2_HEADER *) &msgs[indx];
/*
            //memcpy(msg, &msgs[indx], MAX_TRACEBUF_SIZ*sizeof(char));
            //memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
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
            }
            npts = traceHeader.nsamp;
*/
            dtype = 4;
            lswap = 0;
            //if (nsamp0 != traceHeader.nsamp){lswap = 1;}

            npts = trh->nsamp;

            ierr = fastUnpack(npts, lswap, dtype, &msgs[indx], resp);
            if (ierr != 0)
            {
                LOG_ERRMSG("%s", "Error unpacking data");
            }
            // Update the points
            //dt = 1.0/traceHeader.samprate;
            dt = 1.0/trh->samprate;
            tb2Data->traces[k].dt = dt;
            // Is a new chunk beginning?
            if (im > i1)
            {
                //if (fabs( (tb2Data->traces[k].times[kndx] + dt)
                        //- traceHeader.starttime ) < 1.e-6)
                if (fabs( (tb2Data->traces[k].times[kndx] + dt)
                        - trh->starttime ) < 1.e-6)
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
                //tb2Data->traces[k].times[kndx+l] = traceHeader.starttime
                tb2Data->traces[k].times[kndx+l] = trh->starttime
                                                 + (double) l*dt;

                /*
                LOG_MSG("%s.%s.%s.%s t:%f (npts:%d) (int) data:%d",
                    tb2Data->traces[k].stnm, tb2Data->traces[k].chan,
                    tb2Data->traces[k].netw, tb2Data->traces[k].loc,
                    tb2Data->traces[k].times[kndx+l],
                    npts,
                    tb2Data->traces[k].data[kndx+l]);
                */
            }
            kndx = kndx + npts; 
/*
printf("%16.8f %s %s %s %s %d %f\n", trh->starttime,
                                trh->net, trh->sta,
                                trh->chan, trh->loc,
                                trh->nsamp, (double) resp[0]/1000000); 
*/
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
//LOG_MSG("== [unpackTraceBuf t0:%f Third loop over nReadPtr mapping DONE]", ISCL_time_timeStamp());

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
    memory_free8c(&logo);
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
