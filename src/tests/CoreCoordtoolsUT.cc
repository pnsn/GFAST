/**
 * @file CoreCoordtoolsUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/coordtools directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"


/**
 * Test the lat/lon to utm conversions. Based on gfast/unit_tests/coord.c by Ben Baker
 */
void doCoordTest(double lat, double lon, double utmEastReference, double utmNorthReference,
                 int utm_unknown_zone, int utm_zone, int utm_zone_ref) {
    double lat0, lon0, utmEast0, utmEast1, utmNorth0, utmNorth1, tol;
    // int utm_unknown_zone, utm_zone;
    bool lnorthp;
    tol = 0.001; // mm precision

    GFAST_core_coordtools_ll2utm(lat, lon, &utmNorth0, &utmEast0,
                                 &lnorthp, &utm_unknown_zone);
    GFAST_core_coordtools_ll2utm(lat, lon + 360.0, &utmNorth1, &utmEast1,
                                 &lnorthp, &utm_zone);

    if (lat >= 0) {
        EXPECT_TRUE(lnorthp);
    } else {
        EXPECT_FALSE(lnorthp);
    }
    EXPECT_EQ(utm_zone_ref, utm_unknown_zone);
    EXPECT_EQ(utm_zone_ref, utm_zone);
    EXPECT_NEAR(utmEastReference, utmEast0, tol);
    EXPECT_NEAR(utmEastReference, utmEast1, tol);
    EXPECT_NEAR(utmNorthReference, utmNorth0, tol);
    EXPECT_NEAR(utmNorthReference, utmNorth1, tol);

    // Now try the opposite
    GFAST_core_coordtools_utm2ll(utm_zone, lnorthp, utmNorth0, utmEast0,
                                 &lat0, &lon0);

    EXPECT_NEAR(lat, lat0, 1.e-5);
    EXPECT_TRUE((fabs(lon0 - lon) < 1.e-5) || (fabs(lon0 - 360 - lon) < 1.e-5));
    
}

TEST(CoreCoord, testCoordSeattle) {
    double lat, lon, utmEastReference, utmNorthReference;
    int utm_unknown_zone, utm_zone, utm_zone_ref;
    // bool lnorthp = true;
    
    lat = 47.6062;
    lon = -122.3321;
    utmEastReference = 550200.2133598323; //550200.21;
    utmNorthReference = 5272748.591307523; //5272748.59;
    utm_zone_ref = 10;
    utm_zone = 10;
    utm_unknown_zone =-1;

    doCoordTest(lat, lon, utmEastReference, utmNorthReference, utm_unknown_zone, utm_zone, utm_zone_ref);
}

TEST(CoreCoord, testCoordSantiago) {
    double lat, lon, utmEastReference, utmNorthReference;
    int utm_unknown_zone, utm_zone, utm_zone_ref;
    
    lat = -33.4489;
    lon = -70.6693;
    utmEastReference = 344846.7203081217; //344846.72
    utmNorthReference = 6297700.155849966; //6297700.16
    utm_zone_ref = 19;
    utm_zone = 19;
    utm_unknown_zone =-1;

    doCoordTest(lat, lon, utmEastReference, utmNorthReference, utm_unknown_zone, utm_zone, utm_zone_ref);
}

