#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h> 
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Sets the diagonal weight matrix in the inversion.
 *
 * @param[in] n         number of of observations 
 * @param[in] nWts      data weights on north observation [n]
 * @param[in] eWts      data weights on east observation [n]
 * @param[in] uWts      data weights on vertical observations [n]
 *
 * @param[out] diagWt   diagonal matrix of data weights to apply to
 *                      \f$ diag{W} G m = diag{W} U \f$ [3*n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_cmt_setDiagonalWeightMatrix(const int n,
                                     const double *__restrict__ nWts,
                                     const double *__restrict__ eWts,
                                     const double *__restrict__ uWts,
                                     double *__restrict__ diagWt)
{
    const char *fcnm = "core_cmt_setDiagonalWeightMatrix\0";
    int i, i3;
    bool leWts, lnWts, luWts;
    if (n < 1)
    {
        log_errorF("%s: Invalid number of points: %d\n", fcnm, n);
        return -1;
    }
    if (diagWt == NULL)
    {
        log_errorF("%s: Error diagWt is NULL!\n", fcnm);
        return -1;
    }
    if (nWts == NULL || eWts == NULL || uWts == NULL)
    {
        if (nWts == NULL && eWts == NULL && uWts == NULL)
        {
            log_warnF("%s: Setting diagonal weight matrix to unity\n", fcnm);
            #pragma omp simd
            for (i=0; i<3*n; i++)
            {
                diagWt[i] = 1.0;
            }
        }
        else // Selectively make weights 1
        {
            lnWts = true;
            leWts = true;
            luWts = true;
            if (nWts == NULL)
            {
                log_warnF("%s: Setting nWts to unity\n", fcnm);
                lnWts = false;
            }
            if (eWts == NULL)
            {
                log_warnF("%s: Setting eWts to unity\n", fcnm);
                leWts = false;
            }
            if (uWts == NULL)
            {
                log_warnF("%s: Setting uWts to unity\n", fcnm);
                luWts = false;
            }
            i3 = 0;
            if (lnWts && leWts && luWts)
            {
                #pragma omp simd
                for (i=0; i<n; i++)
                {
                    i3 = 3*i;
                    diagWt[i3+0] = nWts[i];
                    diagWt[i3+1] = eWts[i];
                    diagWt[i3+2] = uWts[i];
                }
            }
            else
            {
                for (i=0; i<n; i++)
                {   
                    i3 = 3*i;
                    diagWt[i3+0] = 1.0;
                    if (lnWts){diagWt[i3+0] = nWts[i];}
                    diagWt[i3+1] = 1.0;
                    if (leWts){diagWt[i3+1] = eWts[i];}
                    diagWt[i3+2] = 1.0;
                    if (luWts){diagWt[i3+2] = uWts[i];}
                }
            }
        }
        return 0;
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
