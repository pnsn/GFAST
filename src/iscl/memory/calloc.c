#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#endif
#endif
#include "iscl/array/array.h"

/*!
 * @defgroup memory_calloc calloc
 * @brief Allocates memory and sets all entries to 0.
 * @ingroup memory
 */
//============================================================================//
/*!
 * @brief Allocates a double array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         double array of dimension [n] with all elements of array set
 *         to zero.
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
double *memory_calloc64f(const int n)
{
    double *x = NULL;
    enum isclError_enum ierr;
    if (n < 1)
    {
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    x = array_zeros64f(n, &ierr);
    if (ierr != ISCL_SUCCESS || x == NULL)
    {
        isclPrintError("Error creating array"); 
        memory_free64f(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates an integer array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         integer array of dimension [n] with all elements of array set
 *         to zero.
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
int *memory_calloc32i(const int n)
{
    int *x = NULL;
    enum isclError_enum ierr;
    if (n < 1)
    {
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    x = array_zeros32i(n, &ierr);
    if (ierr != ISCL_SUCCESS || x == NULL)
    {   
        isclPrintError("Error creating array");
        memory_free32i(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocate a complex array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         complex array of dimension [n] with all elements of array set
 *         to zero.
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
double complex *memory_calloc64z(const int n)
{
    double complex *x = NULL;
    enum isclError_enum ierr;
    if (n < 1)
    {
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    x = array_zeros64z(n, &ierr);
    if (ierr != ISCL_SUCCESS || x == NULL)
    {   
        isclPrintError("Error creating array");
        memory_free64z(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a boolean array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         boolean array of dimension [n] with all elements of array set
 *         to false.
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
bool *memory_calloc8l(const int n)
{
    bool *x = NULL; 
    enum isclError_enum ierr;
    if (n < 1)
    {
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    x = array_zeros8l(n, &ierr);
    if (ierr != 0 || x == NULL)
    {   
        isclPrintError("Error creating array");
        memory_free8l(&x);
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a char array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         char array of dimension [n] with all elements of array set
 *         to the null terminator. 
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
char *memory_calloc8c(const int n)
{
    char *x = NULL;
    enum isclError_enum ierr;
    if (n < 1)
    {
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    x = array_zeros8c(n, &ierr);
    if (ierr != ISCL_SUCCESS || x == NULL)
    {
        isclPrintError("Error creating array");
        memory_free8c(&x);
    }
    return x;
}
/*
    const char *fcnm = "memory_calloc8c\0";
    char *x = NULL; 
    size_t nbytes;
    if (n < 1)
    {   
        isclPrintError("Error invalid size %d\n", n);
        return x;
    }
    nbytes = (size_t) (n + ISCL_MEMORY_ALIGN - n%ISCL_MEMORY_ALIGN)
                     *sizeof(char);
#ifdef ISCL_USE_MKL_MEMORY
    x = (char *) mkl_malloc(nbytes, ISCL_MEMORY_ALIGN);
    if (x == NULL)
    {   
        isclPrintError("Error invalid size %d\n", n);
    }   
#else
  #ifdef ISCL_USE_C11
    x = (char *) aligned_alloc(ISCL_MEMORY_ALIGN, nbytes);
    if (x == NULL)
    {   
        isclPrintError("Error invalid size %d\n", n);
    }   
  #else
    enum isclError_enum ierr;
    ierr = posix_memalign( (void **) &x, ISCL_MEMORY_ALIGN, nbytes);
    if (ierr != 0)
    {   
        isclPrintError("Error invalid size %d\n", n);
        x = NULL;
        ierr = 1;
    }
  #endif
#endif
    memset(x, 0, nbytes);
*/
/*
    x = array_zeros8c(n, &ierr);
    if (ierr != 0 || x == NULL)
    {   
        isclPrintError("Error invalid size %d\n", n);
        memory_free8c(&x);
    }
*/
/*
    return x;
}
*/
//============================================================================//
/*!
 * @brief Allocates a float array with 64 byte alignment.
 *
 * @param[in] n      Size of array to allocate.
 *
 * @result On successful exit this is the 64 byte memory aligned
 *         floatr array of dimension [n] with all elements of array set
 *         to zero.
 *
 * @ingroup memory_calloc
 *
 * @author Ben Baker, ISTI
 *
 */
float *memory_calloc32f(const int n)
{
    float *x = NULL;
    enum isclError_enum ierr;
    if (n < 1)
    {   
        isclPrintError("Error invalid size: %d", n);
        return x;
    }
    x = array_zeros32f(n, &ierr);
    if (ierr != ISCL_SUCCESS)
    {
        isclPrintError("Error creating array");
        memory_free32f(&x);
    } 
    return x;
}
