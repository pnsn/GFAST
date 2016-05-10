#include <stdbool.h>
#include "gfast_cmopad.h"
#include "gfast_enum.h"
#include "gfast_log.h"
#include "gfast_numpy.h"
#include "gfast_obspy.h"
#include "gfast_os.h"
#include "gfast_struct.h"
#include "gfast_time.h"
#include "gfast_xml.h"
#ifndef __GFAST__
#define __GFAST__

#define CACHE_LINE_SIZE 64
#define GFAST_ALGORITHM_VERSION 0.01
#define GFAST_VERSION 0
 

#ifdef __cplusplus
extern "C"
{
#endif
/* Data acquisition */
int GFAST_acquisition__init(struct GFAST_props_struct props,
                            struct GFAST_data_struct *gps_acquisition);
int GFAST_acquisition__updateFromSAC(struct GFAST_props_struct props,
                                     double simStartTime,
                                     double eventTime,
                                     double currentTime,
                                     double *latency,
                                     struct GFAST_data_struct *gps_acquisition);
double GFAST_acquisition__getT0FromSAC(struct GFAST_props_struct props,
                                       struct GFAST_data_struct gps_acquisition,
                                       int *ierr);
/* Buffer initialization */
int GFAST_buffer__getNumberOfStreams(struct GFAST_props_struct props);
int GFAST_buffer__setBufferSpace(struct GFAST_props_struct props,
                                struct GFAST_data_struct *gps_data);
int GFAST_buffer__setSitesAndLocations(struct GFAST_props_struct props,
                                       struct GFAST_data_struct *gps_data);
int GFAST_buffer__setSiteSamplingPeriod(struct GFAST_props_struct props,
                                        struct GFAST_data_struct *gps_data);
int GFAST_buffer__readDataFromSAC(int job,
                                  struct GFAST_props_struct props,
                                  struct GFAST_data_struct *gps_data);
void GFAST_buffer_print__samplingPeriod(struct GFAST_data_struct gps_data);
void GFAST_buffer_print__locations(struct GFAST_data_struct gps_data);
void GFAST_buffer__setInitialTime(double epoch0,
                                  struct GFAST_data_struct *gps_data);
/* Coordinate tools */
void GFAST_coordtools_lla2ecef(double lat_in, double lon_in, double alt,
                               double *x, double *y, double *z);
void GFAST_coordtools_ecef2lla(double x, double y, double z,
                               double *lat_deg, double *lon_deg, double *alt);
void GFAST_coordtools_dxyz2dneu(double dx, double dy, double dz,
                                double lat_deg, double lon_deg,
                                double *dn, double *de, double *du);

#if __GNUC__ >= 5
#pragma omp declare simd
#endif
void GFAST_coordtools_ll2utm_ori(double lat_deg, double lon_deg,
                                 double *UTMEasting, double *UTMNorthing,
                                 bool *lnorthp, int *zone);
#if __GNUC__ >= 5
#pragma omp declare simd
#endif
void GFAST_coordtools_utm2ll_ori(int zone, bool lnorthp,
                                 double UTMEasting, double UTMNorthing,
                                 double *lat_deg, double *lon_deg);
#if __GNUC__ >= 5
#pragma omp declare simd
#endif
void GFAST_coordtools__ll2utm(double lat_deg, double lon_deg,
                              double *UTMEasting, double *UTMNorthing,
                              bool *lnorthp, int *zone);
#if __GNUC__ >= 5
#pragma omp declare simd
#endif
void GFAST_coordtools__utm2ll(int zone, bool lnorthp,
                              double UTMEasting, double UTMNorthing,
                              double *lat_deg, double *lon_deg);
/* Handles events */
double GFAST_events__getMinOriginTime(struct GFAST_props_struct props,
                                      struct GFAST_activeEvents_struct events,
                                      bool *lnoEvents);
bool GFAST_events__newEvent(struct GFAST_shakeAlert_struct SA,
                            struct GFAST_activeEvents_struct *events);
bool GFAST_events__updateEvent(struct GFAST_shakeAlert_struct SA,
                               struct GFAST_activeEvents_struct *events,
                               int *ierr);
void GFAST_events__print(struct GFAST_shakeAlert_struct SA);
bool GFAST_events__removeEvent(double maxtime, double currentTime,
                               int verbose,
                               struct GFAST_shakeAlert_struct SA, 
                               struct GFAST_activeEvents_struct *events);
/* Frees memory on pointers and data structures */
void GFAST_memory_freeStrongMotionData(struct GFAST_strongMotion_struct *sm);
void GFAST_memory_freeCollocatedData(struct GFAST_collocatedData_struct *data);
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data);
void GFAST_memory_freeProps(struct GFAST_props_struct *props);
void GFAST_memory_freeEvents(struct GFAST_activeEvents_struct *events);
void GFAST_memory_freePGDResults(struct GFAST_pgdResults_struct *pgd);
void GFAST_memory_freeCMTResults(struct GFAST_cmtResults_struct *cmt);
void GFAST_memory_freeFaultPlane(struct GFAST_faultPlane_struct *fp);
void GFAST_memory_freeFFResults(struct GFAST_ffResults_struct *ff);
int *GFAST_memory_calloc__int(int n);
int *GFAST_memory_alloc__int(int n);
bool *GFAST_memory_calloc__bool(int n);
bool *GFAST_memory_alloc__bool(int n);
double *GFAST_memory_calloc__double(int n);
double *GFAST_memory_alloc__double(int n);
void GFAST_memory_free(void *p);
void GFAST_memory_free__double(double **p);
void GFAST_memory_free__int(int **p);
void memory_free__bool(bool **p);
/* Initializes the GFAST parameters */
int GFAST_properties__init(char *propfilename,
                           struct GFAST_props_struct *props);
