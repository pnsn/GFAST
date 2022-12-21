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
 * @brief Writes the nodal planes defined by their strike, dip, and rake.
 *        The strike is measured clockwise north [0,360].  The dip is 
 *        measured positive down from horizontal [0,90], and rake
 *        represents the angle between the strike and slip direction 
 *        [-180,180].  For further details see Aki and Richards
 *        Quantitative Seismology Ed. 2 pg 101.
 *
 * @param[in] np1             first nodal plane defined by its strike, dip,
 *                            and rake respectively (degrees) [3]
 * @param[in] np2             second nodal plane defined by its strike, dip,
 *                            and rake respectively (degrees) [3]
 *
 * @param[in,out] xml_writer  handle of XML writer to which the nodal planes
 *                            will be written
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_quakeML_writeNodalPlanes(const double np1[3],
                                 const double np2[3],
                                 void *xml_writer)
{
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // <nodalPlanes>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "nodalPlanes\0");
    //---------------------------nodal plane 1--------------------------------//
    //   <nodalPlane1>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "nodalPlane1\0");
    //     <strike>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "strike\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np1[0]);
    rc += xmlTextWriterEndElement(writer); // </strike>
    //     <dip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "dip\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np1[1]);
    rc += xmlTextWriterEndElement(writer); // </dip>
    //     <rake> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "rake\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np1[2]);
    rc += xmlTextWriterEndElement(writer); // </rake>
    //   </nodalPlane1>
    rc += xmlTextWriterEndElement(writer); // </nodalPlane1>
    //---------------------------nodal plane 2--------------------------------//
    //   <nodalPlane2>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "nodalPlane2\0");
    //     <strike>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "strike\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np2[0]);
    rc += xmlTextWriterEndElement(writer); // </strike>
    //     <dip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "dip\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np2[1]);
    rc += xmlTextWriterEndElement(writer); // </dip>
    //     <rake> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "rake\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", np2[2]);
    rc += xmlTextWriterEndElement(writer); // </rake>
    //   </nodalPlane2>
    rc += xmlTextWriterEndElement(writer); // </nodalPlane2>
    // </nodalPlanes>
    rc += xmlTextWriterEndElement(writer); // </nodalPlanes>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing nodal planes");
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
int xml_quakeML_readNodalPlanes(const void *xml_reader,
                                const double NP_NAN,
                                double strike[2],
                                double rake[2],
                                double dip[2])
{
    LOG_ERRMSG("%s", "Error this isn't programmed yet");
    return -1;
}
*/
