#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cblas.h>
#include "gfast.h"
/*
#include <seismic/momentTensor/utils/mopad.h>
#include <seismic/utils/math/lapack_extern.h>
#include <seismic/utils/log/log.h>
*/

struct mt_t{
    double mt[6];
    double tax[3];
    double bax[3];
    double pax[3];
    double sdr1[3]; //strike dip rake; fault plane 2
    double sdr2[3]; //strike dip rake; fault plane 1
    double mag;
    double exp;
};

int decompose(int, struct mt_t);
 
int cmopad_test(int verb)
{
    /*! 
     * Moment tensor decomposition verification: 
     * http://www.globalcmt.org/cgi-bin/globalcmt-cgi-bin/CMT4/form?itype=ymd&yr=2014&mo=1&day=1&otype=ymd&oyr=2014&omo=1&oday=5&jyr=2014&jday=1&ojyr=1976&ojday=1&nday=1&lmw=0&umw=10&lms=0&ums=10&lmb=0&umb=10&llat=-90&ulat=90&llon=-180&ulon=180&lhd=0&uhd=1000&lts=-9999&uts=9999&lpe1=0&upe1=90&lpe2=0&upe2=90&list=5
     */
    // Event 1
    struct mt_t mt;
    int n6 = 6, incx = 1, ierr;
    //double mt[5];
    double xscal;
    //-----------------------------------1------------------------------------//
    if (verb > 0){log_infoF("Testing source 1...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 5.1;
    mt.mt[0] =-4.360;
    mt.mt[1] = 3.200; 
    mt.mt[2] = 1.170;
    mt.mt[3] =-0.794; 
    mt.mt[4] = 1.650;
    mt.mt[5] = 2.060;
    mt.sdr1[0] = 77.0; 
    mt.sdr1[1] = 47.0;
    mt.sdr1[2] =-62.0; 
    mt.sdr2[0] = 219.0;
    mt.sdr2[1] = 50.0;
    mt.sdr2[2] =-117.0;
    mt.tax[0] = 4.49;
    mt.tax[1] = 1.0;
    mt.tax[2] = 328.0;
    mt.bax[0] = 0.56;
    mt.bax[1] = 20.0;
    mt.bax[2] = 237.0;
    mt.pax[0] =-5.04;
    mt.pax[1] = 70.0;
    mt.pax[2] = 61.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){
        log_errorF("Error decomposing 1\n");
        return EXIT_FAILURE;
    }
    if (ierr != 0){ 
        log_errorF("mopad failed test 1\n");
        return EXIT_FAILURE;
    }   
    //--------------------------------------2----------------------------------//
    if (verb > 0){log_infoF("Testing source 1...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 5.0;
    mt.mt[0] =-4.480;
    mt.mt[1] = 1.440; 
    mt.mt[2] = 3.040;
    mt.mt[3] = 0.603; 
    mt.mt[4] = 0.722;
    mt.mt[5] =-1.870;
    mt.sdr1[0] =316.0; 
    mt.sdr1[1] = 44.0;
    mt.sdr1[2] =-105.0; 
    mt.sdr2[0] = 157.0;
    mt.sdr2[1] = 48.0;
    mt.sdr2[2] =-76.0;
    mt.tax[0] = 4.28;
    mt.tax[1] = 2.0;
    mt.tax[2] = 237.0;
    mt.bax[0] = 0.37;
    mt.bax[1] = 10.0;
    mt.bax[2] = 327.0;
    mt.pax[0] =-4.66;
    mt.pax[1] = 79.0;
    mt.pax[2] =137.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 2\n");
        return EXIT_FAILURE;
    }   
    //------------------------------------3-----------------------------------//
    if (verb > 0){log_infoF("Testing source 3...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 4.9;
    mt.mt[0] =-2.460;
    mt.mt[1] = 0.207; 
    mt.mt[2] = 2.250;
    mt.mt[3] = 0.793; 
    mt.mt[4] = 0.267;
    mt.mt[5] =-0.363;
    mt.sdr1[0] =335.0; 
    mt.sdr1[1] = 46.0;
    mt.sdr1[2] =-113.0; 
    mt.sdr2[0] = 186.0;
    mt.sdr2[1] = 49.0;
    mt.sdr2[2] =-68.0;
    mt.tax[0] = 2.32;
    mt.tax[1] = 2.0;
    mt.tax[2] = 261.0;
    mt.bax[0] = 0.38;
    mt.bax[1] = 16.0;
    mt.bax[2] = 351.0;
    mt.pax[0] =-2.70;
    mt.pax[1] = 74.0;
    mt.pax[2] =165.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 3\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------4------------------------------------//
    if (verb > 0){log_infoF("Testing source 4...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 4.9;
    mt.mt[0] =-2.270;
    mt.mt[1] = 0.058; 
    mt.mt[2] = 2.210;
    mt.mt[3] = 0.079; 
    mt.mt[4] =-0.737;
    mt.mt[5] = 0.246;
    mt.sdr1[0] =190.0; 
    mt.sdr1[1] = 36.0;
    mt.sdr1[2] =-84.0; 
    mt.sdr2[0] = 3.0;
    mt.sdr2[1] = 54.0;
    mt.sdr2[2] =-94.0;
    mt.tax[0] = 2.35;
    mt.tax[1] = 9.0;
    mt.tax[2] = 96.0;
    mt.bax[0] = 0.04;
    mt.bax[1] = 4.0;
    mt.bax[2] = 5.0;
    mt.pax[0] =-2.39;
    mt.pax[1] = 80.0;
    mt.pax[2] =253.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 4\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------5------------------------------------//
    if (verb > 0){log_infoF("Testing source 5...\n");}
    mt.exp = 26.0 - 7.0;
    mt.mag = 6.5;
    mt.mt[0] = 0.745;
    mt.mt[1] =-0.036; 
    mt.mt[2] =-0.709;
    mt.mt[3] =-0.242; 
    mt.mt[4] =-0.048;
    mt.mt[5] = 0.208;
    mt.sdr1[0] =181.0; 
    mt.sdr1[1] = 47.0;
    mt.sdr1[2] =114.0; 
    mt.sdr2[0] = 328.0;
    mt.sdr2[1] = 48.0;
    mt.sdr2[2] = 67.0;
    mt.tax[0] = 0.82;
    mt.tax[1] = 73.0;
    mt.tax[2] = 166.0;
    mt.bax[0] =-0.05;
    mt.bax[1] = 17.0;
    mt.bax[2] = 344.0;
    mt.pax[0] =-0.77;
    mt.pax[1] = 1.0;
    mt.pax[2] = 74.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 5\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------6------------------------------------//
    if (verb > 0){log_infoF("Testing source 6...\n");}
    mt.exp = 24.0 - 7.0;
    mt.mag = 5.3;
    mt.mt[0] = 0.700;
    mt.mt[1] =-0.883; 
    mt.mt[2] = 0.183;
    mt.mt[3] = 0.260; 
    mt.mt[4] = 0.289;
    mt.mt[5] = 0.712;
    mt.sdr1[0] =329.0; 
    mt.sdr1[1] = 54.0;
    mt.sdr1[2] =141.0; 
    mt.sdr2[0] = 85.0;
    mt.sdr2[1] = 59.0;
    mt.sdr2[2] = 43.0;
    mt.tax[0] = 1.01;
    mt.tax[1] = 51.0;
    mt.tax[2] = 300.0;
    mt.bax[0] = 0.24;
    mt.bax[1] = 39.0;
    mt.bax[2] = 113.0;
    mt.pax[0] =-1.25;
    mt.pax[1] = 3.0;
    mt.pax[2] = 206.;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 6\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------7------------------------------------//
    if (verb > 0){log_infoF("Testing source 7...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 5.0;
    mt.mt[0] = 3.150;
    mt.mt[1] =-2.470; 
    mt.mt[2] =-0.676;
    mt.mt[3] = 1.650; 
    mt.mt[4] = 1.880;
    mt.mt[5] =-1.470;
    mt.sdr1[0] =252.0; 
    mt.sdr1[1] = 28.0;
    mt.sdr1[2] =111.0; 
    mt.sdr2[0] = 49.0;
    mt.sdr2[1] = 64.0;
    mt.sdr2[2] = 79.0;
    mt.tax[0] = 4.08;
    mt.tax[1] = 69.0;
    mt.tax[2] = 297.0;
    mt.bax[0] = 0.01;
    mt.bax[1] = 10.0;
    mt.bax[2] = 54.0;
    mt.pax[0] =-4.08;
    mt.pax[1] = 18.0;
    mt.pax[2] = 147.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 7\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------8------------------------------------//
    if (verb > 0){log_infoF("Testing source 8...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 4.8;
    mt.mt[0] =-1.870;
    mt.mt[1] = 0.488; 
    mt.mt[2] = 1.380;
    mt.mt[3] =-0.057; 
    mt.mt[4] = 0.600;
    mt.mt[5] = 0.664;
    mt.sdr1[0] = 36.0; 
    mt.sdr1[1] = 38.0;
    mt.sdr1[2] =-76.0; 
    mt.sdr2[0] = 199.0;
    mt.sdr2[1] = 53.0;
    mt.sdr2[2] =-101.0;
    mt.tax[0] = 1.80;
    mt.tax[1] = 8.0;
    mt.tax[2] = 296.0;
    mt.bax[0] = 0.18;
    mt.bax[1] = 9.0;
    mt.bax[2] = 205.0;
    mt.pax[0] =-1.99;
    mt.pax[1] = 78.0;
    mt.pax[2] = 69.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){ 
        log_errorF("mopad failed test 8\n");
        return EXIT_FAILURE;
    }   
    //-----------------------------------9------------------------------------//
    if (verb > 0){log_infoF("Testing source 9...\n");}
    mt.exp = 23.0 - 7.0;
    mt.mag = 5.0;
    mt.mt[0] =-3.540;
    mt.mt[1] = 1.040; 
    mt.mt[2] = 2.510;
    mt.mt[3] =-0.474; 
    mt.mt[4] = 1.640;
    mt.mt[5] = 1.530;
    mt.sdr1[0] = 47.0; 
    mt.sdr1[1] = 38.0;
    mt.sdr1[2] =-63.0; 
    mt.sdr2[0] = 195.0;
    mt.sdr2[1] = 56.0;
    mt.sdr2[2] =-109.0;
    mt.tax[0] = 3.66;
    mt.tax[1] = 10.0;
    mt.tax[2] = 299.0;
    mt.bax[0] = 0.45;
    mt.bax[1] = 16.0;
    mt.bax[2] =206.0;
    mt.pax[0] =-4.10;
    mt.pax[1] = 71.0;
    mt.pax[2] = 58.0;
    xscal = pow(10.0,mt.exp);
    cblas_dscal(n6, xscal, mt.mt, incx);
    ierr = decompose(0,mt);
    if (ierr != 0){
        log_errorF("mopad failed test 9\n");
        return EXIT_FAILURE;
    }
    log_infoF("cmopad_test: Success!\n");
    return EXIT_SUCCESS;
}

//============================================================================//

int compareEV(double tol, double e, double *xe, double *xref)
{
    if (fabs(xe[2]/pow(10.0,e) - xref[0]) > tol){
        log_errorF("Error on eigenvalue %f %f\n",xe[2]/pow(10.0,e),xref[0]);
        return -1;
    }else if (fabs(round(xe[1]) - xref[1]) > tol){
        log_errorF("Error on plunge %f %f\n",xe[1], xref[1]);
        return -1;
    }else if (fabs(round(xe[0]) - xref[2]) > tol){
        log_errorF("Error on the azimuth %f %f\n",xe[2],xref[0]);
        return -1;
    }
    return 0;
}

//============================================================================//

int compareSDR(double tol, double *sdr1, double *sdr2, 
               double *sdr1ref, double *sdr2ref)
{
    int i, lfail, lpass, lrev;
    
    // Fault plane 1 may be fault plane 2, so leave the option to reverse
    lpass = 0; //Hope for the worst
    for (lrev=0; lrev < 2; lrev++){
        lfail = 0;
        for (i=0; i<3; i++){
            if (lrev == 0){ 
                if (fabs(sdr1[i] - sdr1ref[i]) > tol){
                    lfail = 1;
                    break;
                } 
                if (fabs(sdr2[i] - sdr2ref[i]) > tol){
                    lfail = 1;
                    break;
                }
            }else{
                if (fabs(sdr2[i] - sdr1ref[i]) > tol){
                    lfail = 1;
                    break;
                } 
                if (fabs(sdr1[i] - sdr2ref[i]) > tol){
                    lfail = 1;
                    break;
                } 
            } // End check on reversal
        }
        if (lfail == 1){
            lpass = 1;
            break;
        } 
    } // Loop  on reverse
    return lpass;
}
//============================================================================//

int decompose(int iverb, struct mt_t mt)
{
    struct cmopad_struct src;
    const char *fcnm = "decompose\0";
    enum cmopad_basis_enum cin, cloc;
    double M[3][3], pax[3], bax[3], tax[3];
    int ierr;
    M[0][0] = mt.mt[0];           //Mrr
    M[1][1] = mt.mt[1];           //Mtt
    M[2][2] = mt.mt[2];           //Mpp
    M[0][1] = M[1][0] = mt.mt[3]; //Mrt
    M[0][2] = M[2][0] = mt.mt[4]; //Mrp
    M[1][2] = M[2][1] = mt.mt[5]; //Mtp
    // Mopad works in North, East, Down frame but inversion is Up, South, East
    cin = USE;
    cloc = NED;
    cmopad_basis__transformMatrixM33(M, cin, cloc); //USE -> NED
    // Compute the isotropic, CLVD, DC decomposition 
    ierr = cmopad_standardDecomposition(M, &src); 
    if (ierr != 0){ 
        log_errorF("%s: Error in decomposition!\n", fcnm);
        return -1; 
    }  
    // Compute the princple axes with corresponding strikes, dips, and rakes 
    ierr = cmopad_MT2PrincipalAxisSystem(iverb, &src);
    if (ierr != 0){
        log_errorF("%s: Error computing principal axis\n",fcnm);
        return -1;
    }
    // Compute the pressure, null, and, tension principal axes as len,az,plunge
    cin = NED; //MoPaD is in north, east, down 
    ierr = cmopad_Eigenvector2PrincipalAxis(cin, src.eig_pnt[0],
                                            src.p_axis,    pax);
    if (ierr != 0){
        log_errorF("%s: Error converting pax\n",fcnm);
        return -1;
    }   
    ierr = cmopad_Eigenvector2PrincipalAxis(cin, src.eig_pnt[1],
                                            src.null_axis, bax);
    if (ierr != 0){
        log_errorF("%s: Error converting bax\n",fcnm);
        return -1;
    }
    ierr = cmopad_Eigenvector2PrincipalAxis(cin, src.eig_pnt[2],
                                            src.t_axis,    tax);
    if (ierr != 0){
        log_errorF("%s: Error converting tax\n",fcnm);
        return -1;
    }
    // Check the strike/dip/rake
    if (compareSDR(1.0, src.fp1, src.fp2, mt.sdr1, mt.sdr2) != 1){
        log_errorF("%s: Error computing strike/dip/rake\n",fcnm);
        return -1;
    }
    //printf("%f\n",src.seismic_moment/(pow(10.0,mt.exp)));
    if (fabs(src.moment_magnitude - mt.mag) > 0.1){
        log_errorF("%s: Error on magnitude\n");
        return -1;
    }
    if (compareEV(0.01, mt.exp, tax, mt.tax) != 0){
        log_errorF("%s: Error in tax\n",fcnm);
        return -1;
    }
    if (compareEV(0.01, mt.exp, bax, mt.bax) != 0){
        log_errorF("%s: Error in bax\n",fcnm);
        return -1;
    }
    if (compareEV(0.01, mt.exp, pax, mt.pax) != 0){
        log_errorF("%s: Error in pax\n");
        return -1;
    }
/*
    printf("%f %f %f\n",tax[2]/(pow(10.0,mt.exp)),tax[1],tax[0]);
    printf("%f %f %f\n",bax[2]/(pow(10.0,mt.exp)),bax[1],bax[0]);
    printf("%f %f %f\n",pax[2]/(pow(10.0,mt.exp)),pax[1],pax[0]);
    printf("%f %f %f\n",src.fp1[0],src.fp1[1],src.fp1[2]);
    printf("%f %f %f\n",src.fp2[0],src.fp2[1],src.fp2[2]);
*/
    return 0;
}
