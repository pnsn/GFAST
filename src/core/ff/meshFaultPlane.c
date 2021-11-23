#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "gfast_core.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*!
 * @brief Meshes a fault plane from the magnitude, strike, and dip
 *        computed in the CMT inversion.   Note the fault patch at 
 *        (istr, idep) is accessed by kndx = idip*nstr + istr.
 *        For more information on the magnitude to fault size scaling
 *        see D. Dreger and A Kaverina, Seismic Remote Sending for the
 *        Earthquake Source Process and Near-Source STrong Shaking: A Case
 *        Study of the October 16, 1999 Hector Mine Earthquake.
 *        Geophysical Research Letters, Volume 27 (No 13), 1941-1944, (2000).
 *
 * @param[in] ev_lat       event latitude (degrees)
 * @param[in] ev_lon       event longitude (degrees) 
 * @param[in] ev_dep       event depth (km)
 * @param[in] flen_pct     safety factor (percentage) added to fault length 
 *                         s.t. the entire unilateral rupture is accounted
 *                         for.  [0,100] - Dreger and Kaverina recommend
 *                         10 percent
 * @param[in] fwid_pct     safety factor (percentage) added to fault width 
 *                         s.t. the entire unilateral rupture is accounted
 *                         for.  [0,100] - Dreger and Kaverina recommend
 *                         10 percent
 * @param[in] M            moment magnitude
 * @param[in] strikeF      strike angle (degrees) of the fault plane.
 *                         this is measured clockwise positive from north
 *                         [0,360].
 * @param[in] dipF         dip angle (degrees) of the fault plane.
 *                         this is measured positive down from horizontal
 *                         [0,90]
 * @param[in] nstr         number of fault patches along strike
 * @param[in] ndip         number of fault patches down dip
 * @param[in] utm_zone     UTM zone.  If out of bounds [0,60] then the UTM zone
 *                         will be selected by computing the UTM zone for the
 *                         source and applying that too all the receivers
 * @param[in] verbose      controls verbosity (< 2 is quiet)
 *
 * @param[out] fault_ptr   maps from the ifp'th fault to the start index
 *                         (lat_vtx, lon_vtx, dep_vtx) [nstr*ndip + 1]
 * @param[out] lat_vtx     defines the latitudes on each vertex of a 
 *                         rectangular fault patch (degrees) [4*nstr*ndip]
 * @param[out] lon_vtx     defines the longitudes on each vertex of a
 *                         rectangular fault patch (degrees) [4*nstr*ndip]
 * @param[out] dep_vtx     defines the depths on each vertex of a 
 *                         retangular fault patch (km) [4*nstr*ndip] 
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
 */
