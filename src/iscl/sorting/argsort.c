#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/sorting/sorting.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"

struct double2d_struct
{
    double val;
    int indx;
    char pad4[4];
};

struct int2d_struct
{
    int val;
    int indx;
};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
static int cmp_double_array(const void *x, const void *y)
{
    const struct double2d_struct xx = *(const struct double2d_struct *) x;
    const struct double2d_struct yy = *(const struct double2d_struct *) y; 
    if (xx.val < yy.val) return -1; 
    if (xx.val > yy.val) return  1;
    return 0;
}

static int cmp_int_array(const void *x, const void *y) 
{
    const struct int2d_struct xx = *(const struct int2d_struct *) x;
    const struct int2d_struct yy = *(const struct int2d_struct *) y;  
    if (xx.val < yy.val) return -1; 
    if (xx.val > yy.val) return  1;  
    return 0;
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/*!
 * @defgroup sorting_argsort argsort
 * @brief Computes the permutation vector that will sort an array.
 * @ingroup sorting
 */
/*! 
 * @brief Perform an argument sort of a double precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] a       Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort a in
 *                    ascending order.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort a in
 *                    descending order.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Permutation vector so that a[perm[:]] is in the desired order. 
 *         This has dimension [n].
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
int *sorting_argsort64f(const int n, const double *__restrict__ a,
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
{
    int *perm;
    //------------------------------------------------------------------------//
    //
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space and perform argsort
    perm = memory_malloc32i(n, ierr);
    *ierr = sorting_argsort64f_work(n, a, order, perm);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error performing argsort");
        memory_free32i(&perm);
    }
    return perm; 
}
//============================================================================//
/*! 
 * @brief Perform an argument sort of a double precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] a       Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort a in
 *                    ascending order.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort a in
 *                    descending order.
 *
 * @param[out] perm   Permutation vector so that a[perm[:]] would be in 
 *                    the desired order.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_argsort64f_work(const int n,
                            const double *__restrict__ a,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm)
{
    struct double2d_struct *vals;
    int i;
    //------------------------------------------------------------------------//
    //
    // Error check
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("a", a);
    isclReturnNullPointerError("perm", perm);
    // Special case
    if (n == 1)
    {
        perm[0] = 0;
        return ISCL_SUCCESS;
    }
    // Set workspace
    vals = (struct double2d_struct *)
           calloc((size_t) n, sizeof(struct double2d_struct));
    for (i=0; i<n; i++)
    {
        vals[i].val = a[i];
        vals[i].indx = i;
    }
    qsort((void *) vals, (size_t) n,
          sizeof(struct double2d_struct), cmp_double_array);
    // Copy answer back
    if (order == SORT_ASCENDING)
    {   
        for (i=0; i<n; i++)
        {
            perm[i] = vals[i].indx;
        }
    }   
    else
    {   
        for (i=0; i<n; i++)
        {
            perm[i] = vals[n-1-i].indx;
        }
    }
    free(vals);
    return ISCL_SUCCESS;
}
//============================================================================//
/*! 
 * @brief Perform an argument sort of an integer precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] ia      Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort ia in
 *                    ascending order.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort ia in
 *                    descending order.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Permutation vector so that ia[perm[:]] is in the desired order.
 *         This has dimension [n].
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
int *sorting_argsort32i(const int n, const int *__restrict__ ia,
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
{
    int *perm;
    //------------------------------------------------------------------------//
    //  
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    // Set space and perform argsort
    perm = memory_malloc32i(n, ierr);
    *ierr = sorting_argsort32i_work(n, ia, order, perm);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error performing argsort");
        memory_free32i(&perm);
    }   
    return perm;
}
//============================================================================//
/*! 
 * @brief Perform an argument sort of an integer precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] ia      Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort ia in
 *                    ascending order.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort ia in
 *                    descending order.
 *
 * @param[out] perm   Permutation vector so that ia[perm[:]] would be in 
 *                    the desired order.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_argsort32i_work(const int n,
                            const int *__restrict__ ia,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm)
{
    struct int2d_struct *vals;
    int i;
    //------------------------------------------------------------------------//
    //  
    // Error check
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("ia", ia);
    isclReturnNullPointerError("perm", perm);
    // Special case
    if (n == 1)
    {
        perm[0] = 0;
        return ISCL_SUCCESS;
    }
    // Set workspace
    vals = (struct int2d_struct *)
           calloc((size_t) n, sizeof(struct int2d_struct));
    for (i=0; i<n; i++)
    {
        vals[i].val = ia[i];
        vals[i].indx = i;
    }
    qsort((void *) vals, (size_t) n,
          sizeof(struct int2d_struct), cmp_int_array);
    // Copy answer back
    if (order == SORT_ASCENDING)
    {
        for (i=0; i<n; i++)
        {
            perm[i] = vals[i].indx;
        }
    }
    else
    {
        for (i=0; i<n; i++)
        {
            perm[i] = vals[n-1-i].indx;
        }
    }
    free(vals);
    return ISCL_SUCCESS;
}
//============================================================================//
/*! 
 * @brief Perform an argument sort of a double complex precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] ca      Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort a in
 *                    ascending order based on the absolute value of ca.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort a in
 *                    descending order based on the absolute value of ca.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Permutation vector so that ca[perm[:]] is in the desired order.
 *         This has dimension [n].
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
int *sorting_argsort64z(const int n, const double complex *__restrict__ ca,
                        const enum sortOrder_enum order, 
                        enum isclError_enum *ierr)
{
    int *perm;
    //------------------------------------------------------------------------//
    //  
    // Error check
    *ierr = ISCL_SUCCESS;
    isclSetArrayTooSmallErrorAndReturnNULL("n", n, 1, ierr);
    perm = memory_malloc32i(n, ierr);
    *ierr = sorting_argsort64z_work(n, ca, order, perm);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error performing argsort");
        memory_free32i(&perm);
    }
    return perm;
}
//============================================================================//
/*! 
 * @brief Perform an argument sort of a double precision array.
 * 
 * @param[in] n       Length of a.
 * @param[in] ca      Array to be argsorted.  This has dimension [n].
 * @param[in] order   If order is SORT_ASCENDING then perm will sort ca in
 *                    ascending order.
 * @param[in] order   If order is SORT_DESCENDING then perm will sort ca in
 *                    descending order.
 *
 * @param[out] perm   Permutation vector so that ca[perm[:]] would be in 
 *                    the desired order.  This has dimension [n].
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup sorting_argsort
 *
 * @author Ben Baker, ISTI
 *
 */
enum isclError_enum
    sorting_argsort64z_work(const int n, const double complex *__restrict__ ca,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm)
{
    double *a;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //
    // Error check
    ierr = ISCL_SUCCESS;
    isclReturnArrayTooSmallError("n", n, 1);
    isclReturnNullPointerError("ca", ca);
    isclReturnNullPointerError("perm", perm);
    // Special case
    if (n == 1)
    {
        perm[0] = 0;
        return ISCL_SUCCESS;
    }
    // Set workspace 
    //p = (size_t *)calloc(n, sizeof(size_t));
    a = array_abs64z(n, ca, &ierr);
/*
    ierr = memory_malloc64f(n, &a); //(double *)calloc(n, sizeof(double));
    #pragma omp simd
    for (i=0; i<n; i++)
    {
        a[i] = cabs(ca[i]);
    }
*/
    // Perform argsort
    ierr = sorting_argsort64f_work(n, a, order, perm);
    if (ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error calling sorting_argsort64f_work\n");
    }
    memory_free64f(&a);
    return ierr;
}
