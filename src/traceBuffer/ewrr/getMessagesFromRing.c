#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Reads the tracebuffer2 messages off the earthworm ring specified
 *        on ringInfo
 *
 * @param[in] messageBlock   block allocator size.  instead of reallocating
 *                           memory every loop iteration in the acquisition
 *                           one can allocate in a chunk of messages (e.g.
 *                           200 messages at a time i.e. messageBlock = 200).
 *                           if this number is too small there will be 
 *                           overhead in memory reallocation, if this number
 *                           is too big a lot of unnecessary space will be
 *                           allocated. 
 * @param[in] showWarnings   if true the print warnings about having read
 *                           maximum number of messages
 * @param[in] ringInfo       Earthworm ring reader structure
 *
 * @param[out] nRead         number of traceBuffer2 messages read
 *
 * @param[out] ierr          0 indicates success.
 *                          -1 indicates a terminate signal from the ring.
 *                             the user should call traceBuffer_ewrr_finalize
 *                             and quit.
 *                          -2 indicates a read error on the ring.
 *                          -3 indicates the ringInfo structure was not
 *                             initalized.
 *                          -4 indicates tracebuf2 type is unknown.
 *
 * @result traceBuffer2 nRead messages read from the earthworm ring.
 *         the k'th message start index (for k=1,2,...,nRead) is given
 *         by: (k - 1)*MAX_TRACEBUF_SIZE [nRead*MAX_TRACEBUF_SIZE]
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
char *traceBuffer_ewrr_getMessagesFromRing(const int messageBlock,
                                           const bool showWarnings,
                                           struct ewRing_struct *ringInfo,
                                           int *nRead, int *ierr)
{
    const char *fcnm = "traceBuffer_ewrr_getMessagesFromRing\0";
    MSG_LOGO gotLogo; 
    TRACE2_HEADER traceHeader;
    char *msg, *msgs, *msgWork;
    unsigned char sequenceNumber;
    long gotSize;
    int kdx, nblock, ncopy, nwork, retval;
    size_t nbytes; //, npcopy;
    //------------------------------------------------------------------------//
    //  
    // Make sure this is initialized
    *ierr = 0;
    *nRead = 0;
    msg = NULL;
    msgs = NULL;
    if (!ringInfo->linit)
    {
        log_errorF("%s: Error ringInfo not initialized\n", fcnm);
        *ierr =-3;
        return msgs;
    }
    // Avoid a segfault
/*
    if (maxMessages < 1 || msg == NULL)
    {
        if (maxMessages < 1){log_errorF("%s: Error no space\n", fcnm);}
        if (msgs == NULL){log_errorF("%s: Error messages is NULL\n", fcnm);}
        return -4;
    }
*/
    if (messageBlock < 1)
    {
        log_errorF("%s: messageBlock allocator must be postiive\n", fcnm);
        *ierr =-4;
        return msgs;
    }
    // Set space
    msgs = ISCL_memory_calloc__char(MAX_TRACEBUF_SIZ*messageBlock);
    msg  = ISCL_memory_calloc__char(MAX_TRACEBUF_SIZ);
    nblock = 1;
    // Unpack the ring
    while (true)
    {
        // May have a kill signal from the transport layer 
        retval = tport_getflag(&ringInfo->region);
        if (retval == TERMINATE)
        {
            log_errorF("%s: Receiving kill signal from ring %s\n",
                       fcnm, ringInfo->ewRingName);
            *ierr =-1;
            return msgs;
        }
        // Copy from the memory
        retval = tport_copyfrom(&ringInfo->region,
                                ringInfo->getLogo, ringInfo->nlogo,
                                &gotLogo, &gotSize, msg, MAX_TRACEBUF_SIZ,
                                &sequenceNumber);
        // Classify my message
        retval = traceBuffer_ewrr_classifyGetRetval(retval);
        if (retval ==-2)
        {
            log_errorF("%s: An error was encountered getting message\n", fcnm);
            *ierr =-2;
            return msgs;
        }
        // Verify i want this message
        if (gotLogo.type == ringInfo->traceBuffer2Type)
        {
            // Get the header
            memcpy(&traceHeader, msg, sizeof(TRACE2_HEADER));
            *ierr = WaveMsg2MakeLocal(&traceHeader);
            if (*ierr < 0)
            {
                log_errorF("%s: Error flipping bytes\n", fcnm);
                *ierr =-2;
                return msgs;
            }
            nbytes = sizeof(int); 
            if (strcasecmp(traceHeader.datatype, "s2\0") == 0 ||
                strcasecmp(traceHeader.datatype, "i2\0") == 0)
            {
                nbytes = sizeof(short);
            }
            //npcopy = (size_t) ( sizeof(TRACE2_HEADER)*sizeof(char)
            //                  + (size_t) (traceHeader.nsamp)*nbytes);
            // Copy the message
            kdx = *nRead*MAX_TRACEBUF_SIZ;
            memcpy(&msgs[kdx], msg, MAX_TRACEBUF_SIZ*sizeof(char));
            // Reallocate space 
            *nRead = *nRead + 1;
            if (*nRead == messageBlock*nblock)
            {
                if (showWarnings)
                {
                    log_warnF("%s: Reallocating msgs block\n", fcnm);
                }
                // get workspace sizes
                nwork = MAX_TRACEBUF_SIZ*(*nRead + nblock*messageBlock);
                ncopy = MAX_TRACEBUF_SIZ*(*nRead);
                // set workspace and copy old messages
                msgWork = ISCL_memory_calloc__char(ncopy);
                memcpy(msgWork, msgs, (size_t) ncopy);
                // resize msgs
                ISCL_memory_free__char(&msgs);
                msgs = ISCL_memory_calloc__char(nwork);
                // copy back and free workspace
                memcpy(msgs, msgWork, (size_t) ncopy);
                ISCL_memory_free__char(&msgWork);
                // Update block alloc
                nblock = nblock + 1;
            }
        }
        // End of ring - time to leave
        if (retval == GET_NONE){break;}
    }
    ISCL_memory_free__char(&msg);
    if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
    return msgs;
}