void GFAST_properties__print(struct GFAST_props_struct props);
/* Reads the ElarmS file */
int GFAST_readElarmS__xml(const char *message, double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA);
int GFAST_readElarmS(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct *SA);
int GFAST_readElarmS_ElarmSMessage2SAStruct(int verbose, char *buff,
                                            struct GFAST_shakeAlert_struct *SA);
/* PGD Scaling */
int GFAST_scaling_PGD__driver(struct GFAST_pgd_props_struct pgd_props,
                              struct GFAST_shakeAlert_struct SA,
                              struct GFAST_data_struct gps_data,
                              struct GFAST_pgdResults_struct *pgd);
int GFAST_scaling_PGD__depthGridSearch(int l1, int ndeps,
                                       int verbose,
                                       double dist_tol,
                                       double dist_def,
                                       double utmSrcEasting,
                                       double utmSrcNorthing,
                                       double *__restrict__ srcDepths,
                                       double *__restrict__ utmRecvEasting,
                                       double *__restrict__ utmRecvNorthing,
                                       double *__restrict__ staAlt,
                                       double *__restrict__ d,
                                       double *__restrict__ repi,
                                       double *__restrict__ M,
                                       double *__restrict__ VR);
int GFAST_scaling_PGD__init(struct GFAST_pgd_props_struct pgd_props,
                            struct GFAST_data_struct gps_data,
                            struct GFAST_pgdResults_struct *pgd);
int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                       double B, double C,
                                       const double *__restrict__ r,
                                       double *__restrict__ G);
int GFAST_scaling_PGD__setRHS(int n, int verbose,
                              double dist_tol, double dist_def,
                              double A,
                              const double *__restrict__ d,
                              double *__restrict__ b);
/* Centroid moment tensor inversion */
int GFAST_CMT__decomposeMomentTensor(double *M, 
                                     double *DC_pct,
                                     double *Mw,
                                     double *strike1, double *strike2,
                                     double *dip1, double *dip2,
                                     double *rake1, double *rake2);
int GFAST_CMT__depthGridSearch(int l1, int ndeps,
                               int verbose,
                               bool deviatoric,
                               double utmSrcEasting,
                               double utmSrcNorthing,
                               const double *__restrict__ srcDepths,
                               const double *__restrict__ utmRecvEasting,
                               const double *__restrict__ utmRecvNorthing,
                               const double *__restrict__ staAlt,
                               const double *__restrict__ nAvgDisp,
                               const double *__restrict__ eAvgDisp,
                               const double *__restrict__ uAvgDisp,
                               double *__restrict__ cmt_vr,
                               double *__restrict__ mts,
                               double *__restrict__ str1,
                               double *__restrict__ str2,
                               double *__restrict__ dip1,
                               double *__restrict__ dip2,
                               double *__restrict__ rak1,
                               double *__restrict__ rak2,
                               double *__restrict__ Mw);
