#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast.h"
/*!
 * @brief Writes the nodal planes
 *
 * @param[in] strike         strike angle of nodal planes (degrees - positive
 *                           clockwise from north) [0,360]
 * @param[in] dip            dip angle of nodal planes (degrees - positive down
 *                           from horizontal) [0,90]
 * @param[in] rake           angle between strike direction and slip (degrees)
 *                           [-180,180] 
 *
 * @param[inout] xml_writer  handle ot XML writer to which the nodal planes
 *                           will be written
 * 
 * @result 0 indicates success
 *
 */
int GFAST_xml_nodalPlanes__write(const double strike[2],
                                 const double dip[2],
                                 const double rake[2],
                                 void *xml_writer)
{
    const char *fcnm = "GFAST_xml_nodalPlanes__write\0";
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
                                          "%f", strike[0]);
    rc += xmlTextWriterEndElement(writer); // </strike>
    //     <dip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "dip\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", dip[0]);
    rc += xmlTextWriterEndElement(writer); // </dip>
    //     <rake> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "rake\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", rake[0]);
    rc += xmlTextWriterEndElement(writer); // </rake>
    //   </nodalPlane1>
    rc += xmlTextWriterEndElement(writer); // </nodalPlane1>
    //---------------------------nodal plane 2--------------------------------//
    //   <nodalPlane2>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "nodalPlane2\0");
    //     <strike>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "strike\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", strike[1]);
    rc += xmlTextWriterEndElement(writer); // </strike>
    //     <dip>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "dip\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", dip[1]);
    rc += xmlTextWriterEndElement(writer); // </dip>
    //     <rake> 
    rc += xmlTextWriterStartElement(writer, BAD_CAST "rake\0");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "value\0",
                                          "%f", rake[1]);
    rc += xmlTextWriterEndElement(writer); // </rake>
    //   </nodalPlane2>
    rc += xmlTextWriterEndElement(writer); // </nodalPlane1>
    // </nodalPlanes>
    rc += xmlTextWriterEndElement(writer); // </nodalPlanes>
    if (rc < 0)
    {
        log_errorF("%s: Error writing nodal planes\n", fcnm);
        return -1;
    } 
    return 0;
}
