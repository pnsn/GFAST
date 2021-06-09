#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
/*!
 * @brief Removes an event from the active event list if 
 *        (currentTime - SA.time) > maxtime.
 *
 * @param[in] maxtime      Maximum amount of time (s) after the origin
 *                         time which the event can be retained.
 * @param[in] currentTime  Current time (UTC seconds since epoch).
 * @param[in] SA           shakeAlert event information to possibly remove.
 * @param[in] verbose      Controls verbosity.
 *
 * @param[in,out] events   On input contains all events. <br> 
 *                         On output may contain the removed SA event.
 *
 * @result If true then the event SA was removed from the events list.
 *
 * @author Ben Baker (ISTI)
 *
 */
bool core_events_removeExpiredEvent(const double maxtime,
                                    const double currentTime,
                                    const int verbose,
                                    struct GFAST_shakeAlert_struct SA,
                                    struct GFAST_activeEvents_struct *events)
{
    struct GFAST_activeEvents_struct SAtemp;
    int iev, jev, nev0, pop_indx;
    bool lpopped;
    lpopped = false;
    if (events == NULL)
    {
        if (verbose > 0)
        {
            LOG_WARNMSG("%s", "Warning no events in list");
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

            if ((currentTime - SA.time) > maxtime)
            {
              LOG_MSG("time:%lf Remove evid:%s otime:%f from event list",
                       currentTime, SA.eventid, SA.time);
                if (verbose > 0)
                {
                    LOG_INFOMSG("Removing %s %f from event list at %f",
                                SA.eventid, SA.time, currentTime);
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
            LOG_WARNMSG("Strangely cannot find this event %s", SA.eventid);
        }
        return lpopped;
    }
    // Remove the event?
    lpopped = true;
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
        LOG_MSG("Copy nev0-1=%d events into workspace, ignore expired event[%d]", nev0-1, pop_indx);
        SAtemp.nev = nev0 - 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                            calloc((size_t) SAtemp.nev, sizeof(struct GFAST_shakeAlert_struct));
        jev = 0;
        for (iev=0; iev<nev0; iev++)
        {
            if (iev == pop_indx){continue;}
            LOG_MSG("memcpy events.SA[%d] evid:%s into SAtemp.SA[%d]",
                    iev, events->SA[iev].eventid, jev);
            memcpy(&SAtemp.SA[jev], &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
            jev = jev + 1;
        }
        //LOG_MSG("%s", "Copy SAtemp into SA");
        //memcpy(&SAtemp.SA[nev0], &SA, sizeof(struct GFAST_shakeAlert_struct));
        // Resize events
        core_events_freeEvents(events);
        events->nev = SAtemp.nev;
        events->SA = (struct GFAST_shakeAlert_struct *)
                             calloc((size_t) events->nev, sizeof(struct GFAST_shakeAlert_struct));
        // Copy old events back
        for (iev=0; iev<events->nev; iev++)
        {
            LOG_MSG("memcpy SAtemp.SA[%d] evid:%s back to events->SA",
                    iev, SAtemp.SA[iev].eventid);
            memcpy(&events->SA[iev], &SAtemp.SA[iev], sizeof(struct GFAST_shakeAlert_struct));
        }
        // Free SAtemp
        core_events_freeEvents(&SAtemp);
    }
    return lpopped;
}
