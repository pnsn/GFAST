#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include "gfast.h"

int numpy_lstsq_test()
{
    const char *fcnm = "numpy_lstsq_test\0"; 
    double *gmat, *gmatRow, *b, *x, *svals, *R,
           RRef[9], sref[3], x1ref[3], diag[3], rcond;
    int m = 10;
    int n = 3;
    int nrhs = 1;
    int i, ierr, j, rank_out;
    //------------------------------------------------------------------------//
    //
    // Set the projectile least squares problem Parameter Estimation and
    // Inverse Problems - Aster et al. 
    gmat = (double *)calloc(m*n, sizeof(double));
    gmatRow = (double *)calloc(m*n, sizeof(double));
    b    = (double *)calloc(m, sizeof(double));
    x    = (double *)calloc(n, sizeof(double));
    for (i=0; i<m; i++){
        gmat[i] = 1.0;
        gmat[m+i] = (double) (i + 1);
        gmat[2*m+i] = -0.5*pow((double) (i + 1),2);
        gmatRow[3*i+0] = gmat[i]; 
        gmatRow[3*i+1] = gmat[m+i];
        gmatRow[3*i+2] = gmat[2*m+i];
    }
    b[0] = 109.4;
    b[1] = 187.5;
    b[2] = 267.5;
    b[3] = 331.9;
    b[4] = 386.1;
    b[5] = 428.4;
    b[6] = 452.2;
    b[7] = 498.1;
    b[8] = 512.3;
    b[9] = 513.0;
    // Reference solutions and singular values 
    x1ref[0] = 16.40833333333291;
    x1ref[1] = 96.97128787878796;
    x1ref[2] = 9.40833333333332;
    sref[0] = 81.864094582994952;
    sref[1] = 5.088461429273159;
    sref[2] = 0.792198453201135;
    RRef[0] = -3.16227766016838;
    RRef[1] = 0.0;
    RRef[2] = 0.0;
    RRef[3] = -17.3925271309261;
    RRef[4] = 9.08295106229248;
    RRef[5] = 0.0;
    RRef[6] = 60.8738449582413;
    RRef[7] = -49.9562308426086;
    RRef[8] = -11.4891252930761;
    diag[0] = 1.38333333333333219;
    diag[1] = 0.24128787878787863;
    diag[2] = 0.00757575757575757;
    // Solve the least-squares problem
    svals = NULL;
    ierr = numpy_lstsq(LAPACK_COL_MAJOR,
                       m, n, nrhs, gmat, b,
                       NULL, x, NULL, svals);
    if (ierr != 0){
        printf("%s: Failed test 1\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12){
            printf("%s: Failed to solve least squares problem\n", fcnm);
            return EXIT_FAILURE;
        }
    } 
    // Solve the least-squares problem with QR
    ierr = numpy_lstsq__qr(LAPACK_COL_MAJOR,
                           m, n, nrhs, gmat, b, x, NULL);
    if (ierr != 0){ 
        printf("%s: Failed test 1.1\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12){
            printf("%s: Failed to solve QR least squares problem\n", fcnm);
            return EXIT_FAILURE;
        }
    }
    // Get the R matrix
    R = (double *)calloc(n*n, sizeof(double));
    ierr = numpy_lstsq__qr(LAPACK_COL_MAJOR,
                           m, n, nrhs, gmat, b, x, R);
    if (ierr != 0){
        printf("%s: Failed test 1.1\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12){
            printf("%s: Failed to solve QR least squares problem 1.1\n", fcnm);
            return EXIT_FAILURE;
        }
        for (j=0; j<n; j++){
            if (fabs(R[j*n+i] - RRef[j*n+i]) > 1.e-12){
                printf("%s: R retrieval test 1.1 %f ", fcnm, R[j*n+i]);
                return EXIT_FAILURE;
            }
        }
    }
    // Invert the R matrix
    ierr = LAPACKE_dtrtri(LAPACK_COL_MAJOR, 'U', 'N', n, R, n);
    for (i=0; i<n; i++){
        if (fabs(diag[i] -  cblas_ddot(n, &R[i], n, &R[i], n)) > 1.e-12){
            printf("%s: Failed to compute diagonal 1.1 %f %f\n",
                   fcnm, diag[i], cblas_ddot(n, &R[n*i], 1, &R[n*i], 1));
            return EXIT_FAILURE;
        }
    }
    // Do it again with row major format
    ierr = numpy_lstsq__qr(LAPACK_ROW_MAJOR,
                           m, n, nrhs, gmatRow, b, x, R); 
    if (ierr != 0){ 
        printf("%s: Failed test 1.1\n", fcnm);
        return EXIT_FAILURE;
    }   
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12){
            printf("%s: Failed to solve QR least squares problem 1.2\n", fcnm);
            return EXIT_FAILURE;
        }
        for (j=0; j<n; j++){
            if (fabs(R[i*n+j] - RRef[j*n+i]) > 1.e-12){
                printf("%s: R retrieval test 1.2 %f ", fcnm, R[i*n+j]);
                return EXIT_FAILURE;
            }
        }
    }
    // Invert the R matrix
    ierr = LAPACKE_dtrtri(LAPACK_ROW_MAJOR, 'U', 'N', n, R, n);
    for (i=0; i<n; i++){
        
        if (fabs(diag[i] -  cblas_ddot(n, &R[n*i], 1, &R[n*i], 1)) > 1.e-12){
            printf("%s: Failed to compute diagonal\n", fcnm);
            return EXIT_FAILURE;
        } 
    }
    // Solve the least squares problem and get the matrix rank 
    ierr = numpy_lstsq(LAPACK_COL_MAJOR,
                       m, n, nrhs, gmat, b,
                       NULL, x, &rank_out, svals);
    if (rank_out != 3 || ierr != 0){
        printf("%s: Failed test 2\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12){
            printf("%s: Failed to solve least squares problem 2\n", fcnm);
            return EXIT_FAILURE;
        }
    }
    // Solve the least squares and get the rank and singular values
    svals = (double *)calloc(fmin(m, n), sizeof(double));
    rcond =-1.0;
    ierr = numpy_lstsq(LAPACK_COL_MAJOR,
                       m, n, nrhs, gmat, b,
                       &rcond, x, &rank_out, svals);
    if (rank_out != 3 || ierr != 0){ 
        printf("%s: Failed test 3\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<n; i++){
        if (fabs(x1ref[i] - x[i]) > 1.e-12 ||
            fabs(sref[i] - svals[i]) > 1.e-12){
            printf("%s: Failed to solve least squares problem 3\n", fcnm);
            return EXIT_FAILURE;
        }
    }
    // Free space
    if (R != NULL){free(R);}
    if (b != NULL){free(b);}
    if (gmat != NULL){free(gmat);}
    if (x != NULL){free(x);}
    if (svals != NULL){free(svals);}
    if (gmatRow != NULL){free(gmatRow);}
    printf("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
