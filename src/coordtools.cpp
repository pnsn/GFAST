#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <GeographicLib/UTMUPS.hpp>
#include "gfast.h"

using namespace std;

/*!
 * @brief This converts WGS84 coordinates to ITRF xyz
 *
 * @param[in] lat_in    latitude (degrees)
 * @param[in] lon_in    longitude (degrees)
 * @param[in] alt       altitude (m)
 *
 * @param[out] x        x coordinate (m)
 * @param[out] y        y coordinate (m)
 * @param[out] z        z coordinate (m)
 *
 * @author Brendan Crowell, PNSN
 *         Ben Baker, ISTI -> converted from Python to C
 *
 * @date February 2016
 *
 */
extern "C"
void GFAST_coordtools_lla2ecef(double lat_in, double lon_in, double alt,
                               double *x, double *y, double *z)
{
    double cos_lat, cos_lon, e2, lat, lon, N, sin_lat, sin_lon;
    const double a = 6378137.0;
    const double e = 8.1819190842622e-2;
    const double pi180 = M_PI/180.0;

    lat = lat_in*pi180;
    lon = lon_in*pi180;
    cos_lat = cos(lat);
    cos_lon = cos(lon);
    sin_lat = sin(lat);
    sin_lon = sin(lon);

    e2 = pow(e, 2);
    N = a/sqrt(1.0 - e2*pow(sin_lat, 2));

    *x = (N + alt)*cos_lat*cos_lon;
    *y = (N + alt)*cos_lat*sin_lon;
    *z = ((1.0 - e2)*N+alt)*sin_lat;
    return;
}
//============================================================================//
/*!
 * @brief Converts ITRF xyz coordinates to WGS84
 *
 * @param[in] x      x ITRF coordinate (m)
 * @param[in] y      y ITRF coordinate (m)
 * @param[in] z      z ITRF coordinate (m)
 *
 * @param[out] lat_deg   corresponding WGS84 latitude (degrees)
 * @param[out] lon_deg   corresponding WGS84 longitude (degrees)
 * @param[out] alt       altitude (m)
 * 
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
extern "C"
void GFAST_coordtools_ecef2lla(double x, double y, double z,
                               double *lat_deg, double *lon_deg, double *alt)
{
    double a2, b, b2, e2, ep, lat, lon, N, p, th; 
    const double a = 6378137.0;
    const double e = 8.1819190842622e-2;
    const double pi180i = 180.0/M_PI;

    a2 = pow(a, 2);
    e2 = pow(e, 2);
    b = sqrt(a2*(1.0 - pow(e, 2)));
    b2 = pow(b, 2);
    ep = sqrt((a2 - b2)/b2);
    p =  sqrt(pow(x, 2) + pow(y, 2));
    th = atan2(a*z, b*p);
    lon = atan2(y, x);
    lat = atan2((z + pow(ep, 2)*b*pow(sin(th), 3)),
                (p - e2*a*pow(cos(th), 3)));
    N = a/sqrt(1.0 - e2*pow(sin(lat), 2));

    *alt = p/cos(lat) - N;
    *lon_deg = lon*pi180i;
    *lat_deg = lat*pi180i;
    return;
}
//============================================================================//
/*!
 * @brief This takes displacements in x, y, z and converts them to
 *        north, east, up
 *
 * @param[in] dx         displacement in x
 * @param[in] dy         displacement in y
 * @param[in] dz         displacement in z
 * @param[in] lat_deg    latitude (degrees)
 * @param[in] lon_deg    longitude (degrees)
 * 
 * @param[out] dn        displacement in north
 * @param[out] de        displacement in east
 * @param[out] du        displacement in up
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
extern "C"
void GFAST_coordtools_dxyz2dneu(double dx, double dy, double dz,
                                double lat_deg, double lon_deg,
                                double *dn, double *de, double *du)
{
    double cos_lat, cos_lon, lat, lon, sin_lat, sin_lon;
    const double pi180 = M_PI/180.0;
    lat = lat_deg*pi180;
    lon = lon_deg*pi180;
    cos_lat = cos(lat);
    cos_lon = cos(lon);
    sin_lat = sin(lat);
    sin_lon = sin(lon);
    *dn =-sin_lat*cos_lon*dx - sin_lat*sin_lon*dy + cos_lat*dz;
    *de =-sin_lon*dx + cos(lon)*dy;
    *du = cos_lat*cos_lon*dx + cos_lat*sin_lon*dy + sin_lat*dz;
    return;
}
//============================================================================//
/*!
 * @brief Converts latitude/longitudes to UTM location
 *
 * @param[in] lat       latitude (degrees) [-90,90]
 * @param[in] lon       longitude (degrees) [-540,540)
 *
 * @param[out] xutm     corresponding x utm location (meters)
 * @param[out] yutm     corresponding y utm location (meters)
 * @param[out] lnorthp  False -> indicates southern hemisphere
 *                      True  -> indicates northern hemisphere
 *
 * @param[inout] zone   -1 -> then the corresponding UTM zone be returned
 *                      >-1-> force GeographicLib to use this UTM zone 
 *                            [0,60]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 */
