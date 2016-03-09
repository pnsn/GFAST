#include "gfast_numpy.h"
#include "gfast_obspy.h"
#include "gfast_struct.h"
#include "gfast_log.h"
#include "gfast_os.h"
#ifndef __GFAST__
#define __GFAST__
#ifdef __cplusplus
extern "C"
{
#endif
/* Initializes the GFAST parameters */
int GFAST_paraminit(char *propfilename, struct GFAST_props_struct *props);


int GFAST_CMTgreenF(int l1, int ldg, 
                    double *x1, double *y1, double *z1, double *azi, 
                    double *G);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST__ */
