#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Sets the RHS for the finite fault inversion.  The right hand
 *        side for the i'th site is packed 
 *        \f$ \{ e_{avg}^{(i)}, n_{avg}^{(i)}, u_{avg}^{(i)} \} \f$.
 *
 * @param[in] n         number of points
 * @param[in] nOffset   offset on north channel [n]
 * @param[in] eOffset   offset on east channel [n]
 * @param[in] uOffset   offset on vertical channel [n]
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
int core_ff_setRHS(const int n,
                   const double *__restrict__ nOffset,
                   const double *__restrict__ eOffset,
                   const double *__restrict__ uOffset,
                   double *__restrict__ U)
{
    const char *fcnm = "core_ff_setRHS\0";
    int i, i3; 
    if (n < 1)
    {
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n); 
        return -1; 
    }   
    i3 = 0;
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<n; i++)
    {
        i3 = 3*i;
        U[i3+0] = eOffset[i];
        U[i3+1] = nOffset[i];
        U[i3+2] = uOffset[i]; 
    }   
    return 0;
}
