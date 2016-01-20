#ifndef __GFAST_OBSPY__
#define __GFAST_OBSPY__
#ifdef __cplusplus
extern "C" 
{
#endif
int obspy_rotate_NE2RT(int np, double *e, double *n, double ba, 
                       double *r, double *t);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST_OBSPY__ */
