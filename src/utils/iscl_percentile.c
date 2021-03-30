#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/statistics/statistics.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/sorting/sorting.h"

/*!
 * @defgroup statistics_percentile percentile
 * @brief Computes the percentiles of an array of data points.
 * @ingroup statistics
 */
/*!
 * @brief Compute the q'th percentile of the data.
 *
 * @param[in] na      Number of elements in a.
 * @param[in] a       Contains the data.  This has dimension [na].
 * @param[in] nq      Number of quantiles.
 * @param[in] q       Percentiles to compute.  Each element must be in the
 *                    range [0,100].
 * @param[in] interp  This defines the interpolation strategy when a is between
 *                    two percentiles; \f$ i < q_k < j \f$.
 * @param[in] interp  STATS_PERCENTILE_LINEAR interpolates i + (j-1)*fraction
 *                    where fraction is the fractional part of the index 
 *                    surrounded by i and j.
 * @param[in] interp  STATS_PERCENTILE_LOWER will use i.
 * @param[in] interp  STATS_PERCENTILE_UPPER will use j.
 * @param[in] interp  STATS_PERCENTILE_NEAREST will choose i or j depending
 *                    on which is closer to q_k.
 * @param[in] interp  STATS_PRECENTILE_MIDPOINT wil use (i + j)/2.
 *
 * @param[out] ierr   ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @result The q'th percentile of each of the array elements a.  This is
 *         an array of dimension [nq].
 *
 * @ingroup statistics_percentile
 *
 * @author Ben Baker, ISTI
 */
double *statistics_percentile64f(
    const int na,
    const double *__restrict__ a,
    const int nq,
    const double *__restrict__ q,
    const enum isclStatisticsPercentile_enum interp,
    enum isclError_enum *ierr)
{
    double *percentile;
    *ierr = ISCL_SUCCESS;
    percentile = NULL;
    if (nq < 1)
    {
        isclPrintError("No quantiles to compute");
        *ierr = ISCL_ARRAY_TOO_SMALL;
        return percentile;
    }
    percentile = memory_calloc64f(nq);
    *ierr = statistics_percentile64f_work(na, a, nq, q, interp, percentile);
    if (*ierr != ISCL_SUCCESS)
    {
        isclPrintError("%s", "Error computing percentiles");
        memory_free64f(&percentile);
    }
    return percentile; 
}
//============================================================================//
/*!
 * @brief Compute the q'th percentile of the data.
 *
 * @param[in] na           Number of elements in a.
 * @param[in] a            Contains the data.  This has dimension [na].
 * @param[in] nq           Number of quantiles.
 * @param[in] q            Percentiles to compute.  Each element must be in the
 *                         range [0,100].
 * @param[in] interp       This defines the interpolation strategy when a is
 *                         between two percentiles; \f$ i < q_k < j \f$.
 * @param[in] interp       STATS_PERCENTILE_LINEAR interpolates i+(j-1)*fraction
 *                         where fraction is the fractional part of the index 
 *                         surrounded by i and j.
 * @param[in] interp       STATS_PERCENTILE_LOWER will use i.
 * @param[in] interp       STATS_PERCENTILE_UPPER will use j.
 * @param[in] interp       STATS_PERCENTILE_NEAREST will choose i or j depending
 *                         on which is closer to q_k.
 * @param[in] interp       STATS_PRECENTILE_MIDPOINT wil use (i + j)/2.
 *
 * @param[out] percentile  The q'th percentile of each of the array elements a.
 *                         This is an array of dimension [nq].
 * 
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup statistics_percentile
 *
 * @author Ben Baker, ISTI
 */
