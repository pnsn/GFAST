#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

/*!
 * @brief Releases memory on the tb2Data_struct data structure.
 *
 * @param[in,out] tb2data   On input the initialized tb2data structure. \n
 *                          On output the structure has been cleared.
 * 
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
void traceBuffer_ewrr_freetb2Data(struct tb2Data_struct *tb2data)
{
    int i;
    if (!tb2data->linit)
    {
        LOG_WARNMSG("%s", "Structure never set");
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
