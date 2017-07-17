#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
/*!
 * @brief Disconnects from the ring and clears the earthworm ring structure.
 *
 * @param[in,out] ringInfo   On input contains the initialized earthworm ring
 *                           information.
 *                           On output contains the cleared earthworm ring
 *                           information.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker
 *
 * @copyright ISTI distributed under Apache 2.
 *
 */
int traceBuffer_ewrr_finalize(struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ewrr_finalize\0";
    if (!ringInfo->linit)
    {   
        log_errorF("%s: Error ewRing_struct was never initialized\n", fcnm);
        return -1; 
    }
    if (ringInfo->getLogo != NULL && ringInfo->nlogo > 0)
    {
        free(ringInfo->getLogo);
    }
    tport_detach(&ringInfo->region);
    memset(ringInfo, 0, sizeof(struct ewRing_struct));
    return 0;
}
