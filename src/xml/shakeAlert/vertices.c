#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_xml.h"
#include "gfast_core.h"
#include "iscl/log/log.h"

/*!
 * @brief Reads the vertices in a segment
 *
 * @param[in] xml_reader  pointer to xmlNodePtr reader beginning at 
 *                        vertices  
 * @param[in] shape       segment shape
 *                          LINE (2) - segment is a line 
 *                          TRIANGLE (3) - segment is a triangle
 *                          RECTANGLE (4) - segment is a rectangle
 * @param[in] VTX_NAN     default value should a vertex value not be found
 *
 * @param[out] lat        latitude (degrees) at each vertex [shape]
 * @param[out] lon        longitude (degrees) at each vertex [shape] 
 * @param[out] depth      depth (km) at each vertex [shape]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 */
int xml_shakeAlert_readVertices(void *xml_reader,
                                const enum xml_segmentShape_enum shape,
                                const double VTX_NAN,
                                double *__restrict__ lat,
                                double *__restrict__ lon,
                                double *__restrict__ depth)
{
    xmlNodePtr vertices_xml, vertex_xml;
    int ierr, nv, nvref;
    //------------------------------------------------------------------------//
    //
    // Check the input
    ierr = 0; 
    nvref = shape;
    if (nvref < 2 || nvref > 4)
    {
        LOG_ERRMSG("Invalid shape %d\n", shape);
        return -1;
    }
    if (lat == NULL || lon == NULL || depth == NULL)
    {
        if (lat == NULL){LOG_ERRMSG("%s", "Error lat pointer is NULL");}
        if (lon == NULL){LOG_ERRMSG("%s", "Error lon pointer is NULL");}
        if (depth == NULL)
        {
            LOG_ERRMSG("%s", "Error depth pointer is NULL");
        }
        return -1;
    }
    // Initialize result
    for (nv=0; nv<nvref; nv++)
    {
        lat[nv] = VTX_NAN;
        lon[nv] = VTX_NAN;
        depth[nv] = VTX_NAN;
    }
    // Check for null pointer
    if (xml_reader == NULL)
    {   
        LOG_ERRMSG("%s", "Error NULL pointer");
        return -1;
    }
    // Get vertices_xml pointer
    vertices_xml = (xmlNodePtr )xml_reader;
    if (xmlStrcmp(vertices_xml->name, BAD_CAST "vertices\0") != 0)
    {
        LOG_ERRMSG("%s", "Error reader must start at <vertices>");
        return -1;
    }
    // Get the group length
    nv = 0;
    ierr = 0;
    while(vertices_xml != NULL)
    {
        // Only want to unpack vertex's
        if (xmlStrcmp(vertices_xml->name, BAD_CAST "vertex\0") != 0)
        {
            goto NEXT_VERTICES_XML;
        }
        // Update the pointer and read the next vertex
        nv = nv + 1;
        vertex_xml = vertices_xml->xmlChildrenNode;
        ierr = ierr + xml_shakeAlert_readVertex((void *)vertex_xml, VTX_NAN,
                                                &lat[nv], &lon[nv], &depth[nv]);
        if (ierr != 0)
        {
            LOG_ERRMSG("Error unpacking vertex %d\n", nv + 1);
            ierr = ierr + 1;
        }
NEXT_VERTICES_XML:;
        vertices_xml = vertices_xml->next; 
    } // Loop on vertices
    // Note the failures
    if (nv != nvref)
    {
        if (shape == LINE)
        {
            LOG_ERRMSG("%s", "Failed to get all vertices on line");
        }
        if (shape == TRIANGLE)
        {
            LOG_ERRMSG("%s", "Failed to get all vertices on triangle");
        }
        if (shape == RECTANGLE)
        {
            LOG_ERRMSG("%s", "Error failed to get all vertices on rectangle");
        }
        ierr = ierr + 1;
    }
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Errors detect in unpacking vertices");
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Writes the vertices for a segment
 *
 * @param[in] shape          Segment shape.  This can be:
 *                             LINE (2) which has two points.
 *                             TRIANGLE (3) which has three points.
 *                             RECTANGLE (4) which has four points.
 * @param[in] lats            the latitudes at each vertex.  the length should
 *                            correspond to the number of points indicated by
 *                            shape.
 * @param[in] lat_units       units for the input latitudes (e.g. DEGREES)
 * @param[in] lons            the longitudes at each vertex.  the length should
 *                            correspond to the number of points indicated by
 *                            shape.
 * @param[in] lon_units       units for the input longitudes (e.g. DEGREES)
 * @param[in] depths          the depths at each vertex.  the length should
 *                            correspond to the number of points indicated by
 *                            shape.
 * @param[in] depth_units     units for the input depths (e.g. KILOMETERS)
 *
 * @param[in,out] xml_writer  xmlTextWriterPtr to which to add vertices 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date April 2016 
 *
 */
int xml_shakeAlert_writeVertices(const enum xml_segmentShape_enum shape,
                                 const double *lats,
                                 const enum alert_units_enum lat_units,
                                 const double *lons,
                                 const enum alert_units_enum lon_units,
                                 const double *depths,
                                 const enum alert_units_enum depth_units,
                                 void *xml_writer)
{
    xmlTextWriterPtr writer;
    int i, npts, rc; 
    //------------------------------------------------------------------------//
    //
    // Classify segment shape to get the number of points
    rc = 0;
    writer = (xmlTextWriterPtr )xml_writer;
    npts = shape;
    if (shape < 2 || shape > 4)
    {
        LOG_ERRMSG("Invalid shape %d", shape);
        return -1;
    }
    // Begin <vertices>
    rc = xmlTextWriterStartElement(writer, BAD_CAST "vertices\0");
    // Write each vertex 
    for (i=0; i<npts; i++)
    {
        rc = xml_shakeAlert_writeVertex(lats[i], lat_units,
                                        lons[i], lon_units,
                                        depths[i], depth_units,
                                        (void *)writer);
        if (rc < 0)
        {
            LOG_ERRMSG("%s", "Error writing vertex");
            return -1;
        }
    }
    // </vertices>
    rc = xmlTextWriterEndElement(writer); // </vertices>
    if (rc < 0)
    {
        LOG_ERRMSG("Error closing vertices %d", rc);
        return -1;
    } 
    return 0;
}
