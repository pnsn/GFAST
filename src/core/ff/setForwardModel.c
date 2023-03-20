#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "gfast_core.h"
// Small number to test cos(90) = 0 
#define eps 6.1232e-14 /*!< A close number for okadaGreenF */
// Poisson's ratio
#define nu 0.25 /*!< Poisson's ratio for okadaGreenF */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds(const double cos_dip, const double sin_dip,
                           const double xi, const double eta, const double q,
                           double *ux_ss, double *uy_ss, double *uz_ss,
                           double *ux_ds, double *uy_ds, double *uz_ds);
#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds_zeroDip(const double sin_dip,
                                   const double xi, const double eta,
                                   const double q,
                                   double *ux_ss, double *uy_ss, double *uz_ss,
                                   double *ux_ds, double *uy_ds, double *uz_ds);
#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds_withDip(const double cos_dip, const double sin_dip,
                                   const double xi, const double eta,
                                   const double q,
                                   double *ux_ss, double *uy_ss, double *uz_ss,
                                   double *ux_ds, double *uy_ds, double *uz_ds);

/*!
 * @brief This program computes the Green's functions from Okada's formulation
 *        for slip on fault patches at a set of station locations
 *
 * @param[in] l1      number of station locations
 * @param[in] l2      number of fault locations
 * @param[in] e       station/fault offset (m) fault eastings [l1 x l2 - row
 *                    major order]
 * @param[in] n       station/fault offset (m) fault northings [l1 x l2 - row
                      major order]
 * @param[in] depth   station/fault depth offsets (m) [l1 x l2 - row major
                      order]
 * @param[in] strike  fault strikes (degrees) [l2]
 * @param[in] dip     fault dips (degrees) [l2]
 * @param[in] W       fault widths (m) [l2]
 * @param[in] L       fault lengths (m) [l2]
 *
 * @param[out] G      [3*l1 x 2*l2] stored in column major format with
 *                    leading dimension 2*l2
 *
 * @author Brendan Crowell, PNSN
 *         Ben Baker, ISTI -> translated to C
 * @date February 2016
 *
 */
