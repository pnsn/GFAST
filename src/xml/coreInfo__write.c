#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include "gfast.h"

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
int GFAST_xml_coreInfo__write(struct coreInfo_struct core,
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
