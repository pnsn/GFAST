#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_core.h"
/*!
 * @brief Convenience function to pop expired events from the events list.
 *        An event is expired when:
 *          currentTime - eventOriginTime > processingTime.
 *
 * @param[in] processingTime    Max processing time (seconds) for any event.
 * @param[in] currentTime       Current time (UTC epochal seconds).
 * @param[in] verbose           Controls the verbosity.  0 is quiet.
 *
 * @param[in,out] events        On input holds the active events list.
 *                              On output expired events have been removed.
 *
 * @result The number of events removed from the list.
 *
 * @author Ben Baker
 *
 */
int core_events_removeExpiredEvents(const double processingTime,
                                    const double currentTime,
                                    const int verbose,
                                    struct GFAST_activeEvents_struct *events)
{
    struct GFAST_shakeAlert_struct *SAall;
    double t1, t2;
    int iev, nev0, nrem;
    bool ldownDate, lgone;
    // Nothing to remove
    nrem = 0;
    if (events->nev < 1){return 0;}
    // Create a copy of the events
    nev0 = events->nev;
    SAall = (struct GFAST_shakeAlert_struct *)
            calloc((size_t) nev0, sizeof(struct GFAST_shakeAlert_struct));
    for (iev=0; iev<events->nev; iev++)
    {
        memcpy(&SAall[iev], &events->SA[iev],
               sizeof(struct GFAST_shakeAlert_struct));
    }
    // Look for expired events
    for (iev=0; iev<nev0; iev++)
    {
        t1 = SAall[iev].time; // Origin time
        t2 = currentTime;
        ldownDate = false;
        if (t2 - t1 >= processingTime){ldownDate = true;}
LOG_MSG("removeExpiredEvents: iev=%d ldownDate=%d", iev, ldownDate);
        if (ldownDate)
        {
            lgone = core_events_removeExpiredEvent(processingTime,
                                                   currentTime,
                                                   verbose,
                                                   SAall[iev], events);
            if (!lgone)
            {
                LOG_WARNMSG("Strange - but keeping %s", SAall[iev].eventid);
            }
            else
            {
                nrem = nrem + 1;
            }
        }
    }
LOG_MSG("removeExpiredEvents: nrem=%d free(SAall)", nrem);
    free(SAall);
LOG_MSG("removeExpiredEvents: nrem=%d free(SAall) DONE --> return", nrem);
    return nrem;
}
