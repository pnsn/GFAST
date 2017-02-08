#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Applies the diagonal data weight matrix to the observations
 *
 * @param[in] mrows      number of rows (observations - should be 3 x number
 *                       of sites)
 * @param[in] diagWt     diagonal matrix of data weights [mrows]
 * @param[in] b          observations [mrows]
 *
 * @param[out] diagWb    weighted observations such that
 *                       \f$ \tilde{b} = diag \{W\} b \f$ [mrows]
 *
 * @result -1 indicates an error
 *          0 indicates success
 *          1 indicates that diagWt is NULL and it is assumed that diagWt
 *            is identity
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_ff_weightObservations(const int mrows,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ b,
                               double *__restrict__ diagWb)
{
    const char *fcnm = "core_ff_weightObservations\0";
    int i;
    if (mrows < 1)
    {
        log_errorF("%s: Error no rows!\n", fcnm);
        return -1;
    }
    if (b == NULL || diagWb == NULL)
    {
        if (b == NULL){log_errorF("%s: Error b is NULL\n", fcnm);}
        if (diagWb == NULL){log_errorF("%s: Error diagWb is NULL\n", fcnm);}
        return -1;
    }
    if (diagWt == NULL)
    {
        log_warnF("%s: Warning diagWt is NULL - assuming identity\n", fcnm);
        cblas_dcopy(mrows, b, 1, diagWb, 1);
        return 1;
    }
    // Apply diagonal data weights to observations
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<mrows; i++)
    {
        diagWb[i] = diagWt[i]*b[i];
    }
    return 0;
}
