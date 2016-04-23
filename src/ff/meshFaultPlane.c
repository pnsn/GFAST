#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Meshes a fault plane from the magnitude, strike, and dip
 *        computed in the CMT inversion.   Note the fault patch at 
 *        (istr, idep) is accessed by kndx = idip*nstr + istr.
 *
 * @param[in] SA_lat       event latitude (degrees)
 * @param[in] SA_lon       event longitude (degrees) 
 * @param[in] SA_dep       event depth (km)
 * @param[in] flen_pct     safety factor (percentage) added to fault length 
 *                         s.t. the entire unilateral rupture is accounted
 *                         for.  [0,100] - Dreger and Kaverina recommend
 *                         10 percent
 * @param[in] fwid_pct     safety factor (percentage) added to fault width 
 *                         s.t. the entire unilateral rupture is accounted
 *                         for.  [0,100] - Dreger and Kaverina recommend
 *                         10 percent
 * @param[in] M            moment magnitude
 * @param[in] nstr         number of fault patches along strike
 * @param[in] ndip         number of fault patches down dip
 * @param[in] utm_zone     UTM zone.  If out of bounds [0,60] then the UTM zone
 *                         will be selected by computing the UTM zone for the
 *                         source and applying that too all the receivers
 * @param[in] verbose      controls verbosity (< 2 is quiet)
 *
 * @param[out] fault_lat   fault patch latitudes (degrees) [nstr*ndip]
 * @param[out] fault_lon   fault patch longitudes (degrees) [nstr*ndip]
 * @param[out] fault_xutm  fault patch easting UTM (m) [nstr*ndip]
 * @param[out] fault_yutm  fault patch northing UTM (m) [nstr*ndip]
 * @param[out] fault_alt   fault patch depths (km) [nstr*ndip]
 * @param[out] strike      strike on each fault patch [nstr*ndip]
 * @param[out] dip         dip of each fault patch [nstr*ndip]
 * @param[out] length      length of each fault patch (m) [nstr*ndip]
 * @param[out] width       width of each fault patch (m) [nstr*ndip]
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @reference D. Dreger and A Kaverina,
 *            Seismic Remote Sending for the Earthquake Source Process and
 *            Near-Source STrong Shaking: A Case Study of the October 16,
 *            1999 Hector Mine Earthquake.  Geophysical Research Letters,
 *            Volume 27 (No 13), 1941-1944, (2000).
 *
 */
