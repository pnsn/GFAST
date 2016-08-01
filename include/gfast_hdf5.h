#include <limits.h>
#include <hdf5.h>
#include "gfast_struct.h"

#ifndef __GFAST_HDF5_H__
#define __GFAST_HDF5_H__

enum data2h5_enum
{
    COPY_DATA_TO_H5 = 0,  /*!< Copies data structure to HDF5 structure */
    COPY_H5_TO_DATA = 1,  /*!< Copies HDF5 structure to data structure */
};

struct h5_peakDisplacementData_struct
{
    hvl_t stnm;
    hvl_t pd;
    hvl_t wt;
    hvl_t sta_lat;
    hvl_t sta_lon;
    hvl_t sta_alt;
    hvl_t lmask;
    hvl_t lactive;
    hvl_t  nsites;
};

struct h5_pgdResults_struct
{
    hvl_t mpgd;
    hvl_t mpgd_vr;
    hvl_t UP;
    hvl_t UPinp;
    hvl_t srcDepths;
    hvl_t lsiteUsed;
    int ndeps;
    int nsites;
};

struct h5_cmtResults_struct
{
    hvl_t objfn;
    hvl_t mts;
    hvl_t str1;
    hvl_t str2;
    hvl_t dip1;
    hvl_t dip2;
    hvl_t rak1;
    hvl_t rak2;
    hvl_t Mw;
    hvl_t srcDepths;
    hvl_t EN;
    hvl_t NN;
    hvl_t UN;
    hvl_t Einp;
    hvl_t Ninp;
    hvl_t Uinp;
    hvl_t lsiteUsed;
    int opt_indx;
    int ndeps;
    int nsites;
};

struct h5_offsetData_struct
{
    hvl_t stnm[64];
    hvl_t ubuff;
    hvl_t nbuff;
    hvl_t ebuff;
    hvl_t wtu;
    hvl_t wtn;
    hvl_t wte;
    hvl_t sta_lat;
    hvl_t sta_lon;
    hvl_t sta_alt;
    hvl_t lmask;
    hvl_t lactive;
    int nsites;
};

struct h5_faultPlane_struct
{
    hvl_t lon_vtx;
    hvl_t lat_vtx;
    hvl_t dep_vtx;
    hvl_t fault_xutm;
    hvl_t fault_yutm;
    hvl_t fault_alt;
    hvl_t strike;
    hvl_t dip;
    hvl_t length;
    hvl_t width;
    hvl_t sslip;
    hvl_t dslip;
    hvl_t sslip_unc;
    hvl_t dslip_unc;
    hvl_t EN;
    hvl_t NN;
    hvl_t UN;
    hvl_t Einp;
    hvl_t Ninp;
    hvl_t Uinp;
    hvl_t fault_ptr;
    int maxobs;
    int nsites_used;
    int nstr;
    int ndip;
};

struct h5_ffResults_struct
{
    hvl_t fp;
    hvl_t vr;
    hvl_t Mw;
    hvl_t str;
    hvl_t dip;
    double SA_lat;
    double SA_lon;
    double SA_dep;
    double SA_mag;
    int preferred_fault_plane;
    int nfp;
};

#ifdef __cplusplus
extern "C"
{
#endif

int GFAST_HDF5__copy__cmtResults(enum data2h5_enum job,
                                 struct GFAST_cmtResults_struct *cmt,
                                 struct h5_cmtResults_struct *h5_cmt);
int GFAST_HDF5__copy__faultPlane(enum data2h5_enum job,
                                 struct GFAST_faultPlane_struct *fp,
                                 struct h5_faultPlane_struct *h5_fp);
int GFAST_HDF5__copy__ffResults(enum data2h5_enum job,
                                struct GFAST_ffResults_struct *ff,
                                struct h5_ffResults_struct *h5_ff);
int GFAST_HDF5__copy__peakDisplacementData(enum data2h5_enum job,
                          struct GFAST_peakDisplacementData_struct *pgd_data,
                          struct h5_peakDisplacementData_struct *h5_pgd_data);
int GFAST_HDF5__copy__pgdResults(enum data2h5_enum job,
                                 struct GFAST_pgdResults_struct *pgd,
                                 struct h5_pgdResults_struct *h5_pgd);
herr_t GFAST_HDF5__createType__cmtResults(hid_t group_id);
herr_t GFAST_HDF5__createType__faultPlane(hid_t group_id);
herr_t GFAST_HDF5__createType__ffResults(hid_t group_id);
herr_t GFAST_HDF5__createType__offsetData(hid_t group_id);
herr_t GFAST_HDF5__createType__peakDisplacementData(hid_t group_id);
herr_t GFAST_HDF5__createType__pgdResults(hid_t group_id);
int GFAST_HDF5__initialize(const char *adir,
                           const char *evid,
                           const char *propfilename);
int GFAST_HDF5__memory__freeCMTResults(struct h5_cmtResults_struct *cmt);
int GFAST_HDF5__memory__freeFFResults(struct h5_ffResults_struct *ff);
int GFAST_HDF5__memory__freePGDResults(struct h5_pgdResults_struct *pgd);
int GFAST_HDF5__setFileName(const char *adir,
                            const char *evid, 
                            char fname[PATH_MAX]);
int GFAST_HDF5__update__cmt(const char *adir,
                            const char *evid,
                            const int h5k,
                            struct GFAST_offsetData_struct cmt_data,
                            struct GFAST_cmtResults_struct cmt);
int GFAST_HDF5__update__ff(const char *adir,
                           const char *evid,
                           const int h5k,
                           struct GFAST_offsetData_struct ff_data,
                           struct GFAST_ffResults_struct ff);
int GFAST_HDF5__update__getIteration(const char *adir,
                                     const char *evid,
                                     const double epoch);
int GFAST_HDF5__update__pgd(const char *adir,
                            const char *evid,
                            const int h5k,
                            struct GFAST_peakDisplacementData_struct pgd_data,
                            struct GFAST_pgdResults_struct pgd);



//----------------------------------------------------------------------------//
//                            hdf5 convenience functions                      //
//----------------------------------------------------------------------------//
hid_t h5_open_rdonly(const char *flname);
hid_t h5_open_rdwt(const char *flname);
int h5_close(const hid_t file_id);
int h5_write_array__float(const char *dset_name, const hid_t file_id,
                          const int n, const float *x);
int h5_write_array__double(const char *dset_name, const hid_t file_id,
                           const int n, const double *x);
int h5_write_array__int(const char *dset_name, const hid_t file_id,
                        const int n, const int *x);
int h5_write_array__chars(const char *citem_chr, const hid_t file_id,
                          const int n, const char **c);
int h5_read_array__double(const char *dset_name, const hid_t file_id,
                          const int nref, double *x);
int h5_read_array__float(const char *dset_name, const hid_t file_id,
                         const int nref, float *x);
int h5_read_array__int(const char *dset_name, const hid_t file_id,
                       const int nref, int *x);
int h5_write_attribute__double(const char *citem, const hid_t hdf5_id,
                               const int n, const double *attr_data);
int h5_write_attribute__int(const char *citem, const hid_t hdf5_id,
                            const int n, const int *attr_data);
int h5_write_attribute__char(const char *citem, const hid_t hdf5_id,
                             const int n, const char **cattr);
int h5_n_group_members(const char *group_name, const hid_t file_id);
int h5_get_array_size(const hid_t file_id, const char *citem);
bool h5_item_exists(const hid_t file_id, const char *citem_in);
hid_t h5_create_group(const hid_t file_id, const char *cgroup);
#ifdef __cplusplus
}
#endif
#endif /* __GFAST_HDF5_H__ */
