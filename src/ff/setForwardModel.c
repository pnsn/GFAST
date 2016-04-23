#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "gfast.h"
// Small number to test cos(90) = 0 
#define eps 6.1232e-14 /*!< A close number for okadaGreenF */
// Poisson's ratio
#define nu 0.25 /*!< Poisson's ratio for okadaGreenF */

/* TODO atan is used a lot, should it be atan2 */
// Local functions
#pragma omp declare simd
static inline double __ux_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __uy_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __uz_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __ux_ds(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __uy_ds(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __uz_ds(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip); //double dip);
#pragma omp declare simd
static inline double __I1(double cos_dip, double sin_dip,
                          double xi, double eta, double q,
                          bool ldip, double R);
#pragma omp declare simd
static inline double __I2(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R);
#pragma omp declare simd
static inline double __I3(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R);
#pragma omp declare simd
static inline double __I4(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R);
#pragma omp declare simd
static inline double __I5(double cos_dip, double sin_dip,
                          double xi, double eta, double q, 
                          bool ldip, double R);

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
int GFAST_FF__setForwardModel__okadagreenF(int l1, int l2,
                                           double *__restrict__ e,
                                           double *__restrict__ n,
                                           double *__restrict__ depth,
                                           double *__restrict__ strike,
                                           double *__restrict__ dip,
                                           double *__restrict__ W,
                                           double *__restrict__ L,
                                           double *__restrict__ G)
{
    double cos_dip, cos_strike, d, dip1, ec, 
           g1, g1n, g2, g2n, g3, g3n, g4, g4n, g5, g6,
           nc, p, q, strike1, sin_dip, sin_strike, x, y;
    int i, j, ij, indx;
    const double pi180 = M_PI/180.0;
    const double one_twopi = 1.0/(2.0*M_PI);
    //------------------------------------------------------------------------//
    indx = 0;
    // Loop on the observations
    for (j=0; j<l1; j++){
        // Loop on faults
        for (i=0; i<l2; i++){
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

            g1 = -one_twopi*( __ux_ss(cos_dip, sin_dip,
                                      x,p,q, false) 
                            - __ux_ss(cos_dip, sin_dip,
                                      x,p-W[i],q, false)
                            - __ux_ss(cos_dip, sin_dip,
                                      x-L[i],p,q, false) 
                            + __ux_ss(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));

            g2 = -one_twopi*( __ux_ds(cos_dip, sin_dip,
                                      x,p,q, false) 
                            - __ux_ds(cos_dip, sin_dip,
                                      x,p-W[i],q, false) 
                            - __ux_ds(cos_dip, sin_dip,
                                      x-L[i],p,q, false) 
                            + __ux_ds(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));

            g3 = -one_twopi*( __uy_ss(cos_dip, sin_dip,
                                      x,p,q, false)
                            - __uy_ss(cos_dip, sin_dip,
                                      x,p-W[i],q, false)
                            - __uy_ss(cos_dip, sin_dip,
                                      x-L[i],p,q, false)
                            + __uy_ss(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));
            g4 = -one_twopi*( __uy_ds(cos_dip, sin_dip,
                                      x,p,q, false)
                            - __uy_ds(cos_dip, sin_dip,
                                      x,p-W[i],q, false)
                            - __uy_ds(cos_dip, sin_dip,
                                      x-L[i],p,q, false)
                            + __uy_ds(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));

            g5 = -one_twopi*( __uz_ss(cos_dip, sin_dip,
                                      x,p,q, false)
                            - __uz_ss(cos_dip, sin_dip,
                                      x,p-W[i],q, false)
                            - __uz_ss(cos_dip, sin_dip,
                                      x-L[i],p,q, false)
                            + __uz_ss(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));

            g6 = -one_twopi*( __uz_ds(cos_dip, sin_dip,
                                      x,p,q, false)
                            - __uz_ds(cos_dip, sin_dip,
                                      x,p-W[i],q, false)
                            - __uz_ds(cos_dip, sin_dip,
                                      x-L[i],p,q, false)
                            + __uz_ds(cos_dip, sin_dip,
                                      x-L[i],p-W[i],q, false));

            if (cos_dip > eps){
                g1 = -one_twopi*( __ux_ss(cos_dip, sin_dip,
                                          x,p,q, true) 
                                - __ux_ss(cos_dip, sin_dip,
                                          x,p-W[i],q, true)
                                - __ux_ss(cos_dip, sin_dip,
                                          x-L[i],p,q, true) 
                                + __ux_ss(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));

                g2 = -one_twopi*( __ux_ds(cos_dip, sin_dip,
                                          x,p,q, true) 
                                - __ux_ds(cos_dip, sin_dip,
                                          x,p-W[i],q, true) 
                                - __ux_ds(cos_dip, sin_dip,
                                          x-L[i],p,q, true) 
                                + __ux_ds(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));

                g3 = -one_twopi*( __uy_ss(cos_dip, sin_dip,
                                          x,p,q, true)
                                - __uy_ss(cos_dip, sin_dip,
                                          x,p-W[i],q, true)
                                - __uy_ss(cos_dip, sin_dip,
                                          x-L[i],p,q, true)
                                + __uy_ss(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));
                g4 = -one_twopi*( __uy_ds(cos_dip, sin_dip,
                                          x,p,q, true)
                                - __uy_ds(cos_dip, sin_dip,
                                          x,p-W[i],q, true)
                                - __uy_ds(cos_dip, sin_dip,
                                          x-L[i],p,q, true)
                                + __uy_ds(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));

                g5 = -one_twopi*( __uz_ss(cos_dip, sin_dip,
                                          x,p,q, true)
                                - __uz_ss(cos_dip, sin_dip,
                                          x,p-W[i],q, true)
                                - __uz_ss(cos_dip, sin_dip,
                                          x-L[i],p,q, true)
                                + __uz_ss(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));

                g6 = -one_twopi*( __uz_ds(cos_dip, sin_dip,
                                          x,p,q, true)
                                - __uz_ds(cos_dip, sin_dip,
                                          x,p-W[i],q, true)
                                - __uz_ds(cos_dip, sin_dip,
                                          x-L[i],p,q, true)
                                + __uz_ds(cos_dip, sin_dip,
                                          x-L[i],p-W[i],q, true));
            }

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

        } // Loop on stations
    } // Loop on faults
    return 0;
}

#pragma omp declare simd
static inline double __ux_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip)
{
    double R, u;
    R = sqrt(xi*xi + eta*eta + q*q);
    u = xi*q/(R*(R+eta)) 
      + atan(xi*eta/(q*R)) 
      + __I1(cos_dip, sin_dip, xi, eta, q, ldip, R)*sin_dip;
    return u;
}

#pragma omp declare simd
static inline double __uy_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip)
{
    double R, u, yb;
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip+ q*sin_dip;
    u = yb*q/(R*(R + eta)) + q*cos_dip/(R + eta)
      + __I2(cos_dip, sin_dip, eta, q, ldip, R)*sin_dip;
    return u;
}

