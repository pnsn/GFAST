#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include "gfast.h"

//============================================================================//
/*!
 * @brief Generates the char * XML message corresponding to the finite
 *        fault inversion
 *
 */
char *GFAST_FF__makeXML(const int mode,
                        const char *orig_sys,
                        const char *alg_vers,
                        const char *instance,
                        const char *message_type,
                        const char *version,
                        const char *evid,
                        const double SA_lat,
                        const double SA_lon,
                        const double SA_depth,
                        const double SA_mag,
                        const double SA_time,
                        const int nseg,
                        const int *fault_ptr,
                        const double *lat_vtx,
                        const double *lon_vtx,
                        const double *dep_vtx,
                        const double *ss,
                        const double *ds,
                        const double *ss_unc,
                        const double *ds_unc,
                        int *ierr)
{
    const char *fcnm = "GFAST_FF__makeXML\0";
    struct coreInfo_struct core;
    char *xmlmsg, cnow[128], cmode[64], cseg[64];
    enum xml_segmentShape_enum shape;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    //xmlChar *tmp;
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
        log_errorF("%s: Error creating XML buffer!\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    } 
    // Create a new xmlWriter for uri with no compression
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL)
    {
        log_errorF("%s: Error creating xml writer\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    }
    // Start the document with default xml version
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0)
    {
        log_errorF("%s: Error starting writer\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    }
    //-----------------------------<event_message>----------------------------//
    rc = xmlTextWriterStartElement(writer, BAD_CAST "event_message");
    if (rc < 0) 
    {
        log_errorF("%s: Error writing event_message\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    }
    memset(cmode, 0, sizeof(cmode));
    if (mode == 1)
    {
        strcpy(cmode, "live\0");
    }
    else if (mode == 2)
    {
        strcpy(cmode, "playback\0"); 
    }
    else if (mode == 3)
    {
        strcpy(cmode, "offline\0");
    }
    else
    {
        log_warnF("%s: Defaulting to live mode\n", fcnm);
        strcpy(cmode, "live\0");
    }
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "category\0",
                                     BAD_CAST cmode);
    now = time_currentTime();
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
        log_errorF("%s: Error setting attributes\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    }
    //-------------------------------<core_info>------------------------------//
    memset(&core, 0, sizeof(struct coreInfo_struct));
    strcpy(core.id, evid);
    core.mag = SA_mag;
    core.mag_units = MOMENT_MAGNITUDE;
    core.mag_uncer = 0.5;
    core.mag_uncer_units = MOMENT_MAGNITUDE;
    core.lat = SA_lat; 
    core.lat_units = DEGREES;
    core.lat_uncer = 0.5;
    core.lat_uncer_units = DEGREES;
    core.lon = SA_lon;
    core.lon_units = DEGREES;
    core.lon_uncer = 0.5;
    core.lon_uncer_units = DEGREES;
    core.depth = SA_depth;
    core.depth_units = KILOMETERS;
    core.depth_uncer = 5.0;
    core.depth_uncer_units = KILOMETERS;
    core.orig_time = SA_time; 
    core.orig_time_units = UTC;
    core.orig_time_uncer = 20.0;
    core.orig_time_uncer_units = SECONDS;
    core.likelihood = 0.8;
    rc = GFAST_xml_coreInfo__write(core, (void *)writer);
    if (rc != 0)
    {
        log_errorF("%s: Error writing core info\n", fcnm);
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
            log_errorF("%s: Invalid shape!\n", fcnm);
            continue;
        }
        rc = GFAST_xml_segment__write(shape,
                                      &lat_vtx[indx], DEGREES,
                                      &lon_vtx[indx], DEGREES,
                                      &dep_vtx[indx], KILOMETERS, 
                                      ss[iseg], METERS,
                                      ds[iseg], METERS,
                                      ss_unc[iseg], METERS,
                                      ds_unc[iseg], METERS,
                                      (void *) writer);
        if (rc != 0)
        {
            log_errorF("%s: Error writing segment %d\n", fcnm, iseg);
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
        log_errorF("%s: Error closing EVENT_MESSAGE\n", fcnm);
        return xmlmsg;
    }
    // Finalize the writer
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0)
    {
        log_errorF("%s: Error writing ending the document\n", fcnm);
        *ierr = 1;
        return xmlmsg;
    }
    xmlFreeTextWriter(writer);
    xmlCleanupCharEncodingHandlers();
/*
    // Finally write the buffer to disk
    FILE *fp;
    char ofl[PATH_MAX];
    memset(ofl, 0, sizeof(ofl));
    strcpy(ofl, "test.xml\0");
    fp = fopen(ofl, "w");
    if (fp == NULL){
        log_errorF("%s: Error writing XML file for writing\n", fcnm);
        fclose(fp);
    }
    fprintf(fp, "%s", (const char *) buf->content);
    fclose(fp);
*/
    // Finally copy the char * XML message
    msglen = xmlStrlen(buf->content); //strlen((const char *)buf->content);
    xmlmsg = (char *)calloc(msglen+1, sizeof(char));
    strcpy(xmlmsg, (const char *)buf->content);
    xmlCleanupParser();
    xmlBufferFree(buf);
    return xmlmsg;
}
