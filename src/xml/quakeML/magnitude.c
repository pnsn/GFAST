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
 * TODO: need to make a write mag function because this is all wrong
 */
int xml_quakeML_writeMagnitude(const double magnitude,
                               const bool lhaveMag,
                               const double magUncer,
                               const bool lhaveMagUncer,
                               const char *type,
                               const bool lhaveType,
                               void *xml_writer)
{
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    //  
    // nothing to do
    if (!lhaveMag && !lhaveMagUncer && !lhaveType){return 0;}
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <latitude>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "magnitude\0");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting element");
        return -1;
    }
    if (lhaveMag)
    {
        rc += xmlTextWriterStartElement(writer, BAD_CAST "mag\0");
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%e", magnitude);
        rc += xmlTextWriterEndElement(writer); // </mag>
    }
    if (lhaveMagUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%e", magUncer);
    }
    if (lhaveType)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "type\0",
                                             "%s", type);
    }
    // </magnitude>
    rc += xmlTextWriterEndElement(writer); // </magnitude>
    if (rc < 0)
    {   
        LOG_ERRMSG("%s", "Error writing latitude");
        return -1; 
    }
    return 0;
}
