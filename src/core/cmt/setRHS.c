#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Sets the RHS for the moment tensor inversion.  The right hand
 *        side for the i'th site is packed 
 *        \f$ \{ n_{avg}^{(i)}, e_{avg}^{(i)}, -u_{avg}^{(i)} \} \f$.
 *
 * @param[in] n         number of observations 
 * @param[in] nOffset   offset measured on north channel [n]
 * @param[in] eOffset   offset measured on east channel [n]
 * @param[in] uOffset   offset measured on vertical channel [n]
 *
 * @param[out] U        right hand side in Gm = U [3*n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_cmt_setRHS(const int n,
                    const double *__restrict__ nOffset,
                    const double *__restrict__ eOffset,
                    const double *__restrict__ uOffset,
                    double *__restrict__ U)
{
    const char *fcnm = "core_setRHS__cmt\0";
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
        U[i3+0] = nOffset[i];
        U[i3+1] = eOffset[i];
        U[i3+2] =-uOffset[i]; 
    }
    return 0;
}
