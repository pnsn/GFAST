#ifndef GFAST_EEWUTILS_H__
#define GFAST_EEWUTILS_H__ 1
#include "gfast_struct.h"
#include "gfast_traceBuffer.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define SA_NAN -12345.0 /* NaN for shakeAlert event lat/lon/depth */

/* Drive the CMT computation */
int eewUtils_driveCMT(struct GFAST_cmt_props_struct cmt_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_offsetData_struct cmt_data,
                      struct GFAST_cmtResults_struct *cmt);
/* Drive the finite fault computation */
int eewUtils_driveFF(struct GFAST_ff_props_struct ff_props,
                     const double SA_lat,
                     const double SA_lon,
                     struct GFAST_offsetData_struct ff_data,
                     struct GFAST_ffResults_struct *ff);
/* Drive GFAST */
int eewUtils_driveGFAST(const double currentTime,
                        struct GFAST_props_struct props,
                        struct GFAST_activeEvents_struct *events,
                        struct GFAST_data_struct *gps_data,
                        struct h5traceBuffer_struct *h5traceBuffer,
                        struct GFAST_peakDisplacementData_struct *pgd_data,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_pgdResults_struct *pgd,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_ffResults_struct *ff,
                        struct GFAST_xmlMessages_struct *xmlMessages);
/* Drive the PGD computation */
int eewUtils_drivePGD(const struct GFAST_pgd_props_struct pgd_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_peakDisplacementData_struct pgd_data,
                      struct GFAST_pgdResults_struct *pgd);
/* Make finite fault XML for shakeAlert */
char *eewUtils_makeXML__ff(const enum opmode_type mode,
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
/* Make the quakeML */
char *eewUtils_makeXML__quakeML(const char *network,
                                const char *domain,
                                const char *evid,
                                const double evla,
                                const double evlo,
                                const double evdp,
                                const double t0, 
                                const double mt[6],
                                int *ierr);
/* Make the PGD XML */
char *eewUtils_makeXML__pgd(const enum opmode_type mode,
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
                            int *ierr);
/* Parses the core XML message */
int eewUtils_parseCoreXML(const char *message,
                          const double saNaN,
                          struct GFAST_shakeAlert_struct *SA);
/* Make the EEW log file names */
  void eewUtils_setLogFileNames(const char *eventid, const char *outputDir,
                              char errorLogFileName[PATH_MAX],
                              char infoLogFileName[PATH_MAX],
                              char debugLogFileName[PATH_MAX],
                              char warnLogFileName[PATH_MAX]);

#define GFAST_eewUtils_driveCMT(...)       \
              eewUtils_driveCMT(__VA_ARGS__)
#define GFAST_eewUtils_driveFF(...)       \
              eewUtils_driveFF(__VA_ARGS__)
#define GFAST_eewUtils_driveGFAST(...)       \
              eewUtils_driveGFAST(__VA_ARGS__)
#define GFAST_eewUtils_makeXML__ff(...)       \
              eewUtils_makeXML__ff(__VA_ARGS__)
#define GFAST_eewUtils_makeXML__quakeML(...)       \
              eewUtils_makeXML__quakeML(__VA_ARGS__)
#define GFAST_eewUtils_parseCoreXML(...)       \
              eewUtils_parseCoreXML(__VA_ARGS__)
#define GFAST_eewUtils_setLogFileNames(...)       \
              eewUtils_setLogFileNames(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_EEWUTILS_H__ */
