#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include "gfast.h"

/*!
 * @brief Reads from the input shakeAlert XML message into the core structure
 *
 * @param[in] xml_reader  pointer to core_info xmlNodePtr
 * @param[in] SA_NAN      if an item is not found then this is it's default
 *                        value
 *
 * @param[out] core       on successful exit contains the event ID, magnitude,
 *                        magnitude uncertainty, latitude, latitude uncertainty,
 *                        longitude, longitude uncertainty, depth, depth
 *                        uncertainty, origin time, origin time uncertainty,
 *                        with accompanying units as well as likelihood
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker 
 */
int GFAST_xml_coreInfo__read(const void *xml_reader, const double SA_NAN,
                             struct coreInfo_struct *core)
{
    const char *fcnm = "GFAST_xml_coreInfo__read\0";
    xmlNodePtr core_xml, core_xml_info;
    xmlAttrPtr attr;
    xmlChar *value;
    enum unpack_types_enum
    {
        UNPACK_DOUBLE = 1,
        UNPACK_TIME = 2
    };
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
    const int types[11] = {UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_TIME, UNPACK_DOUBLE,
                           UNPACK_DOUBLE};
    int dom, hour, ierr, item, item0, minute, month, nzmsec, nzmusec,
        nzsec, year;
    bool lfound, lunpack;
    // Initialize result
    ierr = 0;
    lfound = false;
    memset(core, 0, sizeof(struct coreInfo_struct));
    for (item=0; item<nitems; item++)
    {
        values[item] = SA_NAN;
        units[item] = UNKNOWN_UNITS;
    }
    // Require there be something to read
    if (xml_reader == NULL)
    {
        log_errorF("%s: Error xml reader is NULL\n", fcnm);
        return -1;
    }
    core_xml = (xmlNodePtr )xml_reader;
    // Make sure we're in the right spot
    if ((xmlStrcmp(core_xml->name, BAD_CAST "core_info\0") != 0))
    {
        log_errorF("%s: Error xml_reader does not start at core_info %s!\n",
                   fcnm, (char *) core_xml->name);
        return -1;
    }
    // Get the eventID
    attr = xmlHasProp(core_xml, BAD_CAST "id\0");
    if (attr)
    {
        value = xmlNodeListGetString(core_xml->doc, attr->children, 1);
        strcpy(core->id, (char *) value);
        free(value);
    }
    else
    {
        log_warnF("%s: Warning couldn't lift the event ID\n", fcnm);
    }
    // Now parse the core info
    item0 =-1;
    core_xml_info = core_xml->xmlChildrenNode;
    while (core_xml_info != NULL)
    {
        // Require we are working on an element node
        if (core_xml_info->type != XML_ELEMENT_NODE)
        {
            goto NEXT_CORE_XML_INFO;
        }
        lfound = true;
        lunpack = false;
        // Try taking the next item in citems
        if (item0 < nitems)
        {
            if (xmlStrcmp(core_xml_info->name, citems[item0+1]) == 0)
            {
                lunpack = true;
                item0 = item0 + 1;
            }
        }
        // Otherwise do a linear search of citems
        if (!lunpack)
        {
            for (item=0; item<nitems; item++){
                if (xmlStrcmp(core_xml_info->name, citems[item]) == 0)
                {
                    lunpack = true;
                    item0 = item + 1;
                    break;
                }
            }
        }
        // I didn't find this value in citems - weird - continue 
        if (!lunpack)
        {
            log_warnF("%s: Warning couldn't find item %s\n",
                      fcnm, (char *) core_xml_info->name);
            goto NEXT_CORE_XML_INFO;
        }
        // Check on item index to avoid a segfault
        if (item0 < 0 || item0 > nitems - 1)
        {
            log_errorF("%s: Invalid index %s %d\n",
                       fcnm, (char *)core_xml_info->name, item0);
           ierr = 1;
           lfound = false;
           goto ERROR;
        }
        // Are there units associated with a double to unpack?
        attr = xmlHasProp(core_xml_info, BAD_CAST "units\0");
        if (attr && types[item0] != UNPACK_TIME)
        {
            value = xmlNodeListGetString(core_xml_info->doc,
                                         attr->children, 1);
            units[item0] = __xml_units__string2enum((char *)value);
            free(value);
        }
        // Is there a value to unpack?
        value = xmlNodeGetContent(core_xml_info);
        if (value != NULL)
        {
             if (types[item0] == UNPACK_DOUBLE){
                 values[item0] = xmlXPathCastStringToNumber(value);
             }
             else if (types[item0] == UNPACK_TIME)
             {
                 sscanf((char *)value, "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ",
                        &year, &month, &dom, &hour,
                        &minute, &nzsec, &nzmsec);
                 nzmusec = nzmsec*1000;
                 values[item0] = time_calendar2epoch2(year, month, dom, hour,
                                                      minute, nzsec, nzmusec);
                 units[item0] = SECONDS;
             }
             else
             {
                 log_warnF("%s: Unknown type %d\n", fcnm, types[item0]); 
             }
             free(value);
        }
NEXT_CORE_XML_INFO:;
        core_xml_info = core_xml_info->next;
    } // End loop on core_xml_info
    if (!lfound){
        log_errorF("%s: Error I couldn't find any elements in core_info\n",
                   fcnm);
        ierr = 1;
    }
ERROR:;
    if (ierr != 0)
    {
        for (item=0; item<nitems; item++)
        {
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
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes the shakeAlert core info to the xmlTextWriter xml_writer
 *
 * @param[in] core           contains the core information which is the 
 *                           event ID, magnitude, magnitude uncertainty,
 *                           latitude, latitude uncertainty, longitude, 
 *                           longitude uncertainty, depth, depth uncertainty,
 *                           origin time, origin time uncertainty, all with
 *                           accompanying units and the likelihood
 *
 * @param[inout] xml_writer  pointer to xmlTextWriterPtr which is updated
 *                           with the shakeAlert core_info
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_xml_coreInfo__write(const struct coreInfo_struct core,
                              void *xml_writer)
{
    const char *fcnm = "GFAST_xml_coreInfo__write\0";
    xmlTextWriterPtr writer;
    char units[128], var[128], cevtime[128];
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;

    rc += xmlTextWriterStartElement(writer, BAD_CAST "core_info\0");
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "id\0",
                                      BAD_CAST core.id);
    // magnitude: <mag units="Mw">float</mag>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "mag\0");
    __xml_units__enum2string(core.mag_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.mag);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // magnitude uncertainty: <mag_uncer units="deg">float</mag_uncer>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "mag_uncer\0");
    __xml_units__enum2string(core.mag_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.mag_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // latitude: <lat units="deg">float</lat>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lat\0");
    __xml_units__enum2string(core.lat_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.lat);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // latitude uncertainty: <lat_uncer units="deg">float</lat_uncer>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lat_uncer\0");
    __xml_units__enum2string(core.lat_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.lat_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // longitude: <lon units="deg">float</lon>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lon\0");
    __xml_units__enum2string(core.lon_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.lon);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // longitude uncertainty: <lon_uncer units="deg">float</lon_uncer>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lon_uncer\0");
    __xml_units__enum2string(core.lon_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.lon_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // depth: <depth units="km">float</depth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "depth\0");
    __xml_units__enum2string(core.depth_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.depth);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // depth uncertainty: <depth_uncer units="km">float</depth_uncer>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "depth_uncer\0");
    __xml_units__enum2string(core.depth_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.depth_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // origin time: <orig_time units="UTC">YYYY-MM-DD:HH:MM:SS.SSSZ"</orig_time>
    rc = xml_epoch2string(core.orig_time, cevtime);
    rc += xmlTextWriterStartElement(writer, BAD_CAST "orig_time\0");
    __xml_units__enum2string(core.orig_time_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    rc += xmlTextWriterWriteString(writer, BAD_CAST cevtime);
    rc += xmlTextWriterEndElement(writer);
    // o.t. uncertainty: <orig_time_uncer units="sec">float</orig_time_uncer>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "orig_time_uncer\0");
    __xml_units__enum2string(core.orig_time_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.orig_time_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // likelihood
    rc += xmlTextWriterStartElement(writer, BAD_CAST "likelihood\0");
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", core.likelihood);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer); 
    // </core_info>
    rc = xmlTextWriterEndElement(writer); // </core_info>
    if (rc < 0){
        log_errorF("%s: Error writing core info\n", fcnm);
    }
    return rc;
}
