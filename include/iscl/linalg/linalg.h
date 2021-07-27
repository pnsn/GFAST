#ifndef LINALG_LINALG_H__
#define LINALG_LINALG_H__ 1
#include <stdbool.h>
#include "iscl/iscl/iscl.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef ISCL_USE_INTEL
enum isclError_enum
     linalg_lsqrCSR64f_work(const int m,
                            const int n,
                            const int nnz, 
                            const double Amat[],
                            const int csrRowPtr[],
                            const int csrColPtr[],
                            const double y[], 
                            const double damp,
                            const double atol, const double btol,
                            const double conlim,
                            const int itnlim,
                            double x[], 
                            double *anorm, double *acond,
                            double *rnorm, double *arnorm,
                            double *xnorm, double se[]);
#endif
/* iteratively reweighted least squares */
enum isclError_enum
    linalg_irls64f_work(const int m, const int n,  
                        const int maxit,
                        const double p,
                        const double eps, const double tol,
                        const double *__restrict__ Amat,
                        const double *__restrict__ y,  
                        double *__restrict__  x,  
                        double *__restrict__ wts);
/* Solve Ax=b in least squares sense with QR factorization */
enum isclError_enum
    linalg_lstsq_qr64f_work(const int mtx_fmt,
                            const int m, const int n, const int nrhs,
                            const bool lwantR,
                            const double *__restrict__ Aref,
                            const double *__restrict__ b,
                            double *__restrict__ x,
                            double *__restrict__ R);

/* Compute LU factorization of matrix */
enum isclError_enum linalg_lu64f_work(const int mtx_fmt,
                                      const int m, const int n,
                                      const double *__restrict__ A,
                                      double *__restrict__ lu,
                                      int *__restrict__ ipiv);
/* Return the L and U factors from the LU decomposition */
enum isclError_enum linalg_getLU64f_work(const int mtx_fmt,
                                         const int m, const int n,
                                         const double *__restrict__ lu,
                                         const int *__restrict__ ipiv,
                                         double *__restrict__ l,
                                         double *__restrict__ u);
/* Compute outer product of two arrays */
enum isclError_enum
    linalg_outerProduct64f_work(const int mtxFormat,
                                const int na, const double *__restrict__ a,
                                const int nb, const double *__restrict__ b,
                                const int ldx, double *__restrict__ X);
enum isclError_enum
    linalg_outerProduct32f_work(const int mtxFormat,
                                const int na, const float *__restrict__ a,
                                const int nb, const float *__restrict__ b,
                                const int ldx, float *__restrict__ X);
/* Transposition */
enum isclError_enum
    linalg_transpose64f_work(const int mtx_fmt,
                             const int m, const int n,
                             const int lda, double *__restrict__ A,
                             const int ldat, double *__restrict__ At);

#define ISCL_linalg_lstsq_qr64f_work(...)       \
             linalg_lstsq_qr64f_work(__VA_ARGS__)

#define ISCL_linalg_lu64f_work(...)       \
             linalg_lu64f_work(__VA_ARGS__)
#define ISCL_linalg_getLU64f_work(...)       \
             linalg_getLU64f_work(__VA_ARGS__)
#ifdef __cplusplus
}
#endif
#endif /* _linalg_linalg_h__ */