int GFAST_FF__meshFaultPlane(double SA_lat, double SA_lon, double SA_dep,
                             double flen_pct,
                             double fwid_pct,
                             double M, double strikeF, double dipF,
                             int nstr, int ndip,
                             int utm_zone, int verbose,
                             double *__restrict__ fault_lat,
                             double *__restrict__ fault_lon,
                             double *__restrict__ fault_xutm,
                             double *__restrict__ fault_yutm,
                             double *__restrict__ fault_alt,
                             double *__restrict__ strike,
                             double *__restrict__ dip,
                             double *__restrict__ length,
                             double *__restrict__ width)
{
    const char *fcnm = "GFAST_FF__meshFaultPlane\0";
    double area, dalt, di, dj, dlen, dwid,
           fact_len, fact_wid, fault_X, fault_Y, fault_Z,
           latF, len, lonF, wid,
           x0, xdoff, xsoff, y0, ydoff, ysoff, z0;
    int i, j, k, zone_loc;
    bool lnorthp;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    //
    // Error handling
    if (ndip < 1 || nstr < 1){
        if (ndip < 1){
            log_errorF("%s: Invalid number of fault patches down dip: %d\n",
                       fcnm, ndip);
        }
        if (nstr < 1){
            log_errorF("%s: Invalid number of fault patches along strike: %d\n",
                       fcnm, nstr);
        }
        return -1;
    }
    // Ensures safety factors are okay 
    fact_len = 0.1;
    if (flen_pct < 0.0){
        if (verbose > 1){
            log_warnF("%s: Width safety factor %f is invalid; setting to 10\n",
                      fcnm, flen_pct);
        }
    }else{
        fact_len = flen_pct/100.0;
    }
    fact_wid = 0.1;
    if (fwid_pct < 0.0){
        if (verbose > 0){
            log_warnF("%s: Length safety factor %f is invalid; setting to 10\n",
                       fcnm, fwid_pct); 
        }
    }else{
        fact_wid = fwid_pct/100.0;
    }
    // Get the source location
    if (utm_zone ==-12345){
        zone_loc =-1;
    }else{
        zone_loc = utm_zone;
    }
    GFAST_coordtools_ll2utm_ori(SA_lat, SA_lon,
                                &y0, &x0,
                                &lnorthp, &zone_loc);
    x0 = x0*1.e-3; // Convert to km
    y0 = y0*1.e-3; // Convert to km
    // Estimate the fault size
    area = pow(10.0, -3.49+0.91*M); // Fault area (Dreger and Kaverina, 2000)
    len  = pow(10.0, -2.44+0.59*M); // Fault length (Dreger and Kaverina, 2000)
    wid  = area/len;                // Fault width is area divided by length
    // Add a safety factor to encapsulate entire rupture
    len = len + fact_len*len;
    wid = wid + fact_wid*wid;
    // Set the initial top depth - either depth-width/2*sin(dip) or 0 
    // depending on how wide and close the surface fault is
    if (wid/2.0*sin(dipF*pi180) > SA_dep){
        z0 = 0.0;
        x0 = x0 - len/2.0*sin(strikeF*pi180) 
           - SA_dep*cos(dipF*pi180)*sin((strikeF + 90.0)*pi180);
        y0 = y0 - len/2.0*cos(strikeF*pi180)
           - SA_dep*cos(dipF*pi180)*cos((strikeF + 90.0)*pi180);
    }else{
        z0 = SA_dep - wid/2.0*sin(dipF*pi180);
        x0 = x0 - len/2.0*sin(strikeF*pi180)
           - wid/2.0*cos(dipF*pi180)*sin((strikeF + 90.0)*pi180);
        y0 = y0 - len/2.0*cos(strikeF*pi180)
           - wid/2.0*cos(dipF*pi180)*cos((strikeF + 90.0)*pi180);
    } 
    // Compute the intervals
    dlen = len/(double) nstr;    // Intervals along strike
    dwid = wid/(double) ndip;    // Intervals down dip
    dalt = dwid*sin(dipF*pi180); // Change in fault altitude
    // Compute step step vectors by first looking down dip then moving
    // taking vector components along strike
    xdoff = dwid*cos(dipF*pi180)*sin((strikeF + 90.0)*pi180);
    ydoff = dwid*cos(dipF*pi180)*cos((strikeF + 90.0)*pi180);
    // Compute step size along strike by taking vector components
    xsoff = dlen*sin(strikeF*pi180);
    ysoff = dlen*cos(strikeF*pi180);
    // Mesh fault by quickly stepping along strike and slowly stepping down dip
    for (j=0; j<ndip; j++){
        for (i=0; i<nstr; i++){
            dj = (double) j;
            di = (double) i;
            fault_X = x0 + (0.5 + di)*xsoff + (0.5 + dj)*xdoff;
            fault_Y = y0 + (0.5 + di)*ysoff + (0.5 + dj)*ydoff;
            fault_Z = z0 + (0.5 + dj)*dalt;

            // Convert from UTMs back to lat/lon 
            fault_X = fault_X*1000.0; // km -> m
            fault_Y = fault_Y*1000.0; // km -> m
            GFAST_coordtools_utm2ll_ori(zone_loc, lnorthp,
                                        fault_Y, fault_X,
                                        &latF, &lonF);
            k = j*nstr + i;
            fault_lat[k] = latF;
            fault_lon[k] = lonF;
            fault_xutm[k] = fault_X;
            fault_yutm[k] = fault_Y;
            fault_alt[k] = fault_Z;
            strike[k] = strikeF;
            dip[k] = dipF;
            length[k] = dlen*1.e3; // km -> m
            width[k] = dwid*1.e3;  // km -> m
        } // Loop on strike 
    } // Loop on dip
    return 0;
}
