#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "iscl/log/log.h"

/*!
 * @brief Writes the moment tensor terms in Up, South, East coordinates
 *        as described in Aki and Richards - Quantitative Seismology
 *        2nd Ed. pg 113.
 *
 * @param[in] Mrr             Mrr component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 * @param[in] Mtt             Mtt component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 * @param[in] Mpp             Mpp component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 * @param[in] Mrt             Mrt component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 * @param[in] Mrp             Mrp component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 * @param[in] Mtp             Mtp component of moment tensor (USE) coordinates.
 *                            Units are Newton-meters.
 *
 * @param[in,out] xml_writer  handle to XML writer to which to write 
 *                            the (moment) tensor
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int xml_quakeML_writeTensor(const double Mrr, const double Mtt,
                            const double Mpp, const double Mrt,
                            const double Mrp, const double Mtp,
                            void *xml_writer)
{
    const char *fcnm = "xml_quakeML_writeTensor\0";
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    //
    // Cast the writer
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // <tensor>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "tensor\0");
    // <Mrr>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mrr\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mrr);
    rc += xmlTextWriterEndElement(writer); // </Mrr>
    // <Mtt>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mtt\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mtt);
    rc += xmlTextWriterEndElement(writer); // </Mtt>
    // <Mpp>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mpp\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mpp);
    rc += xmlTextWriterEndElement(writer); // </Mpp>
    // <Mrt>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mrt\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mrt);
    rc += xmlTextWriterEndElement(writer); // </Mrt>
    // <Mrp>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mrp\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mrp);
    rc += xmlTextWriterEndElement(writer); // </Mrp>
    // <Mtp>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "Mtp\0"); 
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%e", Mtp);
    rc += xmlTextWriterEndElement(writer); // </Mtp>
    // </tensor>
    rc += xmlTextWriterEndElement(writer); // </tensor>
    if (rc < 0)
    {
        log_errorF("%s: Error writing tensor!\n", fcnm);
        return -1;
    }
    return 0;
} 
