#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"
/*!
 * @brief Sets the forward modeling matrix G s.t.
 *        \f$ G = \left [ B + C \log_{10}(r) \right ] \f$
 *        where B and C are scalar shifts and scale factors respectively
 *        and r the hypocentral distances for all stations.
 *
 * @param[in] n        length of r and G (> 0)
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
int core_scaling_pgd_setForwardModel(const int n,
                                     const double B, const double C,
                                     const double *__restrict__ r,
                                     double *__restrict__ G)
{
    int i;
    if (n < 1 || r == NULL || G == NULL)
    {
        if (n < 1){LOG_ERRMSG("Invalid number of points: %d\n", n);}
        if (r == NULL){LOG_ERRMSG("%s", "Error r is NULL");}
        if (G == NULL){LOG_ERRMSG("%s", "Error G is NULL");}
        return -1;
    }
    for (i=0; i<n; i++)
    {
        G[i] = B + C*(log10(r[i]));
    }
    return 0;
}
