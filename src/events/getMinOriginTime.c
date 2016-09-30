#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include "gfast_events.h"
#include "iscl/log/log.h"
/*!
 * @brief Finds the minimum origin time (s) 
 *
 * @param[in] props          controls verboseness
 * @param[in] events         holds the current events in processing
 *
 * @param[out] lnoEvents     if true then there are no events in the events
 *                           structure or there are no valid origin times
 *
 * @result the minimum origin time in the events structure
 *
 * @author Ben Baker, ISTI
 *
 */
double events_getMinOriginTime(struct GFAST_props_struct props,
                               struct GFAST_activeEvents_struct events,
                               bool *lnoEvents)
{
    const char *fcnm = "GFAST_events_getMinOriginTime\0";
    double t0; 
    int iev;
    *lnoEvents = true;
    t0 = DBL_MAX;
    if (events.nev < 1){return t0;}
    *lnoEvents = false;
    for (iev=0; iev<events.nev; iev++)
    {
        t0 = fmin(t0, events.SA[iev].time);
    }   
    if (props.verbose > 1 && fabs(t0 - DBL_MAX) < 1.e-10)
    {
        *lnoEvents = true;
        log_warnF("%s: Warning origin times are screwy\n", fcnm);
    }   
    return t0; 
}
