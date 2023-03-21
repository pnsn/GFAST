/**
 * @file CoreScalingUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/scaling directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"

/**
 * Test the pgd scaling function. Based on gfast/unit_tests/pgd.c by Ben Baker
 */
TEST(CoreScaling, testPGDScaling) {
    int verbose = 4;


    // Nisqually?
    
    double dist_tol = 0.5;
    double disp_def = 0.01;
    double IQR, M, VR;
    double SA_lat = 47.19;
    double SA_lon =-122.66;
    double SA_dep = 57.0;
    // stations: bamf, cabl, chzz, eliz, holb, neah, p058, p159
    int l1 = 9;
    double wts[9];
    double stla[9] = {  48.83532872,  42.83609887,  45.48651503,
                        49.87305293,  50.64035267,  41.90232489,
                        48.29785467,  40.87630594,  40.50478709};
    double stlo[9] = {-125.13510527,-124.56334629,-123.97812400,
                      -127.12266484,-128.13499899,-120.30283244,
                      -124.62490719,-124.07537043,-124.28278289};
    double staAlt[9] = {0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0};
    double utmRecvNorthing[9], utmRecvEasting[9], Uest[9*1];
    double d[9] = {4, 7.1, 8,  3, 1, 9,  6, 5.5, 6.5};
    double srdist[9];
    double SA_xutm, SA_yutm, utmSrcEasting, utmSrcNorthing, xutm, yutm;
    int zone = 10;
    
    // Tohoku
    // double dist_tol = 0.5;
    // double disp_def = 0.01;
    // double IQR, M, VR;
    // double SA_lat = 38.1882;
    // double SA_lon = 142.8821;
    // double SA_dep = 9.0;
    // // stations: 0550, 0036, 0171, 0175, 0172, 0918, 0549
    // int l1 = 7;
    // double wts[7];
    // double stla[7] = {  38.3012,  38.4492,  39.0238,  38.6827,
    //                     38.9029,  38.5098,  38.4251};
    // double stlo[7] = { 141.5007, 141.4412, 141.7398, 141.4494,
    //                    141.5726, 141.3044, 141.2129};
    // double staAlt[7] = {0.0, 0.0, 0.0, 0.0,
    //                     0.0, 0.0, 0.0};
    // double utmRecvNorthing[7], utmRecvEasting[7], Uest[7*1];
    // // see 15:34:13:546
    // double d[7] = {95.702998, 87.900208, 23.212499,  50.531524,
    //                29.145128, 55.347474, 46.665574};
    // double srdist[7];
    // double SA_xutm, SA_yutm, utmSrcEasting, utmSrcNorthing, xutm, yutm;
    // int zone = 54;

    // Begin unchanged test code
    const int ndeps = 1;
    bool lnorth;
    int i, ierr;
    ierr = 0;
    // Compute source location
    GFAST_core_coordtools_ll2utm(SA_lat, SA_lon,
                                 &SA_yutm, &SA_xutm,
                                 &lnorth, &zone);
    utmSrcNorthing = SA_yutm;
    utmSrcEasting = SA_xutm;
    // Compute station locations 
    for (i=0; i<l1; i++)
    {
        GFAST_core_coordtools_ll2utm(stla[i], stlo[i],
                                     &yutm, &xutm,
                                     &lnorth, &zone);
        utmRecvNorthing[i] = yutm;
        utmRecvEasting[i] = xutm;
        wts[i] = 1.0;
    }
    // Compute scaling
    ierr = GFAST_core_scaling_pgd_depthGridSearch(l1, ndeps,
                                              verbose,
                                              dist_tol,
                                              disp_def,
                                              utmSrcEasting,
                                              utmSrcNorthing,
                                              &SA_dep,
                                              utmRecvEasting,
                                              utmRecvNorthing,
                                              staAlt,
                                              d, wts, srdist,
                                              &M, &VR, &IQR, Uest);

    EXPECT_EQ(0, ierr);
    // EXPECT_NEAR(6.6379784, M, 1.e-4);
    // EXPECT_NEAR(6.1681204583368228, VR, 1.e-6);

    std::cout << "mag: " << M << std::endl;
    std::cout << "vr:  " << VR << std::endl;
    std::cout << "iqr: " << IQR << std::endl;
}

