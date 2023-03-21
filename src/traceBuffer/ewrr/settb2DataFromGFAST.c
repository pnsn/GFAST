#include <string.h>
#include "gfast_traceBuffer.h"
#include "gfast_core.h"

//============================================================================//
/*!
* @brief Sets the tb2Data structure and desired SNCL's from the input gpsData
*
* @param[in] gpsData      holds the GPS SNCL's GFAST is interested in
*
* @param[out] tb2Data     on output has space allocated and has a target
*                         list of SNCL's for message reading from the 
*                         earthworm data ring 
*
* @result 0 indicates success
*
* @author Ben Baker (ISTI)
*
*/
int traceBuffer_ewrr_settb2DataFromGFAST(struct GFAST_data_struct *gpsData,
                                         struct tb2Data_struct *tb2Data)
{
    const char *fcnm = __func__;
    char *nscl = NULL;
    int i, it, k;
    int debug = 0;
    if (gpsData->stream_length == 0) {
        LOG_ERRMSG("%s: Error no data to copy\n", fcnm);
        return -1;
    }
    if (tb2Data->linit) {
        LOG_ERRMSG("%s: Error tb2Data already set\n", fcnm);
        return -1;
    }

    // Init tb2Data_struct
    tb2Data->ntraces = 7 * gpsData->stream_length;
    tb2Data->traces = (struct tb2Trace_struct *)
        calloc( (size_t) tb2Data->ntraces, sizeof(struct tb2Trace_struct) );
    tb2Data->hashmap = (struct tb2_hashmap_struct *)
        malloc(sizeof(struct tb2_hashmap_struct));
    // Random prime, 80021 should have at most 4-5 collisions for a 20k nscl list
    const uint32_t hashsize = 80021;
    tb2Data->hashmap->hashsize = hashsize;
    tb2Data->hashmap->map = (struct tb2_node **) calloc(hashsize, sizeof(struct tb2_node *));

    // Copy channel names
    it = 0;
    for (k = 0; k < gpsData->stream_length; k++) {
        // Loop over each channel: Z, N, E, 3, 2, 1, Q
        for (i = 0; i < 7; i++) {
            strcpy(tb2Data->traces[it].netw, gpsData->data[k].netw);
            strcpy(tb2Data->traces[it].stnm, gpsData->data[k].stnm);
            strcpy(tb2Data->traces[it].chan, gpsData->data[k].chan[i]);
            strcpy(tb2Data->traces[it].loc,  gpsData->data[k].loc); 
            it = it + 1;
        }
    }
    if (debug) {
        LOG_DEBUGMSG("Printing %d traces in tb2Data", tb2Data->ntraces);
        for (i = 0; i < tb2Data->ntraces; i++) {
            LOG_DEBUGMSG("CCC %d: %s.%s.%s.%s", i,
                tb2Data->traces[i].netw,
                tb2Data->traces[i].stnm,
                tb2Data->traces[i].chan,
                tb2Data->traces[i].loc);
        }
        LOG_DEBUGMSG("%s", "Done printing traces in tb2Data");
    }

    // Now add trace names to hashmap
    for (i = 0; i < tb2Data->ntraces; i++) {
        nscl = (char *) malloc(31 * sizeof(char));
        sprintf(nscl, "%s.%s.%s.%s",
            tb2Data->traces[i].netw,
            tb2Data->traces[i].stnm,
            tb2Data->traces[i].chan,
            tb2Data->traces[i].loc);
        if (traceBuffer_ewrr_hashmap_add(tb2Data->hashmap, nscl, i) == NULL) {
            LOG_ERRMSG("%s: Couldn't add %s to hashmap!", fcnm, nscl);
            free(nscl);
            return -1;
        }
        free(nscl);
    }

    if (it != tb2Data->ntraces) {
        LOG_ERRMSG("%s: Lost count %d %d\n", fcnm, it, tb2Data->ntraces);
        return -1;
    }
    tb2Data->linit = true;
    return 0;
}
