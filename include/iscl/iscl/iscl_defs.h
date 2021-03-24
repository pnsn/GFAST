#ifndef ISCL_ISCL_DEFS_H__
#define ISCL_ISCL_DEFS_H__ 1

#include <stdbool.h>
#include <math.h>
// #include "iscl/config.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __restrict
#define __restrict __restrict__
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.7182818284590452354
#endif
#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#define CACHE_LINE_SIZE 64 /*!< Default memory alignment */
#define ISCL_MEMORY_ALIGN CACHE_LINE_SIZE

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

//#ifdef __cplusplus
//#include <complex>
//typedef std::complex< double > cdouble;
//#else
#include <complex.h>
//typedef double complex cdouble;
//#endif

#ifndef DOUBLEI
#define DOUBLEI (__extension__ 1.0i)
#endif
#ifndef DCMPLX
#define DCMPLX(r,i) ((double) (r) + (double) (i)*DOUBLEI)
#endif

#endif /* _iscl_iscl_defs_h__ */
