#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Finds the minimum origin time (UTC epochal seconds) in the
 *        events list.
 *
 * @param[in] props          Controls verboseness.  Less than 1 is quiet.
 * @param[in] events         A list of the current events in processing.
 *
 * @param[out] lnoEvents     If true then there are no events in the events
 *                           structure or there are no valid origin times.
 *
 * @result The minimum origin time in the events structure.
 *
 * @author Ben Baker, ISTI
 *
 */
double core_events_getMinOriginTime(struct GFAST_props_struct props,
                                    struct GFAST_activeEvents_struct events,
                                    bool *lnoEvents)
{
    const char *fcnm = "GFAST_core_getMinOriginTime\0";
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
