/**
 * @file CoreWaveformProcessorUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/waveformProcessor directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "iscl/memory/memory.h"
#include "gfast.h"


/* 
 * Test the peakDisplacementHelper function. With input data and uncertainty
 * buffers, return the peak displacement and the maximum uncertainties at
 * the initial reference time and the peak displacement time.
 */
void testPDHelper_helper(int npts, double *ubuff, double *nbuff, double *ebuff,
    double *usigmabuff, double *nsigmabuff, double *esigmabuff,
    double expPeakDisp, double expObsTime,
    double expUMaxSigma, double expNMaxSigma, double expEMaxSigma) 
{
    // Input values
    double dt, ev_time, epoch, tmin, tmax;
    int nMaxLeader;
    // Return values
    double peakDisp, obsTime, uMaxUncertainty, nMaxUncertainty, eMaxUncertainty;

    // Initialize inputs
    dt = 1.0;
    ev_time = 0.0; // Event origin time
    epoch = 2.0; // First time in the time data buffer
    tmin = 2; // distance / svel_window (default 3 km/s)
    tmax = 10; // distance / min_svel_window (default 0.01 km/s)
    nMaxLeader = 2;

    // Initialize outputs to verify they change?
    obsTime = -1;
    uMaxUncertainty = -1;
    nMaxUncertainty = -1;
    eMaxUncertainty = -1;

    peakDisp = core_waveformProcessor_peakDisplacementHelper(
        npts,
        dt,
        ev_time,
        epoch,
        ubuff,
        nbuff,
        ebuff,
        usigmabuff,
        nsigmabuff,
        esigmabuff,
        nMaxLeader,
        tmin,
        tmax,
        &obsTime,
        &uMaxUncertainty,
        &nMaxUncertainty,
        &eMaxUncertainty
    );

    EXPECT_DOUBLE_EQ(expPeakDisp, peakDisp);
    EXPECT_DOUBLE_EQ(expObsTime, obsTime);
    EXPECT_DOUBLE_EQ(expUMaxSigma, uMaxUncertainty);
    EXPECT_DOUBLE_EQ(expNMaxSigma, nMaxUncertainty);
    EXPECT_DOUBLE_EQ(expEMaxSigma, eMaxUncertainty);

}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperBasic) {
    const int NPTS = 4;
    double ubuff[NPTS] = {1, 2, 3, 4};
    double nbuff[NPTS] = {1, 2, 3, 4};
    double ebuff[NPTS] = {1, 2, 3, 4};
    double usigmabuff[NPTS] = {1, 2, 3, 4};
    double nsigmabuff[NPTS] = {1, 2, 3, 4};
    double esigmabuff[NPTS] = {1, 2, 3, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(4 - 1, 2) + pow(4 - 1, 2) + pow(4 - 1, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        usigmabuff, nsigmabuff, esigmabuff,
        expPeakDisp, 5, 4, 4, 4);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperEarlyNans) {
    const int NPTS = 4;
    double ubuff[NPTS] = {NAN, 2, 3, 4};
    double nbuff[NPTS] = {NAN, 2, 3, 4};
    double ebuff[NPTS] = {NAN, 2, 3, 4};
    double usigmabuff[NPTS] = {10, 2, 3, 4};
    double nsigmabuff[NPTS] = {1, 21, 3, 4};
    double esigmabuff[NPTS] = {1, 2, 3, 3};
    int npts = NPTS;

    double expPeakDisp = pow(pow(4 - 2, 2) + pow(4 - 2, 2) + pow(4 - 2, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        usigmabuff, nsigmabuff, esigmabuff,
        expPeakDisp, 5, 4, 21, 3);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperNanSigmas) {
    const int NPTS = 4;
    double ubuff[NPTS] = {1, 2, 3, 4};
    double nbuff[NPTS] = {1, 2, 3, 4};
    double ebuff[NPTS] = {1, 2, 3, 4};
    double usigmabuff[NPTS] = {NAN};
    double nsigmabuff[NPTS] = {NAN};
    double esigmabuff[NPTS] = {NAN};
    int npts = NPTS;

    double expPeakDisp = pow(pow(4 - 1, 2) + pow(4 - 1, 2) + pow(4 - 1, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        usigmabuff, nsigmabuff, esigmabuff,
        expPeakDisp, 5, 0, 0, 0);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperMiddlePeak) {
    const int NPTS = 4;
    double ubuff[NPTS] = {1, 2, 5, 4};
    double nbuff[NPTS] = {1, 2, 3, 4};
    double ebuff[NPTS] = {1, 2, 5, 4};
    double usigmabuff[NPTS] = {1, 2, 3, 4};
    double nsigmabuff[NPTS] = {1, 2, 3, 4};
    double esigmabuff[NPTS] = {1, 2, 3, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(5 - 1, 2) + pow(3 - 1, 2) + pow(5 - 1, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        usigmabuff, nsigmabuff, esigmabuff,
        expPeakDisp, 4, 3, 3, 3);
}
