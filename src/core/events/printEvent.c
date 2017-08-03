#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
/*!
 * @brief Convenience function that prints the details of an event to debug.
 *
 * @param[in] SA     shakeAlert event information structure to print.
 *
 * @author Ben Baker, ISTI
 */
void core_events_printEvents(struct GFAST_shakeAlert_struct SA) 
{
    const char *lspace = "                  \0";
    char line[128], msg[1024];
    memset(msg, 0, 1024*sizeof(char));
    sprintf(msg, "%s: Event %s statistics:\n", lspace, SA.eventid);

    memset(line, 0, 128*sizeof(char));
    sprintf(line, "%s  Event latitude %f (degrees)\n", lspace, SA.lat);
    strcat(msg, line);

    memset(line, 0, 128*sizeof(char));
    sprintf(line, "%s  Event longitude %f (degrees)\n", lspace, SA.lon);
    strcat(msg, line);

    memset(line, 0, 128*sizeof(char));
    sprintf(line, "%s  Event depth %f (km)\n", lspace, SA.dep);
    strcat(msg, line);

    memset(line, 0, 128*sizeof(char));
    sprintf(line, "%s  Event magnitude %f (km)\n", lspace, SA.mag);
    strcat(msg, line);

    memset(line, 0, 128*sizeof(char));
    sprintf(line, "%s  Event epochal time %f (s)\n", lspace, SA.time);
    strcat(msg, line);

    LOG_DEBUGMSG("%s", msg);
    return;
}
