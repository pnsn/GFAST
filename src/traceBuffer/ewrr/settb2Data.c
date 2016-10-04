#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"

/*!
 * @brief Initializes the data list which will be retained from a 
 *        ring read operation
 *
 * @param[in] nets      null terminated list of networks [ntraces]
 * @param[in] stats     null terminated list of stations [ntraces]
 * @param[in] chans     null terminated list of channels [ntraces]
 * @param[in] locs      null terminated list of location codes [ntraces]
 *
 * @param[out] tb2data  on successful output contains the SNCL's to 
 *                      copy after each ring read
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int traceBuffer_ewrr_settb2Data(const int ntraces,
                                const char **nets,
                                const char **stats,
                                const char **chans,
                                const char **locs,
                                struct tb2Data_struct *tb2data)
{
    const char *fcnm = "traceBuffer_ewrr_settb2Data\0";
    int i;
    if (ntraces < 1 || nets == NULL || stats == NULL ||
        chans == NULL || locs == NULL)
    {
        if (ntraces < 1)
        {
            log_errorF("%s: No traces to initialize\n", fcnm);
        }
        if (nets == NULL)
        {
            log_errorF("%s: Error must define network list\n", fcnm);
        }
        if (stats == NULL)
        {
            log_errorF("%s: Error must define station list\n", fcnm);
        }
        if (chans == NULL)
        {
            log_errorF("%s: Error must define channel list\n", fcnm);
        }
        if (locs == NULL)
        {
            log_errorF("%s: Error must define location list\n", fcnm);
        }
        return -1;
    }
    if (tb2data->linit)
    {
        log_errorF("%s: Error tb2data already initialized\n", fcnm);
        return -1;
    }
    tb2data->traces = (struct tb2Trace_struct *)
                      calloc( (size_t) ntraces,
                             sizeof(struct tb2Trace_struct));
    for (i=0; i<ntraces; i++)
    {
        strcpy(tb2data->traces[i].netw, nets[i]);
        strcpy(tb2data->traces[i].stnm, stats[i]);
        strcpy(tb2data->traces[i].chan, chans[i]);
        strcpy(tb2data->traces[i].loc,  locs[i]);
    }
    tb2data->ntraces = ntraces;
    tb2data->linit = true;
    return 0;
}
