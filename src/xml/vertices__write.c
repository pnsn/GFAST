#include <stdio.h>
#include <stdlib.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast.h"

/*!
 * @brief Writes the vertices for a segment
 *
 * @param[in] shape         Segment shape.  This can be:
 *                            LINE (1) which has two points.
 *                            TRIANGLE (2) which has three points.
 *                            RECTANGLE (3) which has four points.
 * @param[in] lats           the latitudes at each vertex.  the length should
 *                           correspond to the number of points indicated by
 *                           shape.
 * @param[in] lat_units      units for the input latitudes (e.g. DEGREES)
 * @param[in] lons           the longitudes at each vertex.  the length should
 *                           correspond to the number of points indicated by
 *                           shape.
 * @param[in] lon_units      units for the input longitudes (e.g. DEGREES)
 * @param[in] lons           the longitudes at each vertex.  the length should
 *                           correspond to the number of points indicated by
 *                           shape.
 * @param[in] depths         the depths at each vertex.  the length should
 *                           correspond to the number of points indicated by
 *                           shape.
 * @param[in] depth_units    units for the input depths (e.g. KILOMETERS)
 *
 * @param[inout] xml_writer  xmlTextWriterPtr to which to add vertices 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date April 2016 
 *
 */
int GFAST_xml_vertices__write(enum xml_segmentShape_enum shape,
                              double *lats, enum alert_units_enum lat_units,
                              double *lons, enum alert_units_enum lon_units,
                              double *depths, enum alert_units_enum depth_units,
                              void *xml_writer)
{
    const char *fcnm = "GFAST_xml_vertices__write\0";
    xmlTextWriterPtr writer;
    int i, npts, rc; 
    //------------------------------------------------------------------------//
    //
    // Classify segment shape to get the number of points
    rc = 0;
    writer = (xmlTextWriterPtr )xml_writer;
    if (shape == LINE){
        npts = 2;
    }else if (shape == TRIANGLE){
        npts = 3;
    }else if (shape == RECTANGLE){
        npts = 4;
    }else{
        log_errorF("%s: Invalid shape %d\n", fcnm, shape);
        return -1; 
    }
    // Begin <vertices>
    rc = xmlTextWriterStartElement(writer, BAD_CAST "vertices\0");
    // Write each vertex 
    for (i=0; i<npts; i++){
        rc = GFAST_xml_vertex__write(lats[i], lat_units,
                                     lons[i], lon_units,
                                     depths[i], depth_units,
                                     (void *)writer);
        if (rc < 0){ 
            log_errorF("%s: Error writing vertex\n", fcnm);
            return -1;
        }
    }
    // </vertices>
    rc = xmlTextWriterEndElement(writer); // </vertices>
    if (rc < 0){
        log_errorF("%s: Error closing vertices %d\n", fcnm, rc);
        return -1;
    } 
    return 0;
}
