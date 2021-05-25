#include <stdbool.h>
#include <string.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/sorting/sorting.h"

/*!
 * @defgroup sorting_issorted issorted
 * @brief Determines if an array is sorted.
 * @ingroup sorting
 */
/*!
 * @brief Determines if a double array is sorted in ascending, descending, or
 *        either order.
 *
 * @param[in] m      Number of elements in a.
 * @param[in] a      Array to verify is sorted.  This has dimension [m].
 * @param[in] order  If order is SORT_ASCENDING then this will check
 *                   that a is in ascending order.
 * @param[in] order  If order is SORT_DESCENDING then this check
 *                   that a is in descending order.
 * @param[in] order  If order is SORT_EITHER then this will check that
 *                   a is in either ascending or descending order.
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @retval False indicates the array is not sorted in the desired order.
 * @retval True indicates the array is sorted in the desired order.
 *
 * @ingroup sorting_issorted
 * 
 * @author Ben Baker, ISTI
 *
 */
bool sorting_issorted64f(const int m, const double *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr)
{
    int i;
    bool lsorted;
    //------------------------------------------------------------------------//
    //
    // Input check
    *ierr = ISCL_SUCCESS;
    lsorted = false;
    if (m < 1)
    {
        isclLogArrayTooSmallError("x", m, 1);
        *ierr = ISCL_ARRAY_TOO_SMALL;
        return lsorted;
    }
    if (a == NULL)
    {
        isclLogPointerIsNullError("a");
        *ierr = ISCL_NULL_PTR;
        return lsorted;
    }
    // Easy
    lsorted = true;
    if (m == 1){return lsorted;}
    // Either
    if (order == SORT_EITHER)
    {
        // Ascending order
        for (i=1; i<m; i++)
        {
            if (a[i] < a[i-1])
            {
                lsorted = false;
                break;
            }
        }
        if (lsorted){return lsorted;}
        lsorted = true;
        // Descending order
        for (i=1; i<m; i++)
        {
            if (a[i] > a[i-1])
            {
                lsorted = false;
                break;
            }
        }
    }
    // Ascending or descending order
    else
    {
        // Descending
        if (order == SORT_DESCENDING)
        {
            for (i=1; i<m; i++)
            {
                if (a[i] > a[i-1])
                {
                    lsorted = false;
                    break;
                }   
            }   
        }
        // Ascending
        else
        {
            if (order != SORT_ASCENDING)
            {
                isclPrintWarning("%s", "Defaulting to ascending order");
            }
            for (i=1; i<m; i++)
            {
                if (a[i] < a[i-1])
                {
                    lsorted = false;
                    break;
                }
            }
        }
    } // End check on ascending/descneinding
    return lsorted;
}
//============================================================================//
/*!
 * @brief Determines if an integer array is sorted in ascending, descending, or
 *        either order.
 *
 * @param[in] m      Number of elements in a.
 * @param[in] a      Array to verify is sorted.  This has dimension [m].
 * @param[in] order  If order is SORT_ASCENDING then this will check
 *                   that a is in ascending order.
 * @param[in] order  If order is SORT_DESCENDING then this check
 *                   that a is in descending order.
 * @param[in] order  If order is SORT_EITHER then this will check that
 *                   a is in either ascending or descending order.
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @retval False indicates the array is not sorted in the desired order.
 * @retval True indicates the array is sorted in the desired order.
 *
 * @ingroup sorting_issorted
 * 
 * @author Ben Baker, ISTI
 *
 */
