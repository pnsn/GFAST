#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "gfast.h"
/*!
 * @brief Sets the forward modeling matrix G s.t.
 *        \f$ G = \left [ B + C \log_{10}(r) \right ] \f$
 *        where B and C are scalar shifts and scale factors respectively
 *        and r the hypocentral distances for all stations.
 *
 * @param[in] n        length of r and G (> 0)
 * @param[in] verbose  controls verbosity (< 2 is quiet)
 * @param[in] B        linear shift in G = B + C*log10(r)
 * @param[in] C        scale factor in G = B + C*log10(r) 
 * @param[in] r        hypocentral distances (km) [n] 
 *
 * @param[out] G       forward modeling matrix for PGD [n x 1].  Note that
 *                     G is in column major format.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                       double B, double C,
                                       const double *__restrict__ r,
                                       double *__restrict__ G)
{
    const char *fcnm = "GFAST_scaling_PGD__setForwardModel\0";
    int i;
    if (n < 1){
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        return -1;
    }
    #pragma omp simd aligned(r,G:CACHE_LINE_SIZE)
    for (i=0; i<n; i++){
        G[i] = B + C*(log10(r[i]));
    }
    return 0;
}
