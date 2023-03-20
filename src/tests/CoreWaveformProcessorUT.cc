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
    double expPeakDisp, int expIRef, int expIPeak) 
{
    // Input values
    double dt, ev_time, epoch, tmin, tmax;
    int nMaxLeader;
    // Return values
    double peakDisp, obsTime, expObsTime;
    int iRef, iPeak;

    // Initialize inputs
    dt = 1.0;
    ev_time = 0.0; // Event origin time
    epoch = 2.0; // First time in the time data buffer
    tmin = 2; // distance / svel_window (default 3 km/s)
    tmax = 10; // distance / min_svel_window (default 0.01 km/s)
    nMaxLeader = 2;

    // The observation time is based on the input epoch time
    expObsTime = epoch + expIPeak * dt;

    // Initialize outputs to verify they change?
    obsTime = -1;
    iRef = -1;
    iPeak = -1;

    peakDisp = core_waveformProcessor_peakDisplacementHelper(
        npts,
        dt,
        ev_time,
        epoch,
        ubuff,
        nbuff,
        ebuff,
        nMaxLeader,
        tmin,
        tmax,
        &obsTime,
        &iRef,
        &iPeak
    );

    EXPECT_DOUBLE_EQ(expPeakDisp, peakDisp);
    EXPECT_DOUBLE_EQ(expObsTime, obsTime);
    EXPECT_EQ(expIRef, iRef);
    EXPECT_EQ(expIPeak, iPeak);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperBasic) {
    const int NPTS = 4;
    double ubuff[NPTS] = {1, 2, 3, 4};
    double nbuff[NPTS] = {1, 2, 3, 4};
    double ebuff[NPTS] = {1, 2, 3, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(4 - 1, 2) + pow(4 - 1, 2) + pow(4 - 1, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        expPeakDisp, 0, 3);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperEarlyNans) {
    const int NPTS = 4;
    double ubuff[NPTS] = {NAN, 2, 3, 4};
    double nbuff[NPTS] = {NAN, 2, 3, 4};
    double ebuff[NPTS] = {NAN, 2, 3, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(4 - 2, 2) + pow(4 - 2, 2) + pow(4 - 2, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        expPeakDisp, 1, 3);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperMiddlePeak) {
    const int NPTS = 4;
    double ubuff[NPTS] = {1, 2, 5, 4};
    double nbuff[NPTS] = {1, 2, 3, 4};
    double ebuff[NPTS] = {1, 2, 5, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(5 - 1, 2) + pow(3 - 1, 2) + pow(5 - 1, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        expPeakDisp, 0, 2);
}

TEST(CoreWaveformProcessor, testPeakDisplacementHelperEarlyNansMiddlePeak) {
    const int NPTS = 4;
    double ubuff[NPTS] = {NAN, 2, 5, 4};
    double nbuff[NPTS] = {NAN, 2, 3, 4};
    double ebuff[NPTS] = {NAN, 2, 5, 4};
    int npts = NPTS;

    double expPeakDisp = pow(pow(5 - 2, 2) + pow(3 - 2, 2) + pow(5 - 2, 2), 0.5);

    testPDHelper_helper(npts, ubuff, nbuff, ebuff,
        expPeakDisp, 1, 2);
}

TEST(CoreWaveformProcessor, testParseQChannel) {
    double value;

    // Temp code to print all values
    // int chimap;
    // double chi2;
    // for (int i = 0; i < 100; i++) {
    //     value = 12300 + i;
    //     chi2 = core_waveformProcessor_parseQChannelChi2CWU(value);
    //     chimap = core_waveformProcessor_parseQChannelChi2CWUmap(value);
    //     std::cout << value << ": " << chimap << ", " << chi2 << std::endl;
    // }

    // Test a few values
    value = 12300;
    EXPECT_EQ(0, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.0001, core_waveformProcessor_parseQChannelChi2CWU(value));

    value = 612325;
    EXPECT_EQ(25, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.001, core_waveformProcessor_parseQChannelChi2CWU(value));

    value = 765450;
    EXPECT_EQ(50, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.01, core_waveformProcessor_parseQChannelChi2CWU(value));

    value = 234575;
    EXPECT_EQ(75, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.1, core_waveformProcessor_parseQChannelChi2CWU(value));
}