int core_ff_setForwardModel__okadagreenF(const int l1, const int l2,
                                         const double *__restrict__ e,
                                         const double *__restrict__ n,
                                         const double *__restrict__ depth,
                                         const double *__restrict__ strike,
                                         const double *__restrict__ dip,
                                         const double *__restrict__ W,
                                         const double *__restrict__ L,
                                         double *__restrict__ G)
{
    double cos_dip, cos_strike, d, dip1, ec, 
           g1, g1n, g2, g2n, g3, g3n, g4, g4n, g5, g6,
           nc, p, q,
           ux_ss_1, uy_ss_1, uz_ss_1,  ux_ds_1, uy_ds_1, uz_ds_1,
           ux_ss_2, uy_ss_2, uz_ss_2,  ux_ds_2, uy_ds_2, uz_ds_2,
           ux_ss_3, uy_ss_3, uz_ss_3,  ux_ds_3, uy_ds_3, uz_ds_3,
           ux_ss_4, uy_ss_4, uz_ss_4,  ux_ds_4, uy_ds_4, uz_ds_4,
           strike1, sin_dip, sin_strike, x, y;
    int i, j, ij, indx;
    bool ldip;
    const double pi180 = M_PI/180.0;
    const double one_twopi = 1.0/(2.0*M_PI);
    const bool lverif = false;
    //------------------------------------------------------------------------//
    if (l1 < 1 || l2 < 1)
    {
        if (l1 < 1){LOG_ERRMSG("%s", "Error no observations");}
        if (l2 < 1){LOG_ERRMSG("%s", "Error no fault patches");}
        return -1;
    }
    // classify the job
    ldip = false;
    for (i=0; i<l2; i++)
    {
        if (cos(dip[i]*pi180) > eps){ldip = true;}
    }
    // This is the non-vertical fault case
    indx = 0;
    if (ldip) 
    {
        // Loop on the observations
        for (j=0; j<l1; j++)
        {
            // Loop on faults
            for (i=0; i<l2; i++)
            {
                ij = l1*i + j;

                strike1 = strike[i]*pi180;
                dip1 = dip[i]*pi180;
                cos_strike = cos(strike1);
                sin_strike = sin(strike1);
                cos_dip = cos(dip1);
                sin_dip = sin(dip1);

                d = depth[ij] + sin_dip*W[i]*0.5;
                ec = e[ij] + cos_strike*cos_dip*W[i]*0.5;
                nc = n[ij] - sin_strike*cos_dip*W[i]*0.5;
                x = cos_strike*nc + sin_strike*ec + L[i]*0.5;
                y = sin_strike*nc - cos_strike*ec + cos_dip*W[i];

                p = y*cos_dip + d*sin_dip;
                q = y*sin_dip - d*cos_dip;

               __ss_ds_withDip(cos_dip, sin_dip,
                               x, p, q,
                               &ux_ss_1, &uy_ss_1, &uz_ss_1,
                               &ux_ds_1, &uy_ds_1, &uz_ds_1);
               __ss_ds_withDip(cos_dip, sin_dip,
                               x, p-W[i], q,
                               &ux_ss_2, &uy_ss_2, &uz_ss_2,
                               &ux_ds_2, &uy_ds_2, &uz_ds_2);
               __ss_ds_withDip(cos_dip, sin_dip,
                               x-L[i], p, q,
                               &ux_ss_3, &uy_ss_3, &uz_ss_3,
                               &ux_ds_3, &uy_ds_3, &uz_ds_3);
               __ss_ds_withDip(cos_dip, sin_dip,
                               x-L[i], p-W[i], q,
                               &ux_ss_4, &uy_ss_4, &uz_ss_4,
                               &ux_ds_4, &uy_ds_4, &uz_ds_4);

                g1 =-one_twopi*( ux_ss_1 - ux_ss_2 - ux_ss_3 + ux_ss_4);
                g2 =-one_twopi*( ux_ds_1 - ux_ds_2 - ux_ds_3 + ux_ds_4);
                g3 =-one_twopi*( uy_ss_1 - uy_ss_2 - uy_ss_3 + uy_ss_4);
                g4 =-one_twopi*( uy_ds_1 - uy_ds_2 - uy_ds_3 + uy_ds_4);
                g5 =-one_twopi*( uz_ss_1 - uz_ss_2 - uz_ss_3 + uz_ss_4);
                g6 =-one_twopi*( uz_ds_1 - uz_ds_2 - uz_ds_3 + uz_ds_4);

                g1n = sin_strike*g1 - cos_strike*g3;
                g3n = cos_strike*g1 + sin_strike*g3;

                g2n = sin_strike*g2 - cos_strike*g4;
                g4n = cos_strike*g2 + sin_strike*g4;

                indx = 3*2*j*l2 + 2*i;
                G[indx+0] = g1n;
                G[indx+1] = g2n;

                //indx = 3*(j+1)*l2 + 2*i;
                G[indx+2*l2+0] = g3n;
                G[indx+2*l2+1] = g4n;

                //indx = 3*(j+2)*l2 + 2*i;
                G[indx+4*l2+0] = g5;
                G[indx+4*l2+1] = g6;
            } // Loop on faults (i)
        } // Loop on observations (j)
    }
    // This is the vertical fault case
    else
    {
        // Loop on the observations
        for (j=0; j<l1; j++)
        {
            // Loop on faults
            for (i=0; i<l2; i++)
            {
                ij = l1*i + j;

                strike1 = strike[i]*pi180;
                dip1 = dip[i]*pi180;
                cos_strike = cos(strike1);
                sin_strike = sin(strike1);
                cos_dip = 0.0; //cos(dip1);
                sin_dip = sin(dip1);

                d = depth[ij] + sin_dip*W[i]*0.5;
                ec = e[ij] + cos_strike*cos_dip*W[i]*0.5;
                nc = n[ij] - sin_strike*cos_dip*W[i]*0.5;
                x = cos_strike*nc + sin_strike*ec + L[i]*0.5;
                y = sin_strike*nc - cos_strike*ec + cos_dip*W[i];

                p = y*cos_dip + d*sin_dip;
                q = y*sin_dip - d*cos_dip;

               __ss_ds_zeroDip(sin_dip,
                               x, p, q,
                               &ux_ss_1, &uy_ss_1, &uz_ss_1,
                               &ux_ds_1, &uy_ds_1, &uz_ds_1);
               __ss_ds_zeroDip(sin_dip,
                               x, p-W[i], q,
                               &ux_ss_2, &uy_ss_2, &uz_ss_2,
                               &ux_ds_2, &uy_ds_2, &uz_ds_2);
               __ss_ds_zeroDip(sin_dip,
                               x-L[i], p, q,
                               &ux_ss_3, &uy_ss_3, &uz_ss_3,
                               &ux_ds_3, &uy_ds_3, &uz_ds_3);
               __ss_ds_zeroDip(sin_dip,
                               x-L[i], p-W[i], q,
                               &ux_ss_4, &uy_ss_4, &uz_ss_4,
                               &ux_ds_4, &uy_ds_4, &uz_ds_4);


                g1 =-one_twopi*( ux_ss_1 - ux_ss_2 - ux_ss_3 + ux_ss_4);
                g2 =-one_twopi*( ux_ds_1 - ux_ds_2 - ux_ds_3 + ux_ds_4);
                g3 =-one_twopi*( uy_ss_1 - uy_ss_2 - uy_ss_3 + uy_ss_4);
                g4 =-one_twopi*( uy_ds_1 - uy_ds_2 - uy_ds_3 + uy_ds_4);
                g5 =-one_twopi*( uz_ss_1 - uz_ss_2 - uz_ss_3 + uz_ss_4);
                g6 =-one_twopi*( uz_ds_1 - uz_ds_2 - uz_ds_3 + uz_ds_4);

                g1n = sin_strike*g1 - cos_strike*g3;
                g3n = cos_strike*g1 + sin_strike*g3;

                g2n = sin_strike*g2 - cos_strike*g4;
                g4n = cos_strike*g2 + sin_strike*g4;

                indx = 3*2*j*l2 + 2*i;
                G[indx+0] = g1n;
                G[indx+1] = g2n;

                //indx = 3*(j+1)*l2 + 2*i;
                G[indx+2*l2+0] = g3n;
                G[indx+2*l2+1] = g4n;

                //indx = 3*(j+2)*l2 + 2*i;
                G[indx+4*l2+0] = g5;
                G[indx+4*l2+1] = g6;
            } // Loop on faults (i)
        } // Loop on observations (j)
    }
    // Verification loop - should only exist for debugging purposes
    if (lverif)
    {
        indx = 0;
        // Loop on the observations
        for (j=0; j<l1; j++)
        {
            // Loop on faults
            for (i=0; i<l2; i++)
            {
                ij = l1*i + j;

                strike1 = strike[i]*pi180;
                dip1 = dip[i]*pi180;
                cos_strike = cos(strike1);
                sin_strike = sin(strike1);
                cos_dip = cos(dip1);
                sin_dip = sin(dip1);

                d = depth[ij] + sin_dip*W[i]*0.5;
                ec = e[ij] + cos_strike*cos_dip*W[i]*0.5;
                nc = n[ij] - sin_strike*cos_dip*W[i]*0.5;
                x = cos_strike*nc + sin_strike*ec + L[i]*0.5;
                y = sin_strike*nc - cos_strike*ec + cos_dip*W[i];

                p = y*cos_dip + d*sin_dip;
                q = y*sin_dip - d*cos_dip;

                __ss_ds(cos_dip, sin_dip,
                        x, p, q,
                        &ux_ss_1, &uy_ss_1, &uz_ss_1,
                        &ux_ds_1, &uy_ds_1, &uz_ds_1);
                __ss_ds(cos_dip, sin_dip,
                        x, p-W[i], q,
                        &ux_ss_2, &uy_ss_2, &uz_ss_2,
                        &ux_ds_2, &uy_ds_2, &uz_ds_2);
                __ss_ds(cos_dip, sin_dip,
                        x-L[i], p, q,
                        &ux_ss_3, &uy_ss_3, &uz_ss_3,
                        &ux_ds_3, &uy_ds_3, &uz_ds_3);
                __ss_ds(cos_dip, sin_dip,
                        x-L[i], p-W[i], q,
                        &ux_ss_4, &uy_ss_4, &uz_ss_4,
                        &ux_ds_4, &uy_ds_4, &uz_ds_4);

                g1 =-one_twopi*( ux_ss_1 - ux_ss_2 - ux_ss_3 + ux_ss_4);
                g2 =-one_twopi*( ux_ds_1 - ux_ds_2 - ux_ds_3 + ux_ds_4);
                g3 =-one_twopi*( uy_ss_1 - uy_ss_2 - uy_ss_3 + uy_ss_4);
                g4 =-one_twopi*( uy_ds_1 - uy_ds_2 - uy_ds_3 + uy_ds_4);
                g5 =-one_twopi*( uz_ss_1 - uz_ss_2 - uz_ss_3 + uz_ss_4);
                g6 =-one_twopi*( uz_ds_1 - uz_ds_2 - uz_ds_3 + uz_ds_4);

                g1n = sin_strike*g1 - cos_strike*g3;
                g3n = cos_strike*g1 + sin_strike*g3;

                g2n = sin_strike*g2 - cos_strike*g4;
                g4n = cos_strike*g2 + sin_strike*g4;

                indx = 3*2*j*l2 + 2*i;
                if (fabs(G[indx+0] - g1n) > eps ||
                    fabs(G[indx+1] - g2n) > eps)
                {
                    LOG_WARNMSG("%s", "g1n g2n failed");
                    G[indx+0] = g1n;
                    G[indx+1] = g2n;
                }

                //indx = 3*(j+1)*l2 + 2*i;
                if (fabs(G[indx+2*l2+0]) - g3n > eps ||
                    fabs(G[indx+2*l2+1]) - g4n > eps)
                {
                    LOG_WARNMSG("%s", "g3n g4n failed");
                    G[indx+2*l2+0] = g3n;
                    G[indx+2*l2+1] = g4n;
                }

                //indx = 3*(j+2)*l2 + 2*i;
                if (fabs(G[indx+4*l2+0]) - g5 > eps ||
                    fabs(G[indx+4*l2+1]) - g6 > eps)
                {
                    LOG_WARNMSG("%s", "g5 g6 failed");
                    G[indx+4*l2+0] = g5;
                    G[indx+4*l2+1] = g6;
                }
            } // Loop on faults (i)
        } // Loop on observations (j) 
    } // End check on verification
    return 0;
}

