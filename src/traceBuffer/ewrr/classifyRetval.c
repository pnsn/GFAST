#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
/*!
 * @brief Classifies return value from an earthworm get transport activity 
 *
 * @param[in] retval    earthworm return code to classify
 *
 * @result  1 the requested message was received.
 *          0 there are no more messages
 *         -1 messages were received but there may be a loss of information
 *         -2 an error occurred and no messages were received
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_ewrr_classifyGetRetval(const int retval)
{
    const char *fcnm = "traceBuffer_ewrr_classifyGetRetval\0";
    char msg[128];
    // Got a requested message (modid, type, class)
    if (retval == GET_OK){return 1;} 
    // No logos of requested messages in in memory 
    if (retval == GET_NONE){return 0;} 
    memset(msg, 0, 128*sizeof(char)); 
    // Got a message but missed some
    if (retval == GET_MISS)
    {   
        sprintf(msg, "%s: Some messages were missed\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1; 
    }
    // Got a message but ntrack_get was exceeded
    if (retval == GET_NOTRACK)
    {   
        sprintf(msg, "%s: Message exceeded NTRACK_GET\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1; 
    }   
    // Message requested exceeded my buffer
    if (retval == GET_TOOBIG)
    {
        sprintf(msg,
               "%s: Next message of requested logo(s) is too big\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -2;
    }
    // Didn't check ring fast enough and missed a message
    if (retval == GET_MISS_LAPPED)
    {
        sprintf(msg, "%s: Some messages were overwritten\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // Message contains a gap
    if (retval == GET_MISS_SEQGAP)
    {
        sprintf(msg, "%s: A gap in messages was detected\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // I don't know
    sprintf(msg, "%s: Could not classify return value\n", fcnm);
    logit("et", msg);
    log_warnF("%s", msg);
    return -2;
}

