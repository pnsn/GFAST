#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "gfast.h"
/*!
 * @brief Sets the diagonal weight matrix in the inversion.
 *
 * @param[in] n         number of of observations 
 * @param[in] verbose   controls verbosity (0 is quiet)
 * @param[in] nWts      data weights on north observation [n]
 * @param[in] eWts      data weights on east observation [n]
 * @param[in] uWts      data weights on vertical observations [n]
 *
 * @param[out] diagWt   diagonal matrix of data weights to apply to
 *                      $\f diag{W} G m = diag{W} U $\f [3*n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_CMT__setDiagonalWeightMatrix(int n, int verbose,
                                       const double *__restrict__ nWts,
                                       const double *__restrict__ eWts,
                                       const double *__restrict__ uWts,
                                       double *__restrict__ diagWt)
{
    const char *fcnm = "GFAST_CMT__setDiagonalWeightMatrix\0";
    int i, i3;
    if (n < 1)
    {
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        return -1;
    }
    i3 = 0;
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        i3 = 3*i;
        diagWt[i3+0] = nWts[i];
        diagWt[i3+1] = eWts[i];
        diagWt[i3+2] = uWts[i]; 
    }
    return 0;
}