enum isclError_enum statistics_percentile64f_work(
    const int na,
    const double *__restrict__ a,
    const int nq,
    const double *__restrict__ q,
    const enum isclStatisticsPercentile_enum interp,
    double *__restrict__ percentile)
{
    double *asort, delta, delta_low, delta_high, dindex, q100;
    int iq, lbounds, lhs;
    bool lsorted;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //
    // Error handling
    ierr = ISCL_SUCCESS;
    asort = NULL;
    isclReturnArrayTooSmallError("na", na, 1);
    isclReturnArrayTooSmallError("nq", nq, 1);
    isclReturnNullPointerError("a", a);
    isclReturnNullPointerError("q", q);
    isclReturnNullPointerError("percentile", percentile);
    if (interp < 1 || interp > 5)
    {
        isclPrintError("Invalid interpolation type=%d", (int) interp);
        ierr = ISCL_INVALID_INPUT;
        return ierr;
    }
    // Check percentiles make sense
    lbounds = 0;
    #pragma omp simd reduction(+:lbounds)
    for (iq=0; iq<nq; iq++)
    {
        if (q[iq] < 0.0 || q[iq] > 100.0){lbounds = lbounds + 1;} //ierr = ierr + 1;}
    }
    if (lbounds != 0)
    {
        isclPrintError("Elements of q must be in range [0,100]");
        ierr = ISCL_INVALID_INPUT;
        return ierr;
    }
    // Get a sorted version of a
    lsorted = sorting_issorted64f(na, a, SORT_ASCENDING, &ierr);
    if (lsorted)
    {
        asort = array_copy64f(na, a, &ierr);
    }
    else
    {
        asort = sorting_sort64f(na, a, SORT_ASCENDING, &ierr);
    }
    // Linear interpolation
    if (interp == STATS_PERCENTILE_LINEAR)
    {
        for (iq=0; iq<nq; iq++)
        {
            q100 = q[iq]/100.0;
            dindex = q100*(double) (na - 1);
            lhs = (int) (fmin(na - 1, (int) dindex));
            delta = dindex - (double) lhs;
            percentile[iq] = asort[lhs];
            if (lhs < na - 1)
            {
                percentile[iq] = (1.0 - delta)*asort[lhs]
                               + delta*asort[lhs + 1];
            }
        }
    }
    else if (interp == STATS_PERCENTILE_HIGHER)
    {
        //#pragma omp simd
        for (iq=0; iq<nq; iq++)
        {
            q100 = q[iq]/100.0;
            dindex = q100*(double) (na - 1);
            lhs = (int) (fmin(na - 1, (int) dindex));
            delta = ceil(dindex - (double) lhs);
            percentile[iq] = asort[lhs];
            if (lhs < na - 1) 
            {
                percentile[iq] = (1.0 - delta)*asort[lhs]
                               + delta*asort[lhs+1];
            }
        }
    }
    else if (interp == STATS_PERCENTILE_LOWER)
    {
        for (iq=0; iq<nq; iq++)
        {
            q100 = q[iq]/100.0;
            dindex = q100*(double) (na - 1); 
            lhs = (int) (fmin(na - 1, (int) dindex));
            delta = floor(dindex - (double) lhs);
            percentile[iq] = asort[lhs];
            if (lhs < na - 1) 
            {
                percentile[iq] = (1.0 - delta)*asort[lhs]
                               + delta*asort[lhs+1];
            }
        }
    } 
    else if (interp == STATS_PERCENTILE_MIDPOINT)
    {
        for (iq=0; iq<nq; iq++)
        {
            q100 = q[iq]/100.0;
            dindex = q100*(double) (na - 1); 
            lhs = (int) (fmin(na - 1, (int) dindex));
            delta_low  = floor(dindex - (double) lhs);
            delta_high = ceil(dindex - (double) lhs); 
            delta = 0.5*(delta_low + delta_high);
            percentile[iq] = asort[lhs];
            if (lhs < na - 1) 
            {
                percentile[iq] = (1.0 - delta)*asort[lhs]
                               + delta*asort[lhs+1];
            }
        }
    }
    else if (interp == STATS_PERCENTILE_NEAREST)
    {
        for (iq=0; iq<nq; iq++)
        {
            q100 = q[iq]/100.0;
            dindex = q100*(double) (na - 1); 
            lhs = (int) dindex;
            delta = dindex - (double) lhs;
            if (delta <= 0.5)
            {
                delta = floor(dindex - (double) lhs);
            }
            else
            {
                delta = ceil(dindex - (double) lhs);
            } 
            if (lhs == na - 1)
            {
                percentile[iq] = asort[lhs];
            }
            else
            {
                percentile[iq] = (1.0 - delta)*asort[lhs]
                               + delta*asort[lhs+1];
            }
        }
    }
    else
    {
        isclPrintError("%s", "Invalid job!");
        ierr = ISCL_INVALID_INPUT;
    }
    // Free memory
    memory_free64f(&asort);
    return ierr;
}
