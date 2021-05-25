#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/iscl/iscl.h"
#include "iscl/memory/memory.h"
#ifdef ISCL_USE_MKL_MEMORY
#include "mkl.h"
#else
#ifdef ISCL_USE_C11
#include <stdalign.h>
#ifndef aligned_alloc
void *aligned_alloc( size_t alignment, size_t size );
#endif
#else
#include <malloc.h>
#endif
#endif

static int computePadding64z(const int n);
static int computePadding64f(const int n);
static int computePadding32f(const int n);
static int computePadding8c(const int n);
static int computePadding8l(const int n);

/*!
 * @defgroup memory_malloc malloc
 * @brief Allocates memory.
 * @ingroup memory
 */
/*!
 * @brief Allocates a double array 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         double array of length n.
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
//enum isclError_enum memory_malloc64f(const int n, double **x)
double *memory_malloc64f(const int n, enum isclError_enum *ierr)
{
    double *x;
    int nnew;
    size_t nbytes;
    //enum isclError_enum ierr;
    *ierr = ISCL_SUCCESS;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x; //ISCL_INVALID_INPUT;
    }
    //nbytes = (size_t) (MAX(n,8))*sizeof(double); 
    //nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
    //                 *sizeof(double);
    nnew = n + computePadding64f(n);
    nbytes = (size_t) nnew*sizeof(double);
#ifdef ISCL_USE_MKL_MEMORY
    x = (double *) mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }
#else
  #ifdef ISCL_USE_C11
    x = (double *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }
  #endif
#endif
    return x; //ierr;
}
//============================================================================//
/*!
 * @brief Allocates an integer array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         integer array of dimension [n].
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
int *memory_malloc32i(const int n, enum isclError_enum *ierr)
{
    size_t nbytes;
    int *x;
    *ierr = ISCL_SUCCESS;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x;
    }
    //nbytes = (size_t) (MAX(n,16))*sizeof(int);
    nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
                     *sizeof(int);
#ifdef ISCL_USE_MKL_MEMORY
    x = (int *)mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }
#else
  #ifdef ISCL_USE_C11
    x = (int *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }   
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }
  #endif
#endif
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a float array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         float array of dimension [n].
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
float *memory_malloc32f(const int n, enum isclError_enum *ierr)
{
    float *x;
    int nnew;
    size_t nbytes;
    *ierr = ISCL_SUCCESS;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x;
    }   
    //nbytes = (size_t) (MAX(n,16))*sizeof(float);
    //nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
    //                 *sizeof(float);
    nnew = n + computePadding32f(n);
    nbytes = (size_t) nnew*sizeof(float);
#ifdef ISCL_USE_MKL_MEMORY
    x = (float *)mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }   
#else
  #ifdef ISCL_USE_C11
    x = (float *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }   
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE; 
        return x;
    } 
  #endif
#endif
    return x;
}
//============================================================================//
/*!
 * @brief Allocate a complex array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * result On successful exit this is the 64 byte memory aligned
 *        double complex array of dimension [n].
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
double complex *memory_malloc64z(const int n, enum isclError_enum *ierr)
{
    double complex *x;
    int nnew;
    size_t nbytes;
    *ierr = ISCL_SUCCESS;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x;
    }
    //nbytes = (size_t) (MAX(n,8))*sizeof(double complex);
    //nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
    //                 *sizeof(double complex);
    nnew = n + computePadding64z(n);
    nbytes = (size_t) nnew*sizeof(double complex);
#ifdef ISCL_USE_MKL_MEMORY
    x = (double complex *) mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }
#else
  #ifdef ISCL_USE_C11
    x = (double complex *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }   
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }
  #endif
#endif
    return x;
}
//============================================================================//
/*!
 * @brief Allocates an boolean array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         boolean array of dimension [n].
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
bool *memory_malloc8l(const int n, enum isclError_enum *ierr)
{
    bool *x;
    int nnew;
    size_t nbytes;
    *ierr = ISCL_SUCCESS;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x;
    }
    //nbytes = (size_t) (MAX(n,64))*sizeof(bool);
    //nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
    //                 *sizeof(bool);
    nnew = n + computePadding8l(n);
    nbytes = (size_t) nnew*sizeof(bool);
#ifdef ISCL_USE_MKL_MEMORY
    x = (bool *)mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }
#else
  #ifdef ISCL_USE_C11
    x = (bool *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }   
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }
  #endif
#endif
/*
    // There's a small price to pay for padding - some blas/lapack routines
    // will hit unitialized variables
    if (nnew > n)
    {
        xtemp = *x;
        for (i=n; i<nnew; i++){xtemp[i] = false;}
        xtemp = NULL;
    }
*/
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a char array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @param[out] ierr  ISCL error code where ISCL success indicates success.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         char array of dimension [n].
 *
 * @ingroup memory_malloc
 *
 * @author Ben Baker, ISTI
 *
 */
