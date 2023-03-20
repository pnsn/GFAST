#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"

/*!
 * @brief This takes lat and lon values and converts to UTM northing and 
 *        easting.
 *
 * @param[in] lon_deg       Longitude to convert to UTM easting (degrees).
 * @param[in] lat_deg       Latitude to convert to UTM northing (degrees).
 * @param[out] UTMNorthing  Corresponding northing UTM coordinate (m).
 * @param[out] UTMEasting   Corresponding easting UTM coordinate (m).
 * @param[out] lnorthp      If true then this UTM point is in the northern
 *                          hemisphere.  \n
 *                          If false then this UTM point is in the southern
 *                          hemisphere.
 * @param[inout] zone       If ==-1 then choose the central meridian from the 
 *                          input longitude. \n
 *                          Otherwise set the desired zone in range [0,60]
 *                          from the input longitude.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
#ifdef _OPENMP
#pragma omp declare simd
#endif
void core_coordtools_ll2utm(const double lat_deg, const double lon_deg,
                            double *UTMNorthing, double *UTMEasting,
                            bool *lnorthp, int *zone)
{
    double A, C, lat, lon, lon_deg_use, lon0, lon0_deg, M, T, v;
    int zone_loc;
    // WGS84 parameters
    const double a = 6378137.0000;
    const double esq = 0.006694380069978522;
    const double epsq = esq/(1.0 - esq);
    const double k0 = 0.9996;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    lon_deg_use = lon_deg;
    if (lon_deg_use > 180.0){lon_deg_use = lon_deg_use - 360.0;} // [0,360]
    lon = lon_deg_use*pi180;
    lat = lat_deg*pi180;
    zone_loc = *zone;
    if (zone_loc ==-1)
    {
        zone_loc = (int) (fmod(floor((lon_deg_use + 180.0)/6.0), 60.0) + 0.5)
                 + 1;
    }
    lon0_deg = (double) (abs(zone_loc)*6) - 183.0;
    lon0 = lon0_deg*pi180;   //central meridian (-123 for zone 10)

    A = (lon - lon0)*cos(lat);
    v = a/sqrt(1.0 - esq*pow(sin(lat), 2));
    T = pow(tan(lat), 2);
    C = esq*pow(cos(lat), 2)/(1.0 - esq);

    M = a*(
           (1.0 - esq/4.0 - 3.0*pow(esq, 2)/64.0 - 5.0*pow(esq, 3)/256.0)*lat
          -(3.0*esq/8.0 + 3.0*pow(esq, 2)/32.0
          + 45.0*pow(esq, 3)/1024.0)*sin(2.0*lat)
          +(15.0*pow(esq, 2)/256.0 + 45.0*pow(esq, 3)/1024.0)*sin(4.0*lat)
          -(35.0*pow(esq, 3)/3072.0)*sin(6.0*lat)
        );

    *UTMNorthing = k0*( M + v*tan(lat)*( A*A/2.0 + (5.0 - T + 9.0*C +4.0*C*C)
                     *pow(A, 4)/24.0 + (61.0 - 58.0*T + T*T
                     + 600.0*C - 330.0*epsq)*pow(A, 6)/720.0 ) );

    *UTMEasting = k0*v*( A + (1.0 - T + C)*pow(A, 3)/6.0
                      + (5.0 - 18.0*T + T*T
                      + 72.0*C - 58.0*epsq)*pow(A, 5)/120.0 ) + 500000.0;
    *lnorthp = true;
    if (lat_deg < 0.0){*UTMNorthing = *UTMNorthing + 10000000.0;}
    if (lat_deg < 0.0){*lnorthp = false;}
    if (*zone ==-1)
    {
        *zone = (int) (fmod(floor((lon0_deg + 180.0)/6.0), 60.0) + 0.5) + 1;
    }
    return;
}
