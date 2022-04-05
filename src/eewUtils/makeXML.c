#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_eewUtils.h"
#include "gfast_core.h"
#include "gfast_xml.h"
#include "compearth.h"
#include "iscl/time/time.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
//============================================================================//
/*!
 * @brief Generates the char * XML message corresponding to the finite
 *        fault inversion
 *
 * @bug fix cast quality
 */
char *eewUtils_makeXML__ff(const enum opmode_type mode,
                           const char *orig_sys,
                           const char *alg_vers,
                           const char *instance,
                           const char *message_type,
                           const char *version,
                           const struct coreInfo_struct *core,
                           const int nseg,
                           const int *fault_ptr,
                           const double *lat_vtx,
                           const double *lon_vtx,
                           const double *dep_vtx,
                           const double *strike,
                           const double *dip,
                           const double *ss,
                           const double *ds,
                           const double *ss_unc,
                           const double *ds_unc,
                           int *ierr)
{
    char *xmlmsg, cnow[128], cmode[64], cseg[64];
    enum xml_segmentShape_enum shape;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    double now;
    int indx, iseg, nv, msglen, rc;
    //------------------------------------------------------------------------//
    //
    // Create a new XML buffer to which the XML document will be written
    *ierr = 0;
    xmlmsg = NULL;
    buf = xmlBufferCreate();
    if (buf == NULL)
    {
        LOG_ERRMSG("%s", "Error creating XML buffer!");
        *ierr = 1;
        return xmlmsg;
    } 
    // Create a new xmlWriter for uri with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL)
    {
        LOG_ERRMSG("%s", "Error creating xml writer");
        *ierr = 1;
        return xmlmsg;
    }
    // Start the document with default xml version
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting writer");
        *ierr = 1;
        return xmlmsg;
    }
    //-----------------------------<event_message>----------------------------//
    rc = xmlTextWriterStartElement(writer, BAD_CAST "event_message");
    if (rc < 0) 
    {
        LOG_ERRMSG("%s", "Error writing event_message");
        *ierr = 1;
        return xmlmsg;
    }
    memset(cmode, 0, 64*sizeof(char));
    if (mode == REAL_TIME_EEW)
    {
        strcpy(cmode, "live\0");
    }
    else if (mode == PLAYBACK)
    {
        strcpy(cmode, "playback\0"); 
    }
    else if (mode == OFFLINE)
    {
         strcpy(cmode, "offline\0");
    }
    else
    {
        LOG_WARNMSG("Defaulting to live mode %d", mode);
        strcpy(cmode, "live\0");
    }
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "category\0",
                                     BAD_CAST cmode);
    now = time_timeStamp();
    rc = xml_epoch2string(now, cnow);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "timestamp\0",
                                      BAD_CAST cnow);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST ("orig_sys\0"),
                                      BAD_CAST orig_sys);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "alg_vers\0",
                                      BAD_CAST alg_vers);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "instance\0",
                                      BAD_CAST instance);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "message_type\0",
                                      BAD_CAST message_type);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST ("version\0"),
                                      BAD_CAST version);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error setting attributes");
        *ierr = 1;
        return xmlmsg;
    }
    //-------------------------------<core_info>------------------------------//
    rc = GFAST_xml_shakeAlert_writeCoreInfo(*core, (void *)writer);
    if (rc != 0)
    {
        LOG_ERRMSG("%s", "Error writing core info");
        *ierr = 1;
        return xmlmsg;
    }
    //-----------------------------<finite_fault>-----------------------------//
    rc = xmlTextWriterStartElement(writer, BAD_CAST "finite_fault\0");
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "atten_geom\0",
                                     BAD_CAST "false\0");
    memset(cseg, 0, sizeof(cseg));
    sprintf(cseg, "%d", nseg); 
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "segment_number\0",
                                     BAD_CAST cseg);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "segment_shape\0",
                                     BAD_CAST "rectangle");
    rc = xmlTextWriterWriteFormatElement(writer,
                                         BAD_CAST "number_of_segments\0",
                                         "%d", nseg);
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "confidence\0",
                                         "%d", 1); 
    for (iseg=0; iseg<nseg; iseg++)
    {
        indx = fault_ptr[iseg];
        nv = fault_ptr[iseg + 1] - fault_ptr[iseg];
        if (nv == 4)
        {
            shape = RECTANGLE;
        }
        else if (nv == 3)
        {
            shape = TRIANGLE;
        }
        else if (nv == 2)
        {
            shape = LINE;
        }
        else
        {
            LOG_ERRMSG("%s", "Invalid shape!");
            continue;
        }
        rc = GFAST_xml_shakeAlert_writeSegment(shape,
                                               &lat_vtx[indx], DEGREES,
                                               &lon_vtx[indx], DEGREES,
                                               &dep_vtx[indx], KILOMETERS,
                                               strike[iseg], DEGREES,
                                               dip[iseg], DEGREES,
                                               ss[iseg], METERS,
                                               ds[iseg], METERS,
                                               ss_unc[iseg], METERS,
                                               ds_unc[iseg], METERS,
                                               (void *) writer);
        if (rc != 0)
        {
            LOG_ERRMSG("Error writing segment %d", iseg);
            *ierr = 1;
            return xmlmsg; 
        }
    }
    // end loop on segments (elements)
    // </finite_fault>
    rc = xmlTextWriterEndElement(writer); // </finite_fault>
    // </event_message>
    rc = xmlTextWriterEndElement(writer); // </event_message>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error closing EVENT_MESSAGE");
        return xmlmsg;
    }
    // Finalize the writer
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing ending the document");
        *ierr = 1;
        return xmlmsg;
    }
    xmlFreeTextWriter(writer);
    xmlCleanupCharEncodingHandlers();
    // Finally copy the char * XML message
    msglen = xmlStrlen(buf->content); //strlen((const char *)buf->content);
    xmlmsg = (char *)calloc((size_t) (msglen+1), sizeof(char));
    strncpy(xmlmsg, (const char *)buf->content, msglen);
    xmlCleanupParser();
    xmlBufferFree(buf);
    xmlDictCleanup();
    xmlCleanupThreads();
    return xmlmsg;
}
//============================================================================//
/*!
 * @brief Generates a CMT QuakeML message.
 *
 * @param[in] network    ANSS network code.
 * @param[in] domain     ANSS's network's main web page domain
 *                       (e.g. anss.org, tsunami.gov,
 *                        www.ldeo.columbia.edu, etc.)
 * @param[in] evid       event ID.
 * @param[in] evla       Centroid latitude (degrees).
 * @param[in] evlo       Centroid longitude (degrees).
 * @param[in] evdp       Centroid depth (km).
 * @param[in] t0         Origin time (seconds since epoch).
 * @param[in] mt         Moment tensor in NED format with units of
 *                       Newton-meters.   the moment tensor is packed
 *                       \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                              m_{xy}, m_{xz}, m_{yz} \} \f$.
 *
 * @param[out] ierr      0 indicates success.
 *
 * @note https://github.com/usgs/Quakeml/wiki/ANSS-Quakeml-ID-Standards
 *
 * @author Ben Baker (ISTI)
 *
 * @bug <q:quakeml> results in no-prefix error.
 * @bug memory leak --show-leak-kinds=all: xmlNewRMutex
 *
 */