int GFAST_CMT__driver(struct GFAST_props_struct props,
                      struct GFAST_shakeAlert_struct SA, 
                      struct GFAST_data_struct gps_data,
                      struct GFAST_cmtResults_struct *cmt);
int GFAST_CMT__init(struct GFAST_props_struct props,
                    struct GFAST_data_struct gps_data,
                    struct GFAST_cmtResults_struct *cmt);
int GFAST_CMT__setForwardModel__deviatoric(int l1, 
                                           const double *__restrict__ x1, 
                                           const double *__restrict__ y1, 
                                           const double *__restrict__ z1, 
                                           double *__restrict__ G);
int GFAST_CMT__setRHS(int n, int verbose,
                      const double *__restrict__ nAvg,
                      const double *__restrict__ eAvg,
                      const double *__restrict__ uAvg,
                      double *__restrict__ U);
/* Finite fault inversion */
int GFAST_FF__driver(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct SA, 
                     struct GFAST_data_struct gps_data,
                     struct GFAST_ffResults_struct *ff);
int __GFAST_FF__faultPlaneGridSearch(int l1, int l2,
                                     int nstr, int ndip, int nfp,
                                     int verbose,
                                     const double *__restrict__ nAvgDisp,
                                     const double *__restrict__ eAvgDisp,
                                     const double *__restrict__ uAvgDisp,
                                     const double *__restrict__ utmRecvEasting,
                                     const double *__restrict__ utmRecvNorthing,
                                     const double *__restrict__ staAlt,
                                     const double *__restrict__ fault_xutm,
                                     const double *__restrict__ fault_yutm,
                                     const double *__restrict__ fault_alt,
                                     const double *__restrict__ length,
                                     const double *__restrict__ width,
                                     const double *__restrict__ strike,
                                     const double *__restrict__ dip,
                                     double *__restrict__ sslip,
                                     double *__restrict__ dslip,
                                     double *__restrict__ Mw,
                                     double *__restrict__ vr,
                                     double *__restrict__ NN,
                                     double *__restrict__ EN,
                                     double *__restrict__ UN,
                                     double *__restrict__ sslip_unc,
                                     double *__restrict__ dslip_unc
                                     );
int GFAST_FF__init(struct GFAST_props_struct props,
                   struct GFAST_data_struct gps_data,
                   struct GFAST_ffResults_struct *ff);
int GFAST_FF__meshFaultPlane(double SA_lat, double SA_lon, double SA_dep,
                             double flen_pct,
                             double fwid_pct,
                             double M, double strikeF, double dipF,
                             int nstr, int ndip,
                             int utm_zone, int verbose,
                             int *__restrict__ fault_ptr,
                             double *__restrict__ lat_vtx,
                             double *__restrict__ lon_vtx,
                             double *__restrict__ dep_vtx,
                             double *__restrict__ fault_xutm,
                             double *__restrict__ fault_yutm,
                             double *__restrict__ fault_alt,
                             double *__restrict__ strike,
                             double *__restrict__ dip,
                             double *__restrict__ length,
                             double *__restrict__ width);
int GFAST_FF__setForwardModel__okadagreenF(int l1, int l2,
                                           const double *__restrict__ e,
                                           const double *__restrict__ n,
                                           const double *__restrict__ depth,
                                           const double *__restrict__ strike,
                                           const double *__restrict__ dip,
                                           const double *__restrict__ W,
                                           const double *__restrict__ L,
                                           double *__restrict__ G);
int GFAST_FF__setRegularizer(int l2, int nstr, int ndip, int nt,
                             const double *__restrict__ width,
                             const double *__restrict__ length,
                             double *__restrict__ T);
int GFAST_FF__setRHS(int n, int verbose,
                     const double *__restrict__ nAvg,
                     const double *__restrict__ eAvg,
                     const double *__restrict__ uAvg,
                     double *__restrict__ U);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST__ */
