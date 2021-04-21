#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
/*!
 * @brief Keep xml_status in sync with events list
 *
 * @param[in] events   On input contains all events.
 * @param[in] xml_status   On input contains output_complete status for all events
 *
 * @author Mike Hagerty (ISTI)
 *
 */
bool core_events_syncXMLStatusWithEvents(struct GFAST_activeEvents_struct *events,
                                         struct GFAST_activeEvents_xml_status *xml_status)
{
    struct GFAST_xml_output_status output_status;
    struct GFAST_activeEvents_xml_status Xtemp;

    char *eventid;

    memset(&output_status, 0, sizeof( struct GFAST_xml_output_status));

    int iev, jev, nev, nstats, ifound;
    bool found;

    nev = events->nev;
    nstats = xml_status->nev;

    Xtemp.nev = nev;
    Xtemp.SA_status = (struct GFAST_xml_output_status *) calloc((size_t) Xtemp.nev, sizeof(struct GFAST_xml_output_status));

    for (iev=0; iev<nev; iev++){
      strcpy(eventid, events->SA[iev].eventid);

      found = false;
      for (jev=0; jev<nstats; jev++){
        if (strcasecmp(xml_status->SA_status[jev].eventid, eventid) == 0){
          found = true;
          ifound = jev;
          break;
        }
      }
      if (found) {
        // copy this xml_status output_status record to Xtemp
        LOG_MSG("Events iev:%d eventid:%s : Copy xml_status SA_status[%d] to Xtemp SA_status[%d]",
            iev, eventid, ifound, iev);
        memcpy(&Xtemp.SA_status[iev], &xml_status->SA_status[ifound], sizeof(struct GFAST_xml_output_status));
      }
      else {
        // create new one from this eventid
        LOG_MSG("Events iev:%d eventid:%s not found in xml_status : Create new xml_status SA record for this eventid",
            iev, eventid);
        strcpy(output_status.eventid, eventid);
        memcpy(&Xtemp.SA_status[iev], &output_status, sizeof(struct GFAST_xml_output_status));
      }

    }

    for (iev=0; iev<xml_status->nev; iev++){
      found = false;
      for (jev=0; jev<Xtemp.nev; jev++){
        if (strcasecmp(xml_status->SA_status[iev].eventid, Xtemp.SA_status[jev].eventid) == 0){
          found = true;
          break;
        }
      }
      if (!found) {
        LOG_MSG("Eventid:%s was removed from xml_status", xml_status->SA_status[iev].eventid);
      }
    }

    // Free xml_status and copy Xtemp back in
    free(xml_status->SA_status);
    //memset(xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));
    xml_status->nev = Xtemp.nev;
    xml_status->SA_status = (struct GFAST_xml_output_status *) calloc((size_t) 
                      xml_status.nev, sizeof(struct GFAST_xml_output_status));

    // Copy new list back to xml_status:
    for (iev=0; iev<xml_status->nev; iev++){
      memcpy(&xml_status->SA_status[iev], &Xtemp.SA_status[iev], sizeof(struct GFAST_xml_output_status));
    }

    // Free Xtemp
    free(Xtemp.SA_status);
}
