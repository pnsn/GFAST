#include "gfast_struct.h"
#ifndef __GFAST_EEWUTILS_H__
#define __GFAST_EEWUTILS_H__

#ifdef __cplusplus
extern "C" 
{
#endif

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
                     const double SA_dep,
                     struct GFAST_offsetData_struct ff_data,
                     struct GFAST_ffResults_struct *ff);
/* Drive the PGD computation */
int eewUtils_drivePGD(const struct GFAST_pgd_props_struct pgd_props,
                      const double SA_lat,
                      const double SA_lon,
                      const double SA_dep,
                      struct GFAST_peakDisplacementData_struct pgd_data,
                      struct GFAST_pgdResults_struct *pgd);
/* Parses the core XML message */
int eewUtils_parseCoreXML(const char *message,
                          const double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA);

#define GFAST_eewUtils_driveCMT(...)       \
              eewUtils_driveCMT(__VA_ARGS__)
#define GFAST_eewUtils_driveFF(...)       \
              eewUtils_driveFF(__VA_ARGS__)
#define GFAST_eewUtils_parseCoreXML(...)       \
              eewUtils_parseCoreXML(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_EEWUTILS_H__ */
