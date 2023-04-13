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

#include "gfast_ut_utils.h"


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
    // int chimap, goodness;
    // double chi2;
    // for (int i = 0; i < 100; i++) {
    //     value = 12300 + i;
    //     chi2 = core_waveformProcessor_parseQChannelChi2CWU(value);
    //     chimap = core_waveformProcessor_parseQChannelChi2CWUmap(value);
    //     goodness = core_waveformProcessor_parseQChannelGoodness(value);
    //     std::cout << value << ": " << chimap << ", " << chi2 << ", " << goodness << std::endl;
    // }

    // Test a few values
    value = 12300;
    EXPECT_EQ(0, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.0001, core_waveformProcessor_parseQChannelChi2CWU(value));
    EXPECT_EQ(0, core_waveformProcessor_parseQChannelGoodness(value));

    value = 612325;
    EXPECT_EQ(25, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.001, core_waveformProcessor_parseQChannelChi2CWU(value));
    EXPECT_EQ(1, core_waveformProcessor_parseQChannelGoodness(value));

    value = 765450;
    EXPECT_EQ(50, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.01, core_waveformProcessor_parseQChannelChi2CWU(value));
    EXPECT_EQ(0, core_waveformProcessor_parseQChannelGoodness(value));

    value = 234575;
    EXPECT_EQ(75, core_waveformProcessor_parseQChannelChi2CWUmap(value));
    EXPECT_DOUBLE_EQ(0.1, core_waveformProcessor_parseQChannelChi2CWU(value));
    EXPECT_EQ(1, core_waveformProcessor_parseQChannelGoodness(value));
}

void fill_gps_data(struct GFAST_data_struct *gps_data,
                   const int k,
                   const char *netw,
                   const char *stat,
                   const char *chan,
                   const char *loc,
                   const double lat,
                   const double lon,
                   const double elev,
                   const double dt,
                   const double gain,
                   const int mpts) {
    strcpy(gps_data->data[k].netw, netw);
    strcpy(gps_data->data[k].stnm, stat);
    strncpy(gps_data->data[k].chan[0], chan, 2);
    strcat( gps_data->data[k].chan[0], "Z\0"); 
    strncpy(gps_data->data[k].chan[1], chan, 2);
    strcat( gps_data->data[k].chan[1], "N\0");
    strncpy(gps_data->data[k].chan[2], chan, 2); 
    strcat( gps_data->data[k].chan[2], "E\0");
    strncpy(gps_data->data[k].chan[3], chan, 2); 
    strcat( gps_data->data[k].chan[3], "3\0");
    strncpy(gps_data->data[k].chan[4], chan, 2); 
    strcat( gps_data->data[k].chan[4], "2\0");
    strncpy(gps_data->data[k].chan[5], chan, 2); 
    strcat( gps_data->data[k].chan[5], "1\0");
    strncpy(gps_data->data[k].chan[6], chan, 2); 
    strcat( gps_data->data[k].chan[6], "Q\0");
    strcpy(gps_data->data[k].loc, loc);
    gps_data->data[k].sta_lat = lat;
    gps_data->data[k].sta_lon = lon;
    gps_data->data[k].sta_alt = elev;
    gps_data->data[k].dt = dt; 
    gps_data->data[k].gain[0] = gain;
    gps_data->data[k].gain[1] = gain;
    gps_data->data[k].gain[2] = gain;
    gps_data->data[k].gain[3] = gain;
    gps_data->data[k].gain[4] = gain;
    gps_data->data[k].gain[5] = gain;
    gps_data->data[k].gain[6] = 1; // Quality channel has no gain

    gps_data->data[k].maxpts = mpts;
    gps_data->data[k].ubuff = memory_calloc64f(mpts);
    gps_data->data[k].nbuff = memory_calloc64f(mpts);
    gps_data->data[k].ebuff = memory_calloc64f(mpts);
    gps_data->data[k].usigmabuff = memory_calloc64f(mpts);
    gps_data->data[k].nsigmabuff = memory_calloc64f(mpts);
    gps_data->data[k].esigmabuff = memory_calloc64f(mpts);
    gps_data->data[k].qbuff = memory_calloc64f(mpts);
    gps_data->data[k].tbuff = memory_calloc64f(mpts);
}

/*
 * Fixture for testing peakDisplacement() function
 * Has two "normal" stations that can be put out of spec in various ways
 * in order to not return peak displacement measurements
 */
class CoreWaveformProcessorFixture : public::testing::Test {
    protected:
        struct GFAST_pgd_props_struct pgd_props;
        struct GFAST_data_struct gps_data;
        struct GFAST_peakDisplacementData_struct pgd_data;
        struct GFAST_pgdResults_struct pgd;
        
        double lat, lon, dep, time;
        int ierr, nsites_pgd, k, j;

