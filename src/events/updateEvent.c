#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief Checks if the shakeAlert event differs from the version in the 
 *        current events list and if it does differ then this function 
 *        updates the events list to match the input SA event
 *
 * @param[in] SA         shakeAlert structure to check for in events list
 *
 * @param[inout] events  on input contains the current list of events.
 *                       on output, if SA is not identical to an SA in the 
 *                       events list, the input SA then overrides the 
 *                       the corresponding SA in events.  additionally, if
 *                       SA does not exist in events then the program will
 *                       attempt to add the event.
 *
 * @param[out] ierr      0 indicates success
 *
 * @result true if the event structure has been modified to accomodate the
 *         input shakeAlert event SA
 *
 * @author Ben Baker, ISTI
 * 
 */
bool GFAST_events__updateEvent(struct GFAST_shakeAlert_struct SA,
                               struct GFAST_activeEvents_struct *events,
                               int *ierr)
{
    const char *fcnm = "GFAST_events__updateEvent\0";
    bool lfound, lmodified, lnew_event;
    int iev;
    *ierr = 0;
    lmodified = false;
    // Look for this event and 
    lfound = false;
    for (iev=0; iev<events->nev; iev++){
        if (strcasecmp(events->SA[iev].eventid, SA.eventid) == 0){
            lfound = true;
            if (SA.lat  != events->SA[iev].lat ||
                SA.lon  != events->SA[iev].lon ||
                SA.dep  != events->SA[iev].dep ||
                SA.mag  != events->SA[iev].mag ||
                SA.time != events->SA[iev].time)
            {
                memcpy(&events->SA[iev], &SA,
                       sizeof(struct GFAST_shakeAlert_struct));
                lmodified = true;
            }
            break;
        }
    }
    // Surprisingly i didn't find the event - try to add it
    if (!lfound){
        log_warnF("%s: Warning will try to append event %s to list\n",
                  fcnm, SA.eventid);
        lnew_event = GFAST_events__newEvent(SA, events);
        if (!lnew_event){
            log_warnF("%s: This is really weird\n", fcnm);
        }else{
            lmodified = true;
        }
    }
    return lmodified;
}