#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds(const double cos_dip, const double sin_dip,
                           const double xi, const double eta, const double q,
                           double *ux_ss, double *uy_ss, double *uz_ss,
                           double *ux_ds, double *uy_ds, double *uz_ds)
{
    double atan_xeqr, db, I1, I2, I3, I4, I5, log_rpeta, pow_rpdb2, R, X, yb;
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip + q*sin_dip;
    db = eta*sin_dip - q*cos_dip;
    X = sqrt(xi*xi + q*q);
    log_rpeta = log(R + eta);
    pow_rpdb2 = pow(R + db, 2);
    atan_xeqr = atan(xi*eta/(q*R));
    if (cos_dip > eps)
    {
        I5 = (1.0 - 2.0*nu)*2.0/cos_dip
            *atan( ( eta*(X + q*cos_dip) + X*(R + X)*sin_dip )
                  /(xi*(R + X)*cos_dip));
        I4 = (1.0 - 2.0*nu)/cos_dip*(log(R+db) - sin_dip*log_rpeta);
        I3 = (1.0 - 2.0*nu)*(yb/cos_dip/(R+db) - log_rpeta) 
           + sin_dip/cos_dip*I4;
        I2 = (1.0 - 2.0*nu)*(-log_rpeta) - I3;
        I1 = (1.0 - 2.0*nu)*(-xi/cos_dip/(R+db)) 
           - sin_dip/cos_dip*I5;
    }
    else
    {
        I5 =-(1.0 - 2.0*nu)*xi*sin_dip/(R + db);
        I4 =-(1.0 - 2.0*nu)*q/(R + db);
        I3 = (1.0 - 2.0*nu)/2.0*( eta/(R+db) + yb*q/pow_rpdb2 - log_rpeta );
        I2 = (1.0 - 2.0*nu)*(-log_rpeta) - I3; 
        I1 =-(1.0 - 2.0*nu)/2.0*xi*q/pow_rpdb2;
    }
    *ux_ss = xi*q/(R*(R+eta)) 
           + atan_xeqr
           + I1*sin_dip; 
    *uy_ss = yb*q/(R*(R + eta))
           + q*cos_dip/(R + eta)
           + I2*sin_dip;
    *uz_ss = db*q/(R*(R + eta))
           + q*sin_dip/(R + eta)
           + I4*sin_dip;
    *ux_ds = q/R
           - I3*sin_dip*cos_dip;
    *uy_ds = yb*q/(R*(R + xi))
           + cos_dip*atan_xeqr
           - I1*sin_dip*cos_dip;
    *uz_ds = db*q/(R*(R + xi))
           + sin_dip*atan_xeqr
           - I5*sin_dip*cos_dip;
    return;
}

