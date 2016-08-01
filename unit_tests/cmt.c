#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast.h"

static bool lequal(double a, double b, double tol)
{
    if (a == 0.0 && b == 0.0){return true;}
    if (fabs(a - b)/fabs(a + b) > tol){return false;}
    return true;
}

int cmt_greens_test()
{
    const char *fcnm = "cmt_greens_test\0";
    const int l1 = 4;
    const double x1[4] = {26739.10093066, -22080.60946829,
                          172341.17727517, 89347.93966518};
    const double y1[4] = {65206.90127427, -104816.33293538,
                          15936.72789605, 37791.23706562};
    const double z1[4] = {-1039.536171, -1180.767,
                          -2184.68922, -1118.946446};
    const double Gref[3*4*5]= {4.14238313e-22, -1.61043451e-23, -1.64545230e-22,
                               2.58162686e-22, -5.52745034e-24,
                               3.06830559e-22, -5.52745034e-24, -6.74743229e-23,
                               3.83452299e-23, -4.89152924e-24,
                              -5.52745034e-24,  1.64867566e-22, -1.71290667e-27,
                              -2.80320397e-24,  6.76065018e-23,
                              -1.07114045e-22, -5.72795465e-24,  7.53643367e-23,
                              -1.41160425e-22, -1.02773811e-24,
                              -9.46107508e-23, -1.02773811e-24,  1.58762517e-23,
                              -1.38548157e-23, -1.06580004e-24,
                              -1.02773811e-24, -7.54467746e-23, -3.09557167e-28,
                              -1.16553865e-24, -1.58936181e-23,
                               3.08791337e-23, -3.91440464e-25, -2.71594759e-24,
                              -2.64749287e-24, -2.04886701e-25,
                               1.88799182e-23, -2.04886701e-25, -2.93704961e-23,
                              -5.80147604e-23, -2.58815699e-24,
                              -2.04886701e-25,  2.71984347e-24, -1.78022499e-28,
                               5.49179132e-25,  2.94126265e-23,
                               1.65271763e-22, -2.06977634e-24, -3.65770788e-23,
                              -1.99279630e-23, -2.33197132e-24,
                               2.22799729e-22, -2.33197132e-24, -8.64773658e-23,
                              -1.33626579e-22, -6.59679291e-24,
                              -2.33197132e-24,  3.66208854e-23, -4.32350695e-28,
                               1.13175414e-24, 8.65809354e-23};

    double G[(3*4)*5];
    int i, ierr, indx, j;
    ierr = GFAST_CMT__setForwardModel__deviatoric(l1, y1, x1, z1, G); 
    if (ierr != 0)
    {
        log_errorF("%s: Error setting forward model!\n", fcnm);
        return EXIT_FAILURE;
    }
    indx = 0;
    for (i=0; i<3*l1; i++)
    {
        for (j=0; j<5; j++)
        {
            //if (fabs( (G[indx] - Gref[indx])/Gref[indx] ) > 1.e-8)
            if (!lequal(G[indx], Gref[indx], 1.e-8))
            {
                log_errorF("%s: Failed to set deviatoric G %e %d\n",
                           fcnm, G[indx], Gref[indx]);
                return EXIT_FAILURE;
            }
            indx = indx + 1;
        }
    }
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}

