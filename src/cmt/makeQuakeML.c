#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include "gfast.h" 

/*!
 * @brief Generates a CMT QuakeML message
 *
 * @param[in] network    two chracter ANSS network code
 * @param[in] domain     ANSS's network's main web page domain
 *                       (e.g. anss.org, tsunami.gov,
 *                        www.ldeo.columbia.edu, etc.)
 * @param[in] evid       event ID
 * @param[in] evla       centroid latitude (degrees)
 * @param[in] evlo       centroid longitude (degrees)
 * @param[in] evdp       centroid depth (km)
 * @param[in] t0         origin time (seconds since epoch)
 * @param[in] mt         moment tensor in NED format with units of
 *                       Newton-meters.   the moment tensor is packed
 *                       \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                              m_{xy}, m_{xz}, m_{yz} \} \f$.
 *
 * @param[out] ierr      0 indicates success
 *
 * @note https://github.com/usgs/Quakeml/wiki/ANSS-Quakeml-ID-Standards
 *
 * @bug <q:quakeml> results in no-prefix error
 *
 * @author Ben Baker (ISTI)
 *
 * @bug memory leak --show-leak-kinds=all: xmlNewRMutex
 *
 */
char *GFAST_CMT__makeQuakeML(const char *network,
                             const char *domain,
                             const char *evid,
                             const double evla,
                             const double evlo,
                             const double evdp,
                             const double t0,
                             const double mt[6],
                             int *ierr)
{
    const char *fcnm = "GFAST_CMT__makeQuakeML\0";
    char *qml;
    char publicID[512], publicIDroot[512], datasource[512], dataid[512];
    char networkLower[64];
    int i, msglen, rc;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    const char *method = "gps\0";
    const char *xmlns = "http://quakeml.org/xmlns/bed/1.2\0";
    const char *xmlns_cat = "http://anss.org/xmlns/catalog/0.1\0";
    //------------------------------------------------------------------------//
    // Initialize
    *ierr = 0;
    qml = NULL;
    //------------------------------------------------------------------------//
    //                  Set some stuff to facilitate QML generation           //
    //------------------------------------------------------------------------//
    // Set the network code (all lower case)
    memset(networkLower, 0, sizeof(networkLower));
    for (i=0; i<strlen(network); i++)
    {
        networkLower[i] = tolower(network[i]);
    }
    // Make the root part of the publicID:
    //   quakeml:<network>.<domain>/<type>/<code>
    memset(publicIDroot, 0, sizeof(publicIDroot));
    strcpy(publicIDroot, "quakeml:\0");
    strcat(publicIDroot, networkLower);
    strcat(publicIDroot, ".\0");
    strcat(publicIDroot, domain);
    strcat(publicIDroot, "/\0");
    // Make the event
    memset(publicID, 0, sizeof(publicID));
    strcpy(publicID, publicIDroot);  
    strcat(publicID, "event/\0");
    strcat(publicID, evid);
//printf("%s\n", publicID);
    // Make the data source
    memset(datasource, 0, sizeof(datasource));
    strcpy(datasource, networkLower);
    // Make the dataid
    memset(dataid, 0, sizeof(dataid));
    strcpy(dataid, networkLower);
    strcat(dataid, evid);
    //------------------------------------------------------------------------//
    //                            Make the XML writer                         //
    //------------------------------------------------------------------------//
    // Initialize XML
    buf = xmlBufferCreate();
    if (buf == NULL)
    {
        log_errorF("%s: Error creating XML buffer!\n", fcnm);
        *ierr = 1;
        return qml;
    }
    // Create a new xmlWriter for uri with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL)
    {
        log_errorF("%s: Error creating xml writer\n", fcnm);
        *ierr = 1;
        return qml;
    }
    // Start the document with default xml version
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0)
    {
        log_errorF("%s: Error starting writer\n", fcnm);
        *ierr = 1;
        return qml;
    }
    //------------------------------------------------------------------------//
    //                           Write the QuakeML                            //
    //------------------------------------------------------------------------//
    // <q:quakeml>
    rc = xmlTextWriterStartElement(writer, BAD_CAST "quakeml\0"); /* TODO: q:quakeml results in no prefix error */
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns\0",
                                     BAD_CAST xmlns);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns:catalog\0",
                                     BAD_CAST xmlns_cat);
    // <eventParameters>
    rc = xmlTextWriterStartElement(writer, BAD_CAST "eventParameters\0");
    memset(publicID, 0, sizeof(publicID));
    strcpy(publicID, publicIDroot);
    strcat(publicID, "eventparameters/");
    strcat(publicID, evid);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "publicID\0",
                                     BAD_CAST publicID);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "catalog:dataid\0",
                                     BAD_CAST dataid);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "catalog:eventsource\0",
                                     BAD_CAST networkLower); 
    // Make the event
    rc = xmlTextWriterStartElement(writer, BAD_CAST "event\0");
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "publicID\0",
                                     BAD_CAST publicID);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "catalog:datasource\0",
                                     BAD_CAST networkLower);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "catalog:dataid\0",
                                     BAD_CAST dataid);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "catalog:eventsource\0",
                                     BAD_CAST networkLower);
    // Make the focal mechanism
    *ierr = GFAST_xml_focalMechanism__write(publicIDroot,
                                            evid,
                                            method,
                                            mt,
                                            (void *) writer);
/*
    *ierr = GFAST_xml_focalMechanism__write(publicIDroot,
                                            evid,
                                            method,
                                            mt,
                                            (void *)writer);
*/
    // Make the magnitude

    // Make the origin

    // </event>
    rc = xmlTextWriterEndElement(writer);
    // </eventParameters>
    rc = xmlTextWriterEndElement(writer);
    // </q:quakeml>
    rc = xmlTextWriterEndElement(writer);
    //------------------------------------------------------------------------//
    //              Finalize the XML writer and copy the result               //
    //------------------------------------------------------------------------//
    // Finalize the writer
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0)
    {
        log_errorF("%s: Error writing ending the document\n", fcnm);
        *ierr = 1;
        return qml;
    }
    xmlFreeTextWriter(writer);
    xmlCleanupCharEncodingHandlers();
    // Finally copy the char * XML message
    msglen = xmlStrlen(buf->content); //strlen((const char *)buf->content);
    qml = (char *)calloc(msglen+1, sizeof(char));
    strncpy(qml, (const char *)buf->content, msglen);
    xmlCleanupParser();
    xmlBufferFree(buf);
    xmlDictCleanup();
    xmlCleanupThreads();
    return qml;
}

/*
int main()
{
    GFAST_CMT__cmt2xml("uw\0", "anss.org\0", "abc123\0");
    return 0;
}
*/
