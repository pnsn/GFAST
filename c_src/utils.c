#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_numpy.h"

extern void dcopy_(int *n, double *x, int *incx, double *y, int *incy);
extern void dgelsd_(int *M, int *N, int *NRHS, double *A, int *LDA,
                    double *B, int *LDB, double *S, double *RCOND,
                    int *RANK, double *work, int *lwork, int *iwork, int *info);

/*! 
 * @brief Returns index corresponding to largest value in an array
 *
 * @param[in] n   number of points in array x
 * @param[in] x   array of which to find maximum [n]
 *
 * @result index of x which corresonds to largest value
 *
 * @author Ben Baker, ISTI
 *
 */
inline int numpy_argmax(int n, double *x) 
{
    const char *fcnm = "numpy_argmax\0";
    double cmax;
    int imax, i;
    if (n < 1){ 
        printf("%s: Warning no values in array x!",fcnm);
        return 0;
    }
    cmax = x[0];
    imax = 0;
    for (i=1; i<n; i++){
        if (x[i] > cmax){
            cmax = x[i];
            imax = i;
        }    
    }    
    return imax;
}
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
 * @author Ben Baker, ISTI
 * @date January 2016
 *
 */
inline double numpy_nanmean(int n, double *x, int *iwarn)
{
    const char *fcnm = "numpy_nanmean\0";
    double xavg;
    int i, iavg;
    *iwarn = 0;
    // Size check
    if (n < 1){
        printf("%s: Warning no elements; division by zero\n", fcnm);
        *iwarn = 1;
        xavg = NAN;
        return xavg;
    }
    // Compute the average
    iavg = 0;
    xavg = 0.0;
    for (i=0; i<n; i++){
        if (isnan(x[i]) == 0){
            xavg = xavg + x[i];
            iavg = iavg + 1;
        }
    }
    if (iavg == 0){
        printf("%s: Warning all elements of x are NaN's\n", fcnm);
        *iwarn = 2;
        xavg = NAN; 
    }else{
        xavg = xavg/(double) iavg;
    }
    return xavg;
}
//============================================================================//
/*!
 * @brief Solves the least squares problem Ax = b via the singular value
 *        decomposition.
 *
 * @param[in] m          number of rows in matrix Aref (>= 1)
 * @param[in] n          number of columns in matrix Arev (>= 1)
 * @param[in] nrhs       number of right hand sides to solve (>= 1)
 * @param[in] Aref       matrix A in Ax = b.  this matrix is in column major 
 *                       ord with leading dimension m [m x m]
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
 * @author Ben Baker, ISTI
 * @date January 2016
 *
 */ 
int numpy_lstsq(int m, int n, int nrhs, double *Aref, double *b,
                double *rcond_in, double *x, int *rank_out, double *svals)
{
    const char *fcnm = "numpy_lstsq\0";
    double *A, *bwork, *work, *s, rcond, work8;
    int *iwork, ierr, indx, jndx, info, iwork4, k,
        lda, ldb, liwork, lwork, mn, rank;
    int incx = 1, incy = 1;
    // Initialize
    A = NULL;
    s = NULL;
    work = NULL;
    bwork = NULL;
    iwork = NULL;
    // Check sizes
    ierr = 1;
    if (m < 1){
        printf("%s: Error no rows in matrix\n", fcnm);
        goto ERROR; 
    }
    if (n < 1){
        printf("%s: Error no columns in matrix\n", fcnm);
        goto ERROR; 
    }
    if (nrhs < 1){
        printf("%s: Error there are no right hand sides\n", fcnm);
        goto ERROR; 
    }
    // Figure out the condition number
    rcond =-1.0;
    if (rcond_in != NULL){rcond = *rcond_in;}
    // Set space for A
    ierr = 2;
    lda = m;
    mn = lda*n;
    A = (double *)calloc(mn, sizeof(double));
    if (A == NULL){
        printf("%s: There was an error setting space\n", fcnm);
        goto ERROR; 
    }
    s = (double *)calloc(fmin(n, n), sizeof(double));
    if (s == NULL){
        printf("%s: ERror setting space for singular values\n", fcnm);
        goto ERROR;
    }
    // Set space for right hand side/solution
    ldb = fmax(m,n);
    bwork = (double *)calloc(ldb*nrhs, sizeof(double));
    if (bwork == NULL){
        printf("%s: There was an error setting workspace for bwork\n", fcnm);
        goto ERROR; 
    }
    // Copy RHS
    dcopy_(&mn, Aref, &incx, A, &incy);
    for (k=0; k<nrhs; k++){
        indx = m*k;
        jndx = ldb*k;
        dcopy_(&m, &b[indx], &incx, &bwork[indx], &incy);
    }
    // Space inquiry and then set workspace
    lwork =-1;
    dgelsd_(&m, &n, &nrhs, A, &lda, bwork, &ldb, s, &rcond, &rank,
            &work8, &lwork, &iwork4, &info);
    if (info != 0){
        printf("%s: There was an error in the space query!\n", fcnm);
        return -1;
    }
    lwork = (int) work8;
    liwork = iwork4;
    work = (double *)calloc(lwork, sizeof(double));
    iwork = (int *)calloc(liwork, sizeof(int));
    if (work == NULL){
        printf("%s: Error setting space for work\n", fcnm);
        return 2;
    }
    if (iwork == NULL){
        printf("%s: Error setting space for iwork\n", fcnm);
        return 2;
    }
    // Compute the SVD
    dgelsd_(&m, &n, &nrhs, A, &lda, bwork, &ldb, s, &rcond, &rank,
            work, &lwork, iwork, &info);
    if (info != 0){
        printf("%s: There was an error solving the least squares problem\n",
               fcnm);
        goto ERROR;
    }
    // Copy the solution
    for (k=0; k<nrhs; k++){
        indx = k*ldb;
        jndx = k*n;
        dcopy_(&m, &bwork[indx], &incx, &x[jndx], &incy);
    }
    // Do you want the rank?
    if (rank_out != NULL){*rank_out = rank;}
    // Do you want the singular values?
    if (svals != NULL){
        mn = fmin(m, n);
        dcopy_(&mn, s, &incx, svals, &incy);
    }
    // Free space
ERROR:;
    if (iwork != NULL){free(iwork);}
    if (work  != NULL){free(work);}
    if (bwork != NULL){free(bwork);}
    if (s     != NULL){free(s);}
    if (A     != NULL){free(A);}
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
 * @ref https://docs.obspy.org/_modules/obspy/signal/rotate.html
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
 * @author Ben Baker, ISTI
 * @date January 2016
 *
 */
inline void obspy_rotate_NE2RT(int np, double *e, double *n, double ba,
                               double *r, double *t)
{
    const char *fcnm = "obspy_rotate_NE2RT\0";
    const double pi180 = M_PI/180.0;
    double cosbaz, sinbaz, et, nt;
    int i;
    if (np < 1){
        printf("%s: Warning there are no points %d\n", fcnm, np);
        return;
    }
    if (ba < 0.0 || ba > 360.0){
        printf("%s: Warning back-azimuth=%f should be between [0,360]\n",
               fcnm, ba);
    }
    cosbaz = cos((ba + 180.0)*pi180);
    sinbaz = sin((ba + 180.0)*pi180);
    for (i=0; i<np; i++){
        et = e[i];
        nt = n[i];
        r[i] = et*sinbaz + nt*cosbaz;
        t[i] = et*cosbaz - nt*sinbaz;
    }
    return;
}
