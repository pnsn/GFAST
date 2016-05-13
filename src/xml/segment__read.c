#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"
/*!
 * @brief Reads a segment is the latitude, longitude, and depth of
 *        all it's vertices as well as it's slip along strike and
 *        down dip and it's uncertainties in slip along strike and
 *        down dip 
 *
 * @param[in] xml_reader   
 * @param[in] VTX_NAN       default value all return variables
 * @param[in] shape         shape of fault patch:
 *                            LINE (2) - element is a line
 *                            TRIANGLE (3) - element is a triangle
 *                            RECTANGLE (4) - element is a rectangle
 *
 * @param[out] lat          latitudes (degrees) at all vertices of the
 *                          fault patch [shape]
 * @param[out] lon          longitudes (degrees) at all vertices of the
 *                          fault patch [shape]
 * @param[out] depth        depths (km) at all vertices of the fault
 *                          patch [shape]
 * @param[out] ss           slip along strike (m)
 * @param[out] ss_uncer     uncertainty in slip along strike (m)
 * @param[out] ds           slip down dip (m)
 * @param[out] ds_uncer     uncertainty in slip down dip (m)
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int GFAST_xml_segment__read(void *xml_reader, double SEG_NAN,
                            enum xml_segmentShape_enum shape,
                            double *lat,
                            double *lon,
                            double *depth,
                            double *ss, double *ss_uncer,
                            double *ds, double *ds_uncer)
{
    xmlNodePtr segment_xml, vertices_xml, slip_xml;
    enum unpack_types_enum
    {
        UNPACK_VERTICES = 1,
        UNPACK_SLIP = 2
    };
    const xmlChar ctypes[2] = { BAD_CHAR "vertices\0", 
                                BAD_CHAR "slip\0"};
    const int types[2] = {UNPACK_VERTICES,
                          UNPACK_SLIP};
    int nv, nvwork;
    bool lunpack;
    //------------------------------------------------------------------------//
    //
    // Basic error checks
    ierr = 0;
    nvwork = shape;
    if (nvwork < 2 || nvwork > 4)
    {
        log_errorF("%s: Error invalid shape %d\n", fcnm, shape);
        return -1;
    }
    if (lat == NULL || lon == NULL || depth == NULL)
    {
        if (lat == NULL){log_errorF("%s: Error lat pointer is NULL\n", fcnm);}
        if (lon == NULL){log_errorF("%s: Error lon pointer is NULL\n", fcnm);}
        if (depth == NULL){
            log_errorF("%s: Error depth pointer is NULL\n", fcnm);
        }   
        return -1;
    }
    // Initialize output
    for (iv=0; iv<nvwork; iv++)
    {
        lat[iv] = SEG_NAN;
        lon[iv] = SEG_NAN;
        depth[iv] = SEG_NAN;
    }
    *ds = SEG_NAN;
    *ds_uncer = SGE_NAN;
    *ss = SEG_NAN;
    *ss_uncer = SEG_NAN;
    // Make sure xml reader isn't null
    if (xml_reader == NULL)
    {
        log_errorF("%s: Error segment xml pointer is NULL!\n", fcnm);
        return -1; 
    }
    segment_xml = (xmlNodePtr )xml_reader;
    // Make sure pointer starts at right node
    if (xmlStrcmp(segment_xml->name, "segment\0") != 0)
    {
        log_errorF("%s: Error pointer must start at <segment>\n", fcnm);
        return -1;
    }
    // Loop on segments and get the vertices and slip
    item0 =-1; 
    while (segment_xml != NULL)
    {
        // Only want xml nodes
        if (segment_xml->type != XML_ELEMENT_NODE){goto NEXT_SEGMENT;}
        lunpack = false;
        // Try taking the next item in citems
        if (item0 < nitems)
        {
            if (xmlStrcmp(segment_xml->name, cyptes[item0+1]) == 0)
            {
                lunpack = true;
                item0 = item0 + 1;
            } 
        }
        // Otherwise do a linear search of citems
        if (!lunpack)
        {
            for (item=0; item<nitems; item++){
                if (xmlStrcmp(segment->name, citems[item]) == 0)
                {
                    lunpack = true;
                    item0 = item + 1;
                    break;
                }
            }
        }
        // I didn't find this value in citems - weird - continue            
        if (!lunpack)
        {
            log_warnF("%s: Warning couldn't find item %s\n",
                      fcnm, segment_xml->name);
            goto NEXT_SEGMENT;
        }
        if (types[item0] == UNPACK_VERTICES)
        {
            vertices_xml = xmlGetChil 
            nv = (int) vertices_xml->xmlChildElementCount;
            if (nv < 2 || nv > 4)
            {
                log_errorF("%s: Error invalid number of vertices %d\n",
                           fcnm, nv);
                ierr = 1;
                goto NEXT_SEGMENT;
            }
            if (nv > nvref)
            {
                log_errorF("%s: There is insufficient space for lat/lon/dep\n",
                           fcnm);
                ierr = 1;
                goto NEXT_SEGMENT;
            }
            // Read the vertices
            ierr1 = GFAST_xml_vertices__read((void *) xml_reader,
                                             shape,
                                             SEG_NAN,
                                             lat,
                                             lon,
                                             depth);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error unpacking vertices\n", fcnm);
                ierr = ierr + 1;
            }
        }
        else if (types[item0] == UNPACK_SLIP)
        {
            ierr1 = GFAST_xml_slip__read((void *) xml_reader, SEG_NAN,
                                         ss, ss_uncer,
                                         ds, ds_uncer);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error unpacking slip\n", fcnm);
                ierr = ierr + 1;
            }
        }
        else
        {
           log_warnF("%s: Unknown type %d\n", types[item0]);
        }
NEXT_SEGMENT:;
        segment_xml = segment_xml->next;
    } // Loop on segment_xml
    if (ierr != 0){
        log_errorF("%s: There was an error reading the segment\n", fcnm);
        return -1;
    }
    return 0;
}
