#include <omp.h>
#include "gfast_struct.h"
#ifndef __GFAST_CORE_H__
#define __GFAST_CORE_H__
#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------//
//                                  CMT                                       //
//----------------------------------------------------------------------------//
/* Decompose a moment tensor */
int core_cmt_decomposeMomentTensor(const int nmt,
                                   const double *__restrict__ M,
                                   double *__restrict__ DC_pct,
                                   double *__restrict__ Mw, 
                                   double *__restrict__ strike1,
                                   double *__restrict__ strike2,
                                   double *__restrict__ dip1,
                                   double *__restrict__ dip2,
                                   double *__restrict__ rake1,
                                   double *__restrict__ rake2);
/* CMT depth grid search */
int core_cmt_depthGridSearch(const int l1, const int ndeps,
                             const int verbose,
                             const bool deviatoric,
                             const double utmSrcEasting,
                             const double utmSrcNorthing,
                             const double *__restrict__ srcDepths,
                             const double *__restrict__ utmRecvEasting,
                             const double *__restrict__ utmRecvNorthing,
                             const double *__restrict__ staAlt,
                             const double *__restrict__ nObsOffset,
                             const double *__restrict__ eObsOffset,
                             const double *__restrict__ uObsOffset,
                             const double *__restrict__ nWts,
                             const double *__restrict__ eWts,
                             const double *__restrict__ uWts,
                             double *__restrict__ nEst,
                             double *__restrict__ eEst,
                             double *__restrict__ uEst,
                             double *__restrict__ mts);
/* Initialize CMT data structures */
int core_cmt_initialize(struct GFAST_cmt_props_struct props,
                        struct GFAST_data_struct gps_data,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_offsetData_struct *cmt_data);
/* Set diagonal weight matrix in CMT inversion */
int core_cmt_setDiagonalWeightMatrix(const int n,
                                     const double *__restrict__ nWts,
                                     const double *__restrict__ eWts,
                                     const double *__restrict__ uWts,
                                     double *__restrict__ diagWt);
/* Set forward model in CMT inversion */
int core_cmt_setForwardModel(const int l1, const bool ldeviatoric,
                             const double *__restrict__ x1,
                             const double *__restrict__ y1,
                             const double *__restrict__ z1,
                             double *__restrict__ G);
/* Set RHS In CMT inversion */
int core_cmt_setRHS(const int n,
                    const double *__restrict__ nOffset,
                    const double *__restrict__ eOffset,
                    const double *__restrict__ uOffset,
                    double *__restrict__ U);
/* Weight the forward model by the diagonal weight matrix */
int core_cmt_weightForwardModel(const int mrows, const int ncols,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ G,
                                double *__restrict__ diagWtG);
/* Weight the observations by the diagonal weight matrix */
int core_cmt_weightObservations(const int mrows,
                                const double *__restrict__ diagWt,
                                const double *__restrict__ b,
                                double *__restrict__ diagWb);

//----------------------------------------------------------------------------//
//                              coordtools                                    //
//----------------------------------------------------------------------------//
/* Convert lat/lon to UTM */
#pragma omp declare simd
void core_coordtools_ll2utm(const double lat_deg, const double lon_deg,
                            double *UTMNorthing, double *UTMEasting,
                            bool *lnorthp, int *zone);
/* Convert UTM to lat/lon */
#pragma omp declare simd
void core_coordtools_utm2ll(const int zone, const bool lnorthp,
                            const double UTMNorthing, const double UTMEasting,
                            double *lat_deg, double *lon_deg);

