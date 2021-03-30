#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <complex.h>
#include <omp.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif

/*!
 * @defgroup array_absArgmin absArgmin
 * @brief Computes the index corresponding to the minimum value of an
 *        of an array.
 * @ingroup array
 */
/*! 
 * @brief Returns index corresponding to smallest value in a double array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Index of x which corresonds to smallest value.
 *
 * @ingroup array_absArgmin
 *
 * @author Ben Baker, ISTI
 *
 */
int array_absArgmin64f(const int n, const double *__restrict__ x,
                       enum isclError_enum *ierr)
{
    double xi, val;
    int i, index;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {   
            isclPrintError("Error no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
        }
        if (x == NULL)
        {   
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0;
    }
    if (n == 1){return 0;}
    val = fabs(x[0]);
    index = 0;
    for (i=1; i<n; i++)
    {
        xi = fabs(x[i]);
        if (xi < val){index = i;}
        if (xi < val){val = x[i];}
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
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Index of x which corresonds to smallest value
 *
 * @ingroup array_absArgmin
 *
 * @author Ben Baker, ISTI
 *
 */
int array_absArgmin32i(const int n, const int *__restrict__ x,
                       enum isclError_enum *ierr)
{
    int i, index, xi, val;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {   
            isclPrintError("Error no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
        }
        if (x == NULL)
        {   
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0;
    }
    if (n == 1){return 0;}
    val = abs(x[0]);
    index = 0;
    for (i=1; i<n; i++)
    {
        xi = abs(x[i]);
        if (xi < val){index = i;} 
        if (xi < val){val = x[i];}
    }
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to smallest absolute value in a 
 *        complex array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum absolute value.  This has
 *                   dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Index of x which corresonds to smallest absolute value.
 *
 * @ingroup array_absArgmin
 *
 * @author Ben Baker, ISTI
 *
 */
int array_absArgmin64z(const int n, const double complex *__restrict__ x,
                       enum isclError_enum *ierr)
{
    double cx, val;
    int i, index;
    *ierr = ISCL_SUCCESS;
    if (n < 1)
    {
        if (n < 1)
        {   
            isclPrintError("Error no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
        }
        if (x == NULL)
        {   
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0.0;
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
        if (cx < val){index = i;}
        if (cx < val){val = cx;}
    }
    return index;
}
