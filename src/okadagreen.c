#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast.h"
// Small number to test cos(90) = 0 
#define eps 6.1232e-14 /*!< A close number for okadaGreenF */
// Poisson's ratio
#define nu 0.25 /*!< Poisson's ratio for okadaGreenF */

/* TODO atan is used a lot, should it be atan2 */

// Local functions
static inline double __ux_ss(double xi, double eta, double q, double dip);
static inline double __uy_ss(double xi, double eta, double q, double dip);
static inline double __uz_ss(double xi, double eta, double q, double dip);
static inline double __ux_ds(double xi, double eta, double q, double dip);
static inline double __uy_ds(double xi, double eta, double q, double dip);
static inline double __uz_ds(double xi, double eta, double q, double dip);
static inline double __I1(double xi, double eta, double q,
                          double dip, double R);
static inline double __I2(double eta, double q, double dip, double R);
static inline double __I3(double eta, double q, double dip, double R);
static inline double __I4(double eta, double q, double dip, double R);
static inline double __I5(double xi, double eta, double q, 
                          double dip, double R);
/*!
 * @brief This program computes the Green's functions from Okada's formulation 
 *        for slip on fault patches at a set of station locations
 *
 * @param[in] l1      number of station locations
 * @param[in] l2      number of fault locations
 * @param[in] e       station/fault eastings [l1 x l2 - row major order]
 * @param[in] n       station/fault northings [l1 x l2 - row major order]
 * @param[in] depth   station/fault depths [l1 x l2 - row major order]
 * @param[in] strike  fault strikes [l2]
 * @param[in] dip     fault dips [l2]
 * @param[in] W       fault widths [l2]
 * @param[in] L       fault lengths [l2]
 * @param[out] G      [3*l1 x 2*l2]
 *
 * @author Brendan Crowell, PNSN
 *         Ben Baker, ISTI -> translated to C
 * @date February 2016
 *
 */
int GFAST_okadagreenF(int l1, int l2, double *e, double *n, double *depth,
                      double *strike, double *dip, double *W, double *L,
                      double *G)
{
    double cos_dip, cos_strike, d, dip1, ec, 
           g1, g1n, g2, g2n, g3, g3n, g4, g4n, g5, g6,
           nc, p, q, strike1, sin_dip, sin_strike, x, y;
    int i, j, ij;
    const double pi180i = M_PI/180.0;
    const double one_twopi = 1.0/(2.0*M_PI);
    //------------------------------------------------------------------------//
    //
    // Loop on the faults
    for (i=0; i<l2; i++){
        // Loop on the stations
        for (j=0; j<l1; j++){
            ij = l2*i + j;
            strike1 = strike[i]*pi180i;
            dip1 = dip[i]*pi180i;
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

            g1 = -one_twopi*( __ux_ss(x,p,q,dip1) 
                            - __ux_ss(x,p-W[i],q,dip1) 
                            - __ux_ss(x-L[i],p,q,dip1) 
                            + __ux_ss(x-L[i],p-W[i],q,dip1));
            g2 = -one_twopi*( __ux_ds(x,p,q,dip1) 
                            - __ux_ds(x,p-W[i],q,dip1) 
                            - __ux_ds(x-L[i],p,q,dip1) 
                            + __ux_ds(x-L[i],p-W[i],q,dip1));

            g3 = -one_twopi*( __uy_ss(x,p,q,dip1)
                            - __uy_ss(x,p-W[i],q,dip1)
                            - __uy_ss(x-L[i],p,q,dip1)
                            + __uy_ss(x-L[i],p-W[i],q,dip1));
            g4 = -one_twopi*( __uy_ds(x,p,q,dip1)
                            - __uy_ds(x,p-W[i],q,dip1)
                            - __uy_ds(x-L[i],p,q,dip1)
                            + __uy_ds(x-L[i],p-W[i],q,dip1));

            g5 = -one_twopi*( __uz_ss(x,p,q,dip1)
                            - __uz_ss(x,p-W[i],q,dip1)
                            - __uz_ss(x-L[i],p,q,dip1)
                            + __uz_ss(x-L[i],p-W[i],q,dip1));
            g6 = -one_twopi*( __uz_ds(x,p,q,dip1)
                            - __uz_ds(x,p-W[i],q,dip1)
                            - __uz_ds(x-L[i],p,q,dip1)
                            + __uz_ds(x-L[i],p-W[i],q,dip1));

            g1n = sin_strike*g1 - cos_strike*g3;
            g3n = cos_strike*g1 + sin_strike*g3;

            g2n = sin_strike*g2 - cos_strike*g4;
            g4n = cos_strike*g2 + sin_strike*g4;

            ij = 3*j*l2 + 2*i;
            G[ij]   = g1n;
            G[ij+1] = g2n;

            ij = 3*(j+1)*l2 + 2*i;
            G[ij]   = g3n;
            G[ij+1] = g4n;

            ij = 3*(j+2)*l2 + 2*i;
            G[ij]   = g5;
            G[ij+1] = g6;
        } // Loop on stations
    } // Loop on faults
    return 0;
}

