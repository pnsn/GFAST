#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
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
int core_ff_setDiagonalWeightMatrix(const int n,
                                    const double *__restrict__ nWts,
                                    const double *__restrict__ eWts,
                                    const double *__restrict__ uWts,
                                    double *__restrict__ diagWt)
{
    int i, i3;
    bool leWts, lnWts, luWts;
    if (n < 1)
    {
        LOG_ERRMSG("Invalid number of points: %d", n);
        return -1;
    }
    if (diagWt == NULL)
    {
        LOG_ERRMSG("%s", "Error diagWt is NULL!");
        return -1;
    }
    if (nWts == NULL || eWts == NULL || uWts == NULL)
    {
        // Weights not specified
        if (nWts == NULL && eWts == NULL && uWts == NULL)
        {
            LOG_WARNMSG("%s", "Setting diagonal weight matrix to unity");
            array_set64f_work(3*n, 1.0, diagWt);
        }
        else // Selectively make weights 1
        {
            lnWts = true;
            leWts = true;
            luWts = true;
            if (nWts == NULL)
            {
                LOG_WARNMSG("%s", "Setting nWts to unity");
                lnWts = false;
            }
            if (eWts == NULL)
            {
                LOG_WARNMSG("%s", "Setting eWts to unity");
                leWts = false;
            }
            if (uWts == NULL)
            {
                LOG_WARNMSG("%s", "Setting uWts to unity");
                luWts = false;
            }
            i3 = 0;
            if (lnWts && leWts && luWts)
            {
#ifdef _OPENMP
                #pragma omp simd
#endif
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
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<n; i++)
    {
        i3 = 3*i;
        diagWt[i3+0] = eWts[i];
        diagWt[i3+1] = nWts[i];
        diagWt[i3+2] = uWts[i];
    }
    return 0;
}
