#include <stdbool.h>
#include "gfast_enum.h"
#include "gfast_log.h"
#include "gfast_numpy.h"
#include "gfast_obspy.h"
#include "gfast_os.h"
#include "gfast_struct.h"
#include "gfast_time.h"
#ifndef __GFAST__
#define __GFAST__
#ifdef __cplusplus
extern "C"
{
#endif
/* Buffer initialization */
int GFAST_buffer_setSiteSamplingPeriod(struct GFAST_props_struct props,
                                       struct GFAST_data_struct *gps_data);
void GFAST_buffer_print_samplingPeriod(struct GFAST_data_struct gps_data);
/* Coordinate tools */
void GFAST_coordtools_lla2ecef(double lat_in, double lon_in, double alt,
                               double *x, double *y, double *z);
void GFAST_coordtools_ecef2lla(double x, double y, double z,
                               double *lat_deg, double *lon_deg, double *alt);
void GFAST_coordtools_dxyz2dneu(double dx, double dy, double dz,
                                double lat_deg, double lon_deg,
                                double *dn, double *de, double *du);
#ifdef WITH_LIBGEO
int geodetic_coordtools_ll2utm(double lat, double lon,
                               double *xutm, double *yutm,
                               bool *lnorthp, int *zone);
int geodetic_coordtools_utm2ll(int zone, bool lnorthp, double xutm, double yutm,
                               double *lat, double *lon);
#else
void GFAST_coordtools_ll2utm(double lon_deg, double lat_deg, double lon0_deg,
                             double *UTMNorthing, double *UTMEasting);
void GFAST_coordtools_utm2ll(double UTMEasting, double UTMNorthing,
                             double lon0_deg,
                             double *lat_deg, double *lon_deg);
#endif

/* Initializes station locations/names */
int GFAST_locinit(struct GFAST_props_struct props,
                  struct GFAST_data_struct *gps_data);
void GFAST_locinit_printLocations(struct GFAST_data_struct *gps_data);
/* Frees memory on data structures */
void GFAST_memory_freeStrongMotionData(struct GFAST_strongMotion_struct *sm);
void GFAST_memory_freeCollocatedData(struct GFAST_collocatedData_struct *data);
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data);
void GFAST_memory_freeProps(struct GFAST_props_struct *props);
/* Initializes the GFAST parameters */
int GFAST_paraminit(char *propfilename, struct GFAST_props_struct *props);
void GFAST_paraminit_print(struct GFAST_props_struct props);
/* Reads the ElarmS file */
int GFAST_readElarmS(struct GFAST_props_struct props);
int GFAST_readElarmS_ElarmSMessage2SAStruct(int verbose, char *buff,
                                            struct GFAST_shakeAlert_struct *SA);
/* PGD Scaling */
int GFAST_scaling_PGD(int n, int verbose,
                      double dist_tol, double dist_def,
                      double *d, double *r, double *repi,
                      double *M, double *VR);

int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                       double B, double C, double *__restrict__ r,
                                       double *__restrict__ G);
int GFAST_scaling_PGD__setRHS(int n, int verbose,
                              double dist_tol, double dist_def,
                              double A, double *__restrict__ d,
                              double *__restrict__ b);
/* Matrix of Green's functions for CMT */
int GFAST_CMTgreenF(int l1, int verbose, 
                    double *__restrict__ x1, 
                    double *__restrict__ y1, 
                    double *__restrict__ z1, 
                    double *__restrict__ azi, 
                    double *__restrict__ G);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST__ */