static int read_results(const char *filenm,
                        struct GFAST_cmt_props_struct *cmt_props,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_cmtResults_struct *cmt,
                        double *SA_lat, double *SA_lon, double *SA_dep)
{
    FILE *infl;
    char cline[128];
    double mxx, myy, mzz, mxy, mxz, myz;
    int i, idep, ierr, ldevi;
    ierr = 1;
    infl = fopen(filenm, "r");
    // line 1
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%d %d %d %d %d %lf %lf\n",
           &cmt_data->nsites, &cmt_props->ngridSearch_deps,
           &cmt_props->utm_zone, &cmt_props->min_sites, 
           &ldevi, &cmt_props->window_vel,
           &cmt_props->window_avg);
    cmt_props->ldeviatoric = true;
    if (ldevi != 1){cmt_props->ldeviatoric = false;}
    // line 2
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf\n", SA_lat, SA_lon, SA_dep);
    cmt_data->ubuff = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->ebuff = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->nbuff = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->wtu = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->wte = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->wtn = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->sta_lat = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->sta_lon = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->sta_alt = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt_data->lactive = GFAST_memory_calloc__bool(cmt_data->nsites);
    cmt_data->lmask = GFAST_memory_calloc__bool(cmt_data->nsites);
    // cmt data
    for (i=0; i<cmt_data->nsites; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf\n",
               &cmt_data->sta_lat[i], &cmt_data->sta_lon[i],
               &cmt_data->sta_alt[i],
               &cmt_data->nbuff[i], &cmt_data->ebuff[i], &cmt_data->ubuff[i]);
        cmt_data->wtu[i] = 1.0;
        cmt_data->wte[i] = 1.0;
        cmt_data->wtn[i] = 1.0;
        cmt_data->lactive[i] = true;
    }
    // Results + depths in grid search
    cmt->nsites = cmt_data->nsites;
    cmt->ndeps = cmt_props->ngridSearch_deps;
    cmt->objfn = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->mts = GFAST_memory_calloc__double(cmt->ndeps*6);
    cmt->str1 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->str2 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->dip1 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->dip2 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->rak1 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->rak2 = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->Mw = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->srcDepths = GFAST_memory_calloc__double(cmt->ndeps);
    cmt->EN = GFAST_memory_calloc__double(cmt->ndeps*cmt_data->nsites);
    cmt->NN = GFAST_memory_calloc__double(cmt->ndeps*cmt_data->nsites);
    cmt->UN = GFAST_memory_calloc__double(cmt->ndeps*cmt_data->nsites);
    cmt->Einp = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt->Ninp = GFAST_memory_calloc__double(cmt_data->nsites);
    cmt->Uinp = GFAST_memory_calloc__double(cmt_data->nsites);
    for (idep=0; idep<cmt->ndeps; idep++)
    {
        // depth
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf\n", &cmt->srcDepths[idep]);
        // moment tensor
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf\n",
               &mxx, &myy, &mzz, &mxy, &mxz, &myz);
        cmt->mts[6*idep+0] = mxx;
        cmt->mts[6*idep+1] = myy;
        cmt->mts[6*idep+2] = mzz;
        cmt->mts[6*idep+3] = mxy;
        cmt->mts[6*idep+4] = mxz;
        cmt->mts[6*idep+5] = myz;
        // rest of information
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
               &cmt->str1[idep], &cmt->dip1[idep], &cmt->rak1[idep],
               &cmt->str2[idep], &cmt->dip2[idep], &cmt->rak2[idep],
               &cmt->Mw[idep],   &cmt->objfn[idep]);
        if (cmt->rak1[idep] > 180.0){cmt->rak1[idep] = cmt->rak1[idep] - 360.0;}
        if (cmt->rak2[idep] > 180.0){cmt->rak2[idep] = cmt->rak2[idep] - 360.0;}
    }
    fclose(infl);
    ierr = 0;
