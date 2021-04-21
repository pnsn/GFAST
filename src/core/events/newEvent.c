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
                          struct GFAST_activeEvents_struct *events,
                          struct GFAST_activeEvents_xml_status *xml_status)
{
    struct GFAST_activeEvents_struct SAtemp;

    struct GFAST_xml_output_status output_status;
    struct GFAST_activeEvents_xml_status Xtemp;

    int i=0;
    LOG_MSG("MTH: in newEvent: call output_status i=%d", i);
    memset(&output_status, 0, sizeof( struct GFAST_xml_output_status));

    int iev, nev0;
    bool lnewEvent;
    lnewEvent = true;
    // New event -> copy and update
    nev0 = events->nev;
    LOG_MSG("MTH: in newEvent: nev0=%d", nev0);
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
            memcpy(&SAtemp.SA[iev], &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
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

        // MTH: Same thing for xml output status structs
        // Create a status output_status from incoming SA eventid
        strcpy(output_status.eventid, SA.eventid);
        // Append this output_status to the incoming xml_status list of records
        Xtemp.nev = nev0 + 1;
        Xtemp.SA_status = (struct GFAST_xml_output_status *) calloc((size_t) Xtemp.nev, sizeof(struct GFAST_xml_output_status));
        for (iev=0; iev<nev0; iev++){
          memcpy(&Xtemp.SA_status[iev], &xml_status->SA_status[iev], sizeof(struct GFAST_xml_output_status));
        }
        memcpy(&Xtemp.SA_status[nev0], &output_status, sizeof(struct GFAST_xml_output_status));

        // Free xml_status and copy Xtemp back in
        if (xml_status->nev > 0) {
          free(xml_status->SA_status);
          //memset(xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));
        }
        xml_status->nev = Xtemp.nev;

        // Copy new list back to xml_status:
        for (iev=0; iev<xml_status->nev; iev++){
          memcpy(&xml_status->SA_status[iev], &Xtemp.SA_status[iev], sizeof(struct GFAST_xml_output_status));
        }

        // Free Xtemp
        free(Xtemp.SA_status);
    }
    return lnewEvent;
}
