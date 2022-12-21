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

int xml_quakeML_writeLongitude(const double longitude,
                               const enum alert_units_enum lon_units,
                               const bool lhaveLon,
                               const double lonUncer,
                               const enum alert_units_enum lonUncer_units,
                               const bool lhaveLonUncer,
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
    if (!lhaveLon && !lhaveLonUncer && !lhaveConfidence){return 0;} 
    // get writer
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    // Begin <longitude>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "longitude\0");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting element");
        return -1;
    }
    if (lhaveLon)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                             "%f", longitude);
        __xml_units__enum2string(lon_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveLonUncer)
    {
        rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "uncertainty\0",
                                             "%f", lonUncer);
        __xml_units__enum2string(lonUncer_units, units);
        rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                          BAD_CAST units);
    }
    if (lhaveConfidence)
    {
        rc += xmlTextWriterWriteFormatElement(writer,
                                              BAD_CAST "confidenceLevel\0",
                                             "%f", confidence);
    }
    // </longitude>
    rc += xmlTextWriterEndElement(writer); // </longitude>
    if (rc < 0)
    {   
        LOG_ERRMSG("%s", "Error writing longitude");
        return -1; 
    }
    return 0;
}
