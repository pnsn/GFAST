#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "gfast.h"
/*!
 * @brief Finds the moment tensor decomposition of the NED moment tensor
 *
 * @param[in] nmt         number of moment tensors to decompose
 * @param[in] M           moment tensors (Nm) in NED for all nmt moment
 *                        tensors.  the i'th moment tensor is begins at 
 *                        index 6*i.  the moment tensor packing is:
 *                        \f$ \{ m_{xx}, m_{yy}, m_{zz},
 *                               m_{xy}, m_{xz}, m_{yz} \} \f$
 *                        [6*nmt] 
 * @param[out] DC_pct     double couple percentage for each moment tensor
 *                        [nmt]
 * @param[out] Mw         moment magnitude (e.g. Kanamori 1977) for each
 *                        moment tensor [nmt]
 * @param[out] strike1    strike of nodal plane 1 (degrees) for each
 *                        moment tensor [nmt]
 * @param[out] strike2    strike of nodal plane 2 (degrees) for each
 *                        moment tensor [nmt]
 * @param[out] dip1       dip of nodal plane 1 (degrees) for each moment
 *                        tensor [nmt]
 * @param[out] dip2       dip of nodal plane 2 (degrees) for each moment
 *                        tensor [nmt]
 * @param[out] rake1      rake on nodal plane 1 (degrees) for each moment
 *                        tensor [nmt]
 * @param[out] rake2      rake on nodal plane 2 (degrees) for each moment
 *                        tensor [nmt]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_CMT__decomposeMomentTensor(int nmt,
                                     const double *__restrict__ M,
                                     double *__restrict__ DC_pct,
                                     double *__restrict__ Mw,
                                     double *__restrict__ strike1,
                                     double *__restrict__ strike2,
                                     double *__restrict__ dip1,
                                     double *__restrict__ dip2,
                                     double *__restrict__ rake1,
                                     double *__restrict__ rake2)
{
    const char *fcnm = "GFAST_CMT__decomposeMomentTensor\0";
    struct cmopad_struct cmt;
    double M33[3][3];
    int i, ierr, ierr1;
    int verbose = 0;
    // Initialize output
    ierr = 0;
    for (i=0; i<nmt; i++)
    {
        Mw[i] = 0.0;
        DC_pct[i] = 100.0; //Avoid a division by zero if an error is encountered
        strike1[i] = 0.0;
        strike2[i] = 0.0;
        dip1[i] = 0.0;
        dip2[i] = 0.0;
        rake1[i] = 0.0;
        rake2[i] = 0.0;
        // Pack up the moment tensor
        M33[0][0] = M[i*6+0];             //Mxx
        M33[1][1] = M[i*6+1];             //Myy
        M33[2][2] = M[i*6+2];             //Mzz
        M33[0][1] = M33[1][0] = M[i*6+3]; //Mxy
        M33[0][2] = M33[2][0] = M[i*6+4]; //Mxz
        M33[1][2] = M33[2][1] = M[i*6+5]; //Myz
        // Compute the isotropic, CLVD, and DC decomposition
        ierr1 = cmopad_standardDecomposition(M33, &cmt);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error decomposing moment tensor\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Get the corresponding strikes, dips, and rakes
        ierr1 = cmopad_MT2PrincipalAxisSystem(verbose, &cmt);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error computing strike, dips, and rakes!\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Double couple percentage
        DC_pct[i] = cmt.DC_percentage;
        // Moment magnitude
        Mw[i] = cmt.moment_magnitude;
        // Nodal plane 1
        strike1[i] = cmt.fp1[0];
        dip1[i]    = cmt.fp1[1];
        rake1[i]   = cmt.fp1[2];
        // Nodal plane 2
        strike2[i] = cmt.fp2[0];
        dip2[i]    = cmt.fp2[1];
        rake2[i]   = cmt.fp2[2];
    }
    if (ierr != 0)
    {
        log_errorF("%s: Errors during moment tensor decomposition\n", fcnm);
    }
    return ierr;
}
