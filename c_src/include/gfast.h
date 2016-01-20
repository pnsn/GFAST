#include "gfast_numpy.h"
#include "gfast_obspy.h"
#ifndef __GFAST__
#define __GFAST__
#ifdef __cplusplus
extern "C"
{
#endif
int GFAST_CMTgreenF(int l1, int ldg, 
                    double *x1, double *y1, double *z1, double *azi, 
                    double *G);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST__ */