//----------------------------------------------------------------------------//
//                              finite fault                                  //
//----------------------------------------------------------------------------//
/* Gridsearch on fault planes */
int core_ff_faultPlaneGridSearch(const int l1, const int l2, 
                                 const int nstr, const int ndip,
                                 const int nfp, const int verbose,
                                 const double *__restrict__ nObsOffset,
                                 const double *__restrict__ eObsOffset,
                                 const double *__restrict__ uObsOffset,
                                 const double *__restrict__ nWts,
                                 const double *__restrict__ eWts,
                                 const double *__restrict__ uWts,
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
/* Initializes finite fault data structures */
int core_ff_initialize(struct GFAST_ff_props_struct props,
                       struct GFAST_data_struct gps_data,
                       struct GFAST_ffResults_struct *ff,
                       struct GFAST_offsetData_struct *ff_data);
/* Meshes the fault plane */
int core_ff_meshFaultPlane(const double ev_lat,
                           const double ev_lon,
                           const double ev_dep,
                           const double flen_pct,
                           const double fwid_pct,
                           const double M,
                           const double strikeF, const double dipF,
                           const int nstr, const int ndip,
                           const int utm_zone, const int verbose,
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
/* Set the diagonal data weight matrix */
int core_ff_setDiagonalWeightMatrix(const int n,
                                    const double *__restrict__ nWts,
                                    const double *__restrict__ eWts,
                                    const double *__restrict__ uWts,
                                    double *__restrict__ diagWt);
/* Set the (unregularized) forward modeling matrix */
int core_ff_setForwardModel__okadagreenF(const int l1, const int l2, 
                                         const double *__restrict__ e,
                                         const double *__restrict__ n,
                                         const double *__restrict__ depth,
                                         const double *__restrict__ strike,
                                         const double *__restrict__ dip,
                                         const double *__restrict__ W,
                                         const double *__restrict__ L,
                                         double *__restrict__ G);
/* Set the regularization matrix for the finite fault inversion */
int core_ff_setRegularizer(const int l2, const int nstr,
                           const int ndip, const int nt, 
                           const double *__restrict__ width,
                           const double *__restrict__ length,
                           double *__restrict__ T);
/* Set the RHS */
int core_ff_setRHS(const int n,
                   const double *__restrict__ nOffset,
                   const double *__restrict__ eOffset,
                   const double *__restrict__ uOffset,
                   double *__restrict__ U);
/* Weight the forward modeling matrix */
int core_ff_weightForwardModel(const int mrows, const int ncols,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ G,
                               double *__restrict__ diagWtG);
/* Weight the observations */
int core_ff_weightObservations(const int mrows,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ b,
                               double *__restrict__ diagWb);

//----------------------------------------------------------------------------//
//                              PGD scaling                                   //
//----------------------------------------------------------------------------//

/* Depth grid search in PGD scaling */
int core_scaling_pgd_depthGridSearch(const int l1, const int ndeps,
                                     const int verbose,
                                     const double dist_tol,
                                     const double disp_def,
                                     const double utmSrcEasting,
                                     const double utmSrcNorthing,
                                     const double *__restrict__ srcDepths,
                                     const double *__restrict__ utmRecvEasting,
                                     const double *__restrict__ utmRecvNorthing,
                                     const double *__restrict__ staAlt,
                                     const double *__restrict__ d,
                                     const double *__restrict__ wts,
                                     double *__restrict__ M,
                                     double *__restrict__ VR,
                                     double *__restrict__ Uest);
/* Initialize PGD scaling data structures */
int core_scaling_pgd_initialize(struct GFAST_pgd_props_struct pgd_props,
                                struct GFAST_data_struct gps_data,
                                struct GFAST_pgdResults_struct *pgd,
                                struct GFAST_peakDisplacementData_struct *pgd_data);
/* Set the diagonal data weighting matrix */
int core_scaling_pgd_setDiagonalWeightMatrix(const int l1, 
                                             const double *__restrict__ repi,
                                             const double *__restrict__ wts,
                                             double *__restrict__ W);
/* Set the forward modeling matrix */
int core_scaling_pgd_setForwardModel(const int n,
                                     const double B, const double C,
                                     const double *__restrict__ r,
                                     double *__restrict__ G);
/* Set the RHS */
int core_scaling_pgd_setRHS(const int n,
                            const double dist_tol,
                            const double dist_def,
                            const double A,
                            const double *__restrict__ d,
                            double *__restrict__ b);
/* Weight the PGD forward modeling operator */
int core_scaling_pgd_weightForwardModel(const int l1, 
                                        const double *__restrict__ W,
                                        const double *__restrict__ G,
                                        double *__restrict__ WG);
/* Weight the PGD RHS */
int core_scaling_pgd_weightObservations(const int l1, 
                                        const double *__restrict__ W,
                                        const double *__restrict__ b,
                                        double *__restrict__ Wb);

//----------------------------------------------------------------------------//
//                            Waveform processor                              //
//----------------------------------------------------------------------------//
/* Compute the offset in north, east, and up */
int core_waveformProcessor_offset(const int utm_zone,
                                  const double svel_window,
                                  const double ev_lat,
                                  const double ev_lon,
                                  const double ev_dep,
                                  const double ev_time,
                                  struct GFAST_data_struct gps_data,
                                  struct GFAST_offsetData_struct *offset_data,
                                  int *ierr);
/* Compute the peak displacement */
int core_waveformProcessor_peakDisplacement(
    const int utm_zone,
    const double svel_window,
    const double ev_lat,
    const double ev_lon,
    const double ev_dep,
    const double ev_time,
    struct GFAST_data_struct gps_data,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    int *ierr);

#define GFAST_core_cmt_decomposeMomentTensor(...)       \
              core_cmt_decomposeMomentTensor(__VA_ARGS__)
#define GFAST_core_cmt_depthGridSearch(...)       \
              core_cmt_depthGridSearch(__VA_ARGS__)
#define GFAST_core_cmt_initialize(...)       \
              core_cmt_initialize(__VA_ARGS__)
#define GFAST_core_cmt_setDiagonalWeightMatrix(...)       \
              core_cmt_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_cmt_setForwardModel(...)       \
              core_cmt_setForwardModel(__VA_ARGS__)
#define GFAST_core_cmt_setRHS(...)       \
              core_cmt_setRHS(__VA_ARGS__)
#define GFAST_core_cmt_setForwardModel__deviatoric(...)       \
              core_cmt_setForwardModel__deviatoric(__VA_ARGS__)
#define GFAST_core_cmt_setRHS(...)       \
              core_cmt_setRHS(__VA_ARGS__)
#define GFAST_core_cmt_weightForwardModel(...)       \
              core_cmt_weightForwardModel(__VA_ARGS__)
#define GFAST_core_cmt_weightObservations(...)       \
              core_cmt_weightObservations(__VA_ARGS__)

#define GFAST_core_coordtools_ll2utm(...) \
              core_coordtools_ll2utm(__VA_ARGS__)
#define GFAST_core_coordtools_utm2ll(...) \
              core_coordtools_utm2ll(__VA_ARGS__)

#define GFAST_core_ff_faultPlaneGridSearch(...)       \
              core_ff_faultPlaneGridSearch(__VA_ARGS__)
#define GFAST_core_ff_initialize(...)       \
              core_ff_initialize(__VA_ARGS__)
#define GFAST_core_ff_meshFaultPlane(...)       \
              core_ff_meshFaultPlane(__VA_ARGS__)
#define GFAST_core_ff_setDiagonalWeightMatrix(...)       \
              core_ff_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_ff_setForwardModel__okadagreenF(...)       \
              core_ff_setForwardModel__okadagreenF(__VA_ARGS__)
#define GFAST_core_ff_setRegularizer(...)       \
              core_ff_setRegularizer(__VA_ARGS__)
#define GFAST_core_ff_setRHS(...)       \
              core_ff_setRHS(__VA_ARGS__)
#define GFAST_core_ff_weightForwardModel(...)       \
              core_ff_weightForwardModel(__VA_ARGS__)
#define GFAST_core_ff_weightObservations(...)       \
              core_ff_weightObservations(__VA_ARGS__)

#define GFAST_core_scaling_pgd_depthGridSearch(...)       \
              core_scaling_pgd_depthGridSearch(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setDiagonalWeightMatrix(...)       \
              core_scaling_pgd_setDiagonalWeightMatrix(__VA_ARGS__)
#define GFAST_core_scaling_pgd_initialize(...)       \
              core_scaling_pgd_initialize(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setForwardModel(...)       \
              core_scaling_pgd_setForwardModel(__VA_ARGS__)
#define GFAST_core_scaling_pgd_setRHS(...)       \
              core_scaling_pgd_setRHS(__VA_ARGS__)
#define GFAST_core_scaling_pgd_weightForwardModel(...)       \
              core_scaling_pgd_weightForwardModel(__VA_ARGS__)
#define GFAST_core_scaling_pgd_weightObservations(...)       \
              core_scaling_pgd_weightObservations(__VA_ARGS__)

#define GFAST_core_waveformProcessor_offset(...)       \
              core_waveformProcessor_offset(__VA_ARGS__)
#define GFAST_core_waveformProcessor_peakDisplacement(...)       \
              core_waveformProcessor_peakDisplacement(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_CORE_H__ */