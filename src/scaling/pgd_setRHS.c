#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h> 
#include "gfast.h"
/*!
 * @brief Computes the right hand side in the peak ground displacement 
 *        estimation s.t.
 *        \f$ \textbf{b} = \left \{ \log_{10}(d) - A \right \} \f$
 *        where A is a scalar shift and d is the distance at each station.
 *
 * @param[in] n         number of points
 * @param[in] verbose   controls verbosity (0 is quiet)
 * @param[in] dist_tol  distance tolerance - if d is less than this then
 *                      it will be set to a default value (cm)
 * @param[in] dist_def  distance default value (cm)
 * @param[in] A         shift so that b = log10(d) - A
 * @param[in] d         max distance (cm) at each site [n]
 *
 * @param[out] b        right hand side in Gm = b [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_scaling_PGD__setRHS(int n, int verbose,
                              double dist_tol, double dist_def,
                              double A,
                              const double *__restrict__ d,
                              double *__restrict__ b)
{
    const char *fcnm = "GFAST_scaling_PGD__setRHS\0";
    double dist;
    int i;
    if (n < 1){
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        return -1;
    }
    #pragma omp simd
    for (i=0; i<n; i++){
        dist = d[i];
        if (dist - dist_tol < 0.0){dist = dist_def;}
        b[i] = log10(dist) - A;
    }
    return 0;
}
