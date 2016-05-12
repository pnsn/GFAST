#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast.h"

/*!
 * @brief Writes a vertex defined by a (latitude, longitude, depth)
 *
 * @param[in] lat            vertex latitude with units given by lat_units 
 * @param[in] lat_units      units corresponding to lat (e.g. DEGREES)
 * @param[in] lon            vertex longitude with units given by lon_units
 * @param[in] lon_units      units corresponding to lon (e.g. DEGREES)
 * @param[in] depth          vertex depth with units given by dep_units
 * @param[in] depth_units    units corresponding to depth (e.g. KILOMETERS)
 *
 * @param[inout] xml_writer  xmlTextWriterPtr to which to add vertex 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date April 2016
 *
 */
int GFAST_xml_vertex__write(double lat, enum alert_units_enum lat_units,
                            double lon, enum alert_units_enum lon_units,
                            double depth, enum alert_units_enum depth_units,
                            void *xml_writer)
{
    const char *fcnm = "__GFAST_xml_vertex__write\0";
    xmlTextWriterPtr writer;
    char units[128], var[128];
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // <vertex>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "vertex");
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
    if (rc < 0){
        log_errorF("%s: Error writing vertex!\n", fcnm);
        return -1;
    }
    return 0;
}
