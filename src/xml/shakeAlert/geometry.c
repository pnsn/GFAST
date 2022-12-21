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
#include <libxml/parser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"

//============================================================================//
/*!
 * @brief Writes the slip on a fault patch in the finite fault.
 *
 * @param[in] strike           Strike direction.
 * @param[in] strike_units     Units for strike (e.g. DEGREES).
 * @param[in] dip              Dip direction.
 * @param[in] dip_units        Units for dip (e.g. DEGREES).
 *
 * @param[in,out] xml_writer   Pointer to xmlTExtWriterPtr to which the slip
 *                             is to be written.
 * 
 * @author Carl Ulberg (UW)
 *
 * @date November 2021
 *
 */
int xml_shakeAlert_writeGeometry(const double strike,
                                 const enum alert_units_enum strike_units,
                                 const double dip,
                                 const enum alert_units_enum dip_units,
                                 void *xml_writer)
{
    xmlTextWriterPtr writer;
    char units[128], var[128];
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // <slip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "geometry\0");
    // strike: <strike units="deg">float</strike>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "strike\0");
    __xml_units__enum2string(strike_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, 128*sizeof(char));
    sprintf(var, "%f", strike);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // dip: <dip units="deg">float</dip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "dip\0");
    __xml_units__enum2string(dip_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, 128*sizeof(char));
    sprintf(var, "%f", dip);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    
    rc += xmlTextWriterEndElement(writer); // </geometry>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing geometry");
        return -1;
    }
    return 0;
} 