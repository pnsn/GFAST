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

int xml_quakeML_writeTime(const double time,
                          const enum alert_units_enum time_units,
                          const bool lhaveTime,
                          const double timeUncer,
                          const enum alert_units_enum timeUncer_units,
                          const bool lhaveTimeUncer,
                          const double confidence,
                          const bool lhaveConfidence,
                          void *xml_writer)
{
    xmlTextWriterPtr writer;
    char units[128];
    int rc; 
    //------------------------------------------------------------------------//
    //
    // nothing to do
    if (!lhaveTime && !lhaveTimeUncer && !lhaveConfidence){return 0;}
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <time>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "time\0");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting element");
        return -1;
    }
    if (lhaveTime)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%f", time);
        __xml_units__enum2string(time_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveTimeUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%f", timeUncer);
        __xml_units__enum2string(timeUncer_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveConfidence)
    {
        rc += xmlTextWriterWriteFormatElement(writer,
                                              BAD_CAST "confidenceLevel\0",
                                             "%f", confidence);
    }
    // </time>
    rc += xmlTextWriterEndElement(writer); // </time>
    if (rc < 0)
    {   
        LOG_ERRMSG("%s", "Error writing time");
        return -1; 
    }
    return 0;
}
