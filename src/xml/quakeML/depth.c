#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#pragma clang diagnostic pop
#include "gfast_xml.h"
#include "iscl/log/log.h"

int xml_quakeML_writeDepth(const double depth,
                           const enum alert_units_enum depth_units,
                           const bool lhaveDepth,
                           const double depthUncer,
                           const enum alert_units_enum depthUncer_units,
                           const bool lhaveDepthUncer,
                           const double confidence,
                           const bool lhaveConfidence,
                           void *xml_writer)
{
    const char *fcnm = "xml_quakeML_writeDepth\0";
    xmlTextWriterPtr writer;
    char units[128];
    int rc; 
    //------------------------------------------------------------------------//
    //
    // nothing to do
    if (!lhaveDepth && !lhaveDepthUncer && !lhaveConfidence){return 0;}
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <depth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "depth\0");
    if (rc < 0)
    {
        log_errorF("%s: Error starting element\n", fcnm);
        return -1;
    }
    if (lhaveDepth)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%f", depth);
        __xml_units__enum2string(depth_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveDepthUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%f", depthUncer);
        __xml_units__enum2string(depthUncer_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveConfidence)
    {
        rc += xmlTextWriterWriteFormatElement(writer,
                                              BAD_CAST "confidenceLevel\0",
                                             "%f", confidence);
    }
    // </depth>
    rc += xmlTextWriterEndElement(writer); // </depth>
    if (rc < 0)
    {   
        log_errorF("%s: Error writing depth\n", fcnm);
        return -1; 
    }
    return 0;
}
