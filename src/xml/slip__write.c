#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include "gfast.h"

/*!
 * @brief Writes the slip on a fault patch in the finite fault 
 *
 * @param[in] ss              slip in strike
 * @param[in] ss_units        units for slip in strike direction on segment
 *                            (e.g. METERS)
 * @param[in] ds              slip in dip direction
 * @param[in] ds_units        units for slip in dip direction on segment
 *                            (e.g. METERS)
 * @param[in] ss_uncer        uncertainty on slip in strike direction for this
 *                            segment with units given by ss_unc_units.  if
 *                            negative this value is ignored.
 * @param[in] ss_uncer_units  units for uncertainty for slip in strike
 *                            direction (e.g. METERS)
 * @param[in] ds_uncer        uncertainty on slip in strike direction for this
 *                            segment with units given by ds_unc_units.  if
 *                            negative this value is ignored. 
 * @param[in] ds_uncer_units  units for ucnertainty for slip in
 *                            dip direction (e.g. METERS)
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int GFAST_xml_slip__write(double ss, enum alert_units_enum ss_units,
                          double ds, enum alert_units_enum ds_units,
                          double ss_uncer, enum alert_units_enum ss_uncer_units,
                          double ds_uncer, enum alert_units_enum ds_uncer_units,
                          void *xml_writer)
{
    const char *fcnm = "__GFAST_xml_slip__write\0";
    xmlTextWriterPtr writer;
    char units[128], var[128];
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // <slip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "slip\0");
    // slip along strike: <ss units="m">float</ss>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "ss\0");
    __xml_units__enum2string(ss_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", ss);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // slip down dip: <ds units="m">float</ds>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "ds\0");
    __xml_units__enum2string(ds_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", ds);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // uncerrtainty in slip along strike <ss_uncer units="m">float</ss_unc>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "ss_uncer\0");
    __xml_units__enum2string(ss_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", ss_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // uncerrtainty in dip down dip <ds_uncer units="m">float</ds_unc>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "ds_uncer\0");
    __xml_units__enum2string(ds_uncer_units, units);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "units\0",
                                      BAD_CAST units);
    memset(var, 0, sizeof(var));
    sprintf(var, "%f", ds_uncer);
    rc += xmlTextWriterWriteString(writer, BAD_CAST var);
    rc += xmlTextWriterEndElement(writer);
    // </slip>
    rc += xmlTextWriterEndElement(writer); // </slip>
    if (rc < 0){
        log_errorF("%s: Error writing slip\n", fcnm);
        return -1;
    }
    return 0;
} 
