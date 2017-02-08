#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
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
    const char *fcnm = "core_scaling_pgd_weightForwardModel\0";
    int i;
    if (l1 < 1)
    {
        log_errorF("%s: Error no observations\n", fcnm);
        return -1;
    }
    if (G == NULL || WG == NULL)
    {
        if (G == NULL){log_errorF("%s: Error G is NULL\n", fcnm);}
        if (WG == NULL){log_errorF("%s: Error WG is NULL\n", fcnm);}
        return -1;
    }
    // Don't break anything if the weights are NULL
    if (W == NULL)
    {
        log_warnF("%s: Warning W is NULL - assuming identity\n", fcnm);
        cblas_dcopy(l1, G, 1, WG, 1);
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
