#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast.h"

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
double GFAST_events__getMinOriginTime(struct GFAST_props_struct props,
                                      struct GFAST_activeEvents_struct events,
                                      bool *lnoEvents)
{
    const char *fcnm = "GFAST_events__getMinOriginTime\0";
    double t0;
    int iev;
    *lnoEvents = true;
    t0 = DBL_MAX;
    if (events.nev < 1){return t0;}
    *lnoEvents = false;
    for (iev=0; iev<events.nev; iev++){
        t0 = fmin(t0, events.SA[iev].time);
    }
    if (props.verbose > 1 && t0 == DBL_MAX){
        *lnoEvents = true;
        log_warnF("%s: Warning origin times are screwy\n", fcnm);
    }
    return t0;
}
//============================================================================//
/*!
 * @brief Checks if the shakeAlert event differs from the version in the 
 *        current events list and if it does updates the events list
 *
 * @param[in] SA         shakeAlert structure to check for in events list
 *
 * @param[inout] events  on input contains the current list of events.
 *                       on output, if SA is not identical an SA in the 
 *                       events list, the input SA then overrides the 
 *                       the corresponding SA in events
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
//============================================================================//
/*!
 * @brief Alarms to a new event and adds it to the working events list
 *
 * @param[in] SA         this is the shakeAlert event which is checked for in
 *                       the events structure.  if it does not exist in events
 *                       then it is added to events
 *
 * @param[inout] events  on input holds the current active list of events.
 *                       on output if events, if SA is not initially in events,
 *                       then it has now been added
 *
 * @result if true then the new event in SA has been added to the events list
 *
 * @author Ben Baker, ISTI
 *
 */
bool GFAST_events__newEvent(struct GFAST_shakeAlert_struct SA,
                            struct GFAST_activeEvents_struct *events)
{
    struct GFAST_activeEvents_struct SAtemp;
    int iev, nev0;
    bool lnewEvent;
    lnewEvent = true;
    // New event -> copy and update
    nev0 = events->nev;
    for (iev=0; iev<nev0; iev++){
        if (strcasecmp(SA.eventid, events->SA[iev].eventid) == 0){
            lnewEvent = false;
            break;
        }
    }
    if (lnewEvent){
        // Copy new event into workspace
        SAtemp.nev = nev0 + 1;
        SAtemp.SA = (struct GFAST_shakeAlert_struct *)
                    calloc(SAtemp.nev, sizeof(struct GFAST_shakeAlert_struct));
        for (iev=0; iev<nev0; iev++){
            memcpy(&SAtemp.SA[iev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
        }
        memcpy(&SAtemp.SA[nev0], &SA, sizeof(struct GFAST_shakeAlert_struct));
        // Resize events
        GFAST_memory_freeEvents(events);
        // Add it
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
        //memory_freeEvents(events);
    }
    return lnewEvent;
}
//============================================================================//
/*!
 * @brief Convenience function that prints the details of an event to debug
 *
 * @param[in] SA     shakeAlert event information structure to print
 *
 * @author Ben Baker, ISTI
 */
void GFAST_events__print__event(struct GFAST_shakeAlert_struct SA)
{
    const char *fcnm   = "GFAST_events__print__event\0";
    const char *lspace = "                          \0";
    log_debugF("%s: Event %s statistics:\n", fcnm, SA.eventid);
    log_debugF("%s  Event latitude %f (degrees)\n", lspace, SA.lat);
    log_debugF("%s  Event longitude %f (degrees)\n", lspace, SA.lon);
    log_debugF("%s  Event depth %f (km)\n", lspace, SA.dep);
    log_debugF("%s  Event magnitude %f (km)\n", lspace, SA.mag);
    log_debugF("%s  Event epochal time %f (s)\n", lspace, SA.time);
    return;
}
//============================================================================//
bool GFAST_events__removeEvent(double bufflen, double currentTime,
                               struct GFAST_shakeAlert_struct SA, 
                               struct GFAST_activeEvents_struct *events)
{
    bool lpopped;
    lpopped = false;
    return lpopped;
}
