#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif
#include "iscl/memory/memory.h"

/*!
 * @defgroup array_reverse reverse
 * @brief Reverses the order of elements in an array.
 * @ingroup array
 */
/*!
 * @brief Returns a reversed copy of double array a.
 *
 * @param[in] n      Number of elements in a.
 * @param[in] a      Array to reverse.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result A an array of dimension [n] which is a reversed copy of a.
 *
 * @ingroup array_reverse
 *
 */
double *array_reverse64f(const int n, double *a, 
                         enum isclError_enum *ierr)
{
    double *ar = NULL;
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    ar = memory_malloc64f(n, ierr);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to allocate memory for ar!");
        return ar;
    }
    // Reverse the array
    *ierr = array_reverse64f_work(n, a, ar);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error reversing array");
        memory_free64f(&ar);
    }
    return ar;
}
//============================================================================//
/*!
 * @brief Reverses a double array.
 *
 * @param[in] n     Size of arrays
 * @param[in] a     Array to be reversed.  This has dimension [n].
 *
 * @param[out] b    Reversed copy of array a.  This has dimension [n] and
 *                  can be called with the same argument as a.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_reverse
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum array_reverse64f_work(const int n, double *a, double *b)
{
#ifdef ISCL_USE_INTEL
    IppStatus status;
#else
    double bj;
    int i, j;
#endif
    isclReturnArrayTooSmallError("n", n, 1); 
    isclReturnNullPointerError("a", a);
    isclReturnNullPointerError("b", b);
    // In place
    if (a == b)
    {
#ifdef ISCL_USE_INTEL
        status = ippsFlip_64f_I(a, n);
        if (status != ippStsNoErr)
        {
            if (status == ippStsNullPtrErr)
            {
                if (a == NULL){isclPrintError("%s", "Array is NULL\n");}
            }
            if (status == ippStsSizeErr)
            {
                isclPrintError("%s", "Invalid size");
            }
            return ISCL_IPP_FAILURE; 
        }
#else
        j = n - 1;
        for (i=0; i<n/2; i++)
        {
            bj = b[j];
            b[j] = b[i];
            b[i] = bj;
            j = j - 1;
        }
#endif
    }
    // Different memory addresses
    else
    {
#ifdef ISCL_USE_INTEL
        status = ippsFlip_64f(a, b, n); 
        if (status != ippStsNoErr)
        {
            if (status == ippStsNullPtrErr)
            {
                if (a == NULL){isclPrintError("%s", "Array is NULL\n");}
            }
            if (status == ippStsSizeErr)
            {
                isclPrintError("%s", "Invalid size");
            }
            return ISCL_IPP_FAILURE;
        }
#else
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            j = n - 1 - i;
            b[j] = a[i];
        }
#endif
    }
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Returns a reversed copy of an integer array a.
 *
 * @param[in] n      Number of elements in a.
 * @param[in] a      Array to reverse.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result A an array of dimension [n] which is a reversed copy of a.
 *
 * @ingroup array_reverse
 *
 */
int *array_reverse32i(const int n, int *a, enum isclError_enum *ierr)
{
    int *ar = NULL;
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    ar = memory_malloc32i(n, ierr);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to allocate memory for ar!");
        return ar; 
    }   
    // Reverse the array
    *ierr = array_reverse32i_work(n, a, ar);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error reversing array");
        memory_free32i(&ar);
    }   
    return ar; 
}
//============================================================================//
/*!
 * @brief Reverses an integer array.
 *
 * @param[in] n     Size of arrays
 * @param[in] a     Array to be reversed.  This has dimension [n].
 *
 * @param[out] b    Reversed copy of array a.  This has dimension [n] and
 *                  can be called with the same argument as a.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_reverse
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum array_reverse32i_work(const int n, int *a, int *b)
{
    int bj;
    int i, j;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    isclReturnNullPointerError("b", b);
    // In place
    if (a == b)
    {
        j = n - 1;
        for (i=0; i<n/2; i++)
        {
            bj = b[j];
            b[j] = b[i];
            b[i] = bj; 
            j = j - 1;
        }
    }
    // Different memory addresses
    else
    {
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            j = n - 1 - i;
            b[j] = a[i];
        }
    }
    return ISCL_SUCCESS;
}
//============================================================================//
/*!
 * @brief Returns a reversed copy of double complex array a.
 *
 * @param[in] n      Number of elements in a.
 * @param[in] a      Array to reverse.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result A an array of dimension [n] which is a reversed copy of a.
 *
 * @ingroup array_reverse
 *
 */
double complex *array_reverse64z(const int n, double complex *a,
                                 enum isclError_enum *ierr)
{
    double complex *ar = NULL;
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space
    ar = memory_malloc64z(n, ierr);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to allocate memory for ar!");
        return ar;
    }
    // Reverse the array
    *ierr = array_reverse64z_work(n, a, ar);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error reversing array");
        memory_free64z(&ar);
    }   
    return ar; 
}
//============================================================================//
/*!
 * @brief Reverses a double complex array.
 *
 * @param[in] n     Size of arrays
 * @param[in] a     Array to be reversed.  This has dimension [n].
 *
 * @param[out] b    Reversed copy of array a.  This has dimension [n] and
 *                  can be called with the same argument as a.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup array_reverse
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    array_reverse64z_work(const int n, double complex *a, double complex *b)
{
#ifdef ISCL_USE_INTEL
    Ipp64fc *pSrc, *pDst, *pSrcDst;
    IppStatus status;
#else
    double complex bj;
    int i, j;
#endif
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    isclReturnNullPointerError("b", b);
    // In place
    if (a == b)
    {
#ifdef ISCL_USE_INTEL
        pSrcDst = (Ipp64fc *) a; 
        status = ippsFlip_64fc_I(pSrcDst, n); 
        if (status != ippStsNoErr)
        {
            isclPrintError("%s", "Error calling ippsFlip_64f_I");
            return ISCL_IPP_FAILURE; 
        }
        pSrcDst = NULL;
#else
        j = n - 1;
        for (i=0; i<n/2; i++)
        {
            bj = b[j];
            b[j] = b[i];
            b[i] = bj; 
            j = j - 1;
        }
#endif
    }
    // Different memory addresses
    else
    {
#ifdef ISCL_USE_INTEL
        pSrc = (Ipp64fc *) a;
        pDst = (Ipp64fc *) b;
        status = ippsFlip_64fc(pSrc, pDst, n);
        pSrc = NULL;
        pDst = NULL;
        if (status != ippStsNoErr)
        {
            isclPrintError("%s", "Error calling ippsFlip_64fc");
            return ISCL_IPP_FAILURE;
        }
#else
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            j = n - 1 - i;
            b[j] = a[i];
        }
#endif
    }
    return ISCL_SUCCESS;
}
