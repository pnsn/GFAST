#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast_core.h"
/*!
 * @brief Checks if the shakeAlert event differs from the version in the 
 *        current events list and if it does differ then this function 
 *        updates the events list to match the input SA event
 *
 * @param[in] SA          shakeAlert structure to check for in events list
 *
 * @param[in,out] events  on input contains the current list of events.
 *                        on output, if SA is not identical to an SA in the 
 *                        events list, the input SA then overrides the 
 *                        the corresponding SA in events.  additionally, if
 *                        SA does not exist in events then the program will
 *                        attempt to add the event.
 *
 * @param[out] ierr      0 indicates success
 *
 * @result true if the event structure has been modified to accomodate the
 *         input shakeAlert event SA
 *
 * @author Ben Baker, ISTI
 * 
 */
bool core_events_updateEvent(struct GFAST_shakeAlert_struct SA,
                             struct GFAST_activeEvents_struct *events,
                             int *ierr)
{
    bool lfound, lmodified, lnew_event;
    int iev;
    *ierr = 0;
    lmodified = false;
    // Look for this event and 
    lfound = false;
    for (iev=0; iev<events->nev; iev++)
    {
        if (strcasecmp(events->SA[iev].eventid, SA.eventid) == 0)
        {
            lfound = true;
            if (fabs(SA.lat  - events->SA[iev].lat)  > 1.e-10 ||
                fabs(SA.lon  - events->SA[iev].lon)  > 1.e-10 ||
                fabs(SA.dep  - events->SA[iev].dep)  > 1.e-10 ||
                fabs(SA.mag  - events->SA[iev].mag)  > 1.e-10 ||
                fabs(SA.time - events->SA[iev].time) > 1.e-5)
            {
                memcpy(&events->SA[iev], &SA,
                       sizeof(struct GFAST_shakeAlert_struct));
                lmodified = true;
            }
            break;
        }
    }
    // Surprisingly i didn't find the event - try to add it
    if (!lfound)
    {
        LOG_WARNMSG("Warning will try to append event %s to list",
                     SA.eventid);
        // MTH: I don't believe updateEvent is used, but just in case:
        //lnew_event = core_events_newEvent(SA, events);
        printf("MTH: ERROR: updateEvent was called!! --> Exit!\n");
        exit(2);

        if (!lnew_event)
        {
            LOG_WARNMSG("%s", "This is really weird");
        }
        else
        {
            lmodified = true;
        }
    }
    return lmodified;
}