extern "C"
int geodetic_coordtools_ll2utm(double lat, double lon,
                               double *xutm, double *yutm,
                               bool *lnorthp, int *zone)
{
    double x, y, gamma, k;
    int setzone, zone_loc;
    bool mgrslimits, northp_loc;
    char error[1024];
    //------------------------------------------------------------------------//
    //
    // Check if the zone is to be specified
    *xutm = 0.0;
    *yutm = 0.0;
    *lnorthp = true;
    if (*zone ==-1){ // Compute zone
        setzone = GeographicLib::UTMUPS::STANDARD;
    }else{
        if (*zone < GeographicLib::UTMUPS::MINZONE ||
            *zone > GeographicLib::UTMUPS::MAXZONE){
            strcpy(error,"geodetic_ll2utm: Invalid zone\n");
            setzone = GeographicLib::UTMUPS::STANDARD;
            log_warnF(error);
        }else{
            setzone = *zone;
        }
    }
    // Convert lat/lon to utm
    mgrslimits = false;
    try{
        GeographicLib::UTMUPS::Forward(lat, lon, zone_loc, northp_loc, x, y,
                                       gamma, k, setzone, mgrslimits);
        *lnorthp = northp_loc;
    }
    catch (const exception &e){
        strcpy(error, "geodetic_ll2utm: Error converting latlon to utm\n");
        log_errorF(error);
        return -1;
    }
    // Return the zone and (x,y) utm's
    if (*zone ==-1){*zone = zone_loc;}
    *xutm = x;
    *yutm = y;
    return 0;
}
//============================================================================//
/*!
 * @brief Converts UTM location to latitude/longitude
 *
 * @param[in] zone     UTM zone containing (xutm,yutm) [0,60]
 * @param[in] lnorthp  False -> then in the southern hemisphere
 *                     True  -> then in the northern hemisphere
 * @param[in] xutm     x utm location (meters) 
 * @param[in] yutm     y utm location (meters) 
 *
 * @param[out] lat     latitude (degrees) [-90,90]
 * @param[out] lon     longitude (degrees) [0,360)
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" 
int geodetic_coordtools_utm2ll(int zone, bool lnorthp, double xutm, double yutm,
                               double *lat, double *lon)
{
    double plat, plon, gamma, k;
    bool mgrslimits;
    char error[1024];
    //------------------------------------------------------------------------//
    //
    // Check if the zone is to be specified
    *lat = 0.0;
    *lon = 0.0;
    plat = 0.0;
    plon = 0.0;
    if (zone < GeographicLib::UTMUPS::MINZONE ||
        zone > GeographicLib::UTMUPS::MAXZONE){
        strcpy(error, "geodetic_utm2ll: Invalid zone\n");
        log_errorF(error);
        return -1;
    }
    // Convert lat/lon to utm
    mgrslimits = false;
    try{
        GeographicLib::UTMUPS::Reverse(zone, lnorthp, xutm, yutm,
                                       plat, plon,
                                       gamma, k, mgrslimits);
        if (plon < 0.0){plon = plon + 360.0;}
    }
    catch (const exception &e){
        strcpy(error, "geodetic_utm2ll: Error converting latlon to utm\n");
        log_errorF(error);
        return -1;
    }
    // Return the zone and (x,y) utm's
    *lat = plat;
    *lon = plon;
    return 0;
}
//============================================================================//
/*!
 * @brief This takes lat and lon values and converts to UTM northing and easting
 *        for the Pacific Northwest.  The central meridian is fixed by lon0
 *        (no lookup table included), so don't use if you are covering a large
 *        geographic area (probably greater than 3 UTM zones).  For Cascadia,
 *        use lon0=-123.  Also, this is northern hemisphere fixed. If in
 *        southern hemisphere, you need to add 10000 km to the northing
 *
 * @param[in] lon_deg       longitude to convert to UTM easting (degrees)
 * @param[in] lat_deg       latitude to convert to UTM northing (degrees)
 * @param[in] lon0_deg      controls UTM zone - see above (degrees)
 * @param[out] lnorthp      if true then this UTM point is in the northern
 *                          hemistphere.  
 *                          if false then this UTM point is in the southern
 *                          hermisphere
 * @param[inout] zone       if ==-1 then choose the central meridian from the 
 *                          input longitude.
 *                          otherwise set the desired zone in range [0,60]
 *                          from the input longitude
 *
 * @param[out] UTMEasting   corresponding easting UTM coordinate (m)
 * @param[out] UTMNorthing  corresponding northing UTM coordinate (m)
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
#pragma omp declare simd
extern "C"
void GFAST_coordtools_ll2utm_ori(double lat_deg, double lon_deg,
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

    lon_deg_use = lon_deg;
    if (lon_deg_use > 180.0){lon_deg_use = lon_deg_use - 360.0;} // [0,360]
    lon = lon_deg_use*pi180;
    lat = lat_deg*pi180;
    zone_loc = *zone;
    if (zone_loc ==-1){
        zone_loc = fmod( floor((lon_deg_use + 180.0)/6.0) , 60.0) + 1;
    }
    lon0_deg = fabs(zone_loc)*6.0 - 183.0;
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
    if (*zone ==-1){*zone = fmod( floor((lon0_deg + 180.0)/6.0) , 60.0) + 1;}
    return;
}
//============================================================================//
/*!
 * @brief Takes UTM easting and northing with a central meridian and converts
 *        to lat and lon. Obviously running ll2utm then putting output into
 *        utm2ll should result in the original value (off by a small amount
 *        due to truncation of the UTM parameters)
 *
 * @param[in] zone         UTM zone
 * @param[in] lnorthp      if true then we are in the northern hemistphere
 *                         if false then we are in the southern hemisphere
 * @param[in] UTMEasting   UTM east coordinate to convert to longitude (m)
 * @param[in] UTMNorthing  UTM north coordinate to convert to latitude (m)
 * @param[in] lon0_deg     controls the UTM zone - see above (degrees)
 *
 * @param[out] lat_deg     corresponding latitude (degrees)
 * @param[out] lon_deg     corresponding longitude (degrees)
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * 
 */
#pragma omp declare simd
extern "C"
void GFAST_coordtools_utm2ll_ori(int zone, bool lnorthp,
                                 double UTMNorthing, double UTMEasting,
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
    return;
}
