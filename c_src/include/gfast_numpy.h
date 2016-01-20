#ifndef __GFAST_NUMPY__
#define __GFAST_NUMPY__
#ifdef __cplusplus
inline extern "C" 
{
#endif
int numpy_argmax(int n, double *x);
double numpy_nanmean(int n, double *x, int *iwarn);
void rotate_NE_RT(int np, double *e, double *n, double ba, 
                  double *r, double *t);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST_NUMPY__ */