int core_ff_meshFaultPlane(const double ev_lat,
                           const double ev_lon,
                           const double ev_dep,
                           const double flen_pct,
                           const double fwid_pct,
                           const double M,
                           const double strikeF, const double dipF,
                           const int nstr, const int ndip,
                           const int utm_zone, const int verbose,
                           int *__restrict__ fault_ptr,
                           double *__restrict__ lat_vtx,
                           double *__restrict__ lon_vtx,
                           double *__restrict__ dep_vtx,
                           double *__restrict__ fault_xutm,
                           double *__restrict__ fault_yutm,
                           double *__restrict__ fault_alt,
                           double *__restrict__ strike,
                           double *__restrict__ dip,
                           double *__restrict__ length,
                           double *__restrict__ width)
{
    double area, dalt, di, dj, dlen, dwid,
           fact_len, fact_wid,
           fault_X, fault_X1, fault_X2, fault_X3, fault_X4,
           fault_Y, fault_Y1, fault_Y2, fault_Y3, fault_Y4,
           fault_Z, fault_Z1, fault_Z2, fault_Z3, fault_Z4,
           latF, len, lonF, wid,
           x0, xdoff, xsoff, y0, ydoff, ysoff, z0;
    int i, j, k, zone_loc;
    bool lnorthp;
    const double pi180 = M_PI/180.0;
    //------------------------------------------------------------------------//
    //
    // Error handling
    if (ndip < 1 || nstr < 1)
    {
        if (ndip < 1)
        {
            LOG_ERRMSG("Invalid number of fault patches down dip: %d", ndip);
        }
        if (nstr < 1)
        {
            LOG_ERRMSG("Invalid number of fault patches along strike: %d",
                       nstr);
        }
        return -1;
    }
    // Ensures safety factors are okay 
    fact_len = 0.1;
    if (flen_pct < 0.0)
    {
        if (verbose > 1)
        {
            LOG_WARNMSG("Width safety factor %f is invalid; setting to 10 pct",
                        flen_pct);
        }
    }
    else
    {
        fact_len = flen_pct/100.0;
    }
    fact_wid = 0.1;
    if (fwid_pct < 0.0)
    {
        if (verbose > 0)
        {
            LOG_WARNMSG("Length safety factor %f is invalid; setting to 10 pct",
                        fwid_pct); 
        }
    }
    else
    {
        fact_wid = fwid_pct/100.0;
    }
    // Get the source location
    zone_loc = utm_zone;
    if (zone_loc ==-12345){zone_loc =-1;} // Get UTM zone from source lat/lon
    core_coordtools_ll2utm(ev_lat, ev_lon,
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
    if (wid/2.0*sin(dipF*pi180) > ev_dep)
    {
        z0 = 0.0;
        x0 = x0 - len/2.0*sin(strikeF*pi180) 
           - ev_dep*cos(dipF*pi180)*sin((strikeF + 90.0)*pi180);
        y0 = y0 - len/2.0*cos(strikeF*pi180)
           - ev_dep*cos(dipF*pi180)*cos((strikeF + 90.0)*pi180);
    }
    else
    {
        z0 = ev_dep - wid/2.0*sin(dipF*pi180);
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
    fault_ptr[0] = 0;
    for (j=0; j<ndip; j++)
    {
        for (i=0; i<nstr; i++)
        {
            dj = (double) j;
            di = (double) i;
            fault_X = x0 + (0.5 + di)*xsoff + (0.5 + dj)*xdoff;
            fault_Y = y0 + (0.5 + di)*ysoff + (0.5 + dj)*ydoff;
            fault_Z = z0 + (0.5 + dj)*dalt;

            fault_X1 = (x0 + di*xsoff + dj*xdoff)*1.e3;
            fault_Y1 = (y0 + di*ysoff + dj*ydoff)*1.e3;
            fault_Z1 = z0 + dj*dalt; // km

            fault_X2 = (x0 + (di + 1.0)*xsoff + dj*xdoff)*1.e3;
            fault_Y2 = (y0 + (di + 1.0)*ysoff + dj*ydoff)*1.e3;
            fault_Z2 = z0 + dj*dalt; // km

            fault_X3 = (x0 + (di + 1.0)*xsoff + (dj + 1.0)*xdoff)*1.e3; //km->m
            fault_Y3 = (y0 + (di + 1.0)*ysoff + (dj + 1.0)*ydoff)*1.e3; //km->m
            fault_Z3 = z0 + (dj + 1.0)*dalt; // km

            fault_X4 = (x0 + di*xsoff + (dj + 1.0)*xdoff)*1.e3; //km->m
            fault_Y4 = (y0 + di*ysoff + (dj + 1.0)*ydoff)*1.e3; //km->m
            fault_Z4 = z0 + (dj + 1.0)*dalt; // km

            // Convert from UTMs back to lat/lon 
            fault_X = fault_X*1000.0; // km -> m
            fault_Y = fault_Y*1000.0; // km -> m
            core_coordtools_utm2ll(zone_loc, lnorthp,
                                   fault_Y, fault_X,
                                   &latF, &lonF);
            k = j*nstr + i;
            fault_ptr[k+1] = 4*(k + 1);
            // Generate output
            core_coordtools_utm2ll(zone_loc, lnorthp,
                                   fault_Y1, fault_X1,
                                   &lat_vtx[4*k+0], &lon_vtx[4*k+0]);
            core_coordtools_utm2ll(zone_loc, lnorthp,
                                   fault_Y2, fault_X2,
                                   &lat_vtx[4*k+1], &lon_vtx[4*k+1]);
            core_coordtools_utm2ll(zone_loc, lnorthp,
                                   fault_Y3, fault_X3,
                                   &lat_vtx[4*k+2], &lon_vtx[4*k+2]);
            core_coordtools_utm2ll(zone_loc, lnorthp,
                                   fault_Y4, fault_X4,
                                   &lat_vtx[4*k+3], &lon_vtx[4*k+3]);
            dep_vtx[4*k+0] = fault_Z1; //depF;
            dep_vtx[4*k+1] = fault_Z2; //depF;
            dep_vtx[4*k+2] = fault_Z3; //depF;
            dep_vtx[4*k+3] = fault_Z4; //depF;
            // Save the fault patch centers for the actual inversion 
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
