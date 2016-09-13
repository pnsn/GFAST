#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast_eewUtils.h"
#include "gfast_xml.h"
#include "iscl/log/log.h"

/*!
 * @brief Reads a elarmS based shakeAlert message into the GFAST 
 *        shakeAlert structure which wants to know the event ID,
 *        origin time, latitude, longitude, depth, and magnitude
 *
 * @param[in] message    null terminated XML shakeAlert message to parse
 * @param[in] SA_NAN     if a value does not exist in the message it will
 *                       be given this default value
 *
 * @param[out] SA        shake alert structure  
 *
 * @author Ben Baker, ISTI
 *
 */
int eewUtils_parseCoreXML(const char *message,
                          const double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_eewUtils_parseCoreXML\0";
    xmlDocPtr doc;
    xmlNodePtr core_xml, event_xml;
    struct coreInfo_struct core;
    int ierr, length;
    bool lfound;
    // Initialize
    ierr = 0;
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    memset(&core, 0, sizeof(struct coreInfo_struct));
    length = strlen(message);
    if (length == 0)
    {
        log_errorF("%s: Error the message is empty\n", fcnm);
        return -1;
    }
    doc = xmlReadMemory(message, length, "noname.xml\0", NULL, 0);
    if (doc == NULL)
    {
        log_errorF("%s: Error - failed to parse xml document\n", fcnm);
        return -1;
    }
    // Make sure there's something in the message
    event_xml = xmlDocGetRootElement(doc);
    if (event_xml == NULL)
    {
        log_errorF("%s: Empty document\n", fcnm);
        return -1;
    }
    lfound = false;
    // Loop on the event_xml
    while (event_xml != NULL)
    {
        if (xmlStrcmp(event_xml->name, BAD_CAST "event_message\0") != 0)
        {
             goto NEXT_EVENT_XML;
        }
        // Find the core_info in the event message
        core_xml = event_xml->xmlChildrenNode;
        while (core_xml != NULL)
        {
            // Require this be core_info
            if (xmlStrcmp(core_xml->name, BAD_CAST "core_info\0") != 0)
            {
                 goto NEXT_CORE_XML;
            }
            lfound = true;
            // Parse it
            ierr = GFAST_xml_shakeAlert_readCoreInfo((void *) core_xml,
                                                     SA_NAN, &core);
            if (ierr != 0)
            {
                 log_errorF("%s: Error reading core info!\n", fcnm);
            }
            break; 
NEXT_CORE_XML:;
            core_xml = core_xml->next;
        } // Loop on search for core_xml
        if (lfound){break;}
NEXT_EVENT_XML:;
        event_xml = event_xml->next;
    } // Loop on events
    if (ierr != 0)
    {
        log_errorF("%s: Error parsing core shakeAlert information\n", fcnm);
    }
    else
    {
        strcpy(SA->eventid, core.id);
        // origin time
        if (core.lhaveOrigTime)
        {
            SA->time = core.origTime;
        }
        else
        {
            log_errorF("%s: Couldn't find origin time\n", fcnm);
            ierr = ierr + 1;
        }
        // latitude
        if (core.lhaveLat)
        {
            SA->lat = core.lat;
        }
        else
        {
            log_errorF("%s: Couldn't find latitude\n", fcnm);
            ierr = ierr + 1;
        }
        // longitude
        if (core.lhaveLon)
        {
            SA->lon = core.lon;
        }
        else
        {
            log_errorF("%s: Couldn't find longitude\n", fcnm);
            ierr = ierr + 1;
        }
        // depth
        if (core.lhaveDepth)
        {
            SA->dep = core.depth;
        }
        else
        {
            log_errorF("%s: Couldn't find depth\n", fcnm);
            ierr = ierr + 1;
        }
        // magnitude
        if (core.lhaveMag)
        {
            SA->mag = core.mag;
        }
        else
        {
            log_errorF("%s: Couldn't find magnitude\n", fcnm);
            ierr = ierr + 1;
        }
        if (SA->lon < 0.0){SA->lon = SA->lon + 360.0;}
    }
    // Clean up
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return ierr;
}