#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds_zeroDip(const double sin_dip,
                                   const double xi, const double eta,
                                   const double q,
                                   double *ux_ss, double *uy_ss, double *uz_ss,
                                   double *ux_ds, double *uy_ds, double *uz_ds)
{
    double atan_xeqr, db, I1, I2, I3, I4, log_rpeta, pow_rpdb2,
           R, Rpdb, yb; 
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = q*sin_dip;
    db = eta*sin_dip;
    //X = sqrt(xi*xi + q*q);
    log_rpeta = log(R + eta);
    Rpdb = R + db;
    pow_rpdb2 = Rpdb*Rpdb; //pow(R + db, 2); 
    atan_xeqr = atan(xi*eta/(q*R));

    //I5 =-(1.0 - 2.0*nu)*xi*sin_dip/(R + db);
    I4 =-(1.0 - 2.0*nu)*q/(R + db);
    I3 = (1.0 - 2.0*nu)/2.0*( eta/(R+db) + yb*q/pow_rpdb2 - log_rpeta );
    I2 = (1.0 - 2.0*nu)*(-log_rpeta) - I3; 
    I1 =-(1.0 - 2.0*nu)/2.0*xi*q/pow_rpdb2;

    *ux_ss = xi*q/(R*(R+eta))
           + atan_xeqr
           + I1*sin_dip;
    *uy_ss = yb*q/(R*(R + eta))
           + I2*sin_dip;
    *uz_ss = db*q/(R*(R + eta))
           + q*sin_dip/(R + eta)
           + I4*sin_dip;
    *ux_ds = q/R;
    *uy_ds = yb*q/(R*(R + xi));
    *uz_ds = db*q/(R*(R + xi))
           + sin_dip*atan_xeqr;
    return;
}

