#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <lapacke.h>
#include <cblas.h>
#include <omp.h>
#include "gfast.h"
#include "gfast_numpy.h"
#include "gfast_log.h"

/*! 
 * @brief Returns index corresponding to largest value in an array
 *
 * @param[in] n   number of points in array x
 * @param[in] x   array of which to find maximum [n]
 *
 * @result index of x which corresonds to largest value
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 *
 */
int numpy_argmax(int n, double *x) 
{
    const char *fcnm = "numpy_array\0";
    double val;
    int i, index;
    //------------------------------------------------------------------------//
    if (n < 1 || x == NULL){
        if (n < 1){log_errorF("%s: Warning no values in array x!", fcnm);}
        if (x == NULL){log_errorF("%s: Error x is NULL\n", fcnm);}
        return 0;
    }
    if (n == 1){return 0;}
    val = x[0];
    index = 0;
    for (i=1; i<n; i++){
        if (x[i] > val){
            index = i;
            val = x[i];
        }
    }
    return index;
}
//============================================================================//
/*! 
 * @brief Returns index corresponding to smallest value in a double array
 *
 * @param[in] n   number of points in array x
 * @param[in] x   array of which to find minimum [n]
 *
 * @result index of x which corresonds to smallest value
 *
 * @author Ben Baker, ISTI
 *
 */
int numpy_argmin(int n, double *x)
{
    const char *fcnm = "numpy_argmin\0";
    double val;
    int i, index;
    if (n < 1 || x == NULL){
        if (n < 1){log_errorF("%s: Warning no values in array x!", fcnm);}
        if (x == NULL){log_errorF("%s: Error x is NULL\n", fcnm);}
        return 0;
    }
    if (n == 1){return 0;}
    val = x[0];
    index = 0;
    for (i=1; i<n; i++){
        if (x[i] < val){
            index = i;
            val = x[i];
        }
    }
    return index;
}
//============================================================================//
/*! 
 * @brief Returns the minimum of a double array
 *
 * @param[in] n   number of points in array x
 * @param[in] x   array of which to find minimum [n]
 *
 * @result minimum value in x
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 *
 */
double numpy_min(int n, double *x)
{
    const char *fcnm = "numpy_min\0";
    double xmin;
    int i;
    if (n < 1 || x == NULL){
        if (n < 1){log_errorF("%s: Warning no values in array x!", fcnm);}
        if (x == NULL){log_errorF("%s: Error x is NULL\n", fcnm);}
        return 0.0;
    }
    if (n == 1){return x[0];}
    xmin = x[0];
    #pragma omp simd reduction(min:xmin)
    for (i=1; i<n; i++){
        xmin = fmin(xmin, x[i]);
    }
    return xmin;
}
//============================================================================//
/*!
 * @brief Returns the maximum of a double array while ignoring NaN's
 *
 * @param[in] n       number of points in array x
 * @param[in] x       array of which to find maximum [n]
 *
 * @param[out] iwarn  0 indicates success
 *                    1 indicates there are no values in x and the result is
 *                    NaN
 *                    2 indicates all values in x are NaN's
 *
 * @result maximum value of x where the max cannot be NaN
 *
 * @author Ben Baker (ISTI)
 *
 */ 
double numpy_nanmax(int n, double *x, int *iwarn)
{
    const char *fcnm = "numpy_nanmean\0";
    double xmax;
    int i;
    *iwarn = 0;
    if (n < 1 || x == NULL){
        if (n < 1){log_errorF("%s: Warning no values in array x!", fcnm);}
        if (x == NULL){log_errorF("%s: Error x is NULL\n", fcnm);}
        *iwarn = 1;
        return NAN;
    }
    xmax =-DBL_MAX;
    #pragma omp simd reduction(max:xmax)
    for (i=0; i<n; i++){
        if (!isnan(x[i])){xmax = fmax(xmax, x[i]);}
    }
    if (xmax ==-DBL_MAX){
        log_warnF("%s: All values are NaNs!\n", fcnm);
        *iwarn = 2;
        xmax = NAN;
    }
    return xmax;
}
//============================================================================//
/*!
 * @brief Computes the arithmetic mean of x where any NaN's are ignored
 * 
 * @param[in] n     number of elements in x
 * @param[in] x     array from which to compute mean (while ignoring NaNs)
 *
 * @param[out] iwarn   0 indicates success
 *                     1 indicates there are no entries in the array x
 *                       and thus there is a division by zero
 *                     2 indicates that all entries in x were NaN and
 *                       thus there is a division by zero
 *
 * @result arithmetic mean of array x where NaN's have been ignored.
 *         if iwarn is 1 or 2 then the result of this function is NaN
 *         as a division by zero was encountered and thus does not
 *         necessarily indicate that the average of all numbers in x
 *         is NaN.
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 *
 */
double numpy_nanmean(int n, double *x, int *iwarn)
{
    const char *fcnm = "numpy_nanmean\0";
    double xavg, xt; 
    int i, iavg, it, lnan;
    *iwarn = 0;
    // Size check
    if (n < 1){ 
        log_warnF("%s: Warning no elements; division by zero\n", fcnm);
        *iwarn = 1;
        xavg = NAN;
        return xavg;
    }   
    // Compute the average
    iavg = 0;
    xavg = 0.0;
    #pragma omp simd reduction(+:xavg,iavg)
    for (i=0; i<n; i++){
        xt = 0.0;
        it = 0;
        lnan = isnan(x[i]);
        if (!lnan){xt = x[i];}
        if (!lnan){it = 1;} 
        xavg = xavg + xt; 
        iavg = iavg + it; 
    }   
    if (iavg == 0){ 
        log_warnF("%s: Warning all elements of x are NaN's\n", fcnm);
        *iwarn = 2;
        xavg = NAN;
    }else{
        xavg = xavg/(double) iavg;
    }   
    return xavg;
}
//============================================================================//
/*!
 * @brief Solves the least squares problem Ax = b via the QR factorization
 *
 * @param[in] mtx_fmt    matrix format: LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR
 *                       for a column or row major ordering matrix respectively
 * @param[in] m          number of rows in matrix Aref (>= 1)
 * @param[in] n          number of columns in matrix Arev (>= 1)
 * @param[in] nrhs       number of right hand sides to solve (>= 1)
 * @param[in] Aref       matrix A in Ax = b.  this matrix is in column major 
 *                       order with leading dimension m [m x m]
 * @param[in] b          right hand side matrix.  this matrix is in column major
 *                       order with leading dimension m [m x nrhs]
 *
 * @param[out] x         on successful exit this is a matrix in column major
 *                       order with leading dimension n that holds the 
 *                       solution to the least squares problem Ax=b [n x nrhs]
 *
 * @param[inout] Q       if Q is NULL then orthogonal matrix will not be 
 *                       returned.
 *                       otherwise, the 
 * @param[inout] R       if R is NULL then the right matrix will not be return.
 *                       otherwise, the upper right [n x n] matrix R will be
 *                       returned in the specified mtx_fmt [n*n] 
 *
 * @result 0 indicates success
 *         1 indicates invalid parameter
 *         2 indicates an error encountered in Lapack
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 */
int numpy_lstsq__qr(int mtx_fmt,
                    int m, int n, int nrhs, double *Aref, double *b,
                    double *x, double *Q, double *R)
{
    const char *fcnm = "numpy_lstsq__qr\0";
    double *A, *bwork;
    int i, ierr, info, indx, j, jndx, k, lda, ldb, mn;
    const int incx = 1, incy = 1;
    //------------------------------------------------------------------------//
    //
    // Initialize
    A = NULL;
    bwork = NULL;
    // Check sizes
    ierr = 1;
    if (m < 1 || n < 1 || nrhs < 1){
        if (m < 1){log_errorF("%s: Error no rows in matrix\n", fcnm);}
        if (n < 1){log_errorF("%s: Error no columns in matrix\n", fcnm);}
        if (nrhs < 1){
            log_errorF("%s: Error there are no right hand sides\n", fcnm);
        }
        goto ERROR;
    }
    if (Aref == NULL || b == NULL || x == NULL){
        if (Aref == NULL){log_errorF("%s: Error input matrix NULL\n", fcnm);}
        if (b == NULL){log_errorF("%s: Error RHS is NULL\n", fcnm);}
        if (x == NULL){log_errorF("%s: Error solution x is NULL\n", fcnm);}
        goto ERROR;
    }
    // Set space for A
    ierr = 2;
    lda = m;
    mn = m*n;
    A = GFAST_memory_calloc__double(mn);
    if (A == NULL){
        log_errorF("%s: There was an error setting space\n", fcnm);
        goto ERROR;
    }
    // Set space for right hand side/solution
    ldb = fmax(m, n);
    bwork = GFAST_memory_calloc__double(ldb*nrhs);
    if (bwork == NULL){
        log_errorF("%s: There was an error setting workspace for bwork\n",
                   fcnm);
        goto ERROR;
    }
    // Copy matrix into column major format
    if (mtx_fmt == LAPACK_COL_MAJOR){
        cblas_dcopy(mn, Aref, incx, A, incy);
    }else{
        jndx = 0;
        #pragma omp simd collapse(2)
        for (j=0; j<n; j++){
            for (i=0; i<m; i++){
                indx = i*n + j;
                jndx = j*lda + i;
                A[jndx] = Aref[indx];
            }
        }
    }
    // Copy RHS
    for (k=0; k<nrhs; k++){
        indx = m*k;
        jndx = ldb*k;
        cblas_dcopy(m, &b[indx], incx, &bwork[jndx], incy);
    }
    // Solve the least squares problem
    info = LAPACKE_dgels(LAPACK_COL_MAJOR, 'N', m, n, nrhs, A, lda, bwork, ldb);
    if (info != 0){
        log_errorF("%s: Error solving the least squares problem\n", fcnm);
        if (info > 0){
            log_errorF("%s: Error rank %d deficient %d\n",
                       fcnm, info - fmin(m,n ), fmin(m, n));
        }
        goto ERROR;
    }
    // Copy the solution
    for (k=0; k<nrhs; k++){
        indx = k*ldb;
        jndx = k*n;
        cblas_dcopy(n, &bwork[indx], incx, &x[jndx], incy);
    }
    ierr = 0;
    // Get the right matrix
    if (R != NULL){
        if (mtx_fmt == LAPACK_COL_MAJOR){
            for (j=0; j<n; j++){
                for (i=0; i<n; i++){
                    R[n*j+i] = 0.0;
                    if (i >= j){R[n*j+i] = A[m*j+i];}
                }
            }
        }else{
            for (i=0; i<n; i++){
                for (j=0; j<n; j++){
                    R[n*i+j] = 0.0;
                    if (j >= i){R[n*i+j] = A[m*j+i];}
                }
            }
        }
    }
ERROR:;
    // Free space
    GFAST_memory_free__double(&bwork);
    GFAST_memory_free__double(&A);
    return ierr;
}
//============================================================================//
/*!
 * @brief Solves the least squares problem Ax = b via the singular value
 *        decomposition.
 *
 * @param[in] mtx_fmt    matrix format: LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR
 *                       for a column or row major ordering matrix respectively
 * @param[in] m          number of rows in matrix Aref (>= 1)
 * @param[in] n          number of columns in matrix Arev (>= 1)
 * @param[in] nrhs       number of right hand sides to solve (>= 1)
 * @param[in] Aref       matrix A in Ax = b.  this matrix is in column major 
 *                       order with leading dimension m [m x m]
 * @param[in] b          right hand side matrix.  this matrix is in column major
 *                       order with leading dimension m [m x nrhs]
 * @param[in] rcond_in   if NULL then the least squares problem will use
 *                       machine epsilon as a cutoff for `small' singular
 *                       values
 *                       if > 0 then this is the cutoff defining `small'
 *                       singular values in the least squares problem 
 *
 * @param[out] x         on successful exit this is a matrix in column major
 *                       order with leading dimension n that holds the 
 *                       solution to the least squares problem Ax=b [n x nrhs]
 * @param[out] rank_out  if not NULL then this will be the rank of the matrix
 *                       Aref.  if NULL this variable will not be set.
 * @param[out] svals     if not NULL then this will be the singular values of 
 *                       A and must be of size min(m,n).  if NULL this 
 *                       variable will not be set.
 *
 * @result 0 indicates success
 *         1 indicates invalid parameter
 *         2 indicates an error encountered in Lapack
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 *
 */ 
