#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <complex.h>
#include <omp.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"

/*!
 * @defgroup array_argmin argmin
 * @brief Computes the min index of an array.
 * @ingroup array
 */
/*! 
 * @brief Returns index corresponding to smallest value in a double array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmin
 *
 * @result Index of x which corresonds to the smallest value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmin64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr)
{
    double val;
    int i, index;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Warning no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0; 
        }
        if (x == NULL)
        {
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0;
    }
    if (n == 1){return 0;}
    val = x[0];
    index = 0;
    for (i=1; i<n; i++)
    {
        if (x[i] < val)
        {
            index = i;
            val = x[i];
        }
    }
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to smallest value in an integer array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmin
 *
 * @result Index of x which corresonds to the smallest value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmin32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr)
{
    int i, index, val;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Warning no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0; 
        }
        if (x == NULL)
        {
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0;
    }
    if (n == 1){return 0;}
    val = x[0];
    index = 0;
    for (i=1; i<n; i++)
    {
        if (x[i] < val)
        {
            index = i;
            val = x[i];
        }
    }
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to smallest absolute value in a 
 *        double complex array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find the smallest absolute value.
 *                   This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmin
 *
 * @result Index of x which corresonds to the smallest absolute value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmin64z(const int n, const double complex *__restrict__ x,
                    enum isclError_enum *ierr)
{
    double cx, val;
    int i, index;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Warning no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0; 
        }
        if (x == NULL)
        {
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0;
    }
    if (n == 1){return 0;}
    val = cabs(x[0]);
    index = 0;
    for (i=1; i<n; i++)
    {
        cx = cabs(x[i]);
        if (cx < val)
        {
            index = i;
            val = cx;
        }
    }
    return index;
}
