#ifndef STATISTICS_STATISTICS_H__
#define STATISTICS_STATISTICS_H__ 1
#include "iscl/iscl/iscl.h"

enum isclStatisticsMAD_enum
{
    STATS_MEAN_AVG_DEVIATION = 0,  /*!< Mean average deviation */ 
    STATS_MEDIAN_AVG_DEVIATION = 1 /*!< Median average deviation */
};

enum isclStatisticsNorm_enum
{
    STATS_DIV_NM1 = 0,   /*!< Normalize by n - 1. */
    STATS_DIV_N = 1      /*!< Normalized by n. */
};

enum isclStatisticsPercentile_enum
{
    STATS_PERCENTILE_LINEAR = 1,
    STATS_PERCENTILE_LOWER = 2,
    STATS_PERCENTILE_HIGHER = 3,
    STATS_PERCENTILE_MIDPOINT = 4,
    STATS_PERCENTILE_NEAREST = 5
}; 

/*!
 * @defgroup statistics Statistics
 * @brief Some basic statistical descriptors for datasets.
 * @copyright ISTI distributed under the Apache 2 license.
 */
#ifdef __cplusplus
extern "C"
{
#endif
/* Kurtosis of an array */
double statistics_kurtosis64f(const int n,
                              const double *__restrict__ x,
                              enum isclError_enum *ierr);
/* Compute mean of an array */
double statistics_mean64f(const int n, const double *__restrict__ x,
                          enum isclError_enum *ierr);
/* Compute median of an array */
double statistics_median64f(const int n, const double *__restrict x,
                            enum isclError_enum *ierr);
/* Compute mean or median average deviation of an array */
double statistics_mad64f(const int n, const double *__restrict__ x,
                         const enum isclStatisticsMAD_enum job,
                         enum isclError_enum *ierr);
/* Percentile of an array */
double *statistics_percentile64f(const int na,
                                const double *__restrict__ a,
                                const int nq,
                                const double *__restrict__ q,
                                const enum isclStatisticsPercentile_enum interp,
                                enum isclError_enum *ierr);
enum isclError_enum
    statistics_percentile64f_work(const int na,
                                const double *__restrict__ a,
                                const int nq, 
                                const double *__restrict__ q,
                                const enum isclStatisticsPercentile_enum interp,
                                double *__restrict__ percentile);
/* Standard deviation */
double statistics_std64f(const int n,
                         const double *__restrict__ x,
                         const enum isclStatisticsNorm_enum type,
                         enum isclError_enum *ierr);
/* Variance */
double statistics_var64f(const int n,
                         const double *__restrict__ x,
                         const enum isclStatisticsNorm_enum type,
                         enum isclError_enum *ierr);

//#ifdef ISCL_LONG_NAMES
#define ISCL_statistics_mean64f(...)       \
             statistics_mean64f(__VA_ARGS__)
#define ISCL_statistics_median64f(...)       \
             statistics_median64f(__VA_ARGS__)
#define ISCL_statistics_mad64f(...)       \
             statistics_mad64f(__VA_ARGS__)
#define ISCL_statistics_percentile64f(...)       \
             statistics_percentile64f(__VA_ARGS__)
#define ISCL_statistics_percentile64f(...)       \
             statistics_percentile64f(__VA_ARGS__)
//#endif

#ifdef __cplusplus
}
#endif
#endif /* _statistics_statistics_h__ */
