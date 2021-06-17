#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "transport.h"
#include "trace_buf.h"
#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Reads the tracebuffer2 messages from the Earthworm ring specified
 *        on ringInfo.
 *
 * @param[in] messageBlock   Block allocator size.  Instead of reallocating
 *                           memory every loop iteration in the acquisition
 *                           one can allocate in a chunk of messages (e.g.
 *                           200 messages at a time i.e. messageBlock = 200).
 *                           If this number is too small there will be 
 *                           overhead in memory reallocation, if this number
 *                           is too big a lot of unnecessary space will be
 *                           allocated. 
 * @param[in] showWarnings   If true then the print warnings about having read
 *                           maximum number of messages.
 * @param[in] ringInfo       Earthworm ring reader structure.
 *
 * @param[out] nRead         Number of traceBuffer2 messages read.
 *
 *
 * @param[out] msgs          An array of [nRead] traceBuffer2 messages read from the Earthworm
 *                           ring.  The k'th message start index (for k=1,2,...,nRead) is given
 *                           by: (k - 1)*MAX_TRACEBUF_SIZE [nRead*MAX_TRACEBUF_SIZE]
 * @result ierr              0 Indicates success. <br>
 *                          -1 Indicates a terminate signal from the ring. <br>
 *                             the user should call traceBuffer_ewrr_finalize
 *                             and quit. <br>
 *                          -2 Indicates a read error on the ring. <br>
 *                          -3 Indicates the ringInfo structure was not
 *                             initalized. <br> 
 *                          -4 Indicates tracebuf2 type is unknown.

 *
 * @author Ben Baker
 *
 * @copyright ISTI distribted under Apache 2.
 *
 */
int traceBuffer_ewrr_getMessagesFromRingSA(const int maxMessages,
					   const bool showWarnings,
					   struct ewRing_struct *ringInfo,
					   int *nRead, char *msgs)
{
  MSG_LOGO gotLogo; 
  TRACE2_HEADER traceHeader;
  char *msg=NULL;
  unsigned char sequenceNumber;
  long gotSize;
  int kdx,retval;
  int debug = 0;
  int ierr=0;
  //size_t nbytes; //, npcopy;
  //------------------------------------------------------------------------//
  //  
  // Make sure this is initialized
  *nRead = 0;

  if (!ringInfo->linit)
    {
      LOG_ERRMSG("%s", "Error ringInfo not initialized");
      ierr =-3;
      return ierr;
    }

  if ((maxMessages == 1)||(maxMessages > INT_MAX/MAX_TRACEBUF_SIZ))
    {
      LOG_ERRMSG("%s: invalid maxMessages %d", __func__, maxMessages);
      ierr =-4;
      return ierr;
    }
  // Set space
  memset(&gotLogo, 0, sizeof(MSG_LOGO));
  msg  = memory_calloc8c(MAX_TRACEBUF_SIZ);

  // Unpack the ring
  while (true)
    {
      // May have a kill signal from the transport layer 
      retval = tport_getflag(&ringInfo->region);
      if (retval == TERMINATE) {
        LOG_ERRMSG("Receiving kill signal from ring %s", ringInfo->ewRingName);
        ierr =-1;
        break;
      }
      // Copy from the memory
      retval = tport_copyfrom(&ringInfo->region, ringInfo->getLogo, ringInfo->nlogo,
                              &gotLogo, &gotSize, msg, MAX_TRACEBUF_SIZ, &sequenceNumber);
      // Classify my message
      retval = traceBuffer_ewrr_classifyGetRetval(retval);
      if (retval ==-2) {
        LOG_ERRMSG("%s", "An error was encountered getting message");
        ierr =-2;
        break;
      }

      // MTH: This is where we should leave the ring:
      if (retval == GET_NONE){
        //printf("MTH: Break out of while loop!\n");
        break;
      }

      // Verify i want this message
      if (gotLogo.type == ringInfo->traceBuffer2Type)
      {
        // Get the header
        memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
        ierr = WaveMsg2MakeLocal(&traceHeader);
        if (ierr < 0) {
          LOG_ERRMSG("%s", "Error flipping bytes");
          ierr =-2;
          break;
        }

        if (debug){
          printf("getMsgs: %s.%s.%s.%s time:%f npts:%d\n",
          traceHeader.net, traceHeader.sta, traceHeader.chan, traceHeader.loc,
          traceHeader.starttime, traceHeader.nsamp);
        }

        kdx = *nRead*MAX_TRACEBUF_SIZ;
        memcpy(&msgs[kdx], msg, MAX_TRACEBUF_SIZ*sizeof(char));
        (*nRead)+=1;

        //exit if message buffer full
        if (*nRead+1 >= maxMessages) {
          LOG_MSG("%s: nRead=%d maxMessages=%d --> Exit read loop with full buffer.",
		  __func__,*nRead, maxMessages);
          if (showWarnings) {
            LOG_WARNMSG("%s: nRead=%d MAX_TRACEBUF_SIZ=%d maxMessages=%d --> Exit read loop with full buffer.",
			__func__,*nRead, MAX_TRACEBUF_SIZ, maxMessages);
          }
	  ierr=0;
          break;
        }
      }
    } // while true

  memory_free8c(&msg);

  if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
  return ierr;
}
