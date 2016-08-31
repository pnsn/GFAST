#include <stdio.h>
#include <stdlib.h>
#include "gfast.h"
#include "iscl/log/log.h"
/*!
 * @brief Convenience function that prints the details of an event to debug
 *
 * @param[in] SA     shakeAlert event information structure to print
 *
 * @author Ben Baker, ISTI
 */
void GFAST_events__print(struct GFAST_shakeAlert_struct SA) 
{
    const char *fcnm   = "GFAST_events__print\0";
    const char *lspace = "                   \0";
    log_debugF("%s: Event %s statistics:\n", fcnm, SA.eventid);
    log_debugF("%s  Event latitude %f (degrees)\n", lspace, SA.lat);
    log_debugF("%s  Event longitude %f (degrees)\n", lspace, SA.lon);
    log_debugF("%s  Event depth %f (km)\n", lspace, SA.dep);
    log_debugF("%s  Event magnitude %f (km)\n", lspace, SA.mag);
    log_debugF("%s  Event epochal time %f (s)\n", lspace, SA.time);
    return;
}
