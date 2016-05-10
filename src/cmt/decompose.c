#include <stdio.h>
#include <stdlib.h>
#include "gfast.h"
/*!
 * @brief Finds the moment tensor decomposition of the NED moment tensor
 *
 * @param[in] M           moment tensor in NED coordinates packed:
 *                        \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                               m_{xy}, m_{xz}, m_{yz} \} \f$
 *
 * @param[out] DC_pct     double couple percentage
 * @param[out] Mw         moment magnitude (e.g. Kanamori 1977)
 * @param[out] strike1    strike of nodal plane 1 (degrees)
 * @param[out] strike2    strike of nodal plane 2 (degrees)
 * @param[out] dip1       dip of nodal plane 1 (degrees)
 * @param[out] dip2       dip of nodal plane 2 (degrees)
 * @param[out] rake1      rake on nodal plane 1 (degrees)
 * @param[out] rake2      rake on nodal plane 2 (degrees) 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_CMT__decomposeMomentTensor(const double *M,
                                     double *DC_pct,
                                     double *Mw,
                                     double *strike1, double *strike2,
                                     double *dip1, double *dip2,
                                     double *rake1, double *rake2)
{
    const char *fcnm = "GFAST_CMT__decomposeMomentTensor\0";
    struct cmopad_struct cmt;
    double M33[3][3];
    int ierr;
    int verbose = 0;
    // Initialize output
    ierr = 0;
    *Mw = 0.0;
    *DC_pct = 100.0; // Avoid a division by zero if an error is encountered 
    *strike1 = 0.0;
    *strike2 = 0.0;
    *dip1 = 0.0;
    *dip2 = 0.0;
    *rake1 = 0.0;
    *rake2 = 0.0;
    // Pack up the moment tensor
    M33[0][0] = M[0];           //Mxx
    M33[1][1] = M[1];           //Myy
    M33[2][2] = M[2];           //Mzz
    M33[0][1] = M33[1][0] = M[3]; //Mxy
    M33[0][2] = M33[2][0] = M[4]; //Mxz
    M33[1][2] = M33[2][1] = M[5]; //Myz
    // Compute the isotropic, CLVD, and DC decomposition
    ierr = cmopad_standardDecomposition(M33, &cmt);
    if (ierr != 0){
        log_errorF("%s: Error decomposing moment tensor\n", fcnm);
        return -1;
    }
    // Get the corresponding strikes, dips, and rakes
    ierr = cmopad_MT2PrincipalAxisSystem(verbose, &cmt);
    if (ierr != 0){
        log_errorF("%s: Error computing strike, dips, and rakes!\n", fcnm);
        return -1;
    }
    // Double couple percentage
    *DC_pct = cmt.DC_percentage;
    // Moment magnitude
    *Mw = cmt.moment_magnitude;
    // Nodal plane 1
    *strike1 = cmt.fp1[0];
    *dip1    = cmt.fp1[1];
    *rake1   = cmt.fp1[2];
    // Nodal plane 2
    *strike2 = cmt.fp2[0];
    *dip2    = cmt.fp2[1];
    *rake2   = cmt.fp2[2];
    
    return ierr;
}
