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

struct tb_struct {
  char netw[8];
  char stnm[8];
  char chan[8];
  char loc[8];
};

struct string_index {
  char logo[15];
  char net[8];
  char sta[8];
  char cha[8];
  char loc[8];
  double time;
  int indx;
  int k;
  int nsamps;
  float data;
};
void print_struct(struct string_index *d, int n);
void sort2(struct string_index *vals, int n);


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
        dtype, i, i1, i2, ierr, im, indx, ir, k, kndx, l, j,
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

    bool found = false;
    int debug = 0;

    int kold;
    struct string_index *vals, *tmp;
    vals = (struct string_index *) calloc((size_t) nRead, sizeof(struct string_index));
    tmp = (struct string_index *) calloc((size_t) nRead, sizeof(struct string_index));
    char *ret;

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

    printf("unpackTB2: Enter  nTraces:%d nRead:%d\n", tb2Data->ntraces, nRead);

    // MTH: load up the msg logos once
    for (i=0; i<nRead; i++)
    {
        indx = i*MAX_TRACEBUF_SIZ;
        trh  = (TRACE2_HEADER *) &msgs[indx];
        /*
        //msg_logos[i] = (char *)malloc(15);
        sprintf(msg_logos[i], "%s.%s.%s.%s",
                trh->net, trh->sta, trh->chan, trh->loc);
        times[i] = trh->starttime;
        nsamps[i]= trh->nsamp;
        if (strcmp(msg_logos[i], "CI.0001.LYZ.20")==0) {
          printf("unpackTB2: msg_logos[%d]=%s time:%f nsamps:%d nRead:%d\n",
              i, msg_logos[i], times[i], nsamps[i], nRead);
        }
        if (debug){
          printf("unpackTB2: incoming msg_logos[%4d]=%s time:%.2f nsamps:%d\n",
              i, msg_logos[i], times[i], nsamps[i]);
        }
        */

        sprintf(logo, "%s.%s.%s.%s", trh->sta, trh->chan, trh->net, trh->loc);
        strcpy(vals[i].logo, logo);
        strcpy(vals[i].sta, trh->sta);
        strcpy(vals[i].cha, trh->chan);
        strcpy(vals[i].net, trh->net);
        strcpy(vals[i].loc, trh->loc);
        vals[i].indx = i;
        vals[i].time = trh->starttime;
        vals[i].nsamps = trh->nsamp;
        //vals[i].data = data;
    }
    //print_struct(vals, nRead);

    for (i=0; i<nRead; i++){
      memcpy(&tmp[i], &vals[i], sizeof(struct string_index));
    }

    // Sort the msg records by scnl + time:
    sort2(tmp, nRead);
    //print_struct(tmp, nRead);
    //exit(0);

    for (i=0; i<nRead; i++){
      tmp[i].k = -9;
      imap[i]  = -9;
    }

    int klast = 0;

    // Loop on waveforms and get workspace count
    // Loop through nRead msgs in sorted order and assign a k value to each
    for (i=0; i<nRead; i++){
      j = tmp[i].indx;
      imsg[i] = j;
      for (k=klast; k<tb2Data->ntraces; k++) {
        // MTH: may want to also check net + loc if mixing networks
          if (strcmp(vals[j].sta, tb2Data->traces[k].stnm) == 0){
            if (strcmp(vals[j].cha, tb2Data->traces[k].chan) == 0){
              //tmp[i].k = k;
              imap[i] = k;
              kpts[k] += vals[j].nsamps;
              nmsg[k] += 1;
              klast = k;
              break;
            }
          }
      }
    }

    // Step through the sorted imap[i]=k and figure out where each new k starts = imap[imapPtr[ir]]
    kold = imap[0];
    ir = 0;
    for (i=0; i<nRead; i++) {
      k = imap[i];
      //printf("i=%d k:%d\n", i,k);
      if (k != kold) {
        //printf("** k has changed from:%d to %d\n", kold, k);
        if (k > -1) {
          imapPtr[ir] = i;
          //printf("   set imapPtr[%d] = %d\n", ir, imapPtr[ir]);
          ir += 1;
        }
        kold = k;
      }
    }

    nReadPtr = ir;

    for (ir=0; ir<nReadPtr; ir++) {
      i1 = imapPtr[ir];
      //i2 = imapPtr[ir+1];
      k = imap[i1]; // k always = ir ?
      i2 = i1 + nmsg[k];
      printf("ir:%d i1:%d i2:%d k:%d nmsg[k]:%d kpts[k]:%d\n", ir, i1, i2, k, nmsg[k], kpts[k]);
      for (im=i1; im<i2; im++){
            j=imsg[im];
            printf("  im:%5d msg %5d: %s time:%.1f\n", im, j, vals[j].logo, vals[j].time);
      }
    }


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

    for (ir=0; ir<nReadPtr; ir++)
    {
        i1 = imapPtr[ir];
        i2 = imapPtr[ir+1];
        k = imap[i1];
        kndx = 0;
        //if (debug) {
        if (1) {
          sprintf(buf, "%s.%s.%s.%s", tb2Data->traces[k].netw, tb2Data->traces[k].stnm,
                  tb2Data->traces[k].chan, tb2Data->traces[k].loc);
        }

        tb2Data->traces[k].nchunks = 1;

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
            dtype = 4;
            lswap = 0;
            npts = trh->nsamp;
            dt = 1.0/trh->samprate;
            tb2Data->traces[k].dt = dt;

            ierr = fastUnpack(npts, lswap, dtype, &msgs[indx], resp);
            if (ierr != 0) {
                LOG_ERRMSG("%s", "Error unpacking data");
            }

            // Is a new chunk beginning?
            if (im > i1) {
                if (fabs( (tb2Data->traces[k].times[kndx-1] + dt) - trh->starttime ) > 1.e-6) {
                //printf("    starttime exceeds dt --> start a new chunk\n");
                printf("ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d start a new chunk\n",
                    ir, i1, im, k, buf, kndx, npts, tb2Data->traces[k].nchunks);
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx;
                    tb2Data->traces[k].nchunks += 1;
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx + npts;
                }
                else {
                //printf("    starttime is within dt --> simply extend current chunk\n");
                printf("ir:%d i1:%d im:%d k:%d %s kndx:%d npts:%d nchunks:%d extend current chunk\n",
                    ir, i1, im, k, buf, kndx, npts, tb2Data->traces[k].nchunks);
                    tb2Data->traces[k].chunkPtr[tb2Data->traces[k].nchunks] = kndx + npts;
                }
            }

            // Update the points
#ifdef _OPENMP
            #pragma omp simd
#endif
            for (l=0; l<npts; l++)
            {
                tb2Data->traces[k].data[kndx+l] = resp[l];
                tb2Data->traces[k].times[kndx+l] = trh->starttime + (double) l*dt;

                if (1) {
                  printf("unpackTB2 k:%4d scnl:%s time:%.2f val:%d\n", 
                            k, buf, tb2Data->traces[k].times[kndx+l], tb2Data->traces[k].data[kndx+l]);
                }

            }
            kndx = kndx + npts;

        } // Loop on messages for this SNCL

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
                printf("**** Inconsistent number of points %d %d\n",
                           tb2Data->traces[k].chunkPtr[nchunks],
                           tb2Data->traces[k].npts);
                LOG_ERRMSG("Inconsistent number of points %d %d",
                           tb2Data->traces[k].chunkPtr[nchunks],
                           tb2Data->traces[k].npts);
                return -1;
            }
        }


        if (0) {
          printf("unpackTB2  k:%4d nchunks:%d chunkPtr[0]:%d chunkPtr[nchunks]:%d total_npts:%d\n",
                   k, tb2Data->traces[k].nchunks, tb2Data->traces[k].chunkPtr[0], tb2Data->traces[k].chunkPtr[nchunks],
                  tb2Data->traces[k].npts);
        }

    } // Loop on pointers
    //exit(0);
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
 exit(0);
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


