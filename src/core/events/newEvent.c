#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast_core.h"
/*!
 * @brief Adds a new event to the working events list.
 *
 * @param[in] SA          This is the shakeAlert event whose existence is
 *                        checked for in events struture.  If it does not
 *                        exist in events then it is added. 
 * 
 * @param[in,out] events  On input holds the current active list of events. <br>
 *                        On output holds the new event in SA provided that 
 *                        SA does not already exist.
 *
 * @result If true then the new event in SA has been added to the events list.
 *
 * @author Ben Baker, ISTI
 *
 */
bool core_events_newEvent(struct GFAST_shakeAlert_struct SA,
                          struct GFAST_activeEvents_struct *events)
                          struct GFAST_activeEvents_xml_intervals *xml_intervals)
{
    struct GFAST_activeEvents_struct SAtemp;
    int iev, nev0;
    bool lnewEvent;
    lnewEvent = true;
    // New event -> copy and update
    nev0 = events->nev;
    for (iev=0; iev<nev0; iev++)
    {
        if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0)
        {
            lnewEvent = false;
            break;
        }
    }
    if (lnewEvent)
    {
        // Copy new event into workspace
        SAtemp.nev = nev0 + 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                    calloc((size_t) SAtemp.nev,
                           sizeof(struct GFAST_shakeAlert_struct));
        for (iev=0; iev<nev0; iev++)
        {
            memcpy(&SAtemp.SA[iev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
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
    return lnewEvent;
}
