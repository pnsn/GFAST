#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Removes an event ID from the active event list in the case that is
 *        was cancelled. 
 *
 * @param[in] evid         event ID to remove from events list
 * @param[in] currentTime  current epochal time
 * @param[in] SA           shakeAlert event information to possibly remove
 * @param[in] verbose      controls verbosity
 *
 * @param[in,out] events   on input contains all events.
 *                         on output may contain the removed SA event
 *
 * @result true if the event SA was removed from the events list
 *
 * @author Ben Baker (ISTI)
 *
 */
bool core_events_removeCancelledEvent(const char *evid,
                                      const double currentTime,
                                      const int verbose,
                                      struct GFAST_shakeAlert_struct SA,
                                      struct GFAST_activeEvents_struct *events)
{
    const char *fcnm = "core_events_removeCancelledEvent\0";
    struct GFAST_activeEvents_struct SAtemp;
    int iev, jev, nev0, pop_indx;
    bool lpopped;
    lpopped = false;
    if (events == NULL)
    {
        if (verbose > 0)
        {
            log_warnF("%s: Warning no events in list\n", fcnm);
        }
        return lpopped;         
    }
    // Find the event and see if I can remove it
    pop_indx =-2;
    nev0 = events->nev;
    for (iev=0; iev<nev0; iev++)
    {
        if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0)
        {
            pop_indx =-1;
            if (strcasecmp(evid, events->SA[iev].eventid) == 0)
            {
                if (verbose > 0)
                {
                    log_infoF("%s: Removing %s %f from event list at %f\n",
                              fcnm, SA.eventid, SA.time, currentTime);
                }
                pop_indx = iev;
                break;
            }
        }
    }
    // I couldn't even find the event in the event list
    if (pop_indx ==-2)
    {
        if (verbose > 0)
        {
            log_warnF("%s: Strangely cannot find this event %s\n",
                      fcnm, SA.eventid);
        }
        return lpopped;
    }
    // Remove the event?
    if (pop_indx >-1)
    {
        // Only event - good bye
        if (nev0 == 1)
        {
            core_events_freeEvents(events);
            events->nev = 0;
            return lpopped;
        }
        // Copy old events into workspace ignoring event at pop_indx 
        SAtemp.nev = nev0 - 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                    calloc((size_t) SAtemp.nev,
                           sizeof(struct GFAST_shakeAlert_struct));
        jev = 0;
        for (iev=0; iev<nev0; iev++)
        {
            if (iev == pop_indx){continue;}
            memcpy(&SAtemp.SA[jev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
            jev = jev + 1;
        }
        memcpy(&SAtemp.SA[nev0], &SA, sizeof(struct GFAST_shakeAlert_struct));
        // Resize events
        core_events_freeEvents(events);
        events->nev = SAtemp.nev;
        events->SA = (struct GFAST_shakeAlert_struct *)
                     calloc((size_t) events->nev,
                            sizeof(struct GFAST_shakeAlert_struct));
        // Copy old events back
        for (iev=0; iev<events->nev; iev++)
        {
            memcpy(&events->SA[iev], &SAtemp.SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
        }
        // Free SAtemp
        core_events_freeEvents(&SAtemp);
    }
    return lpopped;
}