#ifdef _OPENMP
#pragma omp declare simd
#endif
static inline void __ss_ds_withDip(const double cos_dip, const double sin_dip,
                                   const double xi, const double eta,
                                   const double q,
                                   double *ux_ss, double *uy_ss, double *uz_ss,
                                   double *ux_ds, double *uy_ds, double *uz_ds)
{
    double atan_xeqr, db, I1, I2, I3, I4, I5, log_rpeta, R,
           X, yb;
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip + q*sin_dip;
    db = eta*sin_dip - q*cos_dip;
    X = sqrt(xi*xi + q*q);
    log_rpeta = log(R + eta);
    //Rpdb = R + db;
    //pow_rpdb2 = Rpdb*Rpdb; //pow(R + db, 2);
    atan_xeqr = atan(xi*eta/(q*R));
 
    I5 = (1.0 - 2.0*nu)*2.0/cos_dip
        *atan( ( eta*(X + q*cos_dip) + X*(R + X)*sin_dip )
              /(xi*(R + X)*cos_dip));
    I4 = (1.0 - 2.0*nu)/cos_dip*(log(R+db) - sin_dip*log_rpeta);
    I3 = (1.0 - 2.0*nu)*(yb/cos_dip/(R+db) - log_rpeta)
       + sin_dip/cos_dip*I4;
    I2 = (1.0 - 2.0*nu)*(-log_rpeta) - I3;
    I1 = (1.0 - 2.0*nu)*(-xi/cos_dip/(R+db))
       - sin_dip/cos_dip*I5;

    *ux_ss = xi*q/(R*(R+eta))
           + atan_xeqr
           + I1*sin_dip;
    *uy_ss = yb*q/(R*(R + eta))
           + q*cos_dip/(R + eta)
           + I2*sin_dip;
    *uz_ss = db*q/(R*(R + eta))
           + q*sin_dip/(R + eta)
           + I4*sin_dip;
    *ux_ds = q/R
           - I3*sin_dip*cos_dip;
    *uy_ds = yb*q/(R*(R + xi))
           + cos_dip*atan_xeqr
           - I1*sin_dip*cos_dip;
    *uz_ds = db*q/(R*(R + xi))
           + sin_dip*atan_xeqr
           - I5*sin_dip*cos_dip;
    return;
}
