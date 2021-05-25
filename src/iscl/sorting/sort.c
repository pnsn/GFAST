#include <stdio.h>
#include <stdlib.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/sorting/sorting.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

#ifndef ISCL_USE_INTEL
static int cmp_double(const void *x, const void *y)
{
    const double xx = *(const double *) x;
    const double yy = *(const double *) y;
    if (xx < yy) return -1;
    if (xx > yy) return  1;
    return 0;
}

static int cmp_int(const void *x, const void *y) 
{
    const int xx = *(const int *) x;
    const int yy = *(const int *) y;
    if (xx < yy) return -1; 
    if (xx > yy) return  1;  
    return 0;
}
#endif

/*!
 * @defgroup sorting_sort sort
 * @brief Sorts an array.
 * @ingroup sorting
 */
/*!
 * @brief Sorts a double array.
 *
 * @param[in] n       Length of array a.
 * @param[in] a       Array to be sorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Sorted version of input array a.  This is has dimension [n].
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
double *sorting_sort64f(const int n, const double *__restrict__ a,
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
{
    double *sort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    sort = array_copy64f(n, a, ierr);
    *ierr = sorting_sort64f_work(n, sort, order);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to sort array");
        memory_free64f(&sort);
    }
    return sort;
}
//============================================================================//
/*!
 * @brief Sorts a double array in place.
 *
 * @param[in] n       Length of array a.
 * @param[in,out] a   On input this is the array to sort.
 * @param[in,out] a   On output this is the sorted array.  
 *                    This has dimension [n]. 
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
#ifdef ISCL_USE_INTEL
enum isclError_enum sorting_sort64f_work(const int n,
                                         double *__restrict__ a,
                                         const enum sortOrder_enum order)
{
    IppStatus status;
    // Error checking
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    if (order == SORT_ASCENDING)
    {
        status = ippsSortAscend_64f_I(a, n);
    }
    else
    {
        status = ippsSortDescend_64f_I(a, n);
    }
    if (ippStsNoErr != status)
    {
        isclPrintError("%s", "Sort failed!");
        return ISCL_IPP_FAILURE;
    }
    return ISCL_SUCCESS;
}
#else
enum isclError_enum sorting_sort64f_work(const int n, double *__restrict__ a,
                                         const enum sortOrder_enum order)
{
    enum isclError_enum ierr;
    size_t ns = (size_t) n;
    //------------------------------------------------------------------------//
    //  
    // Error checking
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    // Early return
    if (n == 1){return ISCL_SUCCESS;}
    qsort((void *)a, (size_t) ns, sizeof(double), cmp_double);
    if (order == SORT_DESCENDING)
    {
        ierr = array_reverse64f_work(n, a, a);
        if (ierr != ISCL_SUCCESS)
        {
            isclPrintError("%s", "Error reversing array!");
            return ierr;
        }
    }
    return ISCL_SUCCESS;
}
#endif
//============================================================================//
/*!
 * @brief Sorts an integer array.
 *
 * @param[in] n       Length of array a.
 * @param[in] a       Array to be sorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Sorted version of input array a.  This is has dimension [n].
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
int *sorting_sort32i(const int n, const int *__restrict__ a,
                     const enum sortOrder_enum order,
                     enum isclError_enum *ierr)
{
    int *sort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    sort = array_copy32i(n, a, ierr);
    *ierr = sorting_sort32i_work(n, sort, order);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to sort array");
        memory_free32i(&sort);
    }
    return sort;
}
//============================================================================//
/*!
 * @brief Sorts an integer array in place.
 *
 * @param[in] n       Length of array a.
 * @param[in,out] a   On input this is the array to sort.
 * @param[in,out] a   On output this is the sorted array.  
 *                    This has dimension [n]. 
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
#ifdef ISCL_USE_INTEL
enum isclError_enum sorting_sort32i_work(const int n, int *__restrict__ a,
                                         const enum sortOrder_enum order)
{
    IppStatus status;
    // Error checking
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    if (order == SORT_ASCENDING)
    {
        status = ippsSortAscend_32s_I(a, n);
    }
    else
    {
        status = ippsSortDescend_32s_I(a, n);
    }
    if (ippStsNoErr != status)
    {
        isclPrintError("%s", "Sort failed!");
        return ISCL_IPP_FAILURE;
    }
    return ISCL_SUCCESS;
}
#else
enum isclError_enum sorting_sort32i_work(const int n, int *__restrict__ a,
                                         const enum sortOrder_enum order)
{
    int ierr;
    size_t ns = (size_t) n;
    //------------------------------------------------------------------------//
    //  
    // Error checking
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    // Early return
    if (n == 1){return ISCL_SUCCESS;}
    qsort((void *)a, (size_t) ns, sizeof(int), cmp_int);
    if (order == SORT_DESCENDING)
    {
        ierr = array_reverse32i_work(n, a, a);
        if (ierr != ISCL_SUCCESS)
        {
            isclPrintError("%s", "Error reversing array!");
            return ierr;
        }
    }
    return ISCL_SUCCESS;
}
#endif
//============================================================================//
/*!
 * @brief Sorts a double complex array based on absolute value.
 *
 * @param[in] n       Length of array a.
 * @param[in] a       Array to be sorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Sorted version of input array a.  This is has dimension [n].
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
double complex *sorting_sort64z(const int n,
                                const double complex *__restrict__ a,
                                const enum sortOrder_enum order,
                                enum isclError_enum *ierr)
{
    double complex *sort = NULL;
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    sort = array_copy64z(n, a, ierr);
    *ierr = sorting_sort64z_work(n, sort, order);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Failed to sort array");
        memory_free64z(&sort);
    }
    return sort;
}
//============================================================================//
/*!
 * @brief Sorts a double complex array in place based on the absolute value 
 *        the elements.
 *
 * @param[in] n       Length of array a.
 * @param[in,out] a   On input this is the array to sort.
 * @param[in,out] a   On output this is the sorted array.  
 *                    This has dimension [n]. 
 * @param[in] order   If order is SORT_ASCENDING then this function sorts the
 *                    array a ascending order (default).
 * @param[in] order   If order is SORT_DESCENDING then this function sorts the
 *                    array in descending order.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_sort
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_sort64z_work(const int n, double complex *__restrict__ a,
                         const enum sortOrder_enum order)
{
    int *iperm;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //  
    // Error checking
    ierr = ISCL_SUCCESS;
    iperm = NULL;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    // Early return
    if (n == 1){return ISCL_SUCCESS;}
    iperm = (int *) calloc((size_t) n, sizeof(int));
    if (iperm == NULL)
    {
        isclPrintError("%s", "Error setting space for permutation array!");
        goto ERROR;
    }
    // Argsort will compute magnitudes and generate permutation array
    ierr = sorting_argsort64z_work(n, a, SORT_ASCENDING, iperm);
    if (ierr != 0)
    {
        isclPrintError("%s", "Error computing permutation array");
        goto ERROR;
    }
    // Apply permutation vector
    ierr = sorting_applyPermutation64z_work(n, iperm, a, a);
    if (ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error applying permutation");
        goto ERROR; 
    }
    if (order == SORT_DESCENDING)
    {
        ierr = array_reverse64z_work(n, a, a);
        if (ierr != ISCL_SUCCESS)
        {
            isclPrintError("%s", "Error reversing array!");
            goto ERROR;
        }
    }
ERROR:;
    if (iperm != NULL){free(iperm);}
    iperm = NULL;
    return ierr;
}
