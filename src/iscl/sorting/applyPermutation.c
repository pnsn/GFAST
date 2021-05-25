#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <omp.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/sorting/sorting.h"
#ifdef ISCL_USE_INTEL
#include <mkl_cblas.h>
#else
#include <cblas.h>
#endif
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

/*!
 * @defgroup sorting_applyPermutation permute
 * @brief Permutes a vector based on the result of an argument sort.
 * @ingroup sorting
 */
/*! 
 * @brief Applies permutation vector to array x so that x = x[iperm]. 
 *        Useful when sorting many vectors with argsort.
 * 
 * @param[in] n        Number of elements.
 * @param[in] iperm    Permutation array that sorts x into desired order.
 *                     This is an array of dimension [n].
 *
 * @param[in] x        Array to be permuted based on permutation iperm.
 *                     This has dimension [n].
 *
 * @param[out] ierr    ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Permuted version of x s.t. xsort = x[iperm[:]].  This has dimension
 *         [n].
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
double *sorting_applyPermutation64f(const int n, const int *__restrict__ iperm,
                                    double *__restrict__ x,
                                    enum isclError_enum *ierr)
{
    double *xsort;
    xsort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    xsort = memory_calloc64f(n);
    *ierr = sorting_applyPermutation64f_work(n, iperm, x, xsort);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error applying permutation");
        memory_free64f(&xsort);
    }
    return xsort; 
}
//============================================================================//
/*! 
 * @brief Applies permutation vector to array x so that x = x[iperm]. 
 *        Useful when sorting many vectors with argsort.  In-place 
 *        calls are okay.
 * 
 * @param[in] n        Number of elements.
 * @param[in] iperm    Permutation array that sorts x into desired order.
 *                     This has dimension [n].
 *
 * @param[in] x        Array to be sorted based on permutation iperm.
 *                     This has dimension [n].
 *
 * @param[out] xsort   Sorted version of x s.t. xsort = x[iperm[:]].  This
 *                     has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_applyPermutation64f_work(const int n, const int *__restrict__ iperm,
                                     double *__restrict__ x,
                                     double *__restrict__ xsort)
{
    double *work;
    int i;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //
    // Error check
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("iperm", iperm);
    isclReturnNullPointerError("x", x);
    isclReturnNullPointerError("xsort", xsort);
    // Easiest case
    if (n == 1)
    {
        xsort[0] = x[0];
        return ISCL_SUCCESS;
    }
    // Copy and permute
    if (x == xsort)
    {
        //memory_malloc64f(n, &work);
        //cblas_dcopy(n, x, incx, work, incy);
        work = array_copy64f(n, x, &ierr);
        for (i=0; i<n; i++)
        {
            xsort[i] = work[iperm[i]];
        }
        memory_free64f(&work);
    }
    // Just permute
    else
    {
        for (i=0; i<n; i++)
        {
            xsort[i] = x[iperm[i]];
        }
    }
    return ISCL_SUCCESS;
}
//============================================================================//
/*! 
 * @brief Applies permutation vector to array integer array ix so that
 *        ixsort = ix[iperm].  Useful when sorting many vectors with argsort.
 *        In-place calls are okay.
 * 
 * @param[in] n        Number of elements.
 * @param[in] iperm    Permutation array that sorts x into desired order.
 *                     This has dimension [n].
 *
 * @param[in] ix       Array to be reordered based on permutation iperm.
 *                     This has dimension [n].
 *
 * @param[out] ierr    ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Permuted version of x s.t. ixsort = ix[iperm[:]].  This has
 *         dimension [n].
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
int *sorting_applyPermutation32i(const int n, const int *__restrict__ iperm,
                                 int *__restrict__ ix,
                                 enum isclError_enum *ierr)
{
    int *ixsort;
    ixsort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    ixsort = memory_calloc32i(n);
    *ierr = sorting_applyPermutation32i_work(n, iperm, ix, ixsort);
    if (*ierr != ISCL_SUCCESS)
    {   
        isclPrintError("%s", "Error applying permutation");
        memory_free32i(&ixsort);
    }
    return ixsort;
}
//============================================================================//
/*! 
 * @brief Applies permutation vector to array integer array ix so that
 *        ixsort = ix[iperm].  Useful when sorting many vectors with argsort.
 *        In-place calls are okay.
 * 
 * @param[in] n        Number of elements.
 * @param[in] iperm    Permutation array that sorts x into desired order.
 *                     This has dimension [n].
 * @param[in] ix       Array to be permuted based on permutation iperm.  This
 *                     has dimension [n].
 *
 * @param[out] ixsort  Permuted version of x s.t. ixsort = ix[iperm[:]]. 
 *                     This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_applyPermutation32i_work(const int n, const int *__restrict__ iperm,
                                     int *__restrict__ ix,
                                     int *__restrict__ ixsort)
{
    int *iwork, i;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //
    // Error check
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("iperm", iperm);
    isclReturnNullPointerError("ix", ix);
    isclReturnNullPointerError("ixsort", ixsort);
    // Easiest case
    if (n == 1)
    {
        ixsort[0] = ix[0];
        return ISCL_SUCCESS;
    }
    // Copy and permute
    if (ix == ixsort)
    {
        iwork = memory_malloc32i(n, &ierr);
        #pragma omp simd
        for (i=0; i<n; i++)
        {
            iwork[i] = ix[i];
        }
        for (i=0; i<n; i++)
        {
            ixsort[i] = iwork[iperm[i]];
        }
        memory_free32i(&iwork);
    }
    // Just permute
    else
    {
        for (i=0; i<n; i++)
        {
            ixsort[i] = ix[iperm[i]];
        }
    }
    return ISCL_SUCCESS;
}
//============================================================================//
/*! 
 * @brief Applies permutation vector to array cx so that cxsort = cx[iperm].
 *        Useful when sorting many vectors with argsort.  For complex numbers
 *        which can be sorted on their magnitudes.  In place calls are okay.
 * 
 * @param[in] n      Number of elements in arrays.
 * @param[in] iperm  Permutation array that sorts cx into desired order.
 *                   This has dimension [n].
 * @param[in] cx     Array to permute.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result cx reordered s.t. cxsort = cx[iperm[:]].  This has dimension [n].
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
double complex *sorting_applyPermutation64z(const int n,
                                            const int *__restrict__ iperm,
                                            double complex *__restrict__ cx,
                                            enum isclError_enum *ierr)
{
    double complex *cxsort;
    cxsort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    cxsort = memory_calloc64z(n);
    *ierr = sorting_applyPermutation64z_work(n, iperm, cx, cxsort);
    if (*ierr != ISCL_SUCCESS)
    {   
        isclPrintError("%s", "Error applying permutation");
        memory_free64z(&cxsort);
    }
    return cxsort;
}
//============================================================================//
/*! 
 * @brief Applies permutation vector to array cx so that cxsort = cx[iperm].
 *        Useful when sorting many vectors with argsort.  For complex numbers
 *        which can be sorted on their magnitudes.  In place calls are okay.
 * 
 * @param[in] n        Number of elements in arrays.
 * @param[in] iperm    Permutation array that sorts cx into desired order. 
 *                     This has dimension [n].
 * @param[in] cx       Array to permute.  This has dimension [n].
 *
 * @param[out] cxsort  cx reordered s.t. cxsort = cx[iperm[:]].  This has
 *                     dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_applyPermutation
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_applyPermutation64z_work(const int n,
                                     const int *__restrict__ iperm,
                                     double complex *__restrict__ cx,
                                     double complex *__restrict__ cxsort)
{
    double complex *cwork;
    int i;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //  
    // Error check
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("iperm", iperm);
    isclReturnNullPointerError("cx", cx);
    isclReturnNullPointerError("cxsort", cxsort);
    // Easiest case
    if (n == 1)
    {
        cxsort[0] = cx[0];
        return ISCL_SUCCESS;
    }   
    // Copy and permute
    if (cx == cxsort)
    {
        //memory_malloc64z(n, &cwork);
        //cblas_zcopy(n, cx, incx, cwork, incy);
        cwork = array_copy64z(n, cx, &ierr);
        for (i=0; i<n; i++)
        {
            cxsort[i] = cwork[iperm[i]];
        }
        memory_free64z(&cwork);
    }
    // Just permute
    else
    {
        for (i=0; i<n; i++)
        {
            cxsort[i] = cx[iperm[i]];
        }
    }
    return ISCL_SUCCESS;
}

