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
 * @param[in] mt             moment tensor in NED format with units of
 *                           Newton-meters.   the moment tensor is packed
 *                           \f$ \ {m_{xx}, m_{yy}, m_{zz},
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
int GFAST_xml_momentTensor__write(double *mt, void *xml_writer  )
{
    const char *fcnm = "GFAST_xml_momentTensor__write\0";
    xmlTextWriterPtr writer;
    struct cmopad_struct src;
    double M_ned[3][3], M_use[6], nv[3], pv[3], tv[3],
           Mrr, Mtt, Mpp, Mrt, Mrp, Mtp;
    int ierr, rc;
    const double nm2dcm = 10000000.0; // newton-meters to dyne-centimeters
    //------------------------------------------------------------------------//
    //
    // Initialize
    rc = 0;
    ierr = 0;
    writer = (xmlTextWriterPtr ) xml_writer;
    // This is the moment tensor to be decomposed
    M_ned[0][0] = mt[0];               // mxx
    M_ned[1][1] = mt[1];               // myy
    M_ned[2][2] = mt[2];               // mzz
    M_ned[0][1] = M_ned[1][0] = mt[3]; // mxy
    M_ned[0][2] = M_ned[2][0] = mt[4]; // mxz
    M_ned[1][2] = M_ned[2][1] = mt[5]; // myz
    // Do the decomposition
    ierr = cmopad_standardDecomposition(Min[3][3], &src);
    if (ierr != 0)
    {
        log_errorF("%s: Error computing standard decomposition\n", fcnm);
        return -1;
    }
    // Convert pressure, null, and tension principal axes to az, plunge, len 
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.p_axis, pv);
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.null_axis, nv);
    ierr += cmopad_Eigenvector2PrincipalAxis(USE, src.eig_pnt[0],
                                             src.t_axis, tv);
    if (ierr != 0)
    {
        log_errorF("%s: Error converting eigenvector to principal axis!\n",
                   fcnm);
        return -1;
    }
    // From the NED mt get the USE mt to write to QuakeML
    cblas_dcopy(6, mt, 1, M_use, 1); 
    ierr = cmopad_basis__transformMatrixM6(M_use, NED, USE);
    if (ierr != 0)
    {
        log_errorF("%s: Error converting NED to USE moment tensor\n", fcnm);
        return -1;
    }
    // <momentTensor>

    // Write the tensor
    Mrr = M_use[0];
    Mtt = M_use[1];
    Mpp = M_use[2];
    Mrt = M_use[3];
    Mrp = M_use[4];
    Mtp = M_use[5];
    ierr = GFAST_xml_tensor__write(Mrr, Mtt, Mpp,
                                   Mrt, Mrp, Mtp,
                                   (void *) writer);
    if (ierr != 0)
    {
        log_errorF("%s: Error writing tensor\n", fcnm);
        return -1;
    }
    // Write the nodal planes
    ierr = GFAST_xml_nodalPlanes__write(strike, dip, rake, (void *) writer);
    if (ierr != 0)
    {
        log_errorF("%s: Error writing nodal planes\n", fcnm);
        return -1;
    }

    if (ierr != 0)
    {
        log_errorF("%s: Error writing moment tensor!\n", fcnm);
        return -1;
    }
    return 0;
} 
