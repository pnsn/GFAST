#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <cblas.h>
#include "gfast_core.h"
#include "iscl/log/log.h"

/*!
 * @brief Weights a forward modeling matrix by the diagonal data weights
 *
 * @param[in] mrows     number of rows in forward modeling matrix (should
 *                      be 3*number_of_observations)
 * @param[in] ncols     number of columns in forward modeling matrix
 *                      (should be 5 or 6)
 * @param[in] diagWt    diagonal of data weight matrix [mrows]
 * @param[in] G         unweighted forward modeling matrix.  This is
 *                      in row major format and is of size [mrows x ncols]
 *
 * @param[out] diagWtG  weighted forward modeling matrix obtained by
 *                      computing \f$ \tilde{G} \leftarrow diag\{W\} G \f$.
 *                      This is in row major format and is of size
 *                      [mrows x ncols]
 *
 * @result -1 indicates an input error
 *          0 indicates success
 *          1 indicates the diagonal weight matrix is NULL.  G will
 *            not be modified in this instance.
 *
 * @author Ben Baker, ISTI
 *
 */
int core_cmt_weightForwardModel(const int mrows, const int ncols,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ G,
                                double *__restrict__ diagWtG)
{
    const char *fcnm = "core_cmt_weightForwardModel\0";
    int i, j;
    if (mrows < 1 || ncols < 1)
    {
        log_errorF("%s: Error G has no dimension\n", fcnm);
        return -1;
    }
    if (G == NULL || diagWtG == NULL)
    {
        if (G == NULL){log_errorF("%s: Error G cannot be NULL\n", fcnm);}
        if (diagWtG == NULL)
        {
            log_errorF("%s: Error diagWtG cannot be NULL\n", fcnm);
        }
        return -1;
    }
    // Avoid a segmentation fault
    if (diagWt == NULL)
    {
        log_warnF("%s: Warning diagWt is NULL; G will not be weighted\n",
                  fcnm);
        cblas_dcopy(mrows*ncols, G, 1, diagWtG, 1);
        return 1;
    }
    // Compute \tilde{G} = diag\{W\}*G - most likely a deviatoric source
    if (ncols == 5)
    {
        #pragma omp simd collapse(2)
        for (i=0; i<mrows; i++)
        {
            for (j=0; j<5; j++)
            {
                diagWtG[i*ncols+j] = G[i*ncols+j]*diagWt[i];
            }
        }
    }
    // Full blown moment tensor
    else if (ncols == 6)
    {
        #pragma omp simd collapse(2)
        for (i=0; i<mrows; i++)
        {   
            for (j=0; j<6; j++)
            {   
                diagWtG[i*ncols+j] = G[i*ncols+j]*diagWt[i];
            }   
        }
    }
    // Really shouldn't be here
    else
    {
        log_warnF("%s: Strange number of columns\n", fcnm); 
        #pragma omp simd collapse(2)
        for (i=0; i<mrows; i++)
        {   
            for (j=0; j<ncols; j++)
            {   
                diagWtG[i*ncols+j] = G[i*ncols+j]*diagWt[i];
            }   
        }
    }
    return 0;
}
