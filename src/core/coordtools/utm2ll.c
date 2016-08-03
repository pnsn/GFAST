#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "gfast_core.h"
/*!
 * @brief Takes UTM easting and northing with a central meridian and converts
 *        to lat and lon.  This is the inverse function for ll2utm.
 *
 * @param[in] zone         UTM zone
 * @param[in] lnorthp      if true then we are in the northern hemistphere
 *                         if false then we are in the southern hemisphere
 * @param[in] UTMNorthing  UTM north coordinate to convert to latitude (m)
 * @param[in] UTMEasting   UTM east coordinate to convert to longitude (m)
 *
 * @param[out] lat_deg     corresponding latitude (degrees)
 * @param[out] lon_deg     corresponding longitude (degrees)
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * 
 */
#pragma omp declare simd
void core_coordtools_utm2ll(const int zone, const bool lnorthp,
                            const double UTMNorthing, const double UTMEasting,
                            double *lat_deg, double *lon_deg)
{
    double C1, D, e1, lat, lat1, lon, lon0, lon0_deg, M1, mu1, T1, p1, v1;
    // WGS84 parameters
    const double a = 6378137.0000;
    const double esq = 0.006694380069978522;
    const double epsq = esq/(1.0 - esq);
    const double k0 = 0.9996;
    const double pi180 = M_PI/180.0;
    const double pi180i = 180.0/M_PI;
    //------------------------------------------------------------------------//
    lon0_deg = fabs(zone)*6.0 - 183.0;
    lon0 = lon0_deg*pi180;
    M1 = UTMNorthing/k0;
    if (!lnorthp){M1 = (UTMNorthing - 10000000.0)/k0;}
    mu1 = M1/(a * (1.0 - esq/4.0 - 3.0/64.0*pow(esq, 2)
              - 5.0/256.0*pow(esq, 3)) );
    e1 = ( 1.0 - sqrt(1.0 - esq) ) / ( 1.0 + sqrt(1.0 - esq) );
    lat1 = mu1 + (3.0*e1/2.0 - 27.0/32.0*pow(e1, 3))*sin(2.0*mu1)
               + (21.0/16.0*pow(e1, 2) - 55.0/32.0*pow(e1, 4))*sin(4.0*mu1)
               + (151.0/96.0*pow(e1, 3))*sin(6.0*mu1)
               + (1097.0/512.0*pow(e1, 4))*sin(8*mu1);
    T1 = pow(tan(lat1), 2);
    C1 = epsq*pow(cos(lat1), 2);

    v1 = a/sqrt(1.0 - esq*pow(sin(lat1),2));
    p1 = a*(1.0 - esq)/pow(1.0 - esq*pow(sin(lat1), 2), 1.5);
    D = (UTMEasting - 500000.0)/v1/k0;

    lat = lat1 - (v1*tan(lat1)/p1) * ( pow(D, 2)/2.0 - (5.0 + 3.0*T1 + 10.0*C1
                 - 4.0*pow(C1,2) - 9.0*epsq)*pow(D, 4)/24.0 + (61.0 + 90.0*T1
                 + 298.0*C1 + 45.0*pow(T1, 2) - 252.0*epsq - 3.0*pow(C1, 2))
                 *pow(D, 6)/720.0 );
    lon = lon0 + ( D - (1.0 + 2.0*T1 + C1)*pow(D, 3)/6.0 + (5.0 - 2.0*C1
                 + 28.0*T1 - 3.0*pow(C1, 2) + 8.0*epsq + 24.0*pow(T1, 2))
                  *pow(D, 5)/120.0 )/cos(lat1);

    *lat_deg = lat*pi180i;
    *lon_deg = lon*pi180i;
    if (*lon_deg < 0.0){*lon_deg = *lon_deg + 360.0;}
    return;
}