        void SetUp() {
            const int NPTS = 10;

            // Initialize necessary pgd properties
            memset(&pgd_props, 0, sizeof(struct GFAST_pgd_props_struct));
            pgd_props.utm_zone = -12345;
            pgd_props.window_vel = 3;
            pgd_props.min_window_vel = 0.01;
            pgd_props.minimum_pgd_cm = 0.0;
            pgd_props.maximum_pgd_cm = 3500;
            pgd_props.u_raw_sigma_threshold = 35;
            pgd_props.n_raw_sigma_threshold = 17;
            pgd_props.e_raw_sigma_threshold = 14;
            pgd_props.q_value_threshold = 1;
            pgd_props.ngridSearch_deps = 10;
            pgd_props.ngridSearch_lats = 1;
            pgd_props.ngridSearch_lons = 1;
            pgd_props.min_sites = 2;

            // Initialize origin
            lat = 46;
            lon = -122;
            dep = 8;
            time = 0;

            // Initialize gps_data
            memset(&gps_data, 0, sizeof(struct GFAST_data_struct));

            gps_data.stream_length = 2;
            gps_data.data = (struct GFAST_waveform3CData_struct *)
                            calloc((size_t) gps_data.stream_length,
                                    sizeof(struct GFAST_waveform3CData_struct));
            fill_gps_data(&gps_data, 0, "UW\0", "TEST1\0", "LYZ\0", "00\0", 46.02, -122.02, 1, 1, 1, NPTS);
            fill_gps_data(&gps_data, 1, "UW\0", "TEST2\0", "LYZ\0", "00\0", 46.03, -122.03, 1, 1, 1, NPTS);

            // Initialize pgd_data, pgd
            memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
            memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));

            ierr = core_scaling_pgd_initialize(pgd_props, gps_data, &pgd, &pgd_data);

            // Now actually put some data in gps_data
            double tdata[NPTS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            double udata[NPTS] = {0, 3, 5, 6, 4, 3, -1, 0, 3, 4};
            double ndata[NPTS] = {0, 2, 3, 4, 5, 3, 2, 0, -1, -4};
            double edata[NPTS] = {0, -3, -5, -4, -4, -3, -1, 0, 2, 3};
            double usigmadata[NPTS] = {.1, .1, .1, .1, .1, .1, .1, .1, .1, .1};
            double nsigmadata[NPTS] = {.1, .1, .1, .1, .1, .1, .1, .1, .1, .1};
            double esigmadata[NPTS] = {.1, .1, .1, .1, .1, .1, .1, .1, .1, .1};
            double qdata[NPTS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

            for (k = 0; k < gps_data.stream_length; k++) {
                for (j = 0; j < NPTS; j++) {
                    gps_data.data[k].ubuff[j] = udata[j];
                    gps_data.data[k].nbuff[j] = ndata[j];
                    gps_data.data[k].ebuff[j] = edata[j];
                    gps_data.data[k].usigmabuff[j] = usigmadata[j];
                    gps_data.data[k].nsigmabuff[j] = nsigmadata[j];
                    gps_data.data[k].esigmabuff[j] = esigmadata[j];
                    gps_data.data[k].qbuff[j] = qdata[j];
                    gps_data.data[k].tbuff[j] = tdata[j];
                }
                gps_data.data[k].npts = NPTS;
            }
        }

        void TearDown() {}
};

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementNormal) {
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(2, nsites_pgd);
    EXPECT_EQ(1, pgd_data.lactive[0]);
    EXPECT_EQ(1, pgd_data.lactive[1]);

    EXPECT_TRUE(lequal(8.24621, pgd_data.pd[0], .00001)) << "Error pgd is wrong";
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementTooDistant) {
    // If a station is not within the s-wave mask, it should be ignored
    gps_data.data[0].sta_lat = 40;
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(0, pgd_data.lactive[0]);
    EXPECT_EQ(1, pgd_data.lactive[1]);
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementMasked) {
    // If a site is masked (via siteMaskFile) it should be ignored
    pgd_data.lmask[1] = true;
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(1, pgd_data.lactive[0]);
    EXPECT_EQ(0, pgd_data.lactive[1]);
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementMinPGD) {
    // A displacement that is 0.0 (flat-lined) should be ignored
    for (j = 0; j < 10; j++) {
        gps_data.data[0].ubuff[j] = 0;
        gps_data.data[0].nbuff[j] = 0;
        gps_data.data[0].ebuff[j] = 0;
    }
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(0, pgd_data.lactive[0]);
    EXPECT_EQ(1, pgd_data.lactive[1]);
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementMaxPGD) {
    // A displacement that is too large should be ignored
    gps_data.data[1].ebuff[3] = 1000;
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(1, pgd_data.lactive[0]);
    EXPECT_EQ(0, pgd_data.lactive[1]);
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementSigma) {
    // An uncertainty that is too large means the pgd should be ignored
    gps_data.data[0].esigmabuff[0] = 10;
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(0, pgd_data.lactive[0]);
    EXPECT_EQ(1, pgd_data.lactive[1]);
}

TEST_F(CoreWaveformProcessorFixture, testPeakDisplacementQThreshold) {
    // A q value that doesn't meet the criteria means the pgd should be ignored
    gps_data.data[1].qbuff[0] = 0;
    nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
        &pgd_props,
        lat,
        lon,
        dep,
        time,
        gps_data,
        &pgd_data,
        &ierr);

    EXPECT_EQ(1, nsites_pgd);
    EXPECT_EQ(1, pgd_data.lactive[0]);
    EXPECT_EQ(0, pgd_data.lactive[1]);
}
