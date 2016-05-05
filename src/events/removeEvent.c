#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief Removes an event from the active event list if 
 *        (currentTime - SA.time) > maxtime.
 *
 * @param[in] maxtime      maximum amount of time (s) after the origin
 *                         time which the event can be retained
 * @param[in] currentTime  current time
 * @param[in] SA           shakeAlert event information to possibly remove
 * @param[in] verbose      controls verbosity
 *
 * @param[inout] events    on input contains all events.
 *                         on output may contain the removed SA event
 *
 * @result true if the event SA was removed from the events list
 *
 */
bool GFAST_events__removeEvent(double maxtime, double currentTime,
                               int verbose,
                               struct GFAST_shakeAlert_struct SA,
                               struct GFAST_activeEvents_struct *events)
{
    const char *fcnm = "GFAST_events__removeEvent\0";
    struct GFAST_activeEvents_struct SAtemp;
    int iev, jev, nev0, pop_indx;
    bool lpopped;
    lpopped = false;
    if (events == NULL){
        if (verbose > 0){
            log_warnF("%s: Warning no events in list\n", fcnm);
        }
        return lpopped;         
    }
    // Find the event and see if I can remove it
    pop_indx =-2;
    nev0 = events->nev;
    for (iev=0; iev<nev0; iev++){
        if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0){
            pop_indx =-1;
            if ((currentTime - SA.time) > maxtime){
                if (verbose > 0){
                    log_infoF("%s: Removing %s %f from event list at %f\n",
                              fcnm, SA.eventid, SA.time, currentTime);
                }
                pop_indx = iev;
                break;
            }
        }
    }
    // I couldn't even find the event in the event list
    if (pop_indx ==-2){
        if (verbose > 0){
            log_warnF("%s: Strangely cannot find this event %s\n",
                      fcnm, SA.eventid);
        }
        return lpopped;
    }
    // Remove the event?
    if (pop_indx >-1){
        // Only event - good bye
        if (nev0 == 1){
            GFAST_memory_freeEvents(events);
            events->nev = 0;
            return lpopped;
        }
        // Copy old events into workspace ignoring event at pop_indx 
        SAtemp.nev = nev0 - 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                    calloc(SAtemp.nev, sizeof(struct GFAST_shakeAlert_struct));
        jev = 0;
        for (iev=0; iev<nev0; iev++){
            if (iev == pop_indx){continue;}
            memcpy(&SAtemp.SA[jev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
            jev = jev + 1;
        }
        memcpy(&SAtemp.SA[nev0], &SA, sizeof(struct GFAST_shakeAlert_struct));
        // Resize events
        GFAST_memory_freeEvents(events);
        events->nev = SAtemp.nev;
        events->SA = (struct GFAST_shakeAlert_struct *)
                     calloc(events->nev,
                            sizeof(struct GFAST_shakeAlert_struct));
        // Copy old events back
        for (iev=0; iev<events->nev; iev++){
            memcpy(&events->SA[iev], &SAtemp.SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
        }
        // Free SAtemp
        GFAST_memory_freeEvents(&SAtemp);
    }
    return lpopped;
}
