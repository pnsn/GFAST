#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
/*!
 * @brief Flushes the ring on ringInfo
 *
 * @param[in] ringInfo    structure with earthworm ring information to 
 *                        flush
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_ewrr_flushRing(struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ewrr_flushRing\0";
    MSG_LOGO gotLogo;
    char msg[MAX_TRACEBUF_SIZ];
    unsigned char sequenceNumber;
    long gotSize;
    int retval;
    //------------------------------------------------------------------------//
    //
    // Make sure user knows the ring was flushed
    if (ringInfo->linit == false)
    {
        log_errorF("%s: Error the ring was never initialized\n", fcnm);
        return -1;
    }
    // Lift all the messages from the ring
    while (true)
    {
        retval = tport_copyfrom(&ringInfo->region, 
                                ringInfo->getLogo, ringInfo->nlogo,
                                &gotLogo, &gotSize, msg, MAX_TRACEBUF_SIZ,
                                &sequenceNumber);
        if (retval == GET_NONE){break;} // End of ring
        //traceBuffer_ewrr_classifyGetRetval(retval);
    }
    if (ringInfo->msWait > 0){sleep_ew(ringInfo->msWait);}
    return 0;
}
