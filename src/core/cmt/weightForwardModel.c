#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/array/array.h"

/*!
 * @brief Weights a forward modeling matrix by the diagonal data weights.
 *
 * @param[in] mrows     Number of rows in forward modeling matrix.  This
 *                      should be 3 x number of observations.
 * @param[in] ncols     Number of columns in forward modeling matrix.
 *                      This should be 5 or 6.
 * @param[in] diagWt    Diagonal of data weight matrix.  This is an array
 *                      of dimension [mrows].
 * @param[in] G         Unweighted forward modeling matrix.  This is
 *                      in row major format and has dimension [mrows x ncols].
 *
 * @param[out] diagWtG  Weighted forward modeling matrix obtained by
 *                      computing \f$ \tilde{G} \leftarrow diag\{W\} G \f$.
 *                      This is in row major format and has of dimension 
 *                      [mrows x ncols]
 *
 * @result -1 indicates an input error. \n
 *          0 indicates success. \n
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
    int i, j;
    if (mrows < 1 || ncols < 1)
    {
        LOG_ERRMSG("Error G has invalid dimension %d %d", mrows, ncols);
        return -1;
    }
    if (G == NULL || diagWtG == NULL)
    {
        if (G == NULL){LOG_ERRMSG("%s", "Error G cannot be NULL");}
        if (diagWtG == NULL){LOG_ERRMSG("%s", "Error diagWtG cannot be NULL");}
        return -1;
    }
    // Avoid a segmentation fault
    if (diagWt == NULL)
    {
        LOG_WARNMSG("%s", "Warning diagWt is NULL; G will not be weighted");
        array_copy64f_work(mrows*ncols, G, diagWtG);
        return 1;
    }
    // Compute \tilde{G} = diag\{W\}*G - most likely a deviatoric source
    if (ncols == 5)
    {
#ifdef _OPENMP
        #pragma omp simd collapse(2)
#endif
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
#ifdef _OPENMP
        #pragma omp simd collapse(2)
#endif
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
        LOG_WARNMSG("Strange number of columns = %d", ncols); 
#ifdef _OPENMP
        #pragma omp simd collapse(2)
#endif
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
