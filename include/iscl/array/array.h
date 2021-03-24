#ifndef ARRAY_ARRAY_H
#define ARRAY_ARRAY_H 1
#include <stdbool.h>
// #include "iscl/config.h"
#include "iscl/iscl/iscl.h"
// #include "iscl/statistics/statistics.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
enum normType_enum
{
    TWO_NORM = 2,              /*!< \$ L_2 = \sqrt{\sum_i x_i^2} \$ norm */
    ONE_NORM = 1,              /*!< \$ L_1 = \sum_i |x_i| \$ norm */
    P_NORM = 3,                /*!< \$ L_p 
                                    = \left (
                                        \sum_i |x_i|^p \right )^{1/p}
                                      \right ) \$ norm */
    INFINITY_NORM = 4,          /*!< \$ L_\infty = max |x| \$ */
    NEGATIVE_INFINITY_NORM = 5  /*!< \$ L_{-\infty} = min |x| \$ */
};
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/*!
 * @defgroup array Array
 *
 * @brief Performs simple mathematical operations on an array of data.
 *
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif
/* Compute the absolute value of an array */
double *array_abs64f(const int n, double *__restrict__ x, 
                     enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_abs64f_work(const int n, double *__restrict__ x,
                      double *__restrict__ y); 
int *array_abs32i(const int n, int *__restrict__ x, 
                  enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_abs32i_work(const int n, int *__restrict__ x,
                      int *__restrict__ y);
#ifdef __cplusplus
double *array_abs64z(const int n, const void *__restrict__ x,
                     enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_abs64z_work(const int n, const void *__restrict__ x,
                      double *__restrict__ y);
#else
double *array_abs64z(const int n, const double complex *__restrict__ x,
                     enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_abs64z_work(const int n, const double complex *__restrict__ x,
                      double *__restrict__ y);
#endif
/* Index of max absolute value of an array */
int array_absArgmax64f(const int n, const double *__restrict__ x,
                       enum isclError_enum *ierr);
int array_absArgmax32i(const int n, const int *__restrict__ x,
                       enum isclError_enum *ierr);
#ifdef __cplusplus
int array_absArgmax64z(const int n, const void *__restrict__ x,
                       enum isclError_enum *ierr);
#else
int array_absArgmax64z(const int n, const double complex *__restrict__ x,
                       enum isclError_enum *ierr);
#endif
/* Index of min absolute value of an array */
int array_absArgmin64f(const int n, const double *__restrict__ x,
                       enum isclError_enum *ierr);
int array_absArgmin32i(const int n, const int *__restrict__ x,
                       enum isclError_enum *ierr);
#ifdef __cplusplus
int array_absArgmin64z(const int n, const void *__restrict__ x,
                       enum isclError_enum *ierr);
#else
int array_absArgmin64z(const int n, const double complex *__restrict__ x,
                       enum isclError_enum *ierr);
#endif
/* Angle between complex numbers */
#ifdef __cplusplus
double *array_angle64z(const int n, const void *z,
                       enum isclError_enum *ierr);
enum isclError_enum
    array_angle64z_work(const int n, const void *__restrict__ z,
                        double *__restrict__ angle);
#else
double *array_angle64z(const int n, const double complex *z,
                       enum isclError_enum *ierr);
enum isclError_enum
    array_angle64z_work(const int n, const double complex *__restrict__ z,
                        double *__restrict__ angle);
#endif
/* Determines index of max value in an array */
int array_argmax64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr);
int array_argmax32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr);
#ifdef __cplusplus
int array_argmax64z(const int n, const void *__restrict__ x,
                    enum isclError_enum *ierr);
#else
int array_argmax64z(const int n, const double complex *__restrict__ x,
                    enum isclError_enum *ierr);
#endif
int array_argmax32f(const int n, const float *__restrict__ x,
                    enum isclError_enum *ierr);
/* Determines index of min in an array */
int array_argmin64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr);
int array_argmin32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr);
#ifdef __cplusplus
int array_argmin64z(const int n, const void *__restrict__ x,
                    enum isclError_enum *ierr);
