#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wpadded"
#endif
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"
#include "iscl/time/time.h"

/*!
 * @brief Reads from the input shakeAlert XML message into the core structure
 *
 * @param[in] xml_reader  Pointer to core_info xmlNodePtr.
 * @param[in] SA_NAN      If an item is not found then this will be its default
 *                        value.
 *
 * @param[out] core       On successful exit contains the event ID, magnitude,
 *                        magnitude uncertainty, latitude, latitude uncertainty,
 *                        longitude, longitude uncertainty, depth, depth
 *                        uncertainty, origin time, origin time uncertainty,
 *                        with accompanying units as well as likelihood.
 * 
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
int xml_shakeAlert_readCoreInfo(void *xml_reader,
                                const double SA_NAN,
                                struct coreInfo_struct *core)
{
    xmlNodePtr core_xml, core_xml_info;
    xmlAttrPtr attr;
    xmlChar *value = NULL;
    enum unpack_types_enum
    {
        UNPACK_INT = 0,
        UNPACK_DOUBLE = 1,
        UNPACK_TIME = 2
    };
    const int nitems = 12; 
    const xmlChar *citems[12] = { BAD_CAST "mag\0", BAD_CAST "mag_uncer\0",
                                  BAD_CAST "lat\0", BAD_CAST "lat_uncer\0",
                                  BAD_CAST "lon\0", BAD_CAST "lon_uncer\0",
                                  BAD_CAST "depth\0", BAD_CAST "depth_uncer\0",
                                  BAD_CAST "orig_time\0",
                                  BAD_CAST "orig_time_uncer\0",
                                  BAD_CAST "likelihood\0",
                                  BAD_CAST "num_stations\0"};
    double values[12];
    int units[12], ivalues[12];
    const int types[12] = {UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_DOUBLE, UNPACK_DOUBLE,
                           UNPACK_TIME, UNPACK_DOUBLE,
                           UNPACK_INT};
    int dom, hour, ierr, item, item0, minute, month, nzmsec, nzmusec,
        nzsec, year;
    bool lfound, lunpack;
    size_t lenos;
    const double tol = 1.e-14;
    //------------------------------------------------------------------------//
    //
    // Initialize result
    ierr = 0;
    lfound = false;
    memset(core, 0, sizeof(struct coreInfo_struct));
    for (item=0; item<nitems; item++)
    {
        values[item] = SA_NAN;
        ivalues[item] = 0;
        units[item] = UNKNOWN_UNITS;
    }
    // Require there be something to read
    if (xml_reader == NULL)
    {
        LOG_ERRMSG("%s", "Error xml reader is NULL");
        return -1;
    }
    core_xml = (xmlNodePtr )xml_reader;
    // Make sure we're in the right spot
    if ((xmlStrcmp(core_xml->name, BAD_CAST "core_info\0") != 0))
    {
        LOG_ERRMSG("Error xml_reader does not start at core_info %s!",
                   core_xml->name);
        return -1;
    }
    // Get the eventID
    attr = xmlHasProp(core_xml, BAD_CAST "id\0");
    if (attr)
    {
        value = xmlNodeListGetString(core_xml->doc, attr->children, 1);
        if (value == NULL)
        {
            LOG_ERRMSG("%s", "Error getting event ID");
            return -1;
        }
        lenos = MIN(128, (size_t) xmlStrlen(value)); //strlen((const char *) value));
        strncpy(core->id, (const char *) value, lenos);
        xmlFree(value);
    }
    else
    {
        LOG_WARNMSG("%s", "Warning couldn't lift the event ID");
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
            for (item=0; item<nitems; item++)
            {
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
            LOG_WARNMSG("Warning couldn't find item %s", core_xml_info->name);
            goto NEXT_CORE_XML_INFO;
        }
        // Check on item index to avoid a segfault
        if (item0 < 0 || item0 > nitems - 1)
        {
            LOG_ERRMSG("Invalid index %s %d", core_xml_info->name, item0);
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
             if (types[item0] == UNPACK_INT)
             {
                 ivalues[item0] = (int) (xmlXPathCastStringToNumber(value) + 0.5);
             }
             else if (types[item0] == UNPACK_DOUBLE)
             {
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
                 LOG_WARNMSG("Unknown type %d\n", types[item0]); 
             }
             free(value);
        }
NEXT_CORE_XML_INFO:;
        core_xml_info = core_xml_info->next;
    } // End loop on core_xml_info
    if (!lfound)
    {
        LOG_ERRMSG("%s", "Error I couldn't find any elements in core_info");
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
    core->mag           = values[0];
    core->magUncer      = values[1];
    core->lat           = values[2];
    core->latUncer      = values[3];
    core->lon           = values[4];
    core->lonUncer      = values[5];
    core->depth         = values[6];
    core->depthUncer    = values[7];
    core->origTime      = values[8];
    core->origTimeUncer = values[9];
    core->likelihood    = values[10];
    core->numStations   = ivalues[11];
    if (fabs(core->mag           - SA_NAN) > tol){core->lhaveMag = true;}
    if (fabs(core->magUncer      - SA_NAN) > tol){core->lhaveMagUncer = true;}
    if (fabs(core->lat           - SA_NAN) > tol){core->lhaveLat = true;}
    if (fabs(core->latUncer      - SA_NAN) > tol){core->lhaveLatUncer = true;}
    if (fabs(core->lon           - SA_NAN) > tol){core->lhaveLon = true;}
    if (fabs(core->lonUncer      - SA_NAN) > tol){core->lhaveLonUncer = true;}
    if (fabs(core->depth         - SA_NAN) > tol){core->lhaveDepth = true;}
    if (fabs(core->depthUncer    - SA_NAN) > tol){core->lhaveDepthUncer = true;}
    if (fabs(core->origTime      - SA_NAN) > tol){core->lhaveOrigTime = true;}
    if (fabs(core->origTimeUncer - SA_NAN) > tol)
    {
        core->lhaveOrigTimeUncer = true;
    }
    if (fabs(core->likelihood    - SA_NAN) > tol){core->lhaveLikelihood = true;}
    // Copy the units 
    core->magUnits           = (enum alert_units_enum) units[0];
    core->magUncerUnits      = (enum alert_units_enum) units[1];
    core->latUnits           = (enum alert_units_enum) units[2];
    core->latUncerUnits      = (enum alert_units_enum) units[3];
    core->lonUnits           = (enum alert_units_enum) units[4];
    core->lonUncerUnits      = (enum alert_units_enum) units[5];
    core->depthUnits         = (enum alert_units_enum) units[6];
    core->depthUncerUnits    = (enum alert_units_enum) units[7];
    core->origTimeUnits      = (enum alert_units_enum) units[8];
    core->origTimeUncerUnits = (enum alert_units_enum) units[9];
    if (core->magUnits            != UNKNOWN_UNITS)
    {
        core->lhaveMagUnits = true;
    }
    if (core->magUncerUnits       != UNKNOWN_UNITS)
    {
        core->lhaveMagUncerUnits = true;
    }
    if (core->latUnits            != UNKNOWN_UNITS)
    {
        core->lhaveLatUnits = true;
    }
    if (core->latUncerUnits       != UNKNOWN_UNITS)
    {
        core->lhaveLatUncerUnits = true;
    }
    if (core->lonUnits            != UNKNOWN_UNITS)
    {
        core->lhaveLonUnits = true;
    }
    if (core->lonUncerUnits       != UNKNOWN_UNITS)
    {
        core->lhaveLonUncerUnits = true;
    }
    if (core->depthUnits          != UNKNOWN_UNITS)
    {
        core->lhaveDepthUnits = true;
    }
    if (core->depthUncerUnits     != UNKNOWN_UNITS)
    {
        core->lhaveDepthUncerUnits = true;
    }
    if (core->origTimeUnits       != UNKNOWN_UNITS)
    {
        core->lhaveOrigTimeUnits = true;
    }
    if (core->origTimeUncerUnits  != UNKNOWN_UNITS)
    {
        core->lhaveOrigTimeUncerUnits = true;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes the shakeAlert core info to the xmlTextWriter xml_writer.
 *
 * @param[in] core            Contains the core information which is the 
 *                            event ID, magnitude, magnitude uncertainty,
 *                            latitude, latitude uncertainty, longitude, 
 *                            longitude uncertainty, depth, depth uncertainty,
 *                            origin time, origin time uncertainty, all with
 *                            accompanying units and the likelihood.
 *
 * @param[in,out] xml_writer  Pointer to xmlTextWriterPtr which is updated
 *                            with the shakeAlert core_info.
 * 
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
int xml_shakeAlert_writeCoreInfo(struct coreInfo_struct core,
                                 void *xml_writer)
{
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
    if (core.lhaveMag)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "mag\0");
        if (core.lhaveMagUncer)
        {
            __xml_units__enum2string(core.magUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.mag);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // magnitude uncertainty: <mag_uncer units="deg">float</mag_uncer>
    if (core.lhaveMagUncer)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "mag_uncer\0");
        if (core.lhaveMagUncerUnits)
        {
            __xml_units__enum2string(core.magUncerUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.magUncer);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // latitude: <lat units="deg">float</lat>
    if (core.lhaveLat)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "lat\0");
        if (core.lhaveLatUnits)
        {
            __xml_units__enum2string(core.latUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.lat);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // latitude uncertainty: <lat_uncer units="deg">float</lat_uncer>
    if (core.lhaveLatUncer)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "lat_uncer\0");
        if (core.lhaveLatUncerUnits)
        {
            __xml_units__enum2string(core.latUncerUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.latUncer);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // longitude: <lon units="deg">float</lon>
    if (core.lhaveLon)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "lon\0");
        if (core.lhaveLonUnits)
        {
            __xml_units__enum2string(core.lonUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.lon);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // longitude uncertainty: <lon_uncer units="deg">float</lon_uncer>
    if (core.lhaveLonUncer)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "lon_uncer\0");
        if (core.lhaveLonUncerUnits)
        {
            __xml_units__enum2string(core.lonUncerUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.lonUncer);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // depth: <depth units="km">float</depth>
    if (core.lhaveDepth)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "depth\0");
        if (core.lhaveDepthUnits)
        {
            __xml_units__enum2string(core.depthUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.depth);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // depth uncertainty: <depth_uncer units="km">float</depth_uncer>
    if (core.lhaveDepthUncer)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "depth_uncer\0");
        if (core.lhaveDepthUncerUnits)
        {
            __xml_units__enum2string(core.depthUncerUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.depthUncer);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // origin time: <orig_time units="UTC">YYYY-MM-DD:HH:MM:SS.SSSZ"</orig_time>
    if (core.lhaveOrigTime)
    {
        rc = xml_epoch2string(core.origTime, cevtime);
        rc += xmlTextWriterStartElement(writer, BAD_CAST "orig_time\0");
        if (core.lhaveOrigTimeUnits)
        {
            __xml_units__enum2string(core.origTimeUnits, units);
            rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                              BAD_CAST units);
        }
        rc += xmlTextWriterWriteString(writer, BAD_CAST cevtime);
        rc += xmlTextWriterEndElement(writer);
    }
    // o.t. uncertainty: <orig_time_uncer units="sec">float</orig_time_uncer>
    if (core.lhaveOrigTimeUncer)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "orig_time_uncer\0");
        if (core.lhaveOrigTimeUncerUnits)
        {
            __xml_units__enum2string(core.origTimeUncerUnits, units);
             rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                               BAD_CAST units);
        }
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.origTimeUncer);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // likelihood
    if (core.lhaveLikelihood)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "likelihood\0");
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%f", core.likelihood);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer); 
    }
    // number of stations
    if (core.numStations > 0)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "num_stations\0") ;
        memset(var, 0, 128*sizeof(char));
        sprintf(var, "%d", core.numStations);
        rc += xmlTextWriterWriteString(writer, BAD_CAST var);
        rc += xmlTextWriterEndElement(writer);
    }
    // </core_info>
    rc = xmlTextWriterEndElement(writer); // </core_info>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing core info");
    }
    return rc;
}
