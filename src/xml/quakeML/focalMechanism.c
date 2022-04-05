#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "compearth.h"
#include "gfast_xml.h"
#include "gfast_core.h"
//#include "cmopad.h"

/*!
 * @brief Writes the focal mechanism and derived quantities such as the 
 *        scalar moment, double couple and CLVD percentage, nodal planes,
 *        and principal axes to QuakeML.
 *
 * @param[in] publicIDroot    QuakeML public ID root
 *                            (e.g. quakeml:us.anss.org/).
 * @param[in] evid            Event ID.
 * @param[in] method          Method by which the focal mechanism was 
 *                            computed (for this project use gps).
 * @param[in] mt              Moment tensor in NED format with units of
 *                            Newton-meters.   The moment tensor is packed
 *                            \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                                   m_{xy}, m_{xz}, m_{yz} \} \f$.
 *
 * @param[in,out] xml_writer  On input this is a pointer to the
 *                            xmlTextWriterPtr. 
 *                            On successful output the focal mechanism has been
 *                            appended to the xml_writer as a QuakeML
 *                            focal mechanism.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_quakeML_writeFocalMechanism(const char *publicIDroot,
                                    const char *evid,
                                    const char *method,
                                    const double mt[6],
                                    void *xml_writer)
{
    xmlTextWriterPtr writer;
    char publicID[512];
    //struct cmopad_struct src;
    //double M_ned[3][3];
    double M_use[6], naxis[3], paxis[3], taxis[3], fp1[3], fp2[3];
    double clvdPct, dcPct, devPct, isoPct, M0, Mw;
    int ierr, rc;
    size_t lenos;
    //------------------------------------------------------------------------//
    //
    // Initialize
    rc = 0;
    ierr = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // Set the publicID
    memset(publicID, 0, 512*sizeof(char));
    if (publicIDroot != NULL){strcat(publicID, publicIDroot);} 
    lenos = strlen(publicID);
    if (lenos > 0)
    {   
        if (publicID[lenos - 1] != '/'){strcat(publicID, "/\0");}
    }   
    strcat(publicID, "focalmechanism/\0");
    if (evid != NULL)
    {
        strcat(publicID, evid);
        strcat(publicID, "/\0");
    }   
    if (method != NULL){strcat(publicID, method);}
    // Do the decomposition
    ierr = compearth_standardDecomposition(1, mt, CE_NED,
                                           &M0, &Mw, fp1, fp2,
                                           paxis, naxis, taxis,
                                           &isoPct, &devPct, &dcPct, &clvdPct);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error computing standard decomposition");
        return -1;
    }
/*
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
        LOG_ERRMSG("%s", "Error computing standard decomposition");
        return -1;
    }
    ierr = cmopad_MT2PrincipalAxisSystem(0, &src);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error computing principal axes");
        return -1;
    }
    // Convert pressure, null, and tension principal axes to az, plunge, length
    ierr += cmopad_Eigenvector2PrincipalAxis(NED, src.eig_pnt[0],
                                             src.p_axis,    paxis);
    ierr += cmopad_Eigenvector2PrincipalAxis(NED, src.eig_pnt[1],
                                             src.null_axis, naxis);
    ierr += cmopad_Eigenvector2PrincipalAxis(NED, src.eig_pnt[2],
                                             src.t_axis,    taxis);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error converting eigenvector to principal axis!");
        return -1;
    }
    // Switch basis from NED to USE 
    cmopad_basis_transformVector(paxis, NED, USE);
    cmopad_basis_transformVector(naxis, NED, USE); 
    cmopad_basis_transformVector(taxis, NED, USE);
    // From the NED mt get the moment tensor in USE to write to QuakeML
    memcpy(M_use, mt, 6*sizeof(double));
    ierr = cmopad_basis_transformMatrixM6(M_use, NED, USE);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error converting NED to USE moment tensor");
        return -1;
    }
*/
    compearth_convertMT(1, CE_NED, CE_USE, mt, M_use);
    // <focalMechanism>
    rc += xmlTextWriterStartElement(writer, BAD_CAST "focalMechanism\0");
    // Write the moment tensor 
    ierr = xml_quakeML_writeMomentTensor(publicIDroot,
                                         evid,
                                         method,
                                         M_use,
                                         M0, //src.seismic_moment,
                                         dcPct, //src.DC_percentage,
                                         clvdPct, //src.CLVD_percentage,
                                         (void *) xml_writer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing momentTensor!");
        return -1;
    }
    // Write the nodal planes
    ierr = xml_quakeML_writeNodalPlanes(fp1, fp2,
                                        (void *) writer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing nodal planes");
        return -1;
    }
    // Write the principal axes
    ierr = xml_quakeML_writePrincipalAxes(taxis,
                                          paxis,
                                          naxis,
                                          (void *) writer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing principal axes");
        return -1;
    }
    // </focalMechanism>
    rc += xmlTextWriterEndElement(writer); // </focalMechanism>
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error writing focalMechanism!");
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
/*
int xml_quakeML_readFocalMechanism()
{
    const char *fcnm = "xml_quakeML_readFocalMechanism\0";
    LOG_ERRMSG("%s", "Error not yet programmed");
    return -1;
} 
*/