char *eewUtils_makeXML__quakeML(const char *network,
                                const char *domain,
                                const char *evid,
                                const double evla,
                                const double evlo,
                                const double evdp,
                                const double t0,
                                const double mt[6],
                                int *ierr)
{
    char *qml;
    char publicID[512], publicIDroot[512], datasource[512], dataid[512];
    char networkLower[64];
    double mag;
    int i, msglen, rc;
    struct qmlOrigin_struct origin;
    struct qmlMagnitude_struct magnitude;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    int lenos;
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
    memset(networkLower, 0, 64*sizeof(char));
    strcpy(networkLower, network);
    lenos = (int) (strlen(network)); 
    for (i=0; i<lenos; i++)
    {
        putchar(tolower(network[i]));
    }
    // Make the root part of the publicID:
    //   quakeml:<network>.<domain>/<type>/<code>
    memset(publicIDroot, 0, 512*sizeof(char));
    strcpy(publicIDroot, "quakeml:\0");
    strcat(publicIDroot, networkLower);
    strcat(publicIDroot, ".\0");
    strcat(publicIDroot, domain);
    strcat(publicIDroot, "/\0");
    // Make the event
    memset(publicID, 0, 512*sizeof(char));
    strcpy(publicID, publicIDroot);  
    strcat(publicID, "event/\0");
    strcat(publicID, evid);
    // Make the data source
    memset(datasource, 0, 512*sizeof(char));
    strcpy(datasource, networkLower);
    // Make the dataid
    memset(dataid, 0, 512*sizeof(char));
    strcpy(dataid, networkLower);
    strcat(dataid, evid);
    //------------------------------------------------------------------------//
    //                            Make the XML writer                         //
    //------------------------------------------------------------------------//
    // Initialize XML
    buf = xmlBufferCreate();
    if (buf == NULL)
    {
        LOG_ERRMSG("%s", "Error creating XML buffer!");
        *ierr = 1;
        return qml;
    }
    // Create a new xmlWriter for uri with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL)
    {
        LOG_ERRMSG("%s", "Error creating xml writer");
        *ierr = 1;
        return qml;
    }
    // Turn indentation on
    rc = xmlTextWriterSetIndentString(writer, BAD_CAST "  ");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error setting indentation");
    }
    // Start the document with default xml version
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting writer");
        *ierr = 1;
        return qml;
    }
    // Set the indentation
    rc = xmlTextWriterSetIndent(writer, 1);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error activating indentation");
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
    memset(publicID, 0, 512*sizeof(char));
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
    *ierr = GFAST_xml_quakeML_writeFocalMechanism(publicIDroot,
                                                  evid,
                                                  method,
                                                  mt,
                                                  (void *) writer);
    // Make the magnitude
    memset(&magnitude, 0, sizeof(struct qmlMagnitude_struct));
    *ierr = compearth_CMT2mw(1, 1, mt, &mag);
    if (*ierr == 0)
    {
        magnitude.magnitude = mag;
        strcpy(magnitude.type, "Mw_gps\0"); /* TODO need a magnitude type */
        magnitude.lhaveMag = true; 
        magnitude.lhaveType = true;
    }
    else
    {
        LOG_ERRMSG("%s", "Error computing moment magnitude");
    }
    xml_quakeML_writeMagnitude(magnitude.magnitude, magnitude.lhaveMag, 
                               0.5, false, 
                               magnitude.type, magnitude.lhaveType,
                               (void *) writer);
    //*ierr = GFAST_xml_quakeML_writeMagnitude(publicIDroot,
    //                                         evid,
    //                                         method,
    //                                         magnitude,
    //                                         (void *) writer);
    // Make the origin
    memset(&origin, 0, sizeof(struct qmlOrigin_struct));
    origin.originTime.time = t0;
    origin.originTime.time_units = UTC;
    origin.originTime.lhaveTime = true;
    origin.lhaveOriginTime = true;

    origin.latitude.latitude = evla;
    origin.latitude.latitude_units = DEGREES;
    origin.latitude.lhaveLat = true;
    origin.lhaveLatitude = true;

    origin.longitude.longitude = evlo;
    origin.longitude.longitude_units = DEGREES;
    origin.longitude.lhaveLon = true;
    origin.lhaveLongitude = true;

    origin.depth.depth = evdp;
    origin.depth.depth_units = KILOMETERS;
    origin.depth.lhaveDepth = true;
    origin.lhaveDepth = true;

    *ierr = GFAST_xml_quakeML_writeOrigin(publicIDroot,
                                          evid,
                                          method,
                                          origin,
                                          (void *) writer);
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
        LOG_ERRMSG("%s", "Error writing ending the document");
        *ierr = 1;
        return qml;
    }
    xmlFreeTextWriter(writer);
    xmlCleanupCharEncodingHandlers();
    // Finally copy the char * XML message
    msglen = xmlStrlen(buf->content); //strlen((const char *)buf->content);
    qml = (char *)calloc((size_t) msglen+1, sizeof(char));
    strncpy(qml, (const char *)buf->content, msglen);
    xmlCleanupParser();
    xmlBufferFree(buf);
    xmlDictCleanup();
    xmlCleanupThreads();
    return qml;
}
//============================================================================//
/*!
 *
 */