#pragma omp declare simd
static inline double __uz_ss(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip)
{
    double db, R, u;
    R = sqrt(xi*xi + eta*eta + q*q);
    db = eta*sin_dip - q*cos_dip;
    u = db*q/(R*(R + eta)) + q*sin_dip/(R + eta) 
      + __I4(cos_dip, sin_dip, eta, q, ldip, R)*sin_dip;
    return u;
}

#pragma omp declare simd
static inline double __ux_ds(double cos_dip, double sin_dip, 
                             double xi, double eta, double q, bool ldip)
{
    double R, u;
    R = sqrt(xi*xi + eta*eta + q*q);
    u = q/R - __I3(cos_dip, sin_dip, eta, q, ldip, R)*sin_dip*cos_dip;
    return u;
}

#pragma omp declare simd
static inline double __uy_ds(double cos_dip, double sin_dip, 
                             double xi, double eta, double q, bool ldip)
{
    double R, u, yb;
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip + q*sin_dip;
    u = yb*q/(R*(R + xi)) + cos_dip*atan(xi*eta/(q*R)) 
      - __I1(cos_dip, sin_dip, xi, eta, q, ldip, R)*sin_dip*cos_dip;
    return u;
}

#pragma omp declare simd
static inline double __uz_ds(double cos_dip, double sin_dip,
                             double xi, double eta, double q, bool ldip)
{
    double db, R, u;
    R = sqrt(xi*xi + eta*eta + q*q);
    db = eta*sin_dip - q*cos_dip;
    u = db*q/(R*(R + xi)) + sin_dip*atan(xi*eta/(q*R)) 
      - __I5(cos_dip, sin_dip, xi, eta, q, ldip, R)*sin_dip*cos_dip;
    return u;
}

#pragma omp declare simd
static inline double __I1(double cos_dip, double sin_dip,
                          double xi, double eta, double q, bool ldip, double R)
{
    double db, I;
    db = eta*sin_dip - q*cos_dip;
    I =-(1.0 - 2.0*nu)/2.0*xi*q/pow(R+db,2);
    if (ldip){
        I = (1.0 - 2.0*nu)*(-xi/cos_dip/(R+db)) 
          - sin_dip/cos_dip*__I5(cos_dip, sin_dip, xi, eta, q, ldip, R);
    }
    return I;
}

#pragma omp declare simd
static inline double __I2(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R)
{
    double I;
    I = (1.0 - 2.0*nu)*(-log(R+eta)) - __I3(cos_dip, sin_dip, eta, q, ldip, R);
    return I;
}

#pragma omp declare simd
static inline double __I3(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R)
{
    double db, I, log_rpeta, yb;
    yb = eta*cos_dip + q*sin_dip;
    db = eta*sin_dip - q*cos_dip;
    log_rpeta = log(R + eta);
    I = (1.0 - 2.0*nu)/2.0*( eta/(R+db) + yb*q/pow(R+db,2) - log_rpeta ); 
    if (ldip){
        I = (1.0 - 2.0*nu)*(yb/cos_dip/(R+db) - log_rpeta) 
          + sin_dip/cos_dip*__I4(cos_dip, sin_dip, eta, q, ldip, R);
    }
    return I;
}

#pragma omp declare simd
static inline double __I4(double cos_dip, double sin_dip,
                          double eta, double q, bool ldip, double R)
{
    double db, I;
    db = eta*sin_dip - q*cos_dip;
    I =-(1.0 - 2.0*nu)*q/(R+db);
    if (ldip){
        I = (1.0 - 2.0*nu)/cos_dip*(log(R+db) - sin_dip*log(R+eta));
    }
    return I;
}

#pragma omp declare simd
static inline double __I5(double cos_dip, double sin_dip,
                          double xi, double eta, double q, bool ldip, double R)
{
    double db, I, X;
    db = eta*sin_dip - q*cos_dip;
    X = sqrt(xi*xi + q*q);
    I = -(1.0 - 2.0*nu)*xi*sin_dip/(R + db);
    if (ldip){
        I = (1.0 - 2.0*nu)*2.0/cos_dip
           *atan( ( eta*(X + q*cos_dip) + X*(R + X)*sin_dip )
                 /(xi*(R + X)*cos_dip));
    }
    return I;
}

