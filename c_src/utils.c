#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern void dcopy_(int *n, double *x, int *incx, double *y, int *incy);
extern void dgelsd_(int *M, int *N, int *NRHS, double *A, int *LDA,
                    double *B, int *LDB, double *S, double *RCOND,
                    int *RANK, double *work, int *lwork, int *iwork, int *info);
/*!
 * @brief Computes the arithmetic mean of x where any NaN's are ignored
 * 
 * @param[in] n     number of elements in x
 * @param[in] x     array from which to compute mean (while ignoring NaNs)
 *
 * @result arithmetic mean of array x where NaN's have been ignored
 *
 * @author Ben Baker, ISTI
 *
 */
double numpy_nanmean(int n, double *x)
{
    const char *fcnm = "numpy_nanmean\0";
    double xavg;
    int i, iavg;
    iavg = 0;
    xavg = 0.0;
    for (i=0; i<n; i++){
        if (isnan(x[i]) == 0){
            xavg = xavg + x[i];
            iavg = iavg + 1;
        }
     }
    if (iavg == 0){
        if (n > 0){
            printf("%s: Warning all elements of x are NaN's\n", fcnm);
        }else{
            printf("%s: Warning division by zero\n", fcnm);
        }
        xavg = NAN; 
    }else{
        xavg = xavg/(double) iavg;
    }
    return xavg;
}
//============================================================================//

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
 *        https://docs.obspy.org/_modules/obspy/signal/rotate.html
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
 */
inline void rotate_NE_RT(int np, double *e, double *n, double ba,
                         double *r, double *t)
{
    const char *fcnm = "rotate_NE_RT\0";
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
