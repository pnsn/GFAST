#ifndef __GFAST_NUMPY__
#define __GFAST_NUMPY__
#ifdef __cplusplus
extern "C" 
{
#endif
int numpy_argmax(int n, const double *x);
int numpy_argmin(int n, const double *x);
double numpy_nanmean(int n, const double *x, int *iwarn);
double numpy_nanmax(int n, const double *x, int *iwarn);
double numpy_min(int n, const double *x);
int numpy_lstsq(int mtx_fmt,
                int m, int n, int nrhs, double *Aref, double *b,
                double *rcond_in, double *x, int *rank_out, double *svals);
int numpy_lstsq__qr(int mtx_fmt,
                    int m, int n, int nrhs, double *Aref, double *b, 
                    double *x, double *R);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST_NUMPY__ */
