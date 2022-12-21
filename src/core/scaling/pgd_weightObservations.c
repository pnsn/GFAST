#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
/*!
 * @brief Applies the diagonal data weight matrix to the observations.
 *
 * @param[in] l1         Number of observations.
 * @param[in] W          Diagonal matrix of data weights.  This is an
 *                       array of dimension [l1].
 * @param[in] b          Observations.  This is an array of dimension [l1].
 *
 * @param[out] Wb        Weighted observations such that
 *                       \f$ \tilde{b} = diag \{W\} b \f$.
 *                       This is an array of dimension [l1].
 *
 * @retval -1 -> indicates an error.
 * @retval  0 -> indicates success.
 * @retval  1 -> indicates that W is NULL and it is assumed that W
 *               is identity.
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_weightObservations(const int l1,
                                        const double *__restrict__ W,
                                        const double *__restrict__ b,
                                        double *__restrict__ Wb)
{
    int i;
    if (l1 < 1)
    {
        LOG_ERRMSG("Error no observations %d", l1);
        return -1;
    }
    if (b == NULL || Wb == NULL)
    {
        if (b == NULL){LOG_ERRMSG("%s", "Error b is NULL");}
        if (Wb == NULL){LOG_ERRMSG("%s", "Error Wb is NULL");}
        return -1;
    }
    // Don't break anything if the weights are NULL
    if (W == NULL)
    {
        LOG_WARNMSG("%s", "Warning W is NULL - assuming identity");
        array_copy64f_work(l1, b, Wb);
        return 1;
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<l1; i++)
    {
        Wb[i] = W[i]*b[i];
    }
    return 0;
}
