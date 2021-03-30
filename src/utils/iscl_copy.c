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
 * @defgroup array_copy copy
 * @brief Copies an array.
 * @ingroup array
 */
/*!
 * @brief Copies a double precision array.
 *
 * @param[in] n      Length of source array.
 * @param[in] src    Source array to copy.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @result Copy of array src.  This has dimension [n].
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
double *array_copy64f(const int n, const double *src, 
                      enum isclError_enum *ierr)
{
    double *dest = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    dest = memory_malloc64f(n, ierr);
    *ierr = array_copy64f_work(n, src, dest);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error copying array");
        memory_free64f(&dest);
    }
    return dest;
}
//============================================================================//
/*!
 * @brief Copies a double precision array.
 *
 * @param[in] n      Length of source array
 * @param[in] src    Source array to copy to dest.  This has dimension [n].
 *
 * @param[out] dest  Copy of array src.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
#ifdef ISCL_USE_INTEL
enum isclError_enum
    array_copy64f_work(const int n, const double *__restrict__ src,
                       double *__restrict__ dest)
{
    IppStatus status;
    enum isclError_enum ierr;
    // Copy
    ierr = ISCL_SUCCESS;
    status = ippsCopy_64f(src, dest, n); 
    // Error handling
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "An error was encountered");
        if (status == ippStsNullPtrErr)
        {
            if (src == NULL){isclPrintError("%s", "Source array is NULL");}
            if (dest == NULL)
            {
                isclPrintError("%s", "Destination array is NULL");
            }
        }
        if (status == ippStsSizeErr)
        {
            isclPrintError("Error size %d is invalid", n); 
        }
        ierr = ISCL_IPP_FAILURE;
    }   
    return ierr;
}
#else
enum isclError_enum
    array_copy64f_work(const int n, const double *__restrict__ src,
                       double *__restrict__ dest)
{
    int i;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("src", src);
    isclReturnNullPointerError("dest", dest);
    //memcpy(src, dest, n*sizeof(double));
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        dest[i] = src[i];
    }
    return ISCL_SUCCESS;
}
#endif
//============================================================================//
/*!
 * @brief Copies an integer array.
 *
 * @param[in] n      Length of source array.
 * @param[in] src    Source array to copy.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @result Copy of array src.  This has dimension [n].
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
int *array_copy32i(const int n, const int *src,
                   enum isclError_enum *ierr)
{
    int *dest = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    dest = memory_malloc32i(n, ierr);
    *ierr = array_copy32i_work(n, src, dest);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error copying array");
        memory_free32i(&dest);
    }
    return dest;
}
//============================================================================//
/*!
 * @brief Copies an integer array.
 *
 * @param[in] n      Length of source array
 * @param[in] src    Source array to copy to dest.  This has dimension [n].
 *
 * @param[out] dest  Copy of array src.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
#ifdef ISCL_USE_INTEL
enum isclError_enum
    array_copy32i_work(const int n, const int *__restrict__ src,
                       int *__restrict__ dest)
{
    IppStatus status;
    enum isclError_enum ierr;
    // Copy
    ierr = ISCL_SUCCESS;
    status = ippsCopy_32s(src, dest, n); 
    // Error handling
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "An error was encountered");
        if (status == ippStsNullPtrErr)
        {
            if (src == NULL)
            {
                isclPrintError("Source array is NULL");
            }
            if (dest == NULL)
            {
                isclPrintError("Destination array is NULL");
            }
        }
        if (status == ippStsSizeErr)
        {
            isclPrintError("Error size %d is invalid", n); 
        }
        ierr = ISCL_IPP_FAILURE;
    }   
    return ierr;
}
#else
enum isclError_enum
    array_copy32i_work(const int n, const int *__restrict__ src,
                       int *__restrict__ dest)
{
    int i;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("src", src);
    isclReturnNullPointerError("dest", dest);
    //memcpy(src, dest, n*sizeof(int));
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        dest[i] = src[i];
    }   
    return ISCL_SUCCESS;
}
#endif
//============================================================================//
/*!
 * @brief Copies a double complex array.
 *
 * @param[in] n      Length of source array.
 * @param[in] src    Source array to copy.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @result Copy of array src.  This has dimension [n].
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
double complex *array_copy64z(const int n, const double complex *src,
                              enum isclError_enum *ierr)
{
    double complex *dest = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    dest = memory_malloc64z(n, ierr);
    *ierr = array_copy64z_work(n, src, dest);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error copying array");
        memory_free64z(&dest);
    }   
    return dest;
}
//============================================================================//
/*!
 * @brief Copies a double complex array.
 *
 * @param[in] n      Length of source array
 * @param[in] src    Source array to copy to dest.  This has dimension [n].
 *
 * @param[out] dest  Copy of array src.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
#ifdef ISCL_USE_INTEL
enum isclError_enum
    array_copy64z_work(const int n, const double complex *__restrict__ src,
                       double complex *__restrict__ dest)
{
    Ipp64fc *srcIpp, *destIpp;
    IppStatus status;
    enum isclError_enum ierr;
    // Copy
    ierr = ISCL_SUCCESS;
    srcIpp = (Ipp64fc *)src;
    destIpp = (Ipp64fc *)dest;
    status = ippsCopy_64fc(srcIpp, destIpp, n);
    // Error handling
    if (status != ippStsNoErr)
    {
        isclPrintError("An error was encountered");
        if (status == ippStsNullPtrErr)
        {
            if (src == NULL)
            {
                isclPrintError(" Source array is NULL");
            }
            if (dest == NULL)
            {
                isclPrintError("Destination array is NULL");
            }
        }
        if (status == ippStsSizeErr)
        {
            isclPrintError("Error size %d is invalid", n);
        }
        ierr = ISCL_IPP_FAILURE;
    }
    return ierr;
}
#else
enum isclError_enum
    array_copy64z_work(const int n, const double complex *__restrict__ src,
                       double complex *__restrict__ dest)
{
    int i;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("src", src);
    isclReturnNullPointerError("dest", dest);
    //memcpy(src, dest, n*sizeof(double complex));
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        dest[i] = src[i];
    }   
    return ISCL_SUCCESS;
}
#endif
//============================================================================//
/*!
 * @brief Copies a boolean array.
 *
 * @param[in] n      Length of source array.
 * @param[in] src    Source array to copy.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @result Copy of array src.  This has dimension [n].
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
bool *array_copy8l(const int n, const bool *src, enum isclError_enum *ierr)
{
    bool *dest = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    dest = memory_malloc8l(n, ierr);
    *ierr = array_copy8l_work(n, src, dest);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error copying array");
        memory_free8l(&dest);
    }
    return dest;
}
//============================================================================//
/*!
 * @brief Copies a boolean array.
 *
 * @param[in] n      Length of source array
 * @param[in] src    Source array to copy to dest.  This has dimension [n].
 *
 * @param[out] dest  Copy of array src.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success. 
 *
 * @ingroup array_copy
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    array_copy8l_work(const int n, const bool *__restrict__ src,
                      bool *__restrict__ dest)
{
    int i;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("src", src);
    isclReturnNullPointerError("dest", dest);
    #pragma omp simd
    for (i=0; i<n; i++)
    {   
        dest[i] = src[i];
    }
    return ISCL_SUCCESS;
}
