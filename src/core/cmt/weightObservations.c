#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
/*!
 * @brief Applies the diagonal data weight matrix to the observations.
 *
 * @param[in] mrows      Number of rows (observations).  This should be
 *                       3 x number of sites.
 * @param[in] diagWt     Diagonal matrix of data weights.  This is an array
 *                       of dimension [mrows].
 * @param[in] b          Observations.  This is an array of dimensoin [mrows].
 *
 * @param[out] diagWb    Weighted observations such that
 *                       \f$ \tilde{b} = diag \{W\} b \f$.
 *                       This is an array of dimension [mrows].
 *
 * @retval -1 indicates an error.
 * @retval  0 indicates success.
 * @retval  1 indicates that diagWt is NULL and it is assumed that diagWt
 *            is identity.
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_cmt_weightObservations(const int mrows,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ b,
                                double *__restrict__ diagWb)
{
    int i;
    if (mrows < 1)
    {
        LOG_ERRMSG("%s", "Error no rows!");
        return -1;
    }
    if (b == NULL || diagWb == NULL)
    {
        if (b == NULL){LOG_ERRMSG("%s", "Error b is NULL");}
        if (diagWb == NULL){LOG_ERRMSG("%s", "Error diagWb is NULL");}
        return -1;
    }
    if (diagWt == NULL)
    {
        LOG_WARNMSG("%s", "Warning diagWt is NULL - assuming identity");
        array_copy64f_work(mrows, b, diagWb);
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
