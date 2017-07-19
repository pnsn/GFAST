#ifndef _gfast_events_h__
#define _gfast_events_h__ 1
#include <stdbool.h>
#include "gfast_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* Frees memory on an event structure */
void events_freeEvents(struct GFAST_activeEvents_struct *events);
/* Convenience function to find min origin time in event list */
double events_getMinOriginTime(struct GFAST_props_struct props,
                               struct GFAST_activeEvents_struct events,
                               bool *lnoEvents);
/* Adds a new event to the event list */
bool events_newEvent(struct GFAST_shakeAlert_struct SA,
                     struct GFAST_activeEvents_struct *events);
/* Print the events in the event list */
void events_printEvents(struct GFAST_shakeAlert_struct SA);
/* Remove an event from the events list */
bool events_removeEvent(const double maxtime,
                        const double currentTime,
                        const int verbose,
                        struct GFAST_shakeAlert_struct SA,
                        struct GFAST_activeEvents_struct *events);
/* Potentially add the shakeAlert event to the event list */
bool events_updateEvent(struct GFAST_shakeAlert_struct SA,
                        struct GFAST_activeEvents_struct *events,
                        int *ierr);

#define GFAST_events_freeEvents(...)       \
              events_freeEvents(__VA_ARGS__)
#define GFAST_events_getMinOriginTime(...)       \
              events_getMinOriginTime(__VA_ARGS__)
#define GFAST_events_newEvent(...)       \
              events_newEvent(__VA_ARGS__)
#define GFAST_events_printEvents(...)       \
              events_printEvents(__VA_ARGS__)
#define GFAST_events_removeEvent(...)       \
              events_removeEvent(__VA_ARGS__)
#define GFAST_events_updateEvent(...)       \
              events_updateEvent(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* _gfast_events_h__ */
