#include <stdbool.h>
#include "gfast_activeMQ.h"
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_enum.h"
#include "gfast_events.h"
#include "gfast_hdf5.h"
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"
#include "gfast_xml.h"
#ifndef __GFAST__
#define __GFAST__

#define GFAST_ALGORITHM_VERSION "0.01"
#define GFAST_VERSION "0.01"
#define GFAST_INSTANCE "PNSN"

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
/* Frees memory on pointers and data structures */
void GFAST_memory_freeWaveformData(struct GFAST_waveform3CData_struct *data);
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data);
void GFAST_memory_freeProps(struct GFAST_props_struct *props);
void GFAST_memory_freeEvents(struct GFAST_activeEvents_struct *events);
void GFAST_memory_freePGDResults(struct GFAST_pgdResults_struct *pgd);
void GFAST_memory_freeCMTResults(struct GFAST_cmtResults_struct *cmt);
void GFAST_memory_freeFaultPlane(struct GFAST_faultPlane_struct *fp);
void GFAST_memory_freeFFResults(struct GFAST_ffResults_struct *ff);
void GFAST_memory_freePGDData(
     struct GFAST_peakDisplacementData_struct *pgd_data);
void GFAST_memory_freeOffsetData(struct GFAST_offsetData_struct *offset_data);
/* Initializes the GFAST parameters */
int GFAST_properties_initialize(const char *propfilename,
                                const enum opmode_type opmode,
                                struct GFAST_props_struct *props);
void GFAST_properties_print(struct GFAST_props_struct props);
/* Reads the ElarmS file */
int GFAST_readElarmS__xml(const char *message, double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA);
int GFAST_readElarmS(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct *SA);
int GFAST_readElarmS_ElarmSMessage2SAStruct(int verbose, char *buff,
                                            struct GFAST_shakeAlert_struct *SA);
/* PGD Scaling */
int GFAST_scaling_PGD__driver(
   struct GFAST_pgd_props_struct pgd_props,
   double SA_lat, double SA_lon, double SA_dep,
   struct GFAST_peakDisplacementData_struct pgd_data,
   struct GFAST_pgdResults_struct *pgd);
/* Centroid moment tensor inversion */
int GFAST_CMT__driver(struct GFAST_cmt_props_struct cmt_props,
                      double SA_lat, double SA_lon, double SA_dep,
                      struct GFAST_offsetData_struct cmt_data,
                      struct GFAST_cmtResults_struct *cmt);

char *GFAST_CMT__makeQuakeML(const char *network,
                             const char *domain,
                             const char *evid,
                             const double evla,
                             const double evlo,
                             const double evdp,
                             const double t0, 
                             const double mt[6],
                             int *ierr);
/* Finite fault inversion */
int GFAST_FF__driver(struct GFAST_ff_props_struct ff_props,
                     double SA_lat, double SA_lon, double SA_dep,
                     struct GFAST_offsetData_struct ff_data,
                     struct GFAST_ffResults_struct *ff);
char *GFAST_FF__makeXML(const int mode,
                        const char *orig_sys,
                        const char *alg_vers,
                        const char *instance,
                        const char *message_type,
                        const char *version,
                        const char *evid,
                        const double SA_lat,
                        const double SA_lon,
                        const double SA_depth,
                        const double SA_mag,
                        const double SA_time,
                        const int nseg,
                        const int *fault_ptr,
                        const double *lat_vtx,
                        const double *lon_vtx,
                        const double *dep_vtx,
                        const double *ss,
                        const double *ds,
                        const double *ss_unc,
                        const double *ds_unc,
                        int *ierr);
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __GFAST__ */
