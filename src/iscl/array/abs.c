#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif
#include "iscl/memory/memory.h"

/*!
 * @defgroup array_abs abs
 * @brief Computes the absolute value of an array.
 * @ingroup array
 */
/*!
 * @brief Computes the absolute value of each element of a double array x.
 *
 * @param[in] n      Number of elements in x.  This must be positive.
 * @param[in] x      Array of which to compute absolute values.  This has
 *                   dimension [n].
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of length n s.t. \f$ y_i = |x_i| \f$.
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
double *array_abs64f(const int n, double *__restrict__ x, 
                     enum isclError_enum *ierr)
{
    double *y = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    y = memory_calloc64f(n);
    // Compute absolute value
    *ierr = array_abs64f_work(n, x, y);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error calling compute absolute value!");
        memory_free64f(&y);
    }
    return y;
}
//============================================================================//
/*!
 * @brief Computes the absolute value of each element of a double array x.
 *
 * @param[in] n      Number of elements in x.  This must be positive.
 * @param[in] x      Array of which to compute absolute values.  This has
 *                   dimension [n].
 *
 * @param[out] y     Array of dimension [n] s.t. \f$ y_i = |x_i| \f$.
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    array_abs64f_work(const int n,
                      double *__restrict__ x,
                      double *__restrict__ y)
{
#ifdef ISCL_USE_INTEL
    Ipp64f *pSrc, *pDst, *pSrcDst;
    IppStatus status; 
#else
    int i;
#endif
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
    isclReturnNullPointerError("y", y);
#ifdef ISCL_USE_INTEL
    if (x != y)
    {
        pSrc = (Ipp64f *) x;
        pDst = (Ipp64f *) y;
        status = ippsAbs_64f(pSrc, pDst, n);
        pSrc = NULL;
        pDst = NULL;
        if (status != ippStsNoErr)
        {
            isclPrintError("%s", "Error calling abs");
            return ISCL_IPP_FAILURE;
        }
    }
    else
    {
        pSrcDst = (Ipp64f *) x;
        status = ippsAbs_64f_I(pSrcDst, n); 
        if (status != ippStsNoErr)
        {
            isclPrintError("%s", "Error calling abs inplace");
            return ISCL_IPP_FAILURE;
        }
    }
#else
    if (x != y)
    {
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            y[i] = fabs(x[i]);
        }
    }
    else
    {
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            x[i] = fabs(x[i]);
        }
    }
#endif
    return ISCL_SUCCESS; 
}
//============================================================================//
/*!
 * @brief Computes the absolute values of each element of an integer array x.
 *
 * @param[in] n      Number of elements in x.  This must be positive.
 * @param[in] x      Array of which to compute absolute values.  This has
 *                   dimension [n].
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n]. s.t. \f$ y_i = |x_i| \f$.
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
int *array_abs32i(const int n, int *__restrict__ x,
                   enum isclError_enum *ierr)
{
    int *y = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    y = memory_calloc32i(n);
    // Compute absolute value
    *ierr = array_abs32i_work(n, x, y);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error compute absolute value!");
        memory_free32i(&y);
    }
    return y;
}
//============================================================================//
/*!
 * @brief Computes the absolute value of each element of an integer array x.
 *
 * @param[in] n      Number of elements in x.
 * @param[in] x      Array of which to compute absolute values.  This has 
 *                   dimension [n].
 *
 * @param[out] y     Array of dimension [n] s.t. \f$ y_i = |x_i| \f$.
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success. 
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    array_abs32i_work(const int n,
                      int *__restrict__ x,
                      int *__restrict__ y)
{
    int i;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
    isclReturnNullPointerError("y", y);
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        y[i] = abs(x[i]);
    }
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Computes the absolute values of each element of a complex array x.
 *
 * @param[in] n      Number of elements in x.  This must be positive.
 * @param[in] x      Array of which to compute absolute values.  This has 
 *                   dimension [n].
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] s.t. \f$ y_i = |x_i| \f$.
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
double *array_abs64z(const int n,
                     const double complex *__restrict__ x, 
                     enum isclError_enum *ierr)
{
    double *y = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    y = memory_calloc64f(n);
    // Compute absolute value
    *ierr = array_abs64z_work(n, x, y);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error calling compute absolute value!");
        memory_free64f(&y);
    }
    return y;
}
//============================================================================//
/*!
 * @brief Computes the absolute value of each element of a complex array x.
 *
 * @param[in] n      Number of elements in x.
 * @param[in] x      Array of which to compute absolute values.  This has
 *                   dimension [n].
 *
 * @param[out] y     Array of dimension [n] s.t. \f$ y_i = |x_i| \f$.
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_abs
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    array_abs64z_work(const int n, const double complex *__restrict__ x,
                      double *__restrict__ y)
{
#ifdef ISCL_USE_INTEL
    Ipp64fc *pSrc;
    Ipp64f *pDst;
    IppStatus status;
#else
    int i;
#endif
    enum isclError_enum ierr;
    ierr = ISCL_SUCCESS;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
    isclReturnNullPointerError("y", y);
#ifdef ISCL_USE_INTEL
    pSrc = (Ipp64fc *) x;
    pDst = (Ipp64f *) y; 
    status = ippsMagnitude_64fc(pSrc, pDst, n);
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Failed to compute magnitude");
        ierr = ISCL_IPP_FAILURE;
    }
    pSrc = NULL;
    pDst = NULL;
#else
    for (i=0; i<n; i++)
    {
        y[i] = cabs(x[i]);
    }
#endif
    return ierr;
}
