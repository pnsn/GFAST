#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"

/*!
 * @brief Reads from the input shakeAlert XML message into the core structure
 *
 * @param[in] message  null terminated XML message from which to extract
 *                     core_info
 * @param[in] SA       if an item is not found then this is it's default value
 *
 * @param[out] core    on successful exit contains the event ID, magnitude,
 *                     magnitude uncertainty, latitude, latitude uncertainty,
 *                     longitude, longitude uncertainty, depth, depth
 *                     uncertainty, origin time, origin time uncertainty,
 *                     with accompanying units as well as likelihood
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker 
 */
int GFAST_xml_read__SACoreInfo(const char *message, double SA_NAN,
                               struct coreInfo_struct *core)
{
    const char *fcnm = "GFAST_xml_read__SACoreInfo\0";
    xmlDocPtr doc;
    xmlNodePtr event_xml, core_xml, core_xml_info;
    xmlAttrPtr attr;
    xmlChar *value;
    const int nitems = 11; 
    const xmlChar *citems[11] = { BAD_CAST "mag\0", BAD_CAST "mag_uncer\0",
                                  BAD_CAST "lat\0", BAD_CAST "lat_uncer\0",
                                  BAD_CAST "lon\0", BAD_CAST "lon_uncer\0",
                                  BAD_CAST "depth\0", BAD_CAST "depth_uncer\0",
                                  BAD_CAST "orig_time\0",
                                  BAD_CAST "orig_time_uncer\0",
                                  BAD_CAST "likelihood\0"};
    double values[11];
    int units[11];
    const int types[11] = {1, 1,
                           1, 1,
                           1, 1,
                           1, 1,
                           2, 1,
                           1};
    long int length;
    int dom, hour, ierr, item, item0, minute, month, nzmsec, nzmusec,
        nzsec, year;
    bool lfound, lunpack;
    // Initialize result
    ierr = 0;
    lfound = false;
    memset(core, 0, sizeof(struct coreInfo_struct));
    for (item=0; item<nitems; item++){
        values[item] = SA_NAN;
        units[item] = UNKNOWN_UNITS;
    }
    // Parse the XML message
    length = strlen(message);
    if (length == 0){
        log_errorF("%s: Error message is empty\n", fcnm);
        return -1;
    }
    doc = xmlReadMemory(message, length, "noname.xml\0", NULL, 0);
    if (doc == NULL){
        log_errorF("%s: Error - failed to parse xml document\n", fcnm);
        return -1;
    }
    // Make sure there's somethign in the message
    event_xml = xmlDocGetRootElement(doc);
    if (event_xml == NULL){
        log_errorF("%s: Empty document\n", fcnm);
        return -1;
    }
    item0 =-1;
    // Find the event_message
    while (event_xml != NULL){
        if ((xmlStrcmp(event_xml->name, (const xmlChar *) "event_message"))){
             event_xml = event_xml->next;
             continue;
        }
        // Find the core_info in the event message
        core_xml = event_xml->xmlChildrenNode;
        while (core_xml != NULL){
            // Require this be core_info
            if ((xmlStrcmp(core_xml->name, (const xmlChar *) "core_info\0"))){
                 core_xml = core_xml->next;
                 continue;
            }
            // Get the eventID
            attr = xmlHasProp(core_xml, BAD_CAST "id\0");
            if (attr){
                value = xmlNodeListGetString(core_xml->doc, attr->children, 1);
                strcpy(core->id, (char *)value);
                free(value);
            }else{
                log_warnF("%s: Warning couldn't lift the event ID\n", fcnm);
            }
            // Now parse the core info
            core_xml_info = core_xml->xmlChildrenNode;
            while (core_xml_info != NULL){
                // Require we are working on an element node
                if (core_xml_info->type != XML_ELEMENT_NODE){
                    core_xml_info = core_xml_info->next;
                    continue;
                }
                lunpack = false;
                // Try taking the next item in citems
                if (item0 < nitems){
                    if (!xmlStrcmp(core_xml_info->name,
                        citems[item0+1])){
                        lunpack = true;
                        item0 = item0 + 1;
                    }
                }
                // Otherwise do a linear search of citems
                if (!lunpack){
                    for (item=0; item<nitems; item++){
                        if (!xmlStrcmp(core_xml_info->name, citems[item])){
                            lunpack = true;
                            item0 = item + 1;
                            break;
                         }
                    }
                }
                // I didn't find this value in citems - weird - continue 
                if (!lunpack){
                    log_warnF("%s: Warning couldn't find item %s\n",
                              fcnm, (char *) core_xml_info->name);
                    core_xml_info = core_xml_info->next;
                    continue;
                }
                // Check on item index to avoid a segfault
                if (item0 < 0 || item0 > nitems - 1){
                    log_errorF("%s: Invalid index %s\n",
                               fcnm, (char *)core_xml_info->name);
                    ierr = 1;
                    lfound = false;
                    goto ERROR;
                }
                // Are there units to unpack?
                attr = xmlHasProp(core_xml_info, BAD_CAST "units\0");
                if (attr && types[item0] != 2){
                    value = xmlNodeListGetString(core_xml_info->doc,
                                                 attr->children, 1);
                    units[item0] = __xml_units__string2enum((char *)value);
                    free(value);
                }
                // Is there a value to unpack?
                value = xmlNodeGetContent(core_xml_info);
                if (value != NULL){
                    // Double
                    if (types[item0] == 1){
                        values[item0] = xmlXPathCastStringToNumber(value);
                    // Time
                    }else if (types[item0] == 2){
                        sscanf((char *)value, "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ",
                               &year, &month, &dom, &hour,
                               &minute, &nzsec, &nzmsec);
                        nzmusec = nzmsec*1000;
                        values[item0] = time_calendar2epoch2(year, month,
                                                             dom, hour,
                                                             minute, nzsec,
                                                             nzmusec);
                        units[item0] = SECONDS;
                    }
                    free(value);
                }
                core_xml_info = core_xml_info->next;
            } // End loop on core_xml_info
            lfound = true;
            core_xml = core_xml->next;
        } // End loop on core_xml
        if (lfound){break;} // Got more core info - finished
        event_xml = event_xml->next;
    } // End loop on event_xml
    if (!lfound){
        log_errorF("%s: Error I couldn't find core_info\n", fcnm);
        ierr = 1;
    }
ERROR:;
    if (ierr != 0){
        for (item=0; item<nitems; item++){
            values[item] = SA_NAN;
            units[item] = UNKNOWN_UNITS;
        }
    }
    // Copy the results - values
    core->mag             = values[0];
    core->mag_uncer       = values[1];
    core->lat             = values[2];
    core->lat_uncer       = values[3];
    core->lon             = values[4];
    core->lon_uncer       = values[5];
    core->depth           = values[6];
    core->depth_uncer     = values[7];
    core->orig_time       = values[8];
    core->orig_time_uncer = values[9];
    core->likelihood      = values[10];
    // Copy the units 
    core->mag_units             = units[0];
    core->mag_uncer_units       = units[1];
    core->lat_units             = units[2];
    core->lat_uncer_units       = units[3];
    core->lon_units             = units[4];
    core->lon_uncer_units       = units[5];
    core->depth_units           = units[6];
    core->depth_uncer_units     = units[7];
    core->orig_time_units       = units[8];
    core->orig_time_uncer_units = units[9];
    // Clean up
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return ierr;
}
