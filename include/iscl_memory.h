#ifndef MEMORY_MEMORY_H__
#define MEMORY_MEMORY_H__ 1
#include <stdbool.h>
#include "iscl/config.h"
#include "iscl/iscl/iscl.h"
#include "iscl/iscl/iscl_enum.h"
#include "iscl/iscl/iscl_defs.h"

/*!
 * @defgroup memory Memory
 * @brief Handles allocation and deallocation of memory in ISCL.
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif

#define memory_isAligned(POINTER, BYTE_COUNT) \
    (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

/* calloc with ISCL_MEOMRY_ALIGN byte alignment */
double *memory_calloc64f(const int n) 
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
int *memory_calloc32i(const int n)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
#ifdef __cplusplus
void *memory_calloc64z(const int n)
     __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
#else
double complex *memory_calloc64z(const int n)
               __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
#endif
bool *memory_calloc8l(const int n)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
char *memory_calloc8c(const int n)
      __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));
float *memory_calloc32f(const int n)
       __attribute__ ((aligned(ISCL_MEMORY_ALIGN)));

double *memory_malloc64f(const int n,
                         enum isclError_enum *ierr);
int *memory_malloc32i(const int n, enum isclError_enum *ierr);
float *memory_malloc32f(const int n, enum isclError_enum *ierr);
#ifdef __cplusplus
void *memory_malloc64z(const int n, enum isclError_enum *ierr);
#else
double complex *memory_malloc64z(const int n, enum isclError_enum *ierr);
#endif
bool *memory_malloc8l(const int n, enum isclError_enum *ierr);
char *memory_malloc8c(const int n, enum isclError_enum *ierr);


/*
#ifdef ISCL_USE_C11
#define memory_malloc(n, arg) _Generic((arg), \
        double: memory_malloc__double, \
        int: memory_malloc__int, \
        double complex: memory_malloc__complex, \
        char: memory_malloc__char, \
        bool: memory_malloc__bool, \
        float: memory_malloc__float, \
        default: memory_malloc__double \
        ) (n, arg)
#endif
*/

/* free */
//void memory_free__void(void **p);
void memory_free64f(double **p);
void memory_free32i(int **p);
#ifdef __cplusplus
void memory_free64z(void **p);
#else
void memory_free64z(double complex **p);
#endif
void memory_free8c(char **p);
void memory_free8l(bool **p);
void memory_free32f(float **p);
void memory_freeZPK(struct signalZPK_struct *zpk);
void memory_freeBA(struct signalBA_struct *ba);
void memory_freeSOS(struct signalSOS_struct *sos);

/*
#ifdef ISCL_USE_C11
#define memory_free(arg) _Generic((arg), \
        double: memory_free64f, \
        int: memory_free32i, \
        double complex: memory_free64z, \
        char: memory_free8c, \
        bool: memory_free8l, \
        float: memory_free32f, \
        default: memory_free64f \
        ) (arg)
#endif
*/

//#ifdef ISCL_LONG_NAMES
#define memory_calloc__double(...)     memory_calloc64f(__VA_ARGS__)
#define memory_calloc__complex(...)    memory_calloc64z(__VA_ARGS__)
#define memory_calloc__float(...)      memory_calloc32f(__VA_ARGS__)
#define memory_calloc__int(...)        memory_calloc32i(__VA_ARGS__)
#define memory_calloc__bool(...)       memory_calloc8l(__VA_ARGS__)
#define memory_calloc__char(...)       memory_calloc8c(__VA_ARGS__)

#define memory_malloc__double(...)     memory_malloc64f(__VA_ARGS__)
#define memory_malloc__complex(...)    memory_malloc64z(__VA_ARGS__)
#define memory_malloc__float(...)      memory_malloc32f(__VA_ARGS__)
#define memory_malloc__int(...)        memory_malloc32i(__VA_ARGS__)
#define memory_malloc__bool(...)       memory_malloc8l(__VA_ARGS__)
#define memory_malloc__char(...)       memory_malloc8c(__VA_ARGS__)

#define memory_free__double(...)     memory_free64f(__VA_ARGS__)
#define memory_free__complex(...)    memory_free64z(__VA_ARGS__)
#define memory_free__float(...)      memory_free32f(__VA_ARGS__)
#define memory_free__int(...)        memory_free32i(__VA_ARGS__)
#define memory_free__bool(...)       memory_free8l(__VA_ARGS__)
#define memory_free__char(...)       memory_free8c(__VA_ARGS__)

//#define memory_free__double(...)       memory_free64f(__VA_ARGS__)

#define ISCL_memory_isAligned(...)       memory_isAligned(__VA_ARGS__)
#define ISCL_memory_calloc__double(...)  memory_calloc__double(__VA_ARGS__)
#define ISCL_memory_calloc__int(...)     memory_calloc__int(__VA_ARGS__)
#define ISCL_memory_calloc__complex(...) memory_calloc__complex(__VA_ARGS__)
#define ISCL_memory_calloc__bool(...)    memory_calloc__bool(__VA_ARGS__)
#define ISCL_memory_calloc__char(...)    memory_calloc__char(__VA_ARGS__)
#define ISCL_memory_calloc__float(...)   memory_calloc__float(__VA_ARGS__)
#define ISCL_memory_alloc__double(...)   memory_alloc__double(__VA_ARGS__)
#define ISCL_memory_alloc__int(...)      memory_alloc__int(__VA_ARGS__)
#define ISCL_memory_alloc__complex(...)  memory_alloc__complex(__VA_ARGS__)
#define ISCL_memory_alloc__char(...)     memory_alloc__char(__VA_ARGS__)
#define ISCL_memory_alloc__bool(...)     memory_alloc__bool(__VA_ARGS__)
#define ISCL_memory_alloc__float(...)    memory_alloc__float(__VA_ARGS__)
#define ISCL_memory_free__void(...)      memory_free__void(__VA_ARGS__)
#define ISCL_memory_free__double(...)    memory_free__double(__VA_ARGS__)
#define ISCL_memory_free__int(...)       memory_free__int(__VA_ARGS__)
#define ISCL_memory_free__complex(...)   memory_free__complex(__VA_ARGS__)
#define ISCL_memory_free__char(...)      memory_free__char(__VA_ARGS__)
#define ISCL_memory_free__bool(...)      memory_free__bool(__VA_ARGS__)
#define ISCL_memory_free__float(...)     memory_free__float(__VA_ARGS__)
#define ISCL_memory_free(...)            memory_free(__VA_ARGS__)
//#endif

#ifdef TESTING
int __memory_calloc_test();
#endif

#ifdef __cplusplus
}
#endif
#endif /* __MEMORY_MEMORY_H__ */
