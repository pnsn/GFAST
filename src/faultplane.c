#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Meshes a fault plane from the magnitude, strike, and dip
 *        computed in the CMT inversion.   Note the fault patch at 
 *        (istr, idep) is accessed by kndx = idip*nstr + istr.
 *
 * @param[in] lat          event latitude (degrees)
 * @param[in] lon          event longitude (degrees) 
 * @param[in] depth        event depth (km)
 * @param[in] fact_pct     safety factor (percentage) added to fault width and
 *                         length s.t. the entire unilateral rupture is
 *                         accounted for.  [0,100] - Dreger and Kaverina
 *                         recommend 10 percent
 * @param[in] M            moment magnitude
 * @param[in] nstr         number of fault patches along strike
 * @param[in] ndip         number of fault patches down dip
 * @param[in] zone_in      UTM zone.  If out of bounds [0,60] then the UTM zone
 *                         will be selected by computing the UTM zone for the
 *                         source and applying that too all the receivers
 * @param[in] verbose      controls verbosity (0 is quiet)
 *
 * @param[out] fault_lon   fault patch longitudes (degrees) [nstr*ndip]
 * @param[out] fault_lat   fault patch latitudes (degrees) [nstr*ndip]
 * @param[out] fault_alt   fault patch depths (km) [nstr*ndip]
 * @param[out] strike      strike on each fault patch [nstr*ndip]
 * @param[out] dip         dip of each fault patch [nstr*ndip]
 * @param[out] length      length of each fault patch (km) [nstr*ndip]
 * @param[out] width       width of each fault patch (km) [nstr*ndip]
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
int GFAST_faultplane_CMT(double lat, double lon, double depth,
                         double fact_pct,
                         double M, double strikeF, double dipF,
                         int nstr, int ndip, int zone_in, int verbose,
                         double *fault_lon, double *fault_lat,
                         double *fault_alt,
                         double *strike, double *dip,
                         double *length, double *width)
{
    const char *fcnm = "GFAST_fault_CMT\0";
    double area, dalt, di, dj, dlen, dwid, fact, fault_X, fault_Y, fault_Z,
           latF, len, lonF, wid, x0, xdoff, xsoff, y0, ydoff, ysoff, z0;
    int i, ierr, j, k, zone;
    bool lnorthp;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    //
    // Error handling
    if (ndip < 1){
        if (verbose > 0){
            log_errorF("%s: Invalid number of fault patches down dip: %d\n",
                       fcnm, ndip);
        }
        return -1;
    }
    if (nstr < 1){
        if (verbose > 0){
            log_errorF("%s: Invalid number of fault patches along strike: %d\n",
                       fcnm, nstr);
        }
        return -1;
    }
    if (lat <-90.0 || lat > 90.0){
        if (verbose > 0){
            log_errorF("%s: Source latitude %f is invalid\n", fcnm, lat);
        }
        return -1;
    }
    if (lon < -540.0 || lon >= 540.0){
        if (verbose > 0){
            log_errorF("%s: Source longitude %f is invalid\n", fcnm, lon);
        }
        return -1;
    }
    // Compute location
    fact = 0.1;
    if (fact_pct < 0.0 || fact_pct > 100.0){
        if (verbose > 1){
            log_warnF("%s: Safety factor %f is invalid; setting to 10\n", fcnm);
        }
    }else{
        fact = fact_pct/100.0;
    }
    // Set the fault origin
    if (zone_in < 0 || zone_in > 60){
        zone =-1;
    }else{
        zone = zone_in;
    }   
    ierr = geodetic_coordtools_ll2utm(lat, lon,
                                      &x0, &y0,
                                      &lnorthp, &zone);
    if (ierr != 0){
        if (verbose > 0){
            log_errorF("%s: Error computing UTM origin\n", fcnm);
        }
        return -1;
    }
    x0 = x0*1.e-3;
    y0 = y0*1.e-3;
    // Estimate the fault size
    area = pow(10.0, -3.49+0.91*M); // Fault area (Dreger and Kaverina, 2000)
    len  = pow(10.0, -2.44+0.59*M); // Fault length (Dreger and Kaverina, 2000)
    wid  = area/len;                // Fault width is area divided by length
    // Add a safety factor to encapsulate entire rupture
    len = len + fact*len;
    wid = wid + fact*wid;
    // Set the initial top depth - either depth-width/2*sin(dip) or 0 
    // depending on how wide and close the surface fault is
    if (wid/2.0*sin(dipF*pi180) > depth){
        z0 = 0.0;
        x0 = x0 - len/2.0*sin(strikeF*pi180) 
           - depth*cos(dipF*pi180)*sin((strikeF + 90.0)*pi180);
        y0 = y0 - len/2.0*cos(strikeF*pi180)
           - depth*cos(dipF*pi180)*cos((strikeF + 90.0)*pi180);
    }else{
        z0 = depth - wid/2.0*sin(dipF*pi180);
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
    k = 0;
    for (j=0; j<ndip; j++){
        dj = (double) j;
        for (i=0; i<nstr; i++){
            di = (double) i;
            fault_X = x0 + (0.5 + di)*xsoff + (0.5 + dj)*xdoff;
            fault_Y = y0 + (0.5 + di)*ysoff + (0.5 + dj)*ydoff;
            fault_Z = z0 + (0.5 + dj)*dalt;
            // Convert from UTMs back to lat/lon 
            fault_X = fault_X*1000.0;
            fault_Y = fault_Y*1000.0;
            ierr = geodetic_coordtools_utm2ll(zone, lnorthp,
                                              fault_X, fault_Y,
                                              &latF, &lonF);
            if (ierr != 0){
                if (verbose > 0){
                    log_errorF("%s: Error computing fault lat/lon\n",
                               fcnm);
                }
                return -1;
            }
            fault_lat[k] = latF;
            fault_lon[k] = lonF;
            fault_alt[k] = fault_Z;
            strike[k] = strikeF;
            dip[k] = dipF;
            length[k] = dlen;
            width[k] = dwid;
        } // Loop on strike 
    } // Loop on dip
    // Fidelity check
    return 0;
}
