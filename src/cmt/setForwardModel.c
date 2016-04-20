#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include "gfast.h"
/*!
 * @brief Computes matrix of Green's functions required in the CMT inversion
 *
 * @param[in] l1          length of arrays
 * @param[in] deviatoric 
 * @param[in] x1          x source receiver distance (meters) [l1]
 * @param[in] y1          y source receiver distance (meters) [l1]
 * @param[in] z1          z source receiver distance (meters) [l1]
 * @param[in] azi         source receiver angles in (x,y) Cartesian plane
 *                        (degrees) [l1]
 *
 * @param[out] G          matrix of Green's functions stored in row major
 *                        format [3*l1 x ncol] where ncol is 5 if 
 *                        deviatoric is true, otherwise, it is 6
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @date April, 2016
 *
 */
int GFAST_CMT__setForwardModel(int l1, bool deviatoric,
                               double *__restrict__ x1, 
                               double *__restrict__ y1, 
                               double *__restrict__ z1, 
                               double *__restrict__ azi, 
                               double *__restrict__ G)
{
    const char *fcnm = "GFAST_CMT__setForwardModel\0";
    double azi1, C1, C2, cosaz, cosaz_sinaz, cosaz2,  
           g111, g122, g133, g112, g113, g123,
           g211, g222, g233, g212, g213, g223,
           g311, g322, g333, g312, g313, g323,
           R, R3, sinaz, sinaz2, x, y, z;
    int i, indx;
    const double MU = 3.e10;
    const double K = 5.0*MU/3.0;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    //  
    // Size check
    if (l1 < 1){ 
        log_errorF("%s: Error invalid number of points %d\n", fcnm, l1);
        return -1; 
    }   
    // Deviatoric constraints
    if (deviatoric){
        // Loop on points and fill up Green's function deviatoric matrix
        indx = 0;
        #pragma omp simd
        for (i=0; i<l1; i++){
            // compute coefficients in greens functions
            x = x1[i];
            y = y1[i];
            z = z1[i];
            R = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            C1= 1.0/pow(R,2)/MU/M_PI/8.0;
            C2 = (3.0*K + MU)/(3.0*K + 4.0*MU);
            R3 = pow(R, 3);
            // coefficients for row 1
            g111 = C1*(C2*3.0*x*x*x/R3 - 3.0*C2*x/R+2.0*x/R);
            g122 = C1*(C2*3.0*x*y*y/R3 - C2*x/R);
            g133 = C1*(C2*3.0*x*z*z/R3 - C2*x/R);
            g112 = C1*(C2*6.0*x*x*y/R3 - 2.0*C2*y/R + 2.0*y/R);
            g113 = C1*(C2*6.0*x*x*z/R3 - 2.0*C2*z/R + 2.0*z/R);
            g123 = C1*(C2*6.0*x*y*z/R3);
            // coefficients for row 2
            g211 = C1*(C2*3.0*y*x*x/R3 - C2*y/R);
            g222 = C1*(C2*3.0*y*y*y/R3 - 3.0*C2*y/R + 2.0*y/R);
            g233 = C1*(C2*3.0*y*z*z/R3 - C2*y/R);
            g212 = C1*(C2*6.0*y*x*y/R3 - 2.0*C2*x/R + 2.0*x/R);
            g213 = C1*(C2*6.0*y*x*z/R3);
            g223 = C1*(C2*6.0*y*y*z/R3 - 2.0*C2*z/R + 2.0*z/R);
            // coefficients for row 3
            g311 = C1*(C2*3.0*z*x*x/R3 - C2*z/R);
            g322 = C1*(C2*3.0*z*y*y/R3 - C2*z/R);
            g333 = C1*(C2*3.0*z*z*z/R3 - 3.0*C2*z/R + 2.0*z/R);
            g312 = C1*(C2*6.0*z*x*y/R3);
            g313 = C1*(C2*6.0*z*x*z/R3 - 2.0*C2*x/R + 2.0*x/R);
            g323 = C1*(C2*6.0*z*y*z/R3 - 2.0*C2*y/R + 2.0*y/R);
            // Fill row 1
            G[indx+0] = g112;
            G[indx+1] = g113;
            G[indx+2] = g133;
            G[indx+3] = 0.5*(g111 - g122);
            G[indx+4] = g123;
            // Fill row 2
            G[indx+5] = g212;
            G[indx+6] = g213;
            G[indx+7] = g233;
            G[indx+8] = 0.5*(g211 - g222);
            G[indx+9] = g223;
            // Fill row 3
            G[indx+10] = g312;
            G[indx+11] = g313;
            G[indx+12] = g333;
            G[indx+13] = 0.5*(g311 - g322);
            G[indx+14] = g323;
            // Update the index 3 rows
            indx = indx + 15;
        } // Loop on points
    // General moment tensor 
    }else{
        // Loop on points and fill up Green's functions matrix
        log_warnF("%s: This is deprecated\n", fcnm);
        indx = 0;
        #pragma omp simd
        for (i=0; i<l1; i++){
            // define some constants for this point
            azi1 = azi[i]*pi180;
            sinaz = sin(azi1);
            cosaz = cos(azi1);
            sinaz2 = pow(sinaz, 2);
            cosaz2 = pow(cosaz, 2);
            cosaz_sinaz = cosaz*sinaz;
            // compute coefficients in greens functions
            x = x1[i];
            y = y1[i];
            z = z1[i];
            R = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            C1= 1.0/pow(R,2)/MU/M_PI/8.0;
            C2 = (3.0*K + MU)/(3.0*K + 4.0*MU);
            R3 = pow(R, 3);
            // coefficients for row 1
            g111 = C1*( C2*(3.0*x*x*x/R3 - 3.0*x/R) + 2.0*x/R );
            g122 = C1*( C2*(3.0*x*y*y/R3 - x/R) );
            g133 = C1*( C2*(3.0*x*z*z/R3 - x/R) );
            g112 = C1*( C2*(3.0*x*x*y/R3 - y/R) + 2.0*y/R );
            g113 = C1*( C2*(3.0*x*x*z/R3 - z/R) + 2.0*z/R );
            g123 = C1*( C2*(3.0*x*y*z/R3) );
            // coefficients for row 2
            g211 = C1*( C2*(3.0*y*x*x/R3 - y/R) );
            g222 = C1*( C2*(3.0*y*y*y/R3 - 3.0*y/R) + 2.0*y/R );
            g233 = C1*( C2*(3.0*y*z*z/R3 - y/R) );
            g212 = C1*( C2*(3.0*y*x*y/R3 - x/R) );
            g213 = C1*( C2*(3.0*y*x*z/R3) );
            g223 = C1*( C2*(3.0*y*y*z/R3 - z/R) + 2.0*z/R );
            // coefficients for row 3
            g311 = C1*( C2*(3.0*z*x*x/R3 - z/R) );
            g322 = C1*( C2*(3.0*z*y*y/R3 - z/R) );
            g333 = C1*( C2*(3.0*z*z*z/R3 - 3.0*z/R) + 2.0*z/R );
            g312 = C1*( C2*(3.0*z*x*y/R3) );
            g313 = C1*( C2*(3.0*z*x*z/R3) + 2.0*x/R);
            g323 = C1*( C2*(3.0*z*y*z/R3) + 2.0*y/R);
            // Fill row 1
            G[indx+0]  = g111*cosaz2 + g122*sinaz2 + 2.0*g112*cosaz_sinaz;
            G[indx+1]  = g111*sinaz2 + g122*cosaz2 - 2.0*g112*cosaz_sinaz;
            G[indx+2]  = g133;
            G[indx+3]  =-g111*cosaz_sinaz + g112*cosaz2
                       - g112*sinaz2 + g122*cosaz_sinaz;
            G[indx+4]  = g113*cosaz + g123*sinaz;
            G[indx+5]  = g123*cosaz - g113*sinaz;
            // Fill row 2
            G[indx+6]  = g211*cosaz2 + g222*sinaz2 + 2.0*g212*cosaz_sinaz;
            G[indx+7]  = g211*sinaz2 + g222*cosaz2 - 2.0*g212*cosaz_sinaz;
            G[indx+8]  = g233;
            G[indx+9]  =-g211*cosaz_sinaz + g212*cosaz2
                       - g212*sinaz2 + g222*cosaz_sinaz;
            G[indx+10] = g213*cosaz + g223*sinaz;
            G[indx+11] = g223*cosaz - g213*sinaz;
            // Fill row 3
            G[indx+12] = g311*cosaz2 + g322*sinaz2 + 2.0*g312*cosaz_sinaz;
            G[indx+13] = g311*sinaz2 + g322*cosaz2 - 2.0*g312*cosaz_sinaz;
            G[indx+14] = g333;
            G[indx+15] =-g311*cosaz_sinaz + g312*cosaz2
                       - g312*sinaz2 + g322*cosaz_sinaz;
            G[indx+16] = g313*cosaz + g323*sinaz;
            G[indx+17] = g323*cosaz - g313*sinaz;
            // Update the index 3 rows
            indx = indx + 18;
        } // Loop on points
    }
    return 0;
}
