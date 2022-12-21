#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"
/*!
 * @brief Writes the tension, pressure, and null principal axes. 
 *
 * @param[in] taxis           tension principal axis defined by its 
 *                            azimuth angle (dgrees), plunge angle (degrees),
 *                            and length (Nm) respectively [3]
 * @param[in] paxis           pressure principal axis defined by its 
 *                            azimuth angle (dgrees), plunge angle (degrees),
 *                            and length (Nm) respectively [3]
 * @param[in] naxis           null (intermediate) principal axis defined by its
 *                            azimuth angle (dgrees), plunge angle (degrees),
 *                            and length (Nm) respectively [3]
 *
 * @param[in,out] xml_writer  handle of XML writer to which the principal
 *                            axes will be written
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_quakeML_writePrincipalAxes(const double taxis[3],
                                   const double paxis[3],
                                   const double naxis[3],
                                   void *xml_writer)
{
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // <principalAxess>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "principalAxess\0");
    //-----------------------------tension axis-------------------------------//
    //   <tAxis>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "tAxis\0");
    //     <azimuth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "azimuth\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", taxis[0]);
    rc += xmlTextWriterEndElement(writer); // </azimuth>
    //     <plunge>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "plunge\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", taxis[1]);
    rc += xmlTextWriterEndElement(writer); // </plunge>
    //     <length> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "length\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", taxis[2]);
    rc += xmlTextWriterEndElement(writer); // </length>
    //   </tAxis>
    rc += xmlTextWriterEndElement(writer); // </tAxis>
    //-----------------------------pressure axis------------------------------//
    //   <pAxis>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "pAxis\0");
    //     <azimuth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "azimuth\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", paxis[0]);
    rc += xmlTextWriterEndElement(writer); // </azimuth>
    //     <plunge>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "plunge\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", paxis[1]);
    rc += xmlTextWriterEndElement(writer); // </plunge>
    //     <length> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "length\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", paxis[2]);
    rc += xmlTextWriterEndElement(writer); // </length>
    //   </pAxis>
    rc += xmlTextWriterEndElement(writer); // </pAxis>
    //------------------------------null axis---------------------------------//
    //   <nAxis>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "nAxis\0");
    //     <azimuth>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "azimuth\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", naxis[0]);
    rc += xmlTextWriterEndElement(writer); // </azimuth>
    //     <plunge>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "plunge\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", naxis[1]);
    rc += xmlTextWriterEndElement(writer); // </plunge>
    //     <length> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "length\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", naxis[2]);
    rc += xmlTextWriterEndElement(writer); // </length>
    //   </nAxis>
    rc += xmlTextWriterEndElement(writer); // </nAxis>
    // </principalAxes>
    rc += xmlTextWriterEndElement(writer); // </principalAxes>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing principal axes");
        return -1;
    } 
    return 0;
}
//============================================================================//
/*!
 * @brief Reads the nodal planes
 *
 * @bug This is not yet programmed
 *
 */
/*
int xml_quakeML_readPrincipalAxes(const void *xml_reader,
                                  const double PA_NAN,
                                  double taxis[3],
                                  double paxis[3],
                                  double naxis[3])
{
    const char *fcnm = "xml_quakeML_readPrincipalAxes\0";
    LOG_ERRMSG("%s", "Error this isn't programmed yet");
    return -1; 
}
*/
