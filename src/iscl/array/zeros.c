#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif
#include "iscl/memory/memory.h"

/*!
 * @defgroup array_zeros zeros
 * @brief Sets all elements of an array to zero.
 * @ingroup array
 */
/*!
 * @brief Creates a double array of length n filled with zeros.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] filled with zeros.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker, ISTI
 *
 */
double *array_zeros64f(const int n, enum isclError_enum *ierr)
{
    double *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc64f(n, ierr);
    *ierr = array_zeros64f_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting double zeros");
        memory_free64f(&zeros);
    }
    return zeros; 
}
//============================================================================//
/*!
 * @brief Zeroes out a double array of length n.
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array filled with zeros.  This has dimesnion [n].
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros64f_work(const int n, double *__restrict__ zeros)
{
#ifdef ISCL_USE_INTEL
    Ipp64f *psrc;
    IppStatus status;
#else
    size_t nz;
#endif
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("zeros", zeros);
#ifdef ISCL_USE_INTEL
    psrc = (Ipp64f *) zeros;
    status = ippsZero_64f(psrc, n);
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Error calling ippsZero_64f");
        return ISCL_IPP_FAILURE;
    }
    psrc = NULL;
#else
    nz = (size_t) n*sizeof(double);
    memset(zeros, 0.0, nz);
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Creates an integer array of length n filled with zeros.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] filled with zeros.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
int *array_zeros32i(const int n, enum isclError_enum *ierr)
{
    int *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc32i(n, ierr);
    *ierr = array_zeros32i_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting int zeros");
        memory_free32i(&zeros);
    }
    return zeros;
}
//============================================================================//
/*!
 * @brief Zeroes out an integer array of length n.
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array filled with zeros.  This has dimension [n].
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros32i_work(const int n, int *__restrict__ zeros)
{
#ifdef ISCL_USE_INTEL
    Ipp32s *psrc;
    IppStatus status;
#else
    size_t nz; 
#endif
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("zeros", zeros);
#ifdef ISCL_USE_INTEL
    psrc = (Ipp32s *) zeros;
    status = ippsZero_32s(psrc, n); 
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Error calling ippsZero_32s");
        return ISCL_IPP_FAILURE;
    }
    psrc = NULL;
#else
    nz = (size_t) n*sizeof(int);
    memset(zeros, 0, nz);
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Creates a float array of length n filled with zeros.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] filled with zeros.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
float *array_zeros32f(const int n, enum isclError_enum *ierr)
{
    float *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc32f(n, ierr);
    *ierr = array_zeros32f_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {   
        isclPrintError("%s", "Error setting float zeros\0");
        memory_free32f(&zeros);
    }
    return zeros;
}
//============================================================================//
/*!
 * @brief Zeroes out a float array of length n.
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array filled with zeros.  This has dimension [n].
 *
 * @result ISCL $eturn code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros32f_work(const int n, float *__restrict__ zeros)
{
#ifdef ISCL_USE_INTEL
    Ipp32f *psrc;
    IppStatus status;
#else
    size_t nz;
#endif 
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("zeros", zeros);
#ifdef ISCL_USE_INTEL
    psrc = (Ipp32f *) zeros;
    status = ippsZero_32f(psrc, n);
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Error calling ippsZero_32f");
        return ISCL_IPP_FAILURE;
    }
    psrc = NULL;
#else
    nz = (size_t) n*sizeof(int);
    memset(zeros, 0.0f, nz);
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Creates a complex array of length n filled with zeros.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] filled with zeros.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
double complex *array_zeros64z(const int n, enum isclError_enum *ierr)
{
    double complex *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc64z(n, ierr);
    *ierr = array_zeros64z_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {   
        isclPrintError("%s", "Error setting double complex zeros\0");
        memory_free64z(&zeros);
    }   
    return zeros; 
}
//============================================================================//
/*!
 * @brief Zeroes out a complex array of length n.
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array filled with zeros.  This has dimension [n].
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros64z_work(const int n,
                                        double complex *__restrict__ zeros)
{
#ifdef ISCL_USE_INTEL
    Ipp64fc *psrc;
    IppStatus status;
#else
    size_t nz; 
#endif
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("zeros", zeros);
#ifdef ISCL_USE_INTEL
    psrc = (Ipp64fc *) zeros;
    status = ippsZero_64fc(psrc, n); 
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Error calling ippsZero_64fc");
        return ISCL_IPP_FAILURE;
    }
    psrc = NULL;
#else
    nz = (size_t) n*sizeof(double complex);
    memset(zeros, 0.0, nz);
#endif
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Creates a boolean array of length n where each element is false.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] where each element is false.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
bool *array_zeros8l(const int n, int *ierr)
{
    bool *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc8l(n, ierr);
    *ierr = array_zeros8l_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {   
        isclPrintError("%s", "Error setting logical zeros\0");
        memory_free8l(&zeros);
    }   
    return zeros; 
}
//============================================================================//
/*!
 * @brief Sets to false all element of a boolean array of length n
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array to be filled with false values.  This has
 *                      dimension [n].
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros8l_work(const int n, bool *__restrict__ zeros)
{
    size_t nz;
    isclReturnArrayTooSmallError("n", n, 1); 
    isclReturnNullPointerError("zeros", zeros);
    nz = (size_t) n*sizeof(bool);
    memset(zeros, 0, nz);
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Creates a char array of length n filled with null terminated values.
 *
 * @param[in] n      Length of array.  This must be positive.
 *
 * @param[out] ierr  ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @result Array of dimension [n] filled with null terminated values.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
char *array_zeros8c(const int n, enum isclError_enum *ierr)
{
    char *zeros = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    zeros = memory_malloc8c(n, ierr);
    *ierr = array_zeros8c_work(n, zeros);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error setting char zeros\0");
        memory_free8c(&zeros);
    }
    return zeros; 
}
//============================================================================//
/*!
 * @brief Sets all elements of a length n char array to the null terminator.
 *
 * @param[in] n         Length of array to zero out.
 *
 * @param[out] zeros    Array to be filled with null terminated values.  This
 *                      has dimension [n].
 *
 * @result ISCL return code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_zeros
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
enum isclError_enum array_zeros8c_work(const int n, char *__restrict__ zeros)
{
    size_t nz; 
    isclReturnArrayTooSmallError("n", n, 1); 
    isclReturnNullPointerError("zeros", zeros);
    nz = (size_t) n*sizeof(char);
    memset(zeros, 0, nz);
    return ISCL_SUCCESS;
}