#else
int array_argmin64z(const int n, const double complex *__restrict__ x,
                    enum isclError_enum *ierr);
#endif
/* Matches conjugate pairs up */
#ifdef __cplusplus
void *array_cplxpair64z(const int n,
                        const void *__restrict__ x,
                        const double tol,
                        enum isclError_enum *ierr);
enum isclError_enum
    array_cplxpair64z_work(const int n,
                           const void *__restrict__ x,
                           const double tolIn,
                           void *__restrict__ y);
#else
double complex *array_cplxpair64z(const int n,
                                  const double complex *__restrict__ x,
                                  const double tol,
                                  enum isclError_enum *ierr);
enum isclError_enum
    array_cplxpair64z_work(const int n,
                           const double complex *__restrict__ x,  
                           const double tolIn,
                           double complex *__restrict__ y);
#endif
/* Sign (+1 or -1) of an array */
double *array_copysign64f(const int n, const double *__restrict__ x,
                          enum isclError_enum *ierr);
enum isclError_enum
    array_copysign64f_work(const int n, const double *__restrict__ x,
                           double *__restrict__ sign);
/* Copies an array */
double *array_copy64f(const int n, const double *src,
                      enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_copy64f_work(const int n, const double *__restrict__ src,
                       double *__restrict__ dest);
int *array_copy32i(const int n, const int *src, enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_copy32i_work(const int n, const int *__restrict__ src,
                       int *__restrict__ dest);
#ifdef __cplusplus
void *array_copy64z(const int n, const void *src,
                    enum isclError_enum *ierr)
                    __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_copy64z_work(const int n, const void *__restrict__ src,
                       void *__restrict__ dest);
#else
double complex *array_copy64z(const int n, const double complex *src,
                              enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_copy64z_work(const int n, const double complex *__restrict__ src,
                       double complex *__restrict__ dest);
#endif
bool *array_copy8l(const int n, const bool *src, enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_copy8l_work(const int n, const bool *__restrict__ src,
                      bool *__restrict__ dest);


/*
#ifdef ISCL_USE_C11

#define array_copy(n, x, y) _Generic((x), \
        double: array_copy64f_work, \
        double complex: array_copy64z_work, \
        int: array_copy32i_work, \
        bool: array_copy8l_work, \
        default: array_copy64f_work \
        ) (n, x, y)
#endif
*/

/* Cumulative sum of an array */
double *array_cumsum64f(const int n, const double *x,
                        enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_cumsum64f_work(const int n, const double *__restrict__ x,
                         double *__restrict__ y); 
int *array_cumsum32i(const int n, const int *x,
                     enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_cumsum32i_work(const int n, const int *__restrict__ x,
                         int *__restrict__ y); 
#ifdef __cplusplus
void *array_cumsum64z(const int n, const void *x,
                                enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_cumsum64z_work(const int n, const void *__restrict__ x,
                         void *__restrict__ y);
#else
double complex *array_cumsum64z(const int n, const double complex *x,
                                enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_cumsum64z_work(const int n, const double complex *__restrict__ x,
                         double complex *__restrict__ y);
#endif
/* Differentiate an array */
double *array_diff64f(const int n, const double *__restrict__ x,
                      enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_diff64f_work(const int n,
                                       const double *__restrict__ x,
                                       double *__restrict__ y);
/* Compute dot product of arrays */
double array_dot64f(const int n,
                    const double *__restrict__ x,
                    const double *__restrict__ y,
                    enum isclError_enum *ierr);
#ifdef __cplusplus

#else
double complex array_dot64z(const int n,
                            const bool lconjx,
                            const double complex *__restrict__ x,
                            const double complex *__restrict__ y,
                            enum isclError_enum *ierr);
#endif
/* Exponent of array x */
double *array_exp64f(const int n, double *__restrict__ x, 
                     enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_exp64f_work(const int n, double *__restrict__ x,
                                      double *__restrict__ y);
#ifdef __cplusplus
void *array_exp64z(const int n,
                   void *__restrict__ x,
                   enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_exp64z_work(const int n, void *__restrict__ x,
                      void *__restrict__ y);
#else
double complex *array_exp64z(const int n,
                             double complex *__restrict__ x, 
                             enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_exp64z_work(const int n, double complex *__restrict__ x,
                      double complex *__restrict__ y);
#endif
/* Inserts an array xin into x */
double *array_insert64f(const int ninsert, double *__restrict__ xin,
                        const int nx, double *__restrict__ x,
                        const int idx,
                        int *ny, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_insert64f_work(const int ninsert, double *__restrict__ xin,
                         const int nx, double *__restrict__ x,
                         const int idx,
                         const int ny, double *__restrict__ y);
/* Determine which elements of a complex array are real */
#ifdef __cplusplus
bool *array_isreal64z(const int n, const void *x,
                      enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_isreal64z_work(const int n, const void *__restrict__ x,
                         bool *__restrict__ lreal);
#else
bool *array_isreal64z(const int n, const double complex *x, 
                      enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_isreal64z_work(const int n, const double complex *__restrict__ x,
                         bool *__restrict__ lreal);
#endif
/* log_{base} of x */
double *array_log64f(const int n, const double *__restrict__ x,
                     const double base, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_log64f_work(const int n, const double *__restrict__ x,
                      const double base,
                      double *__restrict__ y);
/* Linearly spaced array */
double *array_linspace64f(const double x1, const double x2, 
                          const int n, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_linspace64f_work(const double x1, const double x2, const int n,
                           double *__restrict__ x);
/* Max value of an array */
double array_max64f(const int n, const double *__restrict__ x,
                    enum isclError_enum *ierr);
int array_max32i(const int n, const int *__restrict__ x,
                 enum isclError_enum *ierr);
#ifdef __cplusplus

#else
double complex array_max64z(const int n, const double complex *__restrict__ x,
                            enum isclError_enum *ierr);
#endif
/* Apply a mask to an array */
double *array_mask64f(const int n, const double *__restrict__ x,
                      const bool *__restrict__ lkeep, int *ny, 
                      enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_mask64f_work(const int n, const double *__restrict__ x,
                       const bool *__restrict__ lkeep,
                       const int nwork, int *ny, double *__restrict__ y);
/* Max value of an array */
double array_maxAbs64f(const int n, const double *__restrict__ x,
                       enum isclError_enum *ierr);
int array_maxAbs32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr);
#ifdef __cplusplus
double array_maxAbs64z(const int n,
                       const void *__restrict__ x,
                       enum isclError_enum *ierr);
#else
double array_maxAbs64z(const int n,
                       const double complex *__restrict__ x,
                       enum isclError_enum *ierr);
#endif
/* Min value in an array */
double array_min64f(const int n, const double *__restrict__ x,
                     enum isclError_enum *ierr);
int array_min32i(const int n, const int *__restrict__ x,
                 enum isclError_enum *ierr);
float array_min32f(const int n, const float *__restrict__ x,
                   enum isclError_enum *ierr);
#ifdef __cplusplus

#else
double complex array_min64z(const int n, const double complex *__restrict__ x,
                            enum isclError_enum *ierr);
#endif
/* Min absolute value of an array */
double array_minAbs64f(const int n, const double *__restrict__ x,
                       enum isclError_enum *ierr);
int array_minAbs32i(const int n, const int *__restrict__ x,
                    enum isclError_enum *ierr);
#ifdef __cplusplus
double array_minAbs64z(const int n,
                       const void *__restrict__ x,
                       enum isclError_enum *ierr);
#else
double array_minAbs64z(const int n,
                       const double complex *__restrict__ x,
                       enum isclError_enum *ierr);
#endif
/* Difference of arrays */
double *array_minus64f(const int n,
                       double *__restrict__ a,
                       const double *__restrict__ b, 
                       enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_minus64f_work(const int n,
                        double *__restrict__ a,
                        const double *__restrict__ b,
                        double *__restrict__ c);
/* Arithmetic mean whilst ignoring NaNs */
double array_nanmean64f(const int n, const double *x, 
                        enum isclError_enum *ierr);
double array_nanmean32i(const int n, const int *x,
                        enum isclError_enum *ierr);
/* pow */
double *array_pow64f(const int n, const double *__restrict__ x,
                     const double p, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_pow64f_work(const int n, const double *__restrict__ x,
                      const double p, double *__restrict__ y);
/* Reverse an array */
double *array_reverse64f(const int n, double *a, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_reverse64f_work(const int n, double *a, double *b);
int *array_reverse32i(const int n, int *a, enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_reverse32i_work(const int n, int *a, int *b);
#ifdef __cplusplus
void *array_reverse64z(const int n, void *a,
                       enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_reverse64z_work(const int n,
                                          void *a,
                                          void *b);
#else
double complex *array_reverse64z(const int n, double complex *a,
                                 enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_reverse64z_work(const int n,
                                          double complex *a,
                                          double complex *b);
#endif
/* Sets all values of an array to a constant */
double *array_set64f(const int n, const double value,
                     enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_set64f_work(const int n, const double value,
                      double *__restrict__ x);
float *array_set32f(const int n, const float value,
                    enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_set32f_work(const int n, const float value,
                      float *__restrict__ x);
int *array_set32i(const int n, const int value, int *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_set32i_work(const int n, const int value,
                      int *__restrict__ x);
#ifdef __cplusplus

#else
double complex *array_set64z(const int n, const double complex value,
                             int *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_set64z_work(const int n, const double complex value,
                      double complex *__restrict__ x);
#endif
bool *array_set8l(const int n, const bool value, 
                  enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_set8l_work(const int n, const bool value,
                     bool *__restrict__ x);
/* sinc function */
double *array_sinc64f(const int n, const double *__restrict__ x,
                      enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_sinc64f_work(const int n, const double *__restrict__ x,
                       double *__restrict__ sinc);
/* Determines the sum of the elements of an array */
double array_sum64f(const int n, const double *x, enum isclError_enum *ierr);
float array_sum32f(const int n, const float *__restrict__ x,
                   enum isclError_enum *ierr);
int array_sum32i(const int n, const int *x, enum isclError_enum *ierr);
#ifdef __cplusplus

#else
double complex array_sum64z(const int n, const double complex *x,
                            enum isclError_enum *ierr);
#endif
int array_sum8l(const int n, const bool *x, enum isclError_enum *ierr);

/* Norm */
double array_norm64f(const int n, const double *__restrict__ x,
                     const enum normType_enum norm, const double p,
                     enum isclError_enum *ierr);
#ifdef __cplusplus
double array_norm64z(const int n, const void *__restrict__ x,  
                     const enum normType_enum norm, const double p,
                     enum isclError_enum *ierr);
#else
double array_norm64z(const int n, const double complex *__restrict__ x, 
                     const enum normType_enum norm, const double p,
                     enum isclError_enum *ierr);
#endif
float array_norm32f(const int n, const float *__restrict__ x,
                    const enum normType_enum norm, const float p,
                    enum isclError_enum *ierr);

/*
#ifdef ISCL_USE_C11
#define array_norm(n, x, norm, p, ierr) _Generic((x), \
        double: array_norm64f, \
        float: array_norm32f, \
        double complex: array_norm64z, \
        default: array_norm64f \
        ) (n, x, norm, p, ierr)
#endif
*/

/* Norm of difference of x and y */
double array_normDiff64f(const int n,
                         const double *__restrict__ x,
                         const double *__restrict__ y,
                         const enum normType_enum norm, const double p,
                         enum isclError_enum *ierr);
#ifdef __cplusplus
double array_normDiff64z(const int n,
                         const void *__restrict__ x,
                         const void *__restrict__ y,
                         const enum normType_enum norm,
                         const double p,
                         enum isclError_enum *ierr);
#else
double array_normDiff64z(const int n,
                         const double complex *__restrict__ x,
                         const double complex *__restrict__ y,
                         const enum normType_enum norm,
                         const double p,
                         enum isclError_enum *ierr);
#endif

/*
#ifdef ISCL_USE_C11
#define array_normDiff(n, x, norm, p, ierr) _Generic((x), \
        double: array_normDiff64f, \
        default: array_normDiff64f \
        ) (n, x, norm, p, ierr)
#endif
*/

/* Multiply arrays */
double *array_times64f(const int n,
                       double *__restrict__ x,
                       double *__restrict__ y,
                       enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_times64f_work(const int n,
                        double *__restrict__ x,
                        double *__restrict__ y,
                        double *__restrict__ z);
#ifdef __cplusplus
void *array_times64z(const int n,
                     void *__restrict__ x,
                     void *__restrict__ y,
                     enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_times64z_work(const int n,
                        void *__restrict__ x,
                        void *__restrict__ y,
                        void *__restrict__ z);
#else
double complex *array_times64z(const int n,
                               double complex *__restrict__ x,
                               double complex *__restrict__ y,
                               enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_times64z_work(const int n,
                        double complex *__restrict__ x,
                        double complex *__restrict__ y,
                        double complex *__restrict__ z);
#endif
/*
#ifdef ISCL_USE_C11
#define array_times(n, x, y, z) _Generic((x, y, z), \
        double: array_times64f_work, \
        double complex: array_times64z_work, \
        default: array_times64f_work \
        ) (n, x, y, z)
#endif
*/

/* Unwrap phase */
enum isclError_enum
    array_unwrap64f_work(const int n, const double *__restrict__ p,
                         const double tol, double *__restrict__ q);
double *array_unwrap64f(const int n, const double *__restrict__ p,
                        const double tol, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    array_unwrap32f_work(const int n, const float *__restrict__ p,
                         const float tol, float *__restrict__ q);
float *array_unwrap32f(const int n, const float *__restrict__ p,
                       const float tol, enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));

/* Zero out an array */
double *array_zeros64f(const int n, enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros64f_work(const int n,
                                        double *__restrict__ zeros);
int *array_zeros32i(const int n, enum isclError_enum *__restrict__ zeros)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros32i_work(const int n, int *__restrict__ zeros);
float *array_zeros32f(const int n, enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros32f_work(const int n, float *__restrict__ zeros);
#ifdef __cplusplus
void *array_zeros64z(const int n, enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros64z_work(const int n,
                                        void *__restrict__ zeros);
#else
double complex *array_zeros64z(const int n, enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros64z_work(const int n,
                                        double complex *__restrict__ zeros);
#endif
bool *array_zeros8l(const int n, enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros8l_work(const int n, bool *__restrict__ zeros);
char *array_zeros8c(const int n, enum isclError_enum *ierr)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum array_zeros8c_work(const int n, char *__restrict__ zeros);

#ifdef ISCL_USE_C11
#define array_zeros(n, x) _Generic((x), \
        double: array_zeros64f_work, \
        double complex: array_zeros64z_work, \
        int: array_zeros32i_work, \
        char: array_zeros8c_work, \
        bool: array_zeros8l_work, \
        default: array_zeros64f_work \
        ) (n, x)
#endif

//#ifdef ISCL_LONG_NAMES
#define ISCL_array_abs__double(...)       \
             array_abs__double(__VA_ARGS__)
#define ISCL_array_abs__int(...)       \
             array_abs__int(__VA_ARGS__)
#define ISCL_array_abs__complex(...)       \
             array_abs__complex(__VA_ARGS__)
#define ISCL__array_abs__double(...)       \
            __array_abs__double(__VA_ARGS__)
#define ISCL__array_abs__int(...)       \
            __array_abs__int(__VA_ARGS__)
#define ISCL__array_abs__complex(...)       \
            __array_abs__complex(__VA_ARGS__)

#define ISCL_array_absArgmax64f(...)       \
             array_absArgmax64f(__VA_ARGS__)
#define ISCL_array_absArgmax32i(...)       \
             array_absArgmax32i(__VA_ARGS__)
#define ISCL_array_absArgmax64z(...)       \
             array_absArgmax64z(__VA_ARGS__)

#define ISCL_array_absArgmin64f(...)       \
             array_absArgmin64f(__VA_ARGS__)
#define ISCL_array_absArgmin32i(...)       \
             array_absArgmin32i(__VA_ARGS__)
#define ISCL_array_absArgmin64z(...)       \
             array_absArgmin64z(__VA_ARGS__)

#define ISCL_array_argmax64f(...)       \
             array_argmax64f(__VA_ARGS__)
#define ISCL_array_argmax32i(...)       \
             array_argmax32i(__VA_ARGS__)
#define ISCL_array_argmax64z(...)       \
             array_argmax64z(__VA_ARGS__)

#define ISCL_array_argmin64f(...)       \
             array_argmin64f(__VA_ARGS__)
#define ISCL_array_argmin32i(...)       \
             array_argmin32i(__VA_ARGS__)
#define ISCL_array_argmin64z(...)       \
             array_argmin64z(__VA_ARGS__)

#define ISCL_array_copy64f(...)       \
             array_copy64f(__VA_ARGS__)
#define ISCL_array_copy32i(...)       \
             array_copy32i(__VA_ARGS__)
#define ISCL_array_copy64z(...)       \
             array_copy64z(__VA_ARGS__)
#define ISCL_array_copy8l(...)       \
             array_copy8l(__VA_ARGS__)
#define ISCL_array_copy64f_work(...)       \
             array_copy64f_work(__VA_ARGS__)
#define ISCL_array_copy32i_work(...)       \
             array_copy32i_work(__VA_ARGS__)
#define ISCL_array_copy64z_work(...)       \
             array_copy64z_work(__VA_ARGS__)
#define ISCL_array_copy8l_work(...)       \
             array_copy8l_work(__VA_ARGS__)

#define ISCL_array_cumsum64f(...)       \
             array_cumsum64f(__VA_ARGS__)
#define ISCL_array_cumsum32i(...)       \
             array_cumsum32i(__VA_ARGS__)
#define ISCL_array_cumsum64z(...)       \
             array_cumsum64z(__VA_ARGS__)
#define ISCL_array_cumsum64f_work(...)       \
             array_cumsum64f_work(__VA_ARGS__)
#define ISCL_array_cumsum32i_work(...)       \
             array_cumsum32i_work(__VA_ARGS__)
#define ISCL_array_cumsum64z_work(...)       \
             array_cumsum64z_work(__VA_ARGS__)

#define ISCL__array_dot64f(...)       \
              array_dot64f(__VA_ARGS__)
#define ISCL__array_dot64z(...)       \
              array_dot64z(__VA_ARGS__)

#define ISCL_array_isreal64z(...)       \
             array_isreal64z(__VA_ARGS__)
#define ISCL_array_isreal64z_work(...)       \
             array_isreal64z_work(__VA_ARGS__)

#define ISCL_array_linspace64f(...)       \
             array_linspace64f(__VA_ARGS__)
#define ISCL_array_linspace64f_work(...)       \
             array_linspace64f_work(__VA_ARGS__)

#define ISCL_array_max64f(...)       \
             array_max64f(__VA_ARGS__)
#define ISCL_array_max32i(...)       \
             array_max32i(__VA_ARGS__)
#define ISCL_array_max64z(...)       \
             array_max64z(__VA_ARGS__)

#define ISCL_array_maxAbs64f(...)       \
             array_maxAbs64f(__VA_ARGS__)
#define ISCL_array_maxAbs32i(...)       \
             array_maxAbs32i(__VA_ARGS__)
#define ISCL_array_maxAbs64z(...)       \
             array_maxAbs64z(__VA_ARGS__)

/*
#define ISCL_array_mean__double(...)            \
             statistics_mean__double(__VA_ARGS__)
#define array_mean__double(...)            \
        statistics_mean__double(__VA_ARGS__)
*/

#define ISCL_array_min64f(...)       \
             array_min64f(__VA_ARGS__)
#define ISCL_array_min32i(...)       \
             array_min32i(__VA_ARGS__)
#define ISCL_array_min64z(...)       \
             array_min64z(__VA_ARGS__)

#define ISCL_array_minAbs64f(...)       \
             array_minAbs64f(__VA_ARGS__)
#define ISCL_array_minAbs32i(...)       \
             array_minAbs32i(__VA_ARGS__)
#define ISCL_array_minAbs64z(...)       \
             array_minAbs64z(__VA_ARGS__)

#define ISCL_array_nanmean__double(...)       \
             array_nanmean__double(__VA_ARGS__)
#define ISCL_array_nanmean__int(...)       \
             array_nanmean__int(__VA_ARGS__)

#define ISCL_array_reverse__double(...)       \
             array_reverse__double(__VA_ARGS__)
#define ISCL_array_reverse__int(...)       \
             array_reverse__int(__VA_ARGS__)
#define ISCL_array_reverse__complex(...)       \
             array_reverse__complex(__VA_ARGS__)
#define ISCL__array_reverse__double(...)       \
            __array_reverse__double(__VA_ARGS__)
#define ISCL__array_reverse__int(...)       \
            __array_reverse__int(__VA_ARGS__)
#define ISCL__array_reverse__complex(...)       \
            __array_reverse__complex(__VA_ARGS__)

#define ISCL_array_set64f(...)       \
             array_set64f(__VA_ARGS__)
#define ISCL_array_set64f_work(...)       \
             array_set64f_work(__VA_ARGS__)
#define ISCL_array_set32f_work(...)       \
             array_set32f_work(__VA_ARGS__)
#define ISCL_array_set32i(...)       \
             array_set32i(__VA_ARGS__)
#define ISCL_array_set32i_work(...)       \
             array_set32i_work(__VA_ARGS__)
#define ISCL_array_set64z(...)       \
             array_set64z(__VA_ARGS__)
#define ISCL_array_set64z_work(...)       \
             array_set64z_work(__VA_ARGS__)

#define ISCL_array_sum64f(...)       \
             array_sum64f(__VA_ARGS__)
#define ISCL_array_sum32f(...)       \
             array_sum32f(__VA_ARGS__)
#define ISCL_array_sum32i(...)       \
             array_sum32i(__VA_ARGS__)
#define ISCL_array_sum64z(...)       \
             array_sum64z(__VA_ARGS__)
#define ISCL_array_sum8l(...)       \
             array_sum8l(__VA_ARGS__)

#define ISCL_array_norm64f(...)       \
             array_norm64f(__VA_ARGS__)
#define ISCL_array_norm64z(...)       \
             array_norm64z(__VA_ARGS__)

#define ISCL_array_normDiff64f(...)       \
             array_normDiff64f(__VA_ARGS__)

#define ISCL_array_times__double(...)       \
             array_times__double(__VA_ARGS__)
#define ISCL__array_times__double(...)       \
            __array_times__double(__VA_ARGS__)
#define ISCL_array_times__complex(...)       \
             array_times__complex(__VA_ARGS__)
#define ISCL__array_times__complex(...)       \
            __array_times__complex(__VA_ARGS__)

#define ISCL_array_unwrap(...)       \
             array_unwrap(__VA_ARGS__)
#define ISCL__array_unwrap(...)       \
            __array_unwrap(__VA_ARGS__)

#define ISCL_array_zeros__double(...)       \
             array_zeros__double(__VA_ARGS__)
#define ISCL__array_zeros__double(...)       \
            __array_zeros___double(__VA_ARGS__)
#define ISCL_array_zeros__int(...)       \
             array_zeros__int(__VA_ARGS__)
#define ISCL__array_zeros__int(...)       \
            __array_zeros__int(__VA_ARGS__)
#define ISCL_array_zeros__complex(...)       \
             array_zeros__complex(__VA_ARGS__)
#define ISCL__array_zeros__bool(...)       \
            __array_zeros__bool(__VA_ARGS__)
#define ISCL__array_zeros__bool(...)       \
            __array_zeros__bool(__VA_ARGS__)
#define ISCL_array_zeros__char(...)       \
             array_zeros__char(__VA_ARGS__)
#define ISCL__array_zeros__char(...)       \
            __array_zeros__char(__VA_ARGS__)
//#endif

#ifdef __cplusplus
}
#endif
#endif /* ARRAY_ARRAY_H */
