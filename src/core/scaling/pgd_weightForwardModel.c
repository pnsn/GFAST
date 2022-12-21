#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
/*!
 * @brief Applies the diagonal data weight matrix to the forward modeling
 *        matrix.  In the case of PGD scaling the forward modeling matrix
 *        is a column vector. 
 *
 * @param[in] l1         number of observations
 * @param[in] W          diagonal matrix of data weights [l1]
 * @param[in] G          forward modeling matrix [l1]
 *
 * @param[out] WG        weighted observations such that
 *                       \f$ \tilde{G} = diag \{W\} G \f$ [l1]
 *
 * @result -1 indicates an error
 *          0 indicates success
 *          1 indicates that W is NULL and it is assumed that W
 *            is identity
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_weightForwardModel(const int l1,
                                        const double *__restrict__ W,
                                        const double *__restrict__ G,
                                        double *__restrict__ WG)
{
    int i;
    if (l1 < 1)
    {
        LOG_ERRMSG("Error no observations %d", l1);
        return -1;
    }
    if (G == NULL || WG == NULL)
    {
        if (G == NULL){LOG_ERRMSG("%s", "Error G is NULL");}
        if (WG == NULL){LOG_ERRMSG("%s", "Error WG is NULL");}
        return -1;
    }
    // Don't break anything if the weights are NULL
    if (W == NULL)
    {
        LOG_WARNMSG("%s", "Warning W is NULL - assuming identity");
        array_copy64f_work(l1, G, WG);
        return 1;
    }
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (i=0; i<l1; i++)
    {
        WG[i] = W[i]*G[i];
    }
    return 0;
}