char *memory_malloc8c(const int n, enum isclError_enum *ierr)
{
    char *x;
    int nnew;
    size_t nbytes;
    x = NULL;
    if (n < 1)
    {
        isclPrintError("Invalid size: %d", n);
        *ierr = ISCL_INVALID_INPUT;
        return x;
    }
    //nbytes = (size_t) (MAX(n,64))*sizeof(char);
    //nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
    //                 *sizeof(char);
    nnew = n + computePadding8c(n);
    nbytes = (size_t) nnew*sizeof(char);
#ifdef ISCL_USE_MKL_MEMORY
    x = (char *)mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_MKL_ALLOC_FAILURE;
    }
#else
  #ifdef ISCL_USE_C11
    x = (char *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
    }   
  #else
    *ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (*ierr != 0)
    {
        isclPrintError("Error allocating array");
        *ierr = ISCL_ALLOC_FAILURE;
        return x;
    }
  #endif
#endif
/*
    // There's a small price to pay for padding - some blas/lapack routines
    // will hit unitialized variables
    if (nnew > n)
    {
        xtemp = *x;
        for (i=n; i<nnew; i++){xtemp[i] = '\0';}
        xtemp = NULL;
    }
*/
    return x;
}

static int computePadding8l(const int n)
{
    size_t mod, pad;
    int ipad;
    pad = 0;
    mod = ((size_t) n*sizeof(bool))%ISCL_MEMORY_ALIGN;
    if (mod != 0)
    {
        pad = (ISCL_MEMORY_ALIGN - mod)/sizeof(bool);
    }
    ipad = (int) pad;
    return ipad;
}

static int computePadding8c(const int n)
{
    size_t mod, pad;
    int ipad;
    pad = 0;
    mod = ((size_t) n*sizeof(char))%ISCL_MEMORY_ALIGN;
    if (mod != 0)
    {        
        pad = (ISCL_MEMORY_ALIGN - mod)/sizeof(char);
    }   
    ipad = (int) pad;
    return ipad;
}

static int computePadding64f(const int n)
{
    size_t mod, pad;
    int ipad;
    pad = 0;
    mod = ((size_t) n*sizeof(double))%ISCL_MEMORY_ALIGN;
    if (mod != 0)
    {    
        pad = (ISCL_MEMORY_ALIGN - mod)/sizeof(double);
    }
    ipad = (int) pad;
    return ipad;
}

static int computePadding32f(const int n)
{
    size_t mod, pad;
    int ipad;
    pad = 0;
    mod = ((size_t) n*sizeof(float))%ISCL_MEMORY_ALIGN;
    if (mod != 0)
    {
        pad = (ISCL_MEMORY_ALIGN - mod)/sizeof(float);
    }
    ipad = (int) pad;
    return ipad;
}

static int computePadding64z(const int n)
{
    size_t mod, pad;
    int ipad;
    pad = 0;
    mod = ((size_t) n*sizeof(double complex))%ISCL_MEMORY_ALIGN;
    if (mod != 0)
    {    
        pad = (ISCL_MEMORY_ALIGN - mod)/sizeof(double complex);
    }   
    ipad = (int) pad;
    return ipad;
}
