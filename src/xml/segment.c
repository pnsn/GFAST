#include <stdio.h>
#include <stdlib.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast.h"

/*!
 * @brief Writes a segment (fault patch) of the finite fault.  A segment
 *        is comprised of vertices and slip.
 *        
 *
 * @param[in] shape           Segment shape.  This can be:
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
 * @param[in] ss              slip in strike
 * @param[in] ss_units        units for slip in strike direction on segment
 *                            (e.g. METERS)
 * @param[in] ds              slip in dip direction
 * @param[in] ds_units        units for slip in dip direction on segment
 *                            (e.g. METERS)
 * @param[in] ss_uncer        uncertainty on slip in strike direction for this
 *                            segment with units given by ss_unc_units.  if
 *                            negative this value is ignored.
 * @param[in] ss_uncer_units  units for uncertainty for slip in strike
 *                            direction (e.g. METERS)
 * @param[in] ds_uncer        uncertainty on slip in strike direction for this
 *                            segment with units given by ds_unc_units.  if
 *                            negative this value is ignored. 
 * @param[in] ds_uncer_units  units for ucnertainty for slip in
 *                            dip direction (e.g. METERS)
 *
 * @param[inout] xml_writer  xmlTextWriterPtr to which to write segment
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_xml_segment__write(const enum xml_segmentShape_enum shape,
                             const double *lats,
                             const enum alert_units_enum lat_units,
                             const double *lons,
                             const enum alert_units_enum lon_units,
                             const double *depths,
                             const enum alert_units_enum depth_units,
                             const double ss,
                             const enum alert_units_enum ss_units,
                             const double ds,
                             const enum alert_units_enum ds_units,
                             const double ss_uncer,
                             const enum alert_units_enum ss_uncer_units,
                             const double ds_uncer,
                             const enum alert_units_enum ds_uncer_units,
                             void *xml_writer)
{
    const char *fcnm = "GFAST_xml_segment__write\0";
    xmlTextWriterPtr writer;
    int rc;
    //------------------------------------------------------------------------//
    rc = 0;
    writer = (xmlTextWriterPtr) xml_writer;
    if (shape != LINE && shape != TRIANGLE && shape != RECTANGLE)
    {
        log_errorF("%s: Invalid shape %d\n", fcnm, shape);
        return -1;
    } 
    // Begin <segment>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "segment\0");
    if (rc < 0)
    {
        log_errorF("%s: Error starting element\n", fcnm);
        return -1;
    }
    // Write the vertices
    rc = GFAST_xml_vertices__write(shape,
                                   lats, lat_units,
                                   lons, lon_units,
                                   depths, depth_units,
                                   (void *)writer);
    if (rc < 0)
    {
        log_errorF("%s: Error writing vertices %d\n", fcnm, rc);
        return -1;
    }
    // Write the slip
    rc = GFAST_xml_slip__write(ss, ss_units,
                               ds, ds_units,
                               ss_uncer, ss_uncer_units,
                               ds_uncer, ds_uncer_units,
                               (void *)writer);
    if (rc < 0)
    {
        log_errorF("%s: Error writing slip\n", fcnm);
        return -1;
    }
    // </segment>
    rc += xmlTextWriterEndElement(writer); // </segment>
    if (rc < 0)
    {
        log_errorF("%s: Error writing segment\n", fcnm);
        return -1;
    }
    return 0;
}

