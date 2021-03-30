#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <complex.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/array/array.h"
#ifdef ISCL_USE_INTEL
#include <ipps.h>
#endif

#ifndef DCMPLX
#define DCMPLX(r,i) ((double) (r) + (double) (i)*I)
#endif

/*!
 * @defgroup array_min min
 * @brief Computes the minimum value of an array.
 * @ingroup array
 */
/*! 
 * @brief Returns the minimum of a double array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 * 
 * @result Minimum value in x
 *
 * @ingroup array_min
 *
 * @author Ben Baker, ISTI
 *
 */
double array_min64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr)
{
    double xmin;
#ifdef ISCL_USE_INTEL
    IppStatus status;
#else
    int i;
#endif
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
        return 0.0;
    }
    if (n == 1){return x[0];}
#ifdef ISCL_USE_INTEL
    status = ippsMin_64f(x, n, &xmin);
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Error computing min!");
        *ierr = ISCL_IPP_FAILURE;
        xmin = 0.0;
    }   
#else
    xmin = x[0];
    #pragma omp simd reduction(min:xmin)
    for (i=1; i<n; i++)
    {
        xmin = fmin(xmin, x[i]);
    }
#endif
    return xmin;
}
//============================================================================//
/*! 
 * @brief Returns the minimum of a float array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 * 
 * @result Minimum value in x
 *
 * @ingroup array_min
 *
 * @author Ben Baker, ISTI
 *
 */
float array_min32f(const int n, const float *__restrict__ x,
                   enum isclError_enum *ierr)
{
    float xmin;
#ifdef ISCL_USE_INTEL
    IppStatus status;
#else
    int i;
#endif
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Error no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return 0.0f;
        }
        if (x == NULL)
        {
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return 0.0f;
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return 0.0;
    }
    if (n == 1){return x[0];}
#ifdef ISCL_USE_INTEL
    status = ippsMin_32f(x, n, &xmin);
    if (status != ippStsNoErr)
    {
        isclPrintError("%s", "Error computing min!");
        *ierr = ISCL_IPP_FAILURE;
        xmin = 0.0f;
    }
#else
    xmin = x[0];
    #pragma omp simd reduction(min:xmin)
    for (i=1; i<n; i++)
    {
        xmin = fminf(xmin, x[i]);
    }
#endif
    return xmin;
}
//============================================================================//
/*! 
 * @brief Returns smallest value in an integer array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Smallest value in array x.
 *
 * @ingroup array_min
 *
 * @author Ben Baker, ISTI
 *
 */
int array_min32i(const int n, const int *__restrict__ x,
                 enum isclError_enum *ierr)
{
    int imin;
#ifdef ISCL_USE_INTEL
    IppStatus status;
#else
    int i;
#endif
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Error no values in array x!");
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
    if (n == 1){return x[0];}
#ifdef ISCL_USE_INTEL
    status = ippsMin_32s(x, n, &imin);
    if (status != ippStsNoErr)
    {   
        isclPrintError("%s", "Error computing min!");
        *ierr = ISCL_IPP_FAILURE;
        imin = 0;
    }   
#else
    imin = x[0];
    #pragma omp simd reduction(min:imin)
    for (i=1; i<n; i++)
    {
        imin = fmin(imin, x[i]);
    }
#endif
    return imin;
}
//============================================================================//
/*! 
 * @brief Returns element with the smallest absolute value in a complex array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find minimum absolute value.  This has
 *                   dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result Element of x with smallest absolute value.
 *
 * @ingroup array_min
 *
 * @author Ben Baker, ISTI
 *
 */
double complex array_min64z(const int n,
                            const double complex *__restrict__ x,
                            enum isclError_enum *ierr)
{
    double xmin;
    int indx;
    *ierr = ISCL_SUCCESS;
    if (n < 1 || x == NULL)
    {
        if (n < 1)
        {
            isclPrintError("Error no values in array x!");
            *ierr = ISCL_NULL_PTR;
            return DCMPLX(0.0, 0.0);
        }
        if (x == NULL)
        {
            isclPrintError("Error x is NULL");
            *ierr = ISCL_NULL_PTR;
            return DCMPLX(0.0, 0.0);
        }
        *ierr = ISCL_UNKNOWN_FAILURE;
        return DCMPLX(0.0, 0.0);
    }
    indx = array_absArgmin64z(n, x, ierr);
    xmin = cabs(x[indx]);
    return xmin;
}
