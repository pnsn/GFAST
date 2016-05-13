#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"
/*!
 * @brief Unpacks the vertex's latitude, longitude, and depth 
 *
 * @param[in] xml_reader  corresponds to the children of the xmlNodePtr
 *                        vertex node pointer
 * @param[in] VTX_NAN     default value for node
 *
 * @param[out] lat        latitude of vertex (degrees)
 * @param[out] lon        longitude of vertex (degrees)
 * @param[out] depth      depth of vertex (km) 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @bug There are no unit conversions thus the shakeAlert XML is expected to
 *      have units degrees for lat and lon, and km for depth
 *
 */
int GFAST_xml_vertex__read(void *xml_reader, double VTX_NAN,
                           double *lat, double *lon, double *depth)
{
    const char *fcnm = "GFAST_xml_vertex__read\0";
    xmlNodePtr vertex_xml;
    xmlChar *value;
    const int nitems = 3;
    const xmlChar *citems[3] = { BAD_CAST "lat\0",
                                 BAD_CAST "lon\0",
                                 BAD_CAST "depth\0" };
    double values[3];
    const int types[3] = {1, 1, 1};
    int ierr, item, item0, nfound;
    bool lunpack;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    for (item=0; item<nitems; item++){
        values[item] = VTX_NAN;
    }
    vertex_xml = (xmlNodePtr )xml_reader;
    if ((xmlStrcmp(vertex_xml->parent->name, (const xmlChar *)"vertex\0"))){
        log_errorF("%s: Error reader must start at <vertex>\n", fcnm);
        ierr = 1;
        goto ERROR;
    }   
    // Try to get the lat, lon, depth
    item0 =-1;
    nfound = 0;
    while (vertex_xml != NULL){
        // Require we are working on an element node
        if (vertex_xml->type != XML_ELEMENT_NODE){
            vertex_xml = vertex_xml->next;
            continue;
        }
        lunpack = false;
        // Try taking the next item in the citems
        if (item0 < nitems){
            if (!xmlStrcmp(vertex_xml->name, citems[item0+1])){
                lunpack = true;
                item0 = item0 + 1;
            }
        }
        // Otherwise do a linear search of citems
        if (!lunpack){
            for (item=0; item<nitems; item++){
                if (!xmlStrcmp(vertex_xml->name, citems[item])){
                    lunpack = true;
                    item0 = item + 1;
                    break;
                }
            }
        }
        // I didn't find this value in citems - weird - continue
        if (!lunpack){
            log_warnF("%s: Warning couldn't find item %s\n", fcnm,
                      (char *)vertex_xml->name);
            vertex_xml = vertex_xml->next;
            continue;
        }
        // Check on item index to avoid a segfault
        if (item0 < 0 || item0 > nitems - 1){
            log_errorF("%s: Invalid index %s %d\n",
                        fcnm, (char *)vertex_xml->name, item0);
            ierr = 1;
            goto ERROR;
        }
        // FIXME - unpack units and convert

        // Is there a value to unpack
        value = xmlNodeGetContent(vertex_xml);
        if (value != NULL){
            if (types[item0] == 1){
                values[item0] = xmlXPathCastStringToNumber(value);
            }else{
                log_errorF("%s: Invalid type %d!\n", fcnm, types[item0]);
                goto ERROR;
            }
            nfound = nfound + 1;
            free(value);
        }
    } // Loop on vertex
ERROR:;
    // Get what we can
    if (ierr == 0){
        *lat = values[0];
        *lon = values[1];
        *depth = values[2];
        if (nfound < 3){
            for (item=0; item<nitems; item++){
                log_warnF("%s: Couldn't find: %s\n", fcnm, citems[item]);
            }
        }
    }else{
        log_errorF("%s: Internal error\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