// Defining comparator function as per the requirement
static int myCompare2(const void *x, const void *y)
{
  const struct string_index xx = *(const struct string_index *) x;
  const struct string_index yy = *(const struct string_index *) y;
  int inet, ista, icha, iloc;

  if (strcmp(xx.net, yy.net)==0) {
    if (strcmp(xx.loc, yy.loc)==0) {
      ista = strcmp(xx.sta, yy.sta);
      if (ista == 0){
        icha = strcmp(xx.cha, yy.cha);
        if (icha == 0){
          if (xx.time > yy.time) {
            return 1;
          }
          else if (xx.time < yy.time) {
            return -1;
          }
          else {
            return 0;
          }
        }
        else {  // order by sta + {LYZ, LYN, LYE} to match tb2Data
          return -1*icha;
        }
      }
      else {
        return ista;
      }
    }
  }

  printf("**** MTH: qsort HERE: xx.logo=%s \t yy.logo=%s\n", xx.logo, yy.logo);
  return strcmp(xx.logo, yy.logo);

  //return strcmp(*(const char**)a, *(const char**)b);
}

// Function to sort the array
void sort2(struct string_index values[], int n)
{
  // calling qsort function to sort the array
  // with the help of Comparator
  qsort((void *) values, (size_t) n, sizeof(struct string_index), myCompare2);
}

void print_struct(struct string_index *d, int n){
  int i;
  for (i=0; i<n; i++){
    printf("struct[%d] indx:%5d: logo:%s nsamps:%d time:%.2f\n",
        i, d[i].indx, d[i].logo, d[i].nsamps, d[i].time);
  }
}
