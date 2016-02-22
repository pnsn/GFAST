#ifndef __GFAST_NUMPY__
#define __GFAST_NUMPY__
#ifdef __cplusplus
extern "C" 
{
#endif
int numpy_argmax(int n, double *x);
double numpy_nanmean(int n, double *x, int *iwarn);
double numpy_min(int n, double *x);
int numpy_lstsq(int mtx_fmt,
                int m, int n, int nrhs, double *Aref, double *b,
                double *rcond_in, double *x, int *rank_out, double *svals);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST_NUMPY__ */
