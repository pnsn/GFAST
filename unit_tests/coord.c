#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"

int coord_test_ll2utm(void);

int coord_test_ll2utm(void)
{
    double lat, lat0, lon, lon0,utmEast0, utmEast1, utmEastReference,
           utmNorthReference, utmNorth0, utmNorth1;
    int utm_unknown_zone, utm_zone, utm_zone_ref;
    bool lnorthp;
    // Seattle
    lat = 47.6062;
    lon =-122.3321;
    utmEastReference = 550200.2133598323; //550200.21;
    utmNorthReference = 5272748.591307523; //5272748.59;
    utm_zone_ref = 10;
    utm_zone = 10;
    utm_unknown_zone =-1;
    GFAST_core_coordtools_ll2utm(lat, lon, &utmNorth0, &utmEast0,
                                 &lnorthp, &utm_unknown_zone);
    GFAST_core_coordtools_ll2utm(lat, lon + 360.0, &utmNorth1, &utmEast1,
                                 &lnorthp, &utm_zone);
    if (!lnorthp)
    {
        LOG_ERRMSG("%s", "Wrong hemisphere");
        return EXIT_FAILURE;
    }
    if (utm_unknown_zone != utm_zone_ref || utm_zone != utm_zone_ref)
    {
        LOG_ERRMSG("%s", "Failed to compute utm_zone");
        return EXIT_FAILURE;
    }
    if (fabs(utmEast0 - utmEastReference) > 0.001 ||
        fabs(utmEast1 - utmEastReference) > 0.001)
    {
        LOG_ERRMSG("%s", "Failed to get easting");
        return EXIT_FAILURE;
    }
    if (fabs(utmNorth0 - utmNorthReference) > 0.001 ||
        fabs(utmNorth1 - utmNorthReference) > 0.001)
    {
        LOG_ERRMSG("%s", "Failed to get northing");
        return EXIT_FAILURE;
    }
    // Recover
    GFAST_core_coordtools_utm2ll(utm_zone, lnorthp, utmNorth0, utmEast0,
                                 &lat0, &lon0);
    if (fabs(lat0 - lat) > 1.e-5)
    {
        LOG_ERRMSG("%s", "Failed to recover lat");
        return EXIT_FAILURE;
    }
    if ((fabs(lon0 - lon) > 1.e-5) && (fabs(lon0 - 360.0 - lon) > 1.e-5))
    {
        LOG_ERRMSG("Failed to recover lon %f %f", lon, lon0);
        return EXIT_FAILURE;
    }
    // Santiago 
    lat =-33.4489;
    lon =-70.6693;
    utmEastReference = 344846.7203081217; //344846.72
    utmNorthReference = 6297700.155849966; //6297700.16
    utm_zone_ref = 19;
    utm_zone = 19;
    utm_unknown_zone =-1;
    GFAST_core_coordtools_ll2utm(lat, lon + 360.0, &utmNorth0, &utmEast0,
                                 &lnorthp, &utm_unknown_zone);
    GFAST_core_coordtools_ll2utm(lat, lon, &utmNorth1, &utmEast1,
                                 &lnorthp, &utm_zone);
    if (lnorthp)
    {
        LOG_ERRMSG("%s", "Wrong hemisphere 2");
        return EXIT_FAILURE;
    }
    if (utm_unknown_zone != utm_zone_ref || utm_zone != utm_zone_ref)
    {
        LOG_ERRMSG("%s", "Failed to compute utm_zone 2");
        return EXIT_FAILURE;
    }
    if (fabs(utmEast0 - utmEastReference) > 0.001 ||
        fabs(utmEast1 - utmEastReference) > 0.001)
    {
        LOG_ERRMSG("Failed to get easting 2 %f %f", utmEast0, utmEast1);
        return EXIT_FAILURE;
    }
    if (fabs(utmNorth0 - utmNorthReference) > 0.001 ||
        fabs(utmNorth1 - utmNorthReference) > 0.001)
    {
        LOG_ERRMSG("%s", "Failed to get northing 2");
        return EXIT_FAILURE;
    }
    // Recover
    GFAST_core_coordtools_utm2ll(utm_zone, lnorthp, utmNorth0, utmEast0,
                                 &lat0, &lon0);
    if (fabs(lat0 - lat) > 1.e-5)
    {
        LOG_ERRMSG("%s", "Failed to recover lat 2");
        return EXIT_FAILURE;
    }
    if ((fabs(lon0 - lon) > 1.e-5) && (fabs(lon0 - 360.0 - lon) > 1.e-5))
    {
        LOG_ERRMSG("Failed to recover lon 2 %f %f", lon, lon0);
        return EXIT_FAILURE;
    }
    LOG_INFOMSG("%s", "Success!");
    return EXIT_SUCCESS;
}

/*
    int nstat = 9;
    double xutm_ref[9] = {343323.63430894, 372234.82479856,  423564.1189781,
                          203806.59303668, 136995.95792472, 723723.33606821,
                          379490.42778671, 409390.05963976, 391311.1466216};
    double yutm_ref[9] = {5411348.46298294, 4743799.38939605, 5037463.63108665,
                          5532670.66847299, 5622424.34340908, 4642449.79043267,
                          5350682.38926825, 4525582.50275946, 4484576.434264};
    double stla[9] = {  48.83532872,  42.83609887,  45.48651503,
                        49.87305293,  50.64035267,  41.90232489,
                        48.29785467,  40.87630594,  40.50478709};
    double stlo[9] = {-125.13510527,-124.56334629,-123.97812400,
                      -127.12266484,-128.13499899,-120.30283244,
                      -124.62490719,-124.07537043,-124.28278289}; 
    double xutm, yutm;
    int zone = 10, zone_out;
    bool lnorth;
    int i, ierr;
    double lon0_deg =-123.0;
    double tol = 1.e-3; // milimeters
*/
/*
    zone_out =-1;
    for (i=0; i<nstat; i++){
        ierr = geodetic_coordtools_ll2utm(stla[i], stlo[i],
                                          &xutm, &yutm,
                                          &lnorth, &zone);
        if (ierr != 0){ 
            printf("%s: Failed to call ll2utm\n", fcnm);
            return -1; 
        }
        if (fabs(xutm - xutm_ref[i]) > tol ||
            fabs(yutm - yutm_ref[i]) > tol){
            printf("%s: Failed to convert coordinate\n", fcnm);
            return -1;
        }
        // For kicks check the original (note the tolerance is finer but
        // that's because I computed the `reference' solution with GFAST
        GFAST_coordtools__ll2utm(stla[i], stlo[i],
                                    &yutm, &xutm,
                                    &lnorth, &zone_out);
        if (fabs(xutm - xutm_ref[i]) > 1.e-5 ||
            fabs(yutm - yutm_ref[i]) > 1.e-5 ||
            !lnorth){
            printf("%s: Failed to convert original coordinate\n", fcnm);
            return -1; 
        } 
    }
    LOG_INFOMSG("%s", "Success!");
    return EXIT_SUCCESS;
}
*/
