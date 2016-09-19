#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast_xml.h"
#include "iscl/log/log.h"

/*!
 * TODO: need to make a write <mag> function because this is all wrong
 */
int xml_quakeML_writeMagnitude(const double magnitude,
                               const bool lhaveMag,
                               const double magUncer,
                               const bool lhaveMagUncer,
                               const char *type,
                               const bool lhaveType,
                               void *xml_writer)
{
    const char *fcnm = "xml_quakeML_writeMagnitude\0";
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    //  
    // nothing to do
    if (!lhaveMag && !lhaveMagUncer && !lhaveType)
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <latitude>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "magnitude\0");
    if (rc < 0)
    {
        log_errorF("%s: Error starting element\n", fcnm);
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
        log_errorF("%s: Error writing latitude\n", fcnm);
        return -1; 
    }
    return 0;
}