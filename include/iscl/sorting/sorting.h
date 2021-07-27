#ifndef _sorting_sorting_h__
#define _sorting_sorting_h__ 1
#include <stdbool.h>
#include "iscl/iscl/iscl.h"

enum sortOrder_enum 
{
    SORT_ASCENDING = 0,  /*!< Sort in ascending order */
    SORT_DESCENDING = 1, /*!< Sort in descending order */
    SORT_EITHER = 2      /*!< When checking if sorted - array can be ascending
                              or descending order */ 
};

/*!
 * @defgroup sorting  Sorting
 * @brief Sorting routines.
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif
/* Argument sort a double precision array */
int *sorting_argsort64f(const int n, const double *__restrict__ a,
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_argsort64f_work(const int n,
                            const double *__restrict__ a,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm);
/* Argument sort an integer array */
int *sorting_argsort32i(const int n, const int *__restrict__ ia,
                        const enum sortOrder_enum order, 
                        enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_argsort32i_work(const int n,
                            const int *__restrict__ ia,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm);
/* Argument sort a complex array */
#ifdef __cplusplus
int *sorting_argsort64z(const int n, const void *__restrict__ ca, 
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_argsort64z_work(const int n, const void *__restrict__ ca, 
                            const enum sortOrder_enum order,
                            int *__restrict__ perm);
#else
int *sorting_argsort64z(const int n, const double complex *__restrict__ ca,
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_argsort64z_work(const int n, const double complex *__restrict__ ca,
                            const enum sortOrder_enum order,
                            int *__restrict__ perm);
#endif
/* Searches a sorted array for a number */
int sorting_bsearch32i(const int key, const int *__restrict__ values,
                       const int n, const enum sortOrder_enum hint,
                       enum isclError_enum *ierr);
int sorting_bsearch64f(const double key, const double *__restrict__ values,
                       const int n, const enum sortOrder_enum hint,
                       enum isclError_enum *ierr);
/* Verify an array is sorted in ascending or descending order */
bool sorting_issorted64f(const int m, const double *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr);
bool sorting_issorted32i(const int m, const int *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr);
#ifdef __cplusplus
bool sorting_issorted64z(const int m, const void *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr);
#else
bool sorting_issorted64z(const int m, const double complex *__restrict__ a,
                         const enum sortOrder_enum order,
                         enum isclError_enum *ierr);
#endif
/* Apply argument sort */
double *sorting_applyPermutation64f(const int n, const int *__restrict__ perm,
                                    double *__restrict__ x, 
                                    enum isclError_enum *ierr)
        __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
int *sorting_applyPermutation32i(const int n, const int *__restrict__ perm,
                                 int *__restrict__ ix, 
                                 enum isclError_enum *ierr)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
#ifdef __cplusplus
void *sorting_applyPermutation64z(const int n,
                                            const int *__restrict__ perm,
                                            void *__restrict__ cx,
                                            enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_applyPermutation64z_work(const int n,
                                     const int *__restrict__ perm,
                                     void *__restrict__ cx,
                                     void *__restrict__ cxsort);
#else
double complex *sorting_applyPermutation64z(const int n,
                                            const int *__restrict__ perm,
                                            double complex *__restrict__ cx,
                                            enum isclError_enum *ierr)
                __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum
    sorting_applyPermutation64z_work(const int n,
                                     const int *__restrict__ perm,
                                     double complex *__restrict__ cx,
                                     double complex *__restrict__ cxsort);
#endif
enum isclError_enum
    sorting_applyPermutation64f_work(const int n, const int *__restrict__ perm,
                                     double *__restrict__ x,
                                     double *__restrict__ xsort);
enum isclError_enum
    sorting_applyPermutation32i_work(const int n,
                                     const int *__restrict__ perm,
                                     int *__restrict__ ix,
                                     int *__restrict__ ixsort);
/* Sorting a double array */
double *sorting_sort64f(const int n, const double *__restrict__ a, 
                        const enum sortOrder_enum order,
                        enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum sorting_sort64f_work(const int n, double *__restrict__ a,
                                         const enum sortOrder_enum order);
/* Sorting an integer array */
int *sorting_sort32i(const int n, const int *__restrict__ a, 
                     const enum sortOrder_enum order,
                     enum isclError_enum *ierr)
    __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum sorting_sort32i_work(const int n, int *__restrict__ a,
                                         const enum sortOrder_enum order);
/* Sort a complex array by magnitude */
#ifdef __cplusplus
void *sorting_sort64z(const int n,
                                const void *__restrict__ a,
                                const enum sortOrder_enum order,
                                enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum sorting_sort64z_work(const int n,
                                         void *__restrict__ a,
                                         const enum sortOrder_enum order);

#else
double complex *sorting_sort64z(const int n,
                                const double complex *__restrict__ a,
                                const enum sortOrder_enum order,
                                enum isclError_enum *ierr)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
enum isclError_enum sorting_sort64z_work(const int n,
                                         double complex *__restrict__ a,
                                         const enum sortOrder_enum order);
#endif

//#ifdef ISCL_LONG_NAMES

#define ISCL_sorting_issorted64f(...)       \
             sorting_issorted64f(__VA_ARGS__)
#define ISCL_sorting_issorted32i(...)       \
             sorting_issorted32i(__VA_ARGS__)
#define ISCL_sorting_issorted64z(...)       \
             sorting_issorted64z(__VA_ARGS__)

#define ISCL_sorting_argsort64f(...)  sorting_argsort64f(__VA_ARGS__)
#define ISCL_sorting_argsort32i(...)  sorting_argsort32i(__VA_ARGS__)
#define ISCL_sorting_argsort64z(...)  sorting_argsort64z(__VA_ARGS__)
#define ISCL_sorting_argsort64f_work(...)       \
             sorting_argsort64f_work(__VA_ARGS__)
#define ISCL_sorting_argsort32i_work(...)       \
             sorting_argsort32i_work(__VA_ARGS__)
#define ISCL_sorting_argsort64z_work(...)       \
             sorting_argsort64z_work(__VA_ARGS__)

#define ISCL_sorting_applyPermutation64f(...)       \
             sorting_applyPermutation64f(__VA_ARGS__)
#define ISCL_sorting_applyPermutation32i(...)       \
             sorting_applyPermutation32i(__VA_ARGS__)
#define ISCL_sorting_applyPermutation64z(...)       \
             sorting_applyPermutation64z(__VA_ARGS__)
#define ISCL_sorting_applyPermutation64f_work(...)       \
             sorting_applyPermutation64f_work(__VA_ARGS__)
#define ISCL_sorting_applyPermutation32i_work(...)       \
             sorting_applyPermutation32i_work(__VA_ARGS__)
#define ISCL_sorting_applyPermutation64z_work(...)       \
             sorting_applyPermutation64z_work(__VA_ARGS__)

#define ISCL_sorting_sort64f(...)       \
             sorting_sort64f(__VA_ARGS__)
#define ISCL_sorting_sort32i(...)       \
             sorting_sort32i(__VA_ARGS__)
#define ISCL_sorting_sort64z(...)       \
             sorting_sort64z(__VA_ARGS__)
#define ISCL_sorting_sort64f_work(...)       \
             sorting_sort64f_work(__VA_ARGS__)
#define ISCL_sorting_sort32i_work(...)       \
             sorting_sort32i_work(__VA_ARGS__)
#define ISCL_sorting_sort64z_work(...)       \
             sorting_sort64z_work(__VA_ARGS__)
//#endif
#ifdef __cplusplus
}
#endif
#endif /* _sorting_sorting_h__ */
