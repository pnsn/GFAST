#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "gfast.h"
/*!
 * @brief Sets the RHS for the finite fault inversion.  The right hand
 *        side for the i'th site is packed 
 *        \f$ \{ e_{avg}^{(i)}, n_{avg}^{(i)}, u_{avg}^{(i)} \} \f$.
 *
 * @param[in] n         number of points
 * @param[in] verbose   controls verbosity (0 is quiet)
 * @param[in] nAvgDisp  average displacement on north channel [n]
 * @param[in] eAvgDisp  average displacement on east channel [n]
 * @param[in] zAvgDisp  average displacement on vertical channel [n]
 *
 * @param[out] U        right hand side in Gm = U [3*n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date April 2016
 *
 */
int GFAST_FF__setRHS(int n, int verbose,
                     double *__restrict__ nAvg,
                     double *__restrict__ eAvg,
                     double *__restrict__ uAvg,
                     double *__restrict__ U)
{
    const char *fcnm = "GFAST_FF__setRHS\0";
    int i, i3; 
    if (n < 1){ 
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n); 
        return -1; 
    }   
    i3 = 0;
    #pragma omp simd
    for (i=0; i<n; i++){
        i3 = 3*i;
        U[i3+0] = eAvg[i];
        U[i3+1] = nAvg[i];
        U[i3+2] = uAvg[i]; 
    }   
    return 0;
}
