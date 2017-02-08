#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
/*!
 * @brief Applies the diagonal data weight matrix to the observations
 *
 * @param[in] l1         number of observations
 * @param[in] W          diagonal matrix of data weights [l1]
 * @param[in] b          observations [l1]
 *
 * @param[out] Wb        weighted observations such that
 *                       \f$ \tilde{b} = diag \{W\} b \f$ [l1]
 *
 * @result -1 -> indicates an error.
 *          0 -> indicates success.
 *          1 -> indicates that W is NULL and it is assumed that W
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
    const char *fcnm = "core_scaling_pgd_weightObservations\0";
    int i;
    if (l1 < 1)
    {
        log_errorF("%s: Error no observations\n", fcnm);
        return -1;
    }
    if (b == NULL || Wb == NULL)
    {
        if (b == NULL){log_errorF("%s: Error b is NULL\n", fcnm);}
        if (Wb == NULL){log_errorF("%s: Error Wb is NULL\n", fcnm);}
        return -1;
    }
    // Don't break anything if the weights are NULL
    if (W == NULL)
    {
        log_warnF("%s: Warning W is NULL - assuming identity\n", fcnm);
        cblas_dcopy(l1, b, 1, Wb, 1);
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
