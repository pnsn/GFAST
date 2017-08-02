#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"
/*!
 * @brief Unpacks the vertex's latitude, longitude, and depth 
 *
 * @param[in] xml_reader  corresponds to the children of the xmlNodePtr
 *                        vertex node pointer
 * @param[in] VTX_NAN     default value for node
 *
 * @param[out] lat        latitude of vertex (degrees)
 * @param[out] lon        longitude of vertex (degrees)
 * @param[out] depth      depth of vertex (km) 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 * @bug There are no unit conversions thus the shakeAlert XML is expected to
 *      have units degrees for lat and lon, and km for depth
 *
 */
int xml_shakeAlert_readVertex(void *xml_reader, const double VTX_NAN,
                              double *lat, double *lon, double *depth)
{
    xmlNodePtr vertex_xml;
    xmlChar *value;
    enum unpack_types_enum
    {
        UNPACK_DOUBLE = 1
    };
    const int nitems = 3;
    const xmlChar *citems[3] = { BAD_CAST "lat\0",
                                 BAD_CAST "lon\0",
                                 BAD_CAST "depth\0" };
    double values[3];
    const int types[3] = {UNPACK_DOUBLE,
                          UNPACK_DOUBLE,
                          UNPACK_DOUBLE};
    int ierr, item, item0, nfound;
    bool lunpack;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    nfound = 0;
    for (item=0; item<nitems; item++)
    {
        values[item] = VTX_NAN;
    }
    if (xml_reader == NULL)
    {
        LOG_ERRMSG("%s", "Error NULL poitner");
        goto ERROR; 
    }
    vertex_xml = (xmlNodePtr )xml_reader;
    if (xmlStrcmp(vertex_xml->parent->name, BAD_CAST "vertex\0") != 0)
    {
        LOG_ERRMSG("%s", "Error reader must start at <vertex>");
        ierr = 1;
        goto ERROR;
    }   
    // Try to get the lat, lon, depth
    item0 =-1;
    while (vertex_xml != NULL)
    {
        // Require we are working on an element node
        if (vertex_xml->type != XML_ELEMENT_NODE){goto NEXT_VERTEX_XML;}
        lunpack = false;
        // Try taking the next item in the citems
        if (item0 < nitems)
        {
            if (!xmlStrcmp(vertex_xml->name, citems[item0+1]))
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
                if (!xmlStrcmp(vertex_xml->name, citems[item]))
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
            LOG_WARNMSG("Warning couldn't find item %s", vertex_xml->name);
            goto NEXT_VERTEX_XML;
        }
        // Check on item index to avoid a segfault
        if (item0 < 0 || item0 > nitems - 1)
        {
            LOG_ERRMSG("Invalid index %s %d", vertex_xml->name, item0);
            ierr = 1;
            goto ERROR;
        }
        // FIXME - unpack units and convert

        // Is there a value to unpack
        value = xmlNodeGetContent(vertex_xml);
        if (value != NULL)
        {
            if (types[item0] == UNPACK_DOUBLE)
            {
                values[item0] = xmlXPathCastStringToNumber(value);
            }
            else
            {
                LOG_ERRMSG("Invalid type %d!", types[item0]);
                goto ERROR;
            }
            nfound = nfound + 1;
            free(value);
        }
NEXT_VERTEX_XML:;
        vertex_xml = vertex_xml->next;
    } // Loop on vertex
ERROR:;
    // Get what we can
    if (ierr == 0)
    {
        *lat = values[0];
        *lon = values[1];
        *depth = values[2];
        if (nfound < 3)
        {
            for (item=0; item<nitems; item++)
            {
                LOG_WARNMSG("Couldn't find: %s", citems[item]);
            }
        }
    }
    else
    {
        LOG_ERRMSG("%s", "Internal error");
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes a vertex defined by a (latitude, longitude, depth)
 *
 * @param[in] lat             vertex latitude with units given by lat_units 
 * @param[in] lat_units       units corresponding to lat (e.g. DEGREES)
 * @param[in] lon             vertex longitude with units given by lon_units
 * @param[in] lon_units       units corresponding to lon (e.g. DEGREES)
 * @param[in] depth           vertex depth with units given by dep_units
 * @param[in] depth_units     units corresponding to depth (e.g. KILOMETERS)
 *
 * @param[in,out] xml_writer  xmlTextWriterPtr to which to add vertex 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date April 2016
 *
 */
int xml_shakeAlert_writeVertex(const double lat,
                               const enum alert_units_enum lat_units,
                               const double lon,
                               const enum alert_units_enum lon_units,
                               const double depth,
                               const enum alert_units_enum depth_units,
                               void *xml_writer)
{
    xmlTextWriterPtr writer;
    char units[128], var[128];
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // <vertex>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "vertex\0");
    // Latitude: <lat units="deg">float</lat> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lat\0");
    __xml_units__enum2string(lat_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", lat);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // Longitude: <lon units="deg">float</lon>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "lon\0");
    __xml_units__enum2string(lon_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", lon);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // Depth: <depth units="km">float</depth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "depth\0");
    __xml_units__enum2string(depth_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", depth);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // </vertex>
    rc += xmlTextWriterEndElement(writer); // </vertex>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing vertex!");
        return -1;
    }
    return 0;
}