bool sorting_issorted32i(const int m, const int *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr)
{
    int i;
    bool lsorted;
    //------------------------------------------------------------------------//
    //  
    // Input check
    *ierr = ISCL_SUCCESS;
    lsorted = false;
    if (m < 1)
    {
        isclLogArrayTooSmallError("x", m, 1);
        *ierr = ISCL_ARRAY_TOO_SMALL;
        return lsorted;
    }   
    if (a == NULL) 
    {   
        isclLogPointerIsNullError("a");
        *ierr = ISCL_NULL_PTR;
        return lsorted;
    }
    // Easy
    lsorted = true;
    if (m == 1){return lsorted;}
    // Either
    if (order == SORT_EITHER)
    {
        // Ascending order
        for (i=1; i<m; i++)
        {
            if (a[i] < a[i-1])
            {
                lsorted = false;
                break;
            }
        }
        if (lsorted){return lsorted;}
        lsorted = true;
        // Descending order
        for (i=1; i<m; i++)
        {
            if (a[i] > a[i-1])
            {
                lsorted = false;
                break;
            }
        }
    }
    // Ascending or descending order
    else
    {
        // Descending
        if (order == SORT_DESCENDING)
        {
            for (i=1; i<m; i++)
            {
                if (a[i] > a[i-1])
                {
                    lsorted = false;
                    break;
                }
            }
        }
        // Ascending
        else
        {
            if (order != SORT_ASCENDING)
            {
                isclPrintWarning("%s", "Defaulting to ascending order");
            }
            for (i=1; i<m; i++)
            {
                if (a[i] < a[i-1])
                {
                    lsorted = false;
                    break;
                }
            }
        }
    } // End check on ascending/descneinding
    return lsorted;
}
//============================================================================//
/*!
 * @brief Determines if a complex array is sorted in ascending, descending, or
 *        either order based on the absolute value of each element.
 *
 * @param[in] m      Number of elements in a.
 * @param[in] a      Array to verify is sorted.  This has dimension [m].
 * @param[in] order  If order is SORT_ASCENDING then this will check
 *                   that a is in ascending order.
 * @param[in] order  If order is SORT_DESCENDING then this check
 *                   that a is in descending order.
 * @param[in] order  If order is SORT_EITHER then this will check that
 *                   a is in either ascending or descending order.
 *
 * @param[out] ierr  ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @retval False indicates the array is not sorted in the desired order.
 * @retval True indicates the array is sorted in the desired order.
 *
 * @ingroup sorting_issorted
 * 
 * @author Ben Baker, ISTI
 *
 */
bool sorting_issorted64z(const int m, const double complex *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr)
{
    int i;
    bool lsorted;
    //------------------------------------------------------------------------//
    //
    // Input check
    *ierr = ISCL_SUCCESS;
    lsorted = false;
    if (m < 1)
    {
        isclLogArrayTooSmallError("x", m, 1);
        *ierr = ISCL_ARRAY_TOO_SMALL;
        return lsorted;
    }
    if (a == NULL) 
    {   
        isclLogPointerIsNullError("a");
        *ierr = ISCL_NULL_PTR;
        return lsorted;
    }
    // Easy
    lsorted = true;
    if (m == 1){return lsorted;}
    // Either
    if (order == SORT_EITHER)
    {
        // Ascending order
        for (i=1; i<m; i++)
        {
            if (cabs(a[i]) < cabs(a[i-1]))
            {
                lsorted = false;
                break;
            }
        }
        if (lsorted){return lsorted;}
        lsorted = true;
        // Descending order
        for (i=1; i<m; i++)
        {
            if (cabs(a[i]) > cabs(a[i-1]))
            {
                lsorted = false;
                break;
            }
        }
    }
    // Ascending or descending order
    else
    {
        // Descending
        if (order == SORT_DESCENDING)
        {
            for (i=1; i<m; i++)
            {
                if (cabs(a[i]) > cabs(a[i-1]))
                {
                    lsorted = false;
                    break;
                }
            }
        }
        // Ascending
        else
        {
            if (order != SORT_ASCENDING)
            {
                isclPrintWarning("%s", "Defaulting to ascending");
            }
            for (i=1; i<m; i++)
            {
                if (cabs(a[i]) < cabs(a[i-1]))
                {
                    lsorted = false;
                    break;
                }
            }
        }
    } // End check on ascending/descneinding
    return lsorted;
}
