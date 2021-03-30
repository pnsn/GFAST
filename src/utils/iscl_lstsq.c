#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/linalg/linalg.h"
#ifdef ISCL_USE_INTEL
#include <mkl_lapacke.h>
#include <mkl_cblas.h>
#else
#include <lapacke.h>
#include <cblas.h>
#endif
#include "iscl/memory/memory.h"
/*!
 * @brief Solves the least squares problem Ax = b via the QR factorization
 *
 * @param[in] mtx_fmt    matrix format: LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR
 *                       for a column or row major ordering matrix respectively
 * @param[in] m          number of rows in matrix Aref (>= 1)
 * @param[in] n          number of columns in matrix Arev (>= 1)
 * @param[in] nrhs       number of right hand sides to solve (>= 1)
 * @param[in] lwantR     if true then the right R matrix is desired.
 *                       otherwise, R, can be NULL.
 * @param[in] Aref       matrix A in Ax = b.  this matrix is in column major 
 *                       order with leading dimension m [m x m]
 * @param[in] b          right hand side matrix.  this matrix is in column major
 *                       order with leading dimension m [m x nrhs]
 *
 * @param[out] x         on successful exit this is a matrix in column major
 *                       order with leading dimension n that holds the 
 *                       solution to the least squares problem Ax=b [n x nrhs]
 *
 * @param[out] R         if R lwantR is false then the right matrix will not
 *                       be returned.
 *                       otherwise, the upper right [n x n] matrix R will be
 *                       returned in the specified mtx_fmt [n*n] 
 *
 * @result 0 indicates success
 *         1 indicates invalid parameter
 *         2 indicates an error encountered in Lapack
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 * @date March 2016
 *
 */
enum isclError_enum
    linalg_lstsq_qr64f_work(const int mtx_fmt,
                            const int m, const int n, const int nrhs,
                            const bool lwantR,
                            const double *__restrict__ Aref,
                            const double *__restrict__ b,
                            double *__restrict__ x,
                            double *__restrict__ R)
{
    double *A, *bwork;
    int i, info, indx, j, jndx, k, lda, ldb, mn;
    enum isclError_enum ierr;
    const int incx = 1, incy = 1;
    //------------------------------------------------------------------------//
    //
    // Initialize
    A = NULL;
    bwork = NULL;
    // Check sizes
    ierr = ISCL_SUCCESS;
    if (m < 1 || n < 1 || nrhs < 1)
    {
        if (m < 1){isclPrintError("Error no rows=%d in matrix", m);}
        if (n < 1){isclPrintError("Error no columns=%d in matrix", n);}
        if (nrhs < 1)
        {
            isclPrintError("Error there are no right hand sides=%d", nrhs);
        }
        return ISCL_INVALID_INPUT;
    }
    isclReturnNullPointerError("Aref", Aref); 
    isclReturnNullPointerError("b", b);
    isclReturnNullPointerError("x", x);
    if (lwantR && R == NULL)
    {
        isclReturnNullPointerError("R", R);
    }
    // Set space for A
    ierr = ISCL_SUCCESS;
    lda = m;
    mn = m*n;
    A = memory_calloc64f(mn);
    if (A == NULL)
    {
        isclPrintError("There was an error setting space for A");
        ierr = ISCL_ALLOC_FAILURE;
        goto ERROR;
    }
    // Set space for right hand side/solution
    ldb = MAX(m, n);
    bwork = memory_calloc64f(ldb*nrhs);
    if (bwork == NULL)
    {
        isclPrintError("There was an error setting workspace for bwork");
        ierr = ISCL_ALLOC_FAILURE;
        goto ERROR;
    }
    // Copy matrix into column major format
    if (mtx_fmt == LAPACK_COL_MAJOR)
    {
        cblas_dcopy(mn, Aref, incx, A, incy);
    }
    else
    {
        jndx = 0;
        #pragma omp simd collapse(2)
        for (j=0; j<n; j++)
        {
            for (i=0; i<m; i++)
            {
                indx = i*n + j;
                jndx = j*lda + i;
                A[jndx] = Aref[indx];
            }
        }
    }
    // Copy RHS
    for (k=0; k<nrhs; k++)
    {
        indx = m*k;
        jndx = ldb*k;
        cblas_dcopy(m, &b[indx], incx, &bwork[jndx], incy);
    }
    // Solve the least squares problem
    info = LAPACKE_dgels(LAPACK_COL_MAJOR, 'N', m, n, nrhs,
                         A, lda, bwork, ldb);
    if (info != 0)
    {
        ierr = ISCL_ALGORITHM_FAILURE;
        isclPrintError("Error solving the least squares problem");
        if (info > 0)
        {
            isclPrintError("Error rank %d deficient %d\n",
                           info - MIN(m, n), MIN(m, n));
            ierr = ISCL_LAPACK_FAILURE;
        }
        goto ERROR;
    }
    // Copy the solution
    for (k=0; k<nrhs; k++)
    {
        indx = k*ldb;
        jndx = k*n;
        cblas_dcopy(n, &bwork[indx], incx, &x[jndx], incy);
    }
    ierr = ISCL_SUCCESS;
    // Get the right matrix
    if (R != NULL)
    {
        if (mtx_fmt == LAPACK_COL_MAJOR)
        {
            #pragma omp simd collapse(2)
            for (j=0; j<n; j++)
            {
                for (i=0; i<n; i++)
                {
                    R[n*j+i] = 0.0;
                    if (i <= j){R[n*j+i] = A[lda*j+i];}
                }
            }
        }
        else
        {
            #pragma omp simd collapse(2)
            for (i=0; i<n; i++)
            {
                for (j=0; j<n; j++)
                {
                    R[n*i+j] = 0.0;
                    if (i <= j){R[n*i+j] = A[lda*j+i];}
                }
            }
        }
    }
ERROR:;
    // Free space
    memory_free64f(&bwork);
    memory_free64f(&A);
    return ierr;
}
