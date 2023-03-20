#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "compearth.h"
#include "iscl/array/array.h"

#define TEST_COMPEARTH 0
#if (TEST_COMPEARTH == 1)
#include "cmopad/cmopad.h"
#endif

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
int core_cmt_decomposeMomentTensor(const int nmt,
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
#if (TEST_COMPEARTH == 1)
    struct cmopad_struct cmt;
    double M33[3][3];
    int ierr1;
    int verbose = 0;
#endif
    double *M0, *fp1, *fp2, *pAxis, *tAxis, *bAxis, *isoPct, *devPct, *clvdPct;
    int i, ierr;
    //------------------------------------------------------------------------//
    // Initialize output
    ierr = 0;
    M0 = (double *) calloc((size_t) nmt, sizeof(double));
    fp1 = (double *) calloc((size_t) (3*nmt), sizeof(double));
    fp2 = (double *) calloc((size_t) (3*nmt), sizeof(double));
    pAxis = (double *) calloc((size_t) (3*nmt), sizeof(double));
    bAxis = (double *) calloc((size_t) (3*nmt), sizeof(double));
    tAxis = (double *) calloc((size_t) (3*nmt), sizeof(double)); 
    isoPct = (double *) calloc((size_t) nmt, sizeof(double));
    devPct = (double *) calloc((size_t) nmt, sizeof(double));
    clvdPct = (double *) calloc((size_t) nmt, sizeof(double));
    ierr = compearth_standardDecomposition(nmt, M, CE_NED,
                                           M0, Mw, fp1, fp2,
                                           pAxis, bAxis, tAxis,
                                           isoPct, devPct, DC_pct, clvdPct);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s", "Error in compearth moment tensor decomposition");
        array_zeros64f_work(nmt, DC_pct);
        array_zeros64f_work(nmt, Mw);
        array_zeros64f_work(nmt, strike1);
        array_zeros64f_work(nmt, strike2);
        array_zeros64f_work(nmt, dip1);
        array_zeros64f_work(nmt, dip2);
        array_zeros64f_work(nmt, rake1);
        array_zeros64f_work(nmt, rake2);
    }
    else
    {
        for (i=0; i<nmt; i++)
        {
            strike1[i] = fp1[3*i];
            dip1[i]    = fp1[3*i+1];
            rake1[i]   = fp1[3*i+2];
            strike2[i] = fp2[3*i];
            dip2[i]    = fp2[3*i+1];
            rake2[i]   = fp2[3*i+2];
        }
    }
    free(M0);
    free(fp1);
    free(fp2);
    free(pAxis);
    free(bAxis);
    free(tAxis);
    free(isoPct);
    free(devPct);
    free(clvdPct);
#if (TEST_COMPEARTH == 1)
    for (i=0; i<nmt; i++)
    {
/*
        Mw[i] = 0.0;
        DC_pct[i] = 100.0; //Avoid a division by zero if an error is encountered
        strike1[i] = 0.0;
        strike2[i] = 0.0;
        dip1[i] = 0.0;
        dip2[i] = 0.0;
        rake1[i] = 0.0;
        rake2[i] = 0.0;
*/
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
            LOG_ERRMSG("%s", "Error decomposing moment tensor");
            ierr = ierr + 1;
            continue;
        }
        // Get the corresponding strikes, dips, and rakes
        ierr1 = cmopad_MT2PrincipalAxisSystem(verbose, &cmt);
        if (ierr1 != 0)
        {
            LOG_ERRMSG("%s", "Error computing strike, dips, and rakes!");
            ierr = ierr + 1;
            continue;
        }
        if (fabs(cmt.DC_percentage - DC_pct[i]) > 1.e-10)
        {
            LOG_ERRMSG("dc mistmatch %f %f\n", cmt.DC_percentage, DC_pct[i]);
            return -1;
        }
        if (fabs(cmt.moment_magnitude - Mw[i]) > 1.e-10)
        {
            LOG_ERRMSG("Mw mistmatch %f %f\n", cmt.moment_magnitude, Mw[i]);
            return -1;
        }
        if (fabs(cmt.fp1[0] - strike1[i]) > 1.e-10)
        {
            LOG_ERRMSG("str1 mistmatch %f %f\n", cmt.fp1[0], strike1[i]);
            return -1;
        }
        if (fabs(cmt.fp1[1] - dip1[i]) > 1.e-10)
        {
            LOG_ERRMSG("dip1 mistmatch %f %f\n", cmt.fp1[1], dip1[i]);
            return -1;
        }
        if (fabs(cmt.fp1[2] - rake1[i]) > 1.e-10)
        {
            LOG_ERRMSG("rak1 mistmatch %f %f\n", cmt.fp1[2], rake1[i]);
            return -1;
        }
        if (fabs(cmt.fp2[0] - strike2[i]) > 1.e-10)
        {
            LOG_ERRMSG("str2 mistmatch %f %f\n", cmt.fp2[0], strike2[i]);
            return -1;
        }
        if (fabs(cmt.fp2[1] - dip2[i]) > 1.e-10)
        {
            LOG_ERRMSG("dip2 mistmatch %f %f\n", cmt.fp2[1], dip2[i]);
            return -1;
        }
        if (fabs(cmt.fp2[2] - rake2[i]) > 1.e-10)
        {
            LOG_ERRMSG("rak2 mistmatch %f %f\n", cmt.fp2[2], rake2[i]);
            return -1;
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
        LOG_ERRMSG("%s", "Errors during moment tensor decomposition");
    }
#endif
    return ierr;
}