char *eewUtils_makeXML__pgd(const enum opmode_type mode,
                            const char *orig_sys,
                            const char *alg_vers,
                            const char *instance,
                            const char *message_type,
                            const char *version,
                            const struct coreInfo_struct *core,
                            int *ierr)
{
    char *xmlmsg, cnow[128], cmode[64];
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    double now;
    int msglen, rc;
    //------------------------------------------------------------------------//
    //
    // Create a new XML buffer to which the XML document will be written
    *ierr = 0;
    xmlmsg = NULL;
    buf = xmlBufferCreate();
    if (buf == NULL)
    {
        LOG_ERRMSG("%s", "Error creating XML buffer!");
        *ierr = 1;
        return xmlmsg;
    }
    // Create a new xmlWriter for uri with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL)
    {
        LOG_ERRMSG("%s", "Error creating xml writer");
        *ierr = 1;
        return xmlmsg;
    }
    // Start the document with default xml version
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error starting writer");
        *ierr = 1;
        return xmlmsg;
    }
    //-----------------------------<event_message>----------------------------//
    rc = xmlTextWriterStartElement(writer, BAD_CAST "event_message");
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing event_message");
        *ierr = 1;
        return xmlmsg;
    }
    memset(cmode, 0, 64*sizeof(char));
    if (mode == REAL_TIME_EEW)
    {
        strcpy(cmode, "live\0");
    }
    else if (mode == PLAYBACK)
    {
        strcpy(cmode, "playback\0");
    }
    else if (mode == OFFLINE)
    {
        strcpy(cmode, "offline\0");
    }
    else
    {
        LOG_WARNMSG("%s", "Defaulting to live mode");
        strcpy(cmode, "live\0");
    }
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "category\0",
                                     BAD_CAST cmode);
    now = time_timeStamp();
    rc = xml_epoch2string(now, cnow);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "timestamp\0",
                                      BAD_CAST cnow);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "orig_sys\0",
                                      BAD_CAST orig_sys);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "alg_vers\0",
                                      BAD_CAST alg_vers);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "instance\0",
                                      BAD_CAST instance);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "message_type\0",
                                      BAD_CAST message_type);
    rc += xmlTextWriterWriteAttribute(writer, BAD_CAST "version\0",
                                      BAD_CAST version);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error setting attributes");
        *ierr = 1;
        return xmlmsg;
    }
    rc = GFAST_xml_shakeAlert_writeCoreInfo(*core, (void *)writer);
    // </event_message>
    rc = xmlTextWriterEndElement(writer); // </event_message>
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error closing EVENT_MESSAGE");
        return xmlmsg;
    }
    // Finalize the writer
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0)
    {
        LOG_ERRMSG("%s", "Error writing ending the document");
        *ierr = 1;
        return xmlmsg;
    }
    xmlFreeTextWriter(writer);
    xmlCleanupCharEncodingHandlers();
    // Finally copy the char * XML message
    msglen = xmlStrlen(buf->content); //strlen((const char *)buf->content);
    xmlmsg = (char *)calloc((size_t) msglen+1, sizeof(char));
    strncpy(xmlmsg, (const char *)buf->content, msglen);
    xmlCleanupParser();
    xmlBufferFree(buf);
    xmlDictCleanup();
    xmlCleanupThreads();
    return xmlmsg;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
