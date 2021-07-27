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

/*!
 * @defgroup array_argmax argmax 
 * @brief Computes the max index of an array.
 * @ingroup array
 */
/*! 
 * @brief Returns index corresponding to largest value in a double array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find maximum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmax
 *
 * @result Index of x which corresonds to largest value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmax64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr)
{
#ifdef ISCL_USE_INTEL
    Ipp64f *pSrc, max;
    IppStatus status;
#else
    double val;
    int i;
#endif
    int index;
    //------------------------------------------------------------------------//
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
    if (n == 1){return 0;}
#ifdef ISCL_USE_INTEL
    pSrc = (Ipp64f *) x;
    status = ippsMaxIndx_64f(pSrc, n, &max, &index);
    if (status != ippStsNoErr)
    {
        isclPrintError("Error calling ippsMaxIndx_64f");
        *ierr = ISCL_IPP_FAILURE;
        return -1;
    }
#else
    val = x[0];
    index = 0;
    for (i=1; i<n; i++)
    {
        if (x[i] > val)
        {
            index = i;
            val = x[i];
        }
    }
#endif
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to largest value in an integer array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find maximum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmax
 *
 * @result Index of x which corresonds to largest value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmax32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr)
{
#ifdef ISCL_USE_INTEL
    Ipp32s *pSrc, max;
    IppStatus status;
#else
    int i, val;
#endif
    int index;
    //------------------------------------------------------------------------//
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
    if (n == 1){return 0;} 
#ifdef ISCL_USE_INTEL
    pSrc = (Ipp32s *) x;
    status = ippsMaxIndx_32s(pSrc, n, &max, &index);
    if (status != ippStsNoErr)
    {   
        isclPrintError("Error calling ippsMaxIndx_32s");
        *ierr = ISCL_IPP_FAILURE;
        return -1;
    }
#else
    val = x[0];
    index = 0;
    for (i=1; i<n; i++)
    {
        if (x[i] > val)
        {
            index = i;
            val = x[i];
        }
    }   
#endif
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to largest absolute value in a
 *        double complex array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find maximum absolute value.
 *                   This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmax
 *
 * @result Index of x which corresonds to largest absolute value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmax64z(const int n, const double complex *__restrict__ x,
                    enum isclError_enum *ierr)
{
    double cx, val;
    int i, index;
    //------------------------------------------------------------------------//
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
    if (n == 1){return 0;} 
    val = cabs(x[0]);
    index = 0;
    for (i=1; i<n; i++)
    {
        cx = cabs(x[i]);
        if (cx > val)
        {
            index = i;
            val = cx;
        }
    }   
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to largest value in a float array.
 *
 * @param[in] n      Number of points in array x.
 * @param[in] x      Array of which to find maximum.  This has dimension [n].
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @ingroup array_argmax
 *
 * @result Index of x which corresonds to largest value.
 *
 * @author Ben Baker, ISTI
 *
 */
int array_argmax32f(const int n, const float *__restrict__ x,
                    enum isclError_enum *ierr)
{
#ifdef ISCL_USE_INTEL
    Ipp32f *pSrc, max;
    IppStatus status;
#else
    float val;
    int i;
#endif
    int index;
    //------------------------------------------------------------------------//
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
    if (n == 1){return 0;}
#ifdef ISCL_USE_INTEL 
    pSrc = (Ipp32f *) x;
    status = ippsMaxIndx_32f(pSrc, n, &max, &index);
    if (status != ippStsNoErr)
    {
        isclPrintError("Error calling ippsMaxIndx_32f");
        *ierr = ISCL_IPP_FAILURE;
        return 0;
    }
#else
    val = x[0];
    index = 0;
    for (i=1; i<n; i++)
    {
        if (x[i] > val)
        {   
            index = i;
            val = x[i];
        }
    }
#endif
    return index;
}
