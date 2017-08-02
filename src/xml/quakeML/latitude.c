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

int xml_quakeML_writeLatitude(const double latitude,
                              const enum alert_units_enum lat_units,
                              const bool lhaveLat,
                              const double latUncer,
                              const enum alert_units_enum latUncer_units,
                              const bool lhaveLatUncer,
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
    if (!lhaveLat && !lhaveLatUncer && !lhaveConfidence){return 0;} 
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <latitude>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "latitude\0");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting element");
        return -1;
    }
    if (lhaveLat)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%f", latitude);
        __xml_units__enum2string(lat_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveLatUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%f", latUncer);
        __xml_units__enum2string(latUncer_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveConfidence)
    {
        rc += xmlTextWriterWriteFormatElement(writer,
                                              BAD_CAST "confidenceLevel\0",
                                             "%f", confidence);
    }
    // </latitude>
    rc += xmlTextWriterEndElement(writer); // </latitude>
    if (rc < 0)
    {   
        LOG_ERRMSG("%s", "Error writing latitude");
        return -1; 
    }
    return 0;
}