int numpy_lstsq(int mtx_fmt,
                int m, int n, int nrhs, double *Aref, double *b,
                double *rcond_in, double *x, int *rank_out, double *svals)
{
    const char *fcnm = "numpy_lstsq\0";
    double *A, *bwork, *s, rcond;
    int i, ierr, indx, jndx, info, j, k, lda, ldb, mn, rank;
    int incx = 1, incy = 1;
    //------------------------------------------------------------------------//
    //
    // Initialize
    A = NULL;
    s = NULL;
    bwork = NULL;
    // Check sizes
    ierr = 1;
    if (m < 1 || n < 1 || nrhs < 1){
        if (m < 1){log_errorF("%s: Error no rows in matrix\n", fcnm);}
        if (n < 1){log_errorF("%s: Error no columns in matrix\n", fcnm);}
        if (nrhs < 1){
            log_errorF("%s: Error there are no right hand sides\n", fcnm);
        }
        goto ERROR; 
    }
    if (Aref == NULL || b == NULL || x == NULL){
        if (Aref == NULL){log_errorF("%s: Error input matrix NULL\n", fcnm);}
        if (b == NULL){log_errorF("%s: Error RHS is NULL\n", fcnm);}
        if (x == NULL){log_errorF("%s: Error solution x is NULL\n", fcnm);}
        goto ERROR;
    }
    // Figure out the condition number
    rcond =-1.0;
    if (rcond_in != NULL){rcond = *rcond_in;}
    // Set space for A
    ierr = 2;
    lda = m;
    mn = m*n;
    A = GFAST_memory_calloc__double(m*n);
    if (A == NULL){
        log_errorF("%s: There was an error setting space\n", fcnm);
        goto ERROR; 
    }
    s = GFAST_memory_calloc__double(fmin(n, m));
    if (s == NULL){
        log_errorF("%s: Error setting space for singular values\n", fcnm);
        goto ERROR;
    }
    // Set space for right hand side/solution
    ldb = fmax(m, n);
    bwork = GFAST_memory_calloc__double(ldb*nrhs);
    if (bwork == NULL){
        log_errorF("%s: There was an error setting workspace for bwork\n",
                   fcnm);
        goto ERROR; 
    }
    // Copy matrix into column major format
    if (mtx_fmt == LAPACK_COL_MAJOR){
        cblas_dcopy(mn, Aref, incx, A, incy);
    }else{
        jndx = 0;
        #pragma omp simd collapse(2)
        for (j=0; j<n; j++){
            for (i=0; i<m; i++){
                indx = i*n + j;
                jndx = j*lda + i;
                A[jndx] = Aref[indx];
            }
        }
    }
    // Copy RHS
    for (k=0; k<nrhs; k++){
        indx = m*k;
        jndx = ldb*k;
        cblas_dcopy(m, &b[indx], incx, &bwork[jndx], incy);
    }
    // Compute the SVD
    info = LAPACKE_dgelsd(LAPACK_COL_MAJOR, m, n, nrhs, A, lda, bwork, ldb,
                          s, rcond, &rank);
    if (info != 0){
        log_errorF("%s: There was an error solving the least squares problem\n",
                   fcnm);
        goto ERROR;
    }
    // Copy the solution
    for (k=0; k<nrhs; k++){
        indx = k*ldb;
        jndx = k*n;
        cblas_dcopy(n, &bwork[indx], incx, &x[jndx], incy);
    }
    // Do you want the rank?
    if (rank_out != NULL){*rank_out = rank;}
    // Do you want the singular values?
    if (svals != NULL){
        mn = fmin(m, n);
        cblas_dcopy(mn, s, incx, svals, incy);
    }
    ierr = 0;
ERROR:;
    // Free space
    GFAST_memory_free__double(&bwork);
    GFAST_memory_free__double(&s);
    GFAST_memory_free__double(&A);
    return ierr;
}
//============================================================================//
/*!
 * @brief Rotates horizontal components of a seismogram. 
 *        The north and east components of a seismogram will be rotated
 *        into radial and transversal components.  The angle is given as
 *        the back-azimuth, that is defined as the angle measured between
 *        the vector pointing from the station to the source, and the 
 *        vector pointing from the station from the station to the north.
 *
 * @reference https://docs.obspy.org/_modules/obspy/signal/rotate.html
 * @reference https://service.iris.edu/irisws/rotation/docs/1/help/
 *
 * @param[in] np   number of data points 
 * @param[in] e    data on east component [np]
 * @param[in] n    data on north component [np]
 * @param[in] ba   the source to station back-azimuth in degrees measured
 *                 positive counter-clockwise from north
 *
 * @param[out] r   data rotated onto the radial component [np]
 * @param[out] t   data rotated onto the east component [np]
 *
 * @result 0 indicates success
 *         1 indicates there is no data to rotate
 *
 * @author Ben Baker (benbaker@isti.com)
 * @date March 2016
 *
 */
int obspy_rotate_NE2RT(int np, double *e, double *n, double ba,
                       double *r, double *t)
{
    const char *fcnm = "obspy_rotate_NE2RT\0";
    const double pi180 = M_PI/180.0;
    double cosaz, sinaz, et, nt;
    int i;
    if (np < 1){
        log_warnF("%s: Warning there are no points %d\n", fcnm, np);
        return 1;
    }
    if (ba < 0.0 || ba > 360.0){
        log_warnF("%s: Warning back-azimuth=%f should be between [0,360]\n",
                  fcnm, ba);
    }
    cosaz = cos((ba + 180.0)*pi180);
    sinaz = sin((ba + 180.0)*pi180);
    for (i=0; i<np; i++){
        et = e[i];
        nt = n[i];
        r[i] = nt*cosaz + et*sinaz;
        t[i] =-nt*sinaz + et*cosaz;
    }
    return 0;
}
