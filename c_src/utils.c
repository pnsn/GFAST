#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*!
 * @brief Computes the arithmetic mean of x where any NaN's are ignored
 * 
 * @param[in] n     number of elements in x
 * @param[in] x     array from which to compute mean (while ignoring NaNs)
 *
 * @result arithmetic mean of array x where NaN's have been ignored
 *
 * @author Ben Baker, ISTI
 *
 */
double numpy_nanmean(int n, double *x)
{
    const char *fcnm = "numpy_nanmean\0";
    double xavg;
    int i, iavg;
    iavg = 0;
    xavg = 0.0;
    for (i=0; i<n; i++){
        if (isnan(x[i]) == 0){
            xavg = xavg + x[i];
            iavg = iavg + 1;
        }
     }
    if (iavg == 0){
        if (n > 0){
            printf("%s: Warning all elements of x are NaN's\n", fcnm);
        }else{
            printf("%s: Warning division by zero\n", fcnm);
        }
        xavg = NAN; 
    }else{
        xavg = xavg/(double) iavg;
    }
    return xavg;
}

/*!
 * @brief Rotates horizontal components of a seismogram. 
 *        The north and east components of a seismogram will be rotated
 *        into radial and transversal components.  The angle is given as
 *        the back-azimuth, that is defined as the angle measured between
 *        the vector pointing from the station to the source, and the 
 *        vector pointing from the station from the station to the north.
 *        https://docs.obspy.org/_modules/obspy/signal/rotate.html
 *
 * @param[in] np   number of data points 
 * @param[in] e    data on east component [np]
 * @param[in] n    data on north component [np]
 * @param[in] ba   the source to station back-azimuth in degrees measured
 *                 positive counter-clockwise from north
 *
 * @param[out] r   data rotated onto the radial component [np]
 * @param[out] t   data rotated onto the east component [np]
 *
 * @author Ben Baker, ISTI
 */
inline void rotate_NE_RT(int np, double *e, double *n, double ba,
                         double *r, double *t)
{
    const char *fcnm = "rotate_NE_RT\0";
    const double pi180 = M_PI/180.0;
    double cosbaz, sinbaz, et, nt;
    int i;
    if (np < 1){
        printf("%s: Warning there are no points %d\n", fcnm, np);
        return;
    }
    if (ba < 0.0 || ba > 360.0){
        printf("%s: Warning back-azimuth=%f should be between [0,360]\n",
               fcnm, ba);
    }
    cosbaz = cos((ba + 180.0)*pi180);
    sinbaz = sin((ba + 180.0)*pi180);
    for (i=0; i<np; i++){
        et = e[i];
        nt = n[i];
        r[i] = et*sinbaz + nt*cosbaz;
        t[i] = et*cosbaz - nt*sinbaz;
    }
    return;
}