static inline double __ux_ss(double xi, double eta, double q, double dip)
{
    double R, u;
    R = sqrt(xi*xi + eta*eta + q*q);
    u = xi*q/(R*(R+eta)) 
      + atan(xi*eta/(q*R)) 
      + __I1(xi, eta, q, dip, R)*sin(dip);
    return u;
}

static inline double __uy_ss(double xi, double eta, double q, double dip)
{
    double cos_dip, R, sin_dip, u, yb;
    cos_dip = cos(dip); 
    sin_dip = sin(dip);
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip+ q*sin_dip;
    u = yb*q/(R*(R + eta)) + q*cos_dip/(R + eta) + __I2(eta, q, dip, R)*sin_dip;
    return u;
}

static inline double __uz_ss(double xi, double eta, double q, double dip)
{
    double cos_dip, db, R, sin_dip, u;
    cos_dip = cos(dip);
    sin_dip = sin(dip);
    R = sqrt(xi*xi + eta*eta + q*q);
    db = eta*sin_dip - q*cos_dip;
    u = db*q/(R*(R + eta)) + q*sin_dip/(R + eta) 
      + __I4(eta, q, dip, R)*sin_dip;
    return u;
}

static inline double __ux_ds(double xi, double eta, double q, double dip)
{
    double cos_dip, R, sin_dip, u;
    cos_dip = cos(dip);
    sin_dip = sin(dip);
    R = sqrt(xi*xi + eta*eta + q*q);
    u = q/R - __I3(eta, q, dip, R)*sin_dip*cos_dip;
    return u;
}

static inline double __uy_ds(double xi, double eta, double q, double dip)
{
    double cos_dip, R, sin_dip, u, yb;
    cos_dip = cos(dip);
    sin_dip = sin(dip);
    R = sqrt(xi*xi + eta*eta + q*q);
    yb = eta*cos_dip + q*sin_dip;
    u = yb*q/(R*(R + xi)) + cos_dip*atan(xi*eta/(q*R)) 
      - __I1(xi, eta, q, dip, R)*sin_dip*cos_dip;
    return u;
}

static inline double __uz_ds(double xi, double eta, double q, double dip)
{
    double cos_dip, db, R, sin_dip, u;
    cos_dip = cos(dip); 
    sin_dip = sin(dip);
    R = sqrt(xi*xi + eta*eta + q*q);
    db = eta*sin_dip - q*cos_dip;
    u = db*q/(R*(R + xi)) + sin_dip*atan(xi*eta/(q*R)) 
      - __I5(xi, eta, q, dip, R)*sin_dip*cos_dip;
    return u;
}

static inline double __I1(double xi, double eta, double q, double dip, double R)
{
    double cos_dip, db, I, sin_dip;
    cos_dip = cos(dip);
    sin_dip = sin(dip);
    db = eta*sin_dip - q*cos_dip;
    if (cos_dip > eps){
        I = (1.0 - 2.0*nu)*(-xi/cos_dip/(R+db)) 
          - sin_dip/cos_dip*__I5(xi, eta, q, dip, R);
    }else{
        I =-(1.0 - 2.0*nu)/2.0*xi*q/pow(R+db,2);
    }
    return I;
}

static inline double __I2(double eta, double q, double dip, double R)
{
    double I;
    I = (1.0 - 2.0*nu)*(-log(R+eta)) - __I3(eta, q, dip, R);
    return I;
}

static inline double __I3(double eta, double q, double dip, double R)
{
    double cos_dip, db, I, sin_dip, yb;
    cos_dip = cos(dip); 
    sin_dip = sin(dip);
    yb = eta*cos_dip + q*sin_dip;
    db = eta*sin_dip - q*cos_dip;
    if (cos_dip > eps){
        I = (1.0 - 2.0*nu)*(yb/cos_dip/(R+db) - log(R+eta)) 
          + sin_dip/cos_dip*__I4(eta, q, dip, R);
    }else{
        I = (1.0 - 2.0*nu)/2.0*( eta/(R+db) + yb*q/pow(R+db,2) -log(R+eta) );
    }
    return I;
}

static inline double __I4(double eta, double q, double dip, double R)
{
    double cos_dip, db, I, sin_dip;
    cos_dip = cos(dip);
    sin_dip = sin(dip);
    db = eta*sin_dip - q*cos_dip;
    if (cos_dip > eps){
        I = (1.0 - 2.0*nu)/cos_dip*(log(R+db) - sin_dip*log(R+eta));
    }else{
        I = -(1-2*nu)*q/(R+db);
    }
    return I;
}

static inline double __I5(double xi, double eta, double q, double dip, double R)
{
    double cos_dip, db, I, sin_dip, X;
    cos_dip = cos(dip); 
    sin_dip = sin(dip);
    db = eta*sin_dip - q*cos_dip;
    X = sqrt(xi*xi + q*q);
    if (cos_dip > eps){
        I = (1.0 - 2.0*nu)*2.0/cos_dip
           *atan( ( eta*(X + q*cos_dip) + X*(R + X)*sin_dip )
                 /(xi*(R + X)*cos_dip));
    }else{
        I = -(1.0 - 2.0*nu)*xi*sin_dip/(R + db);
    }
    return I;
}
