#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
#include "iscl/log/log.h"
/*!
 * @brief Sets the diagonal weighting matrix for the PGD scaling.
 *        The diagonal weight matrix terms are defined by
 *        \f[ W_i = w_i e^{ \left (-\frac{r_i^2}{8 r_{min}} \right ) } \f]
 *        where \f$ r_i\f$ is the epicentral distance and \f$ w_i \f$ the 
 *        data weight.  If \f$ w_i\f$ is not set it is assumed to be
 *        unity.  
 *
 * @param[in] l1         number of observations
 * @param[in] wts        if not NULL then these are the data weights
 *                       for each observation [l1]
 * @param[in] repi       this epicentral distance (km) for the i'th 
 *                       observation [l1]
 *
 * @param[out] W         the diagonal weight matrix corresponding to
 *                       each observation [l1]
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_setDiagonalWeightMatrix(const int l1,
                                             const double *__restrict__ repi,
                                             const double *__restrict__ wts,
                                             double *__restrict__ W)
{
    const char *fcnm = "core_scaling_pgd_setDiagonalWeightMatrix\0";
    double repi_min, repi_min2;
    int i;
    if (l1 < 1)
    {
        log_errorF("%s: Error no observations\n", fcnm);
        return -1;
    }
    if (W == NULL || repi == NULL)
    {
        if (repi == NULL){log_errorF("%s: Error repi is NULL\n", fcnm);}
        if (W == NULL){log_errorF("%s: Error W is NULL\n", fcnm);}
        return -1;
    }
    // Get the min epicentral distance
    repi_min = array_min__double(l1, repi);
    repi_min2 = pow(repi_min, 2);
    // Set standard weights 
    for (i=0; i<l1; i++)
    {
        W[i] = exp(-pow(repi[i], 2)/8.0/repi_min2);
    }
    // Apply the additional data weights 
    if (wts != NULL)
    {
        for (i=0; i<l1; i++)
        {
            W[i] = W[i]*wts[i];
        }
    }
    return 0;
}
