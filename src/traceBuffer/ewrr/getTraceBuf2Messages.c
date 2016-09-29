#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
/*!
 * @brief Reads the tracebuffer2 messages off the earthworm ring specified
 *        on ringInfo
 *
 * @param[in] maxMessages  max number of messages that can be read
 *                         off the ring
 * @param[in] ringInfo     Earthworm ring reader structure
 *
 * @param[out] nRead       number of traceBuffer2 messages read
 * @param[out] msgs        traceBuffer2 messages read from the earthworm ring
 *                         [maxMessages]
 *
 * \retval  0 indicates success
 * \retval -1 indicates a terminate signal from the ring.
 *            the user should call traceBuffer_ewrr_finalize and quit.
 * \retval -2 indicates a read error on the ring.
 * \retval -3 indicates the ringInfo structure was not initalized
 * \retval -4 indicates insufficient space on output
 * \retval -5 indicates tracebuf2 type is unknown
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_ewrr_getTraceBuf2Messages(const int maxMessages,
                                          struct ewRing_struct *ringInfo,
                                          int *nRead,
                                          char **msgs)
{
    const char *fcnm = "traceBuffer_ewrr_getgetTraceBuf2Messages\0";
    MSG_LOGO gotLogo; 
    char msg[MAX_TRACEBUF_SIZ];
    unsigned char sequenceNumber;
    long gotSize;
    int retval;
    //------------------------------------------------------------------------//
    //  
    // Make sure this is initialized
    *nRead = 0;
    if (!ringInfo->linit)
    {
        log_errorF("%s: Error ringInfo not initialized\n", fcnm);
        return -3;
    }
    // Avoid a segfault
    if (maxMessages < 1 || msgs == NULL)
    {
        if (maxMessages < 1){log_errorF("%s: Error no space\n", fcnm);}
        if (msgs == NULL){log_errorF("%s: Error messages is NULL\n", fcnm);}
        return -4;
    }
    // Unpack the ring
    while (true)
    {
        // May have a kill signal from the transport layer 
        retval = tport_getflag(&ringInfo->region);
        if (retval == TERMINATE)
        {
            log_errorF("%s: Receiving kill signal from ring %s\n",
                       fcnm, ringInfo->ewRingName);
            return -1;
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
            return -2;
        }
        // Verify i want this message
        if (gotLogo.type == ringInfo->traceBuffer2Type)
        {
            // Copy the message
            memcpy(msgs[*nRead], msg, sizeof(msg));
            // Save the user from themselves 
            *nRead = *nRead + 1;
            if (*nRead == maxMessages)
            {
                log_warnF("%s: Insufficient space for messages - leaving\n",
                          fcnm);
                break;
            }
        }
        // End of ring - time to leave
        if (retval == GET_NONE){break;}
    }
    if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
    return 0;
}
