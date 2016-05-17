#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast.h"

/*!
 * @brief Writes the moment tensor and derived quantities such as the 
 *        scalar moment, double couple and CLVD percentage, nodal planes,
 *        and principal axes to QuakeML
 *
 * @param[in] publicIDroot   QuakeML public ID root
 *                           (e.g. quakeml:us.anss.org/)
 * @param[in] evid           event ID
 * @param[in] method         method (for this project use gps)
 * @param[in] mt             moment tensor in NED format with units of
 *                           Newton-meters.   the moment tensor is packed
 *                           \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                                  m_{xy}, m_{xz}, m_{yz} \} \f$.
 *
 * @param[inout] xml_writer  handle to XML writer to which moment tensor
 *                           and derived quantities are to be written. 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_xml_focalMechanism__write(char *publicIDroot,
                                    char *evid,
                                    char *method,
                                    const double mt[6],
                                    void *xml_writer)
{
    const char *fcnm = "GFAST_xml_focalMechanism__write\0";
    xmlTextWriterPtr writer;
    char publicID[512];
    struct cmopad_struct src;
    double M_ned[3][3], M_use[6], naxis[3], paxis[3], taxis[3];
    int ierr, lenos, rc;
    //------------------------------------------------------------------------//
    //
    // Initialize
    rc = 0;
    ierr = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // Set the publicID
    memset(publicID, 0, sizeof(publicID));
    if (publicIDroot != NULL){strcat(publicID, publicIDroot);} 
    lenos = strlen(publicID);
    if (lenos > 0)
    {   
        if (publicID[lenos] != '/'){strcat(publicID, "/\0");}
    }   
    strcat(publicID, "focalmechanism/\0");
    if (evid != NULL){
        strcat(publicID, evid);
        strcat(publicID, "/\0");
    }   
    if (method != NULL){strcat(publicID, method);}
    // This is the moment tensor to be decomposed
    M_ned[0][0] = mt[0];               // mxx
    M_ned[1][1] = mt[1];               // myy
    M_ned[2][2] = mt[2];               // mzz
    M_ned[0][1] = M_ned[1][0] = mt[3]; // mxy
    M_ned[0][2] = M_ned[2][0] = mt[4]; // mxz
    M_ned[1][2] = M_ned[2][1] = mt[5]; // myz
    // Do the decomposition
    ierr = cmopad_standardDecomposition(M_ned, &src);
    if (ierr != 0)
    {
        log_errorF("%s: Error computing standard decomposition\n", fcnm);
        return -1;
    }
    // Convert pressure, null, and tension principal axes to az, plunge, length
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.p_axis,    paxis);
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.null_axis, naxis);
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.t_axis,    taxis);
    if (ierr != 0)
    {
        log_errorF("%s: Error converting eigenvector to principal axis!\n",
                   fcnm);
        return -1;
    }
    // From the NED mt get the moment tensor in USE to write to QuakeML
    memcpy(M_use, mt, 6*sizeof(double));
    ierr = cmopad_basis__transformMatrixM6(M_use, NED, USE);
    if (ierr != 0)
    {
        log_errorF("%s: Error converting NED to USE moment tensor\n", fcnm);
        return -1;
    }
    // <focalMechanism>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "momentTensor\0");
    // Write the moment tensor 
    ierr = GFAST_xml_momentTensor__write(publicIDroot,
                                         evid,
                                         method,
                                         M_use,
                                         src.seismic_moment,
                                         src.DC_percentage,
                                         src.CLVD_percentage,
                                         (void *) xml_writer);
    if (ierr != 0)
    {
        log_errorF("%s: Error writing momentTensor!\n", fcnm);
        return -1;
    }
    // Write the nodal planes
    ierr = GFAST_xml_nodalPlanes__write(src.fp1, src.fp2, (void *) writer);
    if (ierr != 0)
    {
        log_errorF("%s: Error writing nodal planes\n", fcnm);
        return -1;
    }
    // Write the principal axes
    ierr = GFAST_xml_principalAxes__write(taxis,
                                          paxis,
                                          naxis,
                                          (void *) writer);
    if (ierr != 0)
    {
        log_errorF("%s: Error writing principal axes\n", fcnm);
        return -1;
    }
    // </focalMechanism>
    rc += xmlTextWriterEndElement(writer); // </focalMechanism>
    if (ierr != 0)
    {
        log_errorF("%s: Error writing focalMechanism!\n", fcnm);
        return -1;
    }
    return 0;
} 
//============================================================================//
/*!
 * @brief Reads a focal mechanism
 *
 * @bug This isn't yet programmed
 *
 */
int GFAST_xml_focalMechanism__read()
{
    const char *fcnm = "GFAST_xml_focalMechanism__read\0";
    log_errorF("%s: Error not yet programmed\n", fcnm);
    return -1;
} 