ERROR:;
    return ierr;
}
//============================================================================//
int cmt_inversion_test()
{
    const char *fcnm = "cmt_inversion_test\0";
    const char *filenm = "files/final_cmt.maule.txt\0";
    struct GFAST_cmt_props_struct cmt_props;
    struct GFAST_offsetData_struct cmt_data;
    struct GFAST_cmtResults_struct cmt_ref, cmt; 
    double SA_lat, SA_lon, SA_dep;
    int i, ierr, j;
    memset(&cmt_props, 0, sizeof(cmt_props));
    memset(&cmt_data, 0, sizeof(cmt_data));
    memset(&cmt_ref, 0, sizeof(cmt_ref));
    memset(&cmt, 0, sizeof(cmt));
    ierr = read_results(filenm,
                        &cmt_props,
                        &cmt_data,
                        &cmt_ref,
                        &SA_lat, &SA_lon, &SA_dep);
    if (ierr != 0)
    {   
        log_errorF("%s: Error reading input file\n", fcnm);
        return EXIT_FAILURE;
    }
    // Set space
    cmt.nsites = cmt_ref.nsites;
    cmt.ndeps = cmt_ref.ndeps;
    cmt.objfn = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.mts = GFAST_memory_calloc__double(cmt.ndeps*6);
    cmt.str1 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.str2 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.dip1 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.dip2 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.rak1 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.rak2 = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.Mw = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.srcDepths = GFAST_memory_calloc__double(cmt.ndeps);
    cmt.EN = GFAST_memory_calloc__double(cmt.ndeps*cmt_data.nsites);
    cmt.NN = GFAST_memory_calloc__double(cmt.ndeps*cmt_data.nsites);
    cmt.UN = GFAST_memory_calloc__double(cmt.ndeps*cmt_data.nsites);
    cmt.Einp = GFAST_memory_calloc__double(cmt_data.nsites);
    cmt.Ninp = GFAST_memory_calloc__double(cmt_data.nsites);
    cmt.Uinp = GFAST_memory_calloc__double(cmt_data.nsites);
    cmt.lsiteUsed = GFAST_memory_calloc__bool(cmt_data.nsites);
    for (i=0; i<cmt.ndeps; i++)
    {
        cmt.srcDepths[i] = cmt_ref.srcDepths[i];
    }
    ierr = GFAST_CMT__driver(cmt_props,
                             SA_lat, SA_lon, SA_dep,
                             cmt_data,
                             &cmt);
    if (ierr != CMT_SUCCESS)
    {
        log_errorF("%s: Error computing CMT\n", fcnm);
        return EXIT_FAILURE;
    }
    else
    {
        for (i=0; i<cmt.ndeps; i++)
        {
            if (!lequal(cmt.objfn[i], cmt_ref.objfn[i], 1.e-4))
            {
                log_errorF("%s: Error objfn is wrong %f %f %f\n", fcnm,
                           cmt.srcDepths[i], cmt.objfn[i], cmt_ref.objfn[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(cmt.Mw[i], cmt_ref.Mw[i], 1.e-3))
            {
                log_errorF("%s: Error Mw is wrong %f %f %f\n", fcnm,
                           cmt.srcDepths[i], cmt.Mw[i], cmt_ref.Mw[i]);
                return EXIT_FAILURE;
            } 
            if (!lequal(cmt.str1[i], cmt_ref.str1[i], 1.e-4) &&
                !lequal(cmt.str1[i], cmt_ref.str2[i], 1.e-4))
            {
                log_errorF("%s: Error str1 is wrong %f %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.str1[i], cmt_ref.str1[i], cmt_ref.str2[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(cmt.str2[i], cmt_ref.str1[i], 1.e-4) &&
                !lequal(cmt.str2[i], cmt_ref.str2[i], 1.e-4))
            {
                log_errorF("%s: Error str2 is wrong %f %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.str2[i], cmt_ref.str1[i], cmt_ref.str2[i]);
                return EXIT_FAILURE;
            }

            if (!lequal(cmt.dip1[i], cmt_ref.dip1[i], 1.e-4) &&
                !lequal(cmt.dip1[i], cmt_ref.dip2[i], 1.e-4))
            {
                log_errorF("%s: Error dip1 is wrong %f %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.dip1[i], cmt_ref.dip1[i], cmt_ref.dip2[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(cmt.dip2[i], cmt_ref.dip1[i], 1.e-4) &&
                !lequal(cmt.dip2[i], cmt_ref.dip2[i], 1.e-4))
            {
                log_errorF("%s: Error dip2 is wrong %f %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.dip2[i], cmt_ref.dip1[i], cmt_ref.dip2[i]);
                return EXIT_FAILURE;
            }

            if (!lequal(cmt.rak1[i], cmt_ref.rak1[i], 1.e-4) &&
                !lequal(cmt.rak1[i], cmt_ref.rak2[i], 1.e-4))
            {
                log_errorF("%s: Error rak1 is wrong %f %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.rak1[i], cmt_ref.rak1[i], cmt_ref.rak2[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(cmt.rak2[i], cmt_ref.rak1[i], 1.e-4) &&
                !lequal(cmt.rak2[i], cmt_ref.rak2[i], 1.e-4))
            {
                log_errorF("%s: Error rak2 is wrong %f %f %f\n", fcnm,
                           cmt.srcDepths[i],
                           cmt.rak2[i], cmt_ref.rak1[i], cmt_ref.rak2[i]);
                return EXIT_FAILURE;
            }

            for (j=0; j<6; j++)
            {
                if (!lequal(cmt.mts[6*i+j], cmt_ref.mts[6*i+j], 1.e-4))
                {
                    log_errorF("%s: Error mt %d %f %f %f\n", fcnm, j+1,
                               cmt.srcDepths[i],
                               cmt.mts[6*i+j], cmt_ref.mts[6*i+j]); 
                    return EXIT_FAILURE;
                }
            } 
        }
    }
    // Clean up
    GFAST_memory_freeOffsetData(&cmt_data);
    GFAST_memory_freeCMTResults(&cmt);
    GFAST_memory_freeCMTResults(&cmt_ref);
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS; 
}
/*
int cmt_greens_test2()
{
    const char *fcnm = "cmt_greens_test\0";
    double SA_lat = 47.19;
    double SA_lon =-122.66;
    double SA_dep = 57.0;
    // stations: bamf, cabl, chzz, eliz, holb, neah, p058, p159
    double stla[9] = {  48.83532872,  42.83609887,  45.48651503,
                        49.87305293,  50.64035267,  41.90232489,
                        48.29785467,  40.87630594,  40.50478709};
    double stlo[9] = {-125.13510527,-124.56334629,-123.97812400,
                      -127.12266484,-128.13499899,-120.30283244,
                      -124.62490719,-124.07537043,-124.28278289};
    double Gref[3*9*6] = {-3.76788285e-23,2.17200612e-23,7.38917316e-24,
                          -3.69575363e-24,9.19341257e-24,3.68430600e-24,
                           1.18952339e-23,4.28928186e-24,-7.49369841e-24,
                          -4.57397090e-24,-9.21633513e-24,3.88971755e-24,
                           4.98469330e-24,-2.67599785e-24,3.68814296e-25,
                          -1.07631540e-25,-3.82932035e-23,5.37907498e-25,
                          -4.64693902e-24,2.63360089e-24,9.87413584e-25,
                          -6.40046383e-24,-9.23642564e-25,-4.82546578e-26,
                           3.31513449e-25,-6.65963506e-24,3.10353892e-24,
                          -4.59933863e-24,-8.52310708e-25,-1.60358542e-24,
                          -4.14753894e-27,3.70756519e-25,1.42984298e-26,
                           5.32162386e-25,5.93921791e-24,8.38897297e-24,
                          -3.98036435e-23,2.49534491e-23,6.61176100e-24,
                          -2.37850801e-23,-1.32129523e-23,1.02073223e-24,
                          -1.12262305e-23,-1.62196364e-23,1.22197398e-23,
                          -2.14072895e-23,-1.38546256e-23,-1.42794211e-23,
                           4.42862233e-24,-7.40785049e-25,9.07307944e-25,
                           5.89752917e-24,4.63361017e-23,3.02831949e-23,
                          -1.36728200e-23,7.49645708e-24,3.01134323e-24,
                          -2.34318580e-24,1.96919291e-24,8.71419521e-25,
                           4.27432940e-24,1.60248772e-24,-2.86529689e-24,
                          -1.22014996e-24,-1.95053245e-24,7.15897946e-25,
                           1.08941927e-24,-5.56273136e-25,2.72077946e-26,
                           8.20510513e-26,-1.33021504e-23,-6.61579079e-25,
                          -8.93543645e-24,5.03872376e-24,1.91736618e-24,
                          -8.04470383e-25,1.01761957e-24,3.95300642e-25,
                           2.97858443e-24,9.91588433e-25,-1.95351205e-24,
                          -1.08297157e-24,-1.02132988e-24,4.25717033e-25,
                           5.71034265e-25,-2.89910679e-25,9.08753427e-27,
                          -1.60858449e-26,-8.56417102e-24,1.59956650e-25,
                           3.38098407e-24,-1.92485219e-24,-7.18691185e-25,
                          -4.28778207e-24,-5.25513015e-25,2.46259689e-26,
                           6.16417402e-27,-4.30089841e-24,2.11971709e-24,
                           3.19946651e-24,5.00140562e-25,-8.76975885e-25,
                           2.12900578e-26,1.85640975e-25,5.39850278e-27,
                          -3.04915905e-25,-4.22585690e-24,5.51550904e-24,
                          -6.09188495e-23,3.26275262e-23,1.21889368e-23,
                          -1.94038770e-23,2.11378719e-23,1.16830517e-23,
                           1.51280000e-23,8.92375321e-24,-1.03623750e-23,
                          -3.35219222e-24,-2.06213013e-23,6.32554218e-24,
                           1.05770380e-23,-5.82701331e-24,1.52507007e-24,
                           2.74793179e-24,-6.76479308e-23,-1.10307640e-23,
                          -1.18759281e-24,6.24442891e-25,2.78846658e-25,
                          -3.45358390e-24,-2.96736690e-25,-1.87255386e-26,
                           1.22489136e-24,-4.61613604e-24,1.67919271e-24,
                          -1.56852420e-24,-1.77979783e-25,-6.62279883e-25,
                          -9.58969391e-26,2.32484462e-25,2.67281171e-27,
                           1.26965020e-25,1.69354114e-24,4.95857130e-24,
                          -1.15785646e-24,6.12290428e-25,2.70436957e-25,
                          -3.06417042e-24,-2.51328501e-25,-1.65007738e-26,
                           1.00844814e-24,-4.01842048e-24,1.49204261e-24,
                          -1.49683649e-24,-1.61768298e-25,-5.50134948e-25,
                          -7.37834672e-26,1.88438674e-25,1.98928199e-27,
                          1.14351108e-25,1.63354244e-24,4.35821132e-24};
    double *backazi, *G, *xrs, *yrs, *zrs, *azims, azi, x1, x2, y1, y2;
    int l1 = 9;
    int i, ierr, indx, j, ldg;
    int zone = 10;
    bool lnorth;
    const double pi180i = 180.0/M_PI;
    const int n6 = 6;

    ldg = 3*l1;
    xrs = (double *)calloc(l1, sizeof(double));
    yrs = (double *)calloc(l1, sizeof(double));
    zrs = (double *)calloc(l1, sizeof(double));
    backazi = (double *)calloc(l1, sizeof(double));
    azims = (double *)calloc(l1, sizeof(double));
    G = (double *)calloc(ldg*n6, sizeof(double));

    ierr = geodetic_coordtools_ll2utm(SA_lat, SA_lon,
                                      &x2, &y2,
                                      &lnorth, &zone);
    if (ierr != 0){ 
        printf("%s: Failed to convert source ll2utm\n", fcnm);
        return -1; 
    }

    for (i=0; i<l1; i++){
        ierr = geodetic_coordtools_ll2utm(stla[i], stlo[i],
                                          &xrs[i], &yrs[i],
                                          &lnorth, &zone);
        if (ierr != 0){ 
            printf("%s: Failed to convert recv ll2utm\n", fcnm);
            return -1; 
        }
    }
    for (i=0; i<l1; i++){
        // Compute the station UTM location
        x1 = xrs[i];
        y1 = yrs[i];
        // Compute azimuth and back-azimuth 
        azi = 90.0 - pi180i*atan2(x2-x1, y2-y1);
        if (azi  < 0.0)  {azi = azi + 360.0;}
        if (azi  > 360.0){azi = azi - 360.0;}
        if (azi  < 180.0){backazi[i] = azi + 180.0;}
        if (azi  > 180.0){backazi[i] = azi - 180.0;}
        if (azi == 180.0){backazi[i] = 0.0;}
        // Relative source receiver location in (x,y,z) coordinates
        xrs[i] = x1 - x2; 
        yrs[i] = y1 - y2; 
        zrs[i] = SA_dep*1000.0;
        azims[i] = 90.0 - azi + 180.0;
    } // Loop on stations
    // Compute the corresponding Green's functions
    G = (double *)calloc(ldg*n6, sizeof(double));
    ierr = GFAST_CMT__setForwardModel(l1, false, 
                                      xrs, yrs, zrs, azims,
                                       G);
    if (ierr != 0){
        printf("%s: Failed to compute greens functions\n", fcnm);
        return -1;
    }
    indx = 0;
    for (i=0; i<ldg; i++){
        for (j=0; j<6; j++){
            if (fabs(G[indx] - Gref[indx])/Gref[indx] > 1.e-7){
                printf("%s: Failed to compute G!\n", fcnm); 
                printf("%e %e\n",G[indx],Gref[indx]);
                return -1;
            }
            indx = indx + 1;
        }
    }

    free(G);
    free(xrs);
    free(yrs);
    free(zrs);
    free(backazi);
    free(azims);
    printf("%s: Success!\n", fcnm);
    return 0;
}
*/
