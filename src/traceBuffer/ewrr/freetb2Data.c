#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"

/*!
 * @brief Releases memory on the tb2Data_struct data structure
 *
 * @param[in,out] tb2data   on input the initialized tb2data structure.
 *                          on output the structure has been cleared.
 * 
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
void traceBuffer_ewrr_freetb2Data(struct tb2Data_struct *tb2data)
{
    const char *fcnm = "traceBuffer_ewrr_freetb2Data\0";
    int i;
    if (!tb2data->linit)
    {
        log_warnF("%s: Structure never set\n", fcnm);
        return;
    }
    if (tb2data->ntraces > 0 && tb2data->traces != NULL)
    {
        for (i=0; i<tb2data->ntraces; i++)
        {
             traceBfufer_ewrr_freetb2Trace(true, &tb2data->traces[i]);
        }
        free(tb2data->traces);
    }
    memset(tb2data, 0, sizeof(struct tb2Data_struct));
    return;
}
