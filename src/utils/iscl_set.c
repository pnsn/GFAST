#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif
#include "iscl/memory/memory.h"

/*!
 * @defgroup array_set set
 * @brief Sets all values in an array to a constant value.
 * @ingroup array
 */
/*!
 * @brief Sets all values in an array to a constant value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] ierr   ISCL error flag where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is set to value.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
double *array_set64f(const int n, const double value,
                     enum isclError_enum *ierr)
{
    double *x = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    x = memory_malloc64f(n, ierr);
    *ierr = array_set64f_work(n, value, x);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting values");
        memory_free64f(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Sets all elements in a double array to the given value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] x      Each element in this array is set to the constant value.
 *                    This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
enum isclError_enum
    array_set64f_work(const int n, const double value,
                      double *__restrict__ x)
{
#ifdef ISCL_USE_INTEL
    Ipp64f *pdst;
    IppStatus status;
#else
    int i;
#endif
    //------------------------------------------------------------------------//
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
#ifdef ISCL_USE_INTEL
    pdst = (Ipp64f *) x;
    status = ippsSet_64f(value, pdst, n); 
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Failed setting value!");
        return ISCL_IPP_FAILURE;
    }
    pdst = NULL;
#else
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        x[i] = value;
    }
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Sets all values in an array to a constant value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] ierr   ISCL error flag where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is set to value.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
float *array_set32f(const int n, const float value, 
                    enum isclError_enum *ierr)
{
    float *x = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    x = memory_malloc32f(n, ierr);
    *ierr = array_set32f_work(n, value, x);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting values");
        memory_free32f(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Sets all elements in a float array to the given value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] x      Each element in this array is set to the constant value.
 *                    This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
enum isclError_enum
    array_set32f_work(const int n, const float value,
                      float *__restrict__ x)
{
#ifdef ISCL_USE_INTEL
    Ipp32f *pdst;
    IppStatus status;
#else
    int i;
#endif
    //------------------------------------------------------------------------//
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
#ifdef ISCL_USE_INTEL
    pdst = (Ipp32f *) x;
    status = ippsSet_32f(value, pdst, n);
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Failed setting value!");
        return ISCL_IPP_FAILURE;
    }
    pdst = NULL;
#else
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        x[i] = value;
    }
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Sets all values in an array to a constant value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] ierr   ISCL error flag where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is set to value.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
int *array_set32i(const int n, const int value, 
                  enum isclError_enum *ierr)
{
    int *x = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    x = memory_malloc32i(n, ierr);
    *ierr = array_set32i_work(n, value, x);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting values");
        memory_free32i(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Sets all elements in an integer array to the given value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] x      Each element in this array is set to the constant value.
 *                    This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
enum isclError_enum
    array_set32i_work(const int n, const int value,
                      int *__restrict__ x)
{
#ifdef ISCL_USE_INTEL
    Ipp32s *pdst;
    IppStatus status;
#else
    int i;
#endif
    //------------------------------------------------------------------------//
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
#ifdef ISCL_USE_INTEL
    pdst = (Ipp32s *) x;
    status = ippsSet_32s(value, pdst, n); 
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Failed setting value!");
        return ISCL_IPP_FAILURE;
    }   
    pdst = NULL;
#else
    #pragma omp simd
    for (i=0; i<n; i++)
    {   
        x[i] = value;
    }   
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Sets all values in an array to a constant value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] ierr   ISCL error flag where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is set to value.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
double complex *array_set64z(const int n, const double complex value,
                             enum isclError_enum *ierr)
{
    double complex *x = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    x = memory_malloc64z(n, ierr);
    *ierr = array_set64z_work(n, value, x);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting values");
        memory_free64z(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Sets all elements in a double complex array to the given value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] x      Each element in this array is set to the constant value.
 *                    This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
enum isclError_enum
    array_set64z_work(const int n, const double complex value,
                      double complex *__restrict__ x)
{
#ifdef ISCL_USE_INTEL
    Ipp64fc *pdst, val;
    IppStatus status;
#else
    int i;
#endif
    //------------------------------------------------------------------------//
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("x", x);
#ifdef ISCL_USE_INTEL
    pdst = (Ipp64fc *) x;
    val.re = creal(value);
    val.im = cimag(value);
    status = ippsSet_64fc(val, pdst, n); 
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Failed setting value!");
        return ISCL_IPP_FAILURE;
    }   
    pdst = NULL;
#else
    #pragma omp simd
    for (i=0; i<n; i++)
    {   
        x[i] = value;
    }   
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Sets all values in an array to a constant value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] ierr   ISCL error flag where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is set to value.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
bool *array_set8l(const int n, const bool value,
                  enum isclError_enum *ierr)
{
    bool *x = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    x = memory_malloc8l(n, ierr);
    *ierr = array_set8l_work(n, value, x);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting values");
        memory_free8l(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Sets all elements in a boolean array to the given value.
 *
 * @param[in] n       Number of elements in x.
 * @param[in] value   Value to set all elements of x to.
 *
 * @param[out] x      Each element in this array is set to the constant value.
 *                    This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_set
 *
 * @author Ben Baker (ISTI)
 *
 */
enum isclError_enum
    array_set8l_work(const int n, const bool value,
                     bool *__restrict__ x)
{
    int i;
    //------------------------------------------------------------------------//
    isclReturnArrayTooSmallError("n", n, 1); 
    isclReturnNullPointerError("x", x);
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        x[i] = value;
    }
    return ISCL_SUCCESS;
}