/**
 * Test the readSigmaLookupFile function that reads in the lookup table
 */
TEST(CoreScaling, testReadSigmaLookupFile) {
    const char *sigmaLookupFile;
    int ierr;
    struct GFAST_pgd_props_struct pgd_props;

    memset(&pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));

    sigmaLookupFile = "data/M99.txt\0"; 
    ierr = core_scaling_readSigmaLookupFile(sigmaLookupFile,
                                            &pgd_props);

    EXPECT_EQ(0, ierr);
    EXPECT_EQ(24, pgd_props.n99);
    EXPECT_DOUBLE_EQ(5, pgd_props.t99[0]);
    EXPECT_DOUBLE_EQ(5.79, pgd_props.m99[0]);
    EXPECT_DOUBLE_EQ(120, pgd_props.t99[23]);
    EXPECT_DOUBLE_EQ(8.18, pgd_props.m99[23]);
}

/**
 * Test the readSigmaLookupFile function that reads in the lookup table
 */
TEST(CoreScaling, testReadPgdThresholdLookupFile) {
    const char *pgdThresholdLookupFile;
    int ierr;
    struct GFAST_pgd_props_struct pgd_props;

    memset(&pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));

    pgdThresholdLookupFile = "data/pgd_threshold_PW.txt\0"; 
    ierr = core_scaling_readPgdThresholdLookupFile(pgdThresholdLookupFile,
                                                   &pgd_props);

    EXPECT_EQ(0, ierr);
    EXPECT_EQ(4, pgd_props.n_throttle);
    EXPECT_DOUBLE_EQ(0, pgd_props.throttle_time_threshold[0]);
    EXPECT_DOUBLE_EQ(17.5, pgd_props.throttle_pgd_threshold[0]);
    EXPECT_EQ(3, pgd_props.throttle_num_stations[0]);
    EXPECT_DOUBLE_EQ(70.2, pgd_props.throttle_time_threshold[3]);
    EXPECT_DOUBLE_EQ(95, pgd_props.throttle_pgd_threshold[3]);
    EXPECT_EQ(3, pgd_props.throttle_num_stations[3]);
}

/**
 * Test the readRawSigmaThresholdLookupFile function that reads in the lookup table
 */
TEST(CoreScaling, testReadRawSigmaThresholdLookupFile) {
    const char *rawSigmaThresholdLookupFile;
    int ierr;
    struct GFAST_pgd_props_struct pgd_props;

    memset(&pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));

    rawSigmaThresholdLookupFile = "data/raw_sigma_threshold_PW.txt\0"; 
    ierr = core_scaling_readRawSigmaThresholdLookupFile(
        rawSigmaThresholdLookupFile,
        &pgd_props);

    EXPECT_EQ(0, ierr);
    EXPECT_DOUBLE_EQ(30, pgd_props.u_raw_sigma_threshold);
    EXPECT_DOUBLE_EQ(15, pgd_props.n_raw_sigma_threshold);
    EXPECT_DOUBLE_EQ(12, pgd_props.e_raw_sigma_threshold);
}

/**
 * Test the readRawSigmaThresholdLookupFile function that reads in the lookup table
 */
TEST(CoreScaling, testReadRawSigmaThresholdLookupFileNoExist) {
    const char *rawSigmaThresholdLookupFile;
    int ierr;
    struct GFAST_pgd_props_struct pgd_props;

    memset(&pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));

    rawSigmaThresholdLookupFile = "data/file_does_not_exist.txt\0"; 
    ierr = core_scaling_readRawSigmaThresholdLookupFile(
        rawSigmaThresholdLookupFile,
        &pgd_props);

    EXPECT_EQ(0, ierr);
    EXPECT_DOUBLE_EQ(-1, pgd_props.u_raw_sigma_threshold);
    EXPECT_DOUBLE_EQ(-1, pgd_props.n_raw_sigma_threshold);
    EXPECT_DOUBLE_EQ(-1, pgd_props.e_raw_sigma_threshold);
}