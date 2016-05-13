#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"

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
int GFAST_readElarmS__xml(const char *message, double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_alert__parse__shakeAlertXML\0";
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
            ierr = GFAST_xml_coreInfo__read((void *) core_xml, SA_NAN, &core);
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
    if (ierr != 0){ 
        printf("%s: Error parsing core shakeAlert information\n", fcnm);
    }else{
        strcpy(SA->eventid, core.id);
        SA->time = core.orig_time;
        SA->lat = core.lat;
        SA->lon = core.lon;
        SA->dep = core.depth;
        SA->mag = core.mag;
    }
    // Clean up
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return ierr;
}
/*!
 * @brief Extracts data from an ElarmS message and fills a shake alert struct
 *        A characteristic ElarmS message will look like:
 *         ID  Year Mo Da HH:MM:SS.SSSSSS Mag     lon      lat
 *        1999 2015-08-24 14:33:43.612000 2.60 -122.2033 46.7320
 *
 * @param[in] buff      null terminated string containing the ElarmS
 *                      string
 * @param[in] verbose   controls verbosity (< 2 is quiet)
 *
 * @param[out] SA       shakeAlert event structure (lat, lon, depth, magnitude,
 *                      ID, and time)
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int GFAST_readElarmS_ElarmSMessage2SAStruct(int verbose, char *buff,
                                            struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_readElarmS_ElarmSMessage2SAStruct\0";
    double ev_sec, lon_print;
    int ev_year, ev_month, ev_dom, ev_hour, ev_isec, ev_min, ev_musec;
    //------------------------------------------------------------------------//
    //
    // Zero out output and ensure message isn't NULL
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    if (buff == NULL){
        log_errorF("%s: Error the buffer is NULL\n", fcnm);
        return -1;
    }
    // Parse it:
    //      1999 2015-08-24 14:33:43.612000 2.60 -122.2033 46.7320
    sscanf(buff, "%s %04d-%02d-%02d %02d:%02d:%lf %lf %lf %lf",
           SA->eventid,
           &ev_year, &ev_month, &ev_dom,
           &ev_hour, &ev_min, &ev_sec,
           &SA->mag, &SA->lon, &SA->lat);
    // Warn on latitude
    if (SA->lat <-90.0 || SA->lat > 90.0){
        if (verbose > 1){
            log_warnF("%s: eLarms latitude %f is invalid\n",
                      fcnm, SA->lat);
        }
    }
    // Put longitude in [0,360]
    if (SA->lon < 0.0){SA->lon = SA->lon + 360.0;}
    // Warn on screwy longitudes
    if ((SA->lon < 0.0 || SA->lon > 360.0) && verbose > 1){
        log_warnF("%s: eLarms longitude %f is strange\n", fcnm, SA->lon);
    }
    // Create the time
    ev_isec = (int) ev_sec;
    ev_musec = (int) ((ev_sec - (double) ev_isec)*1.e6);
    SA->time = time_calendar2epoch2(ev_year, ev_month, ev_dom, ev_hour,
                                    ev_min, ev_isec, ev_musec);
    if (verbose > 2){
        lon_print = SA->lon;
        if (lon_print > 180.0){lon_print = lon_print - 360.0;}
        log_debugF("%s: eLarms location (time,lat,lon)=(%lf %f %f)\n",
                   fcnm, SA->time, SA->lat, lon_print);
    }
    return 0;
}
//============================================================================//
/*!
 * @brief Reads the ElarmS file into the Shake Alert structure 
 *
 * @param[in] props     GFAST properties structure (verbosity, 
                        eewsfile name, and default earthquake depth)
 *
 * @param[out] SA       shakeAlert structure
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_readElarmS(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_readElarmS\0";
    FILE *ew;
    char buffer[128], line[128];
    int lenb;
    bool lempty;
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    if (!os_path_isfile(props.eewsfile)){
        log_errorF("%s: Error cannot find EEW file %s\n",
                   fcnm, props.eewsfile);
        return -1;
    }
    // Open file and read until end
    ew = fopen(props.eewsfile, "r");
    memset(buffer, 0, sizeof(buffer));
    memset(line, 0, sizeof(line));
    lempty = true;
    while (fgets(buffer, 128, ew) != NULL){
        lempty = false;
        // Replace carriage return with NULL terminator
        lenb = strlen(buffer);
        if (lenb > 0){
            if (buffer[lenb-1] == '\n'){buffer[lenb-1] = '\0';}
        }
        memset(line, 0, sizeof(line));
        strcpy(line, buffer); 
        memset(buffer, 0, sizeof(buffer));
    }
    if (!lempty){
        sscanf(buffer, "%s\n", line);
        GFAST_readElarmS_ElarmSMessage2SAStruct(props.verbose, line, SA);
        if (props.eqDefaultDepth >= 0.0){SA->dep = props.eqDefaultDepth;}
    }
    fclose(ew);
    return 0;
}
