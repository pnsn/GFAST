#ifndef _gfast_hdf5_h__
#define _gfast_hdf5_h__ 1
#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <hdf5.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_struct.h"
#include "gfast_enum.h"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

enum data2h5_enum
{
    COPY_DATA_TO_H5 = 0,  /*!< Copies data structure to HDF5 structure */
    COPY_H5_TO_DATA = 1,  /*!< Copies HDF5 structure to data structure */
};

struct h5_waveform3CData_struct
{
    hvl_t netw;
    hvl_t stnm;
    hvl_t chan;
    hvl_t loc;
    hvl_t ubuff;
    hvl_t nbuff;
    hvl_t ebuff;
    hvl_t tbuff;
    hvl_t gain;
    double dt;
    double sta_lat;
    double sta_lon;
    double sta_alt;
    int maxpts;
    int npts;
    int lskip_pgd;
    int lskip_cmt;
    int lskip_ff; 
};

struct h5_gpsData_struct
{
    hvl_t data;
    int stream_length;
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
    int nsites;
};

struct h5_pgdResults_struct
{
    hvl_t mpgd;
    hvl_t mpgd_vr;
    hvl_t dep_vr_pgd;
    hvl_t UP;
    hvl_t UPinp;
    hvl_t srcDepths;
    hvl_t srdist;
    hvl_t iqr;
    hvl_t lsiteUsed;
    int ndeps;
    int nsites;
    int nlats;
    int nlons;
};

struct h5_cmtResults_struct
{
    hvl_t l2;
    hvl_t pct_dc;
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
    int nlats;
    int nlons;
};

struct h5_offsetData_struct
{
    hvl_t stnm;
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

struct h5_hypocenter_struct
{
    char eventid[128]; /*!< Event ID */
    double lat;        /*!< Event latitude (degrees) */
    double lon;        /*!< Event longitude (degrees) */
    double dep;        /*!< Event depth (kilometers) */ 
    double mag;        /*!< Event magnitude */
    double time;       /*!< Event epochal time (s) */
};

#ifdef __cplusplus
extern "C"
{
#endif


int hdf5_copyCMTResults(const enum data2h5_enum job,
                        struct GFAST_cmtResults_struct *cmt,
                        struct h5_cmtResults_struct *h5_cmt);
int hdf5_copyFaultPlane(const enum data2h5_enum job,
                        struct GFAST_faultPlane_struct *fp,
                        struct h5_faultPlane_struct *h5_fp);
int hdf5_copyFFResults(const enum data2h5_enum job,
                       struct GFAST_ffResults_struct *ff,
                       struct h5_ffResults_struct *h5_ff);
int hdf5_copyGPSData(const enum data2h5_enum job,
                     struct GFAST_data_struct *gps_data,
                     struct h5_gpsData_struct *h5_gpsData);
int hdf5_copyHypocenter(const enum data2h5_enum job,
                        struct GFAST_shakeAlert_struct *hypo,
                        struct h5_hypocenter_struct *h5_hypo);
int hdf5_copyOffsetData(const enum data2h5_enum job,
                        struct GFAST_offsetData_struct *offset_data,
                        struct h5_offsetData_struct *h5_offset_data);
int hdf5_copyPeakDisplacementData(
    const enum data2h5_enum job,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct h5_peakDisplacementData_struct *h5_pgd_data);
int hdf5_copyPGDResults(const enum data2h5_enum job,
                        struct GFAST_pgdResults_struct *pgd,
                        struct h5_pgdResults_struct *h5_pgd);
int hdf5_copyWaveform3CData(const enum data2h5_enum job,
                            struct GFAST_waveform3CData_struct *data,
                            struct h5_waveform3CData_struct *h5_data);

herr_t hdf5_createType_cmtResults(hid_t group_id);
herr_t hdf5_createType_faultPlane(hid_t group_id);
herr_t hdf5_createType_ffResults(hid_t group_id);
herr_t hdf5_createType_gpsData(hid_t group_id);
herr_t hdf5_createType_hypocenter(hid_t group_id);
herr_t hdf5_createType_offsetData(hid_t group_id);
herr_t hdf5_createType_peakDisplacementData(hid_t group_id);
herr_t hdf5_createType_pgdResults(hid_t group_id);
herr_t hdf5_createType_waveform3CData(hid_t group_id);

int hdf5_getMaxGroupNumber(const hid_t h5fl);

int hdf5_initialize(const char *adir,
                    const char *evid,
                    const char *propfilename);


int hdf5_memory_freeCMTResults(struct h5_cmtResults_struct *cmt);
int hdf5_memory_freeFaultPlane(struct h5_faultPlane_struct *fp);
int hdf5_memory_freeFFResults(struct h5_ffResults_struct *ff);
int hdf5_memory_freeGPSData(struct h5_gpsData_struct *gpsData);
int hdf5_memory_freeOffsetData(
    struct h5_offsetData_struct *h5_offset_data);
int hdf5_memory_freePGDData(
    struct h5_peakDisplacementData_struct *h5_pgd_data);
int hdf5_memory_freePGDResults(struct h5_pgdResults_struct *pgd);
int hdf5_memory_freeWaveform3CData(struct h5_waveform3CData_struct *data);

int hdf5_setFileName(const char *adir,
                     const char *evid, 
                     char fname[PATH_MAX]);

int hdf5_updateCMT(const char *adir,
                   const char *evid,
                   const int h5k,
                   struct GFAST_offsetData_struct cmt_data,
                   struct GFAST_cmtResults_struct cmt);
int hdf5_updateFF(const char *adir,
                  const char *evid,
                  const int h5k,
                  struct GFAST_ffResults_struct ff);
int hdf5_update_gpsData(const char *adir,
                        const char *evid,
                        const int h5k, 
                        struct GFAST_data_struct data);
int hdf5_updateXMLMessage(const char *adir,
                          const char *evid,
                          const int h5k,
                          const char *messageName, char *message);
int hdf5_updateHypocenter(const char *adir,
                          const char *evid,
                          const int h5k,
                          struct GFAST_shakeAlert_struct hypo);
int hdf5_updateGetIteration(const char *adir,
                            const char *evid,
                            const double epoch);
int hdf5_updatePGD(const char *adir,
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
                          const int n, char **c);
int h5_read_array__double(const char *dset_name, const hid_t file_id,
                          const int nref, double *x);
char **h5_read_array__string(const char *citem, const hid_t file_id,
                             int *nitems, int *ierr);
int h5_read_array__float(const char *dset_name, const hid_t file_id,
                         const int nref, float *x);
int h5_read_array__int(const char *dset_name, const hid_t file_id,
                       const int nref, int *x);
char **h5_read_array__char(const char *citem, const hid_t file_id,
                           int *nitems, int *ierr);
int h5_write_attribute__double(const char *citem, const hid_t hdf5_id,
                               const int n, const double *attr_data);
int h5_write_attribute__int(const char *citem, const hid_t hdf5_id,
                            const int n, const int *attr_data);
int h5_write_attribute__char(const char *citem, const hid_t hdf5_id,
                             const int n, const char **cattr);
int h5_n_group_members(const char *group_name, const hid_t file_id);
int h5_get_array_size(const hid_t file_id, const char *citem);
bool h5_item_exists(const hid_t file_id, const char *citem_in);
herr_t h5_create_group(const hid_t file_id, const char *cgroup);

#define GFAST_hdf5_setFileName(...)       \
              hdf5_setFileName(__VA_ARGS__)

#define GFAST_hdf5_copyCMTResults(...)       \
              hdf5_copyCMTResults(__VA_ARGS__)
#define GFAST_hdf5_copyFaultPlane(...)       \
              hdf5_copyFaultPlane(__VA_ARGS__)
#define GFAST_hdf5_copyFFResults(...)       \
              hdf5_copyFFResults(__VA_ARGS__)
#define GFAST_hdf5_copyGPSData(...)       \
              hdf5_copyGPSData(__VA_ARGS__)
#define GFAST_hdf5_copyHypocenter(...)       \
              hdf5_copyHypocenter(__VA_ARGS__)
#define GFAST_hdf5_copyOffsetData(...)       \
              hdf5_copyOffsetData(__VA_ARGS__)
#define GFAST_hdf5_copyPeakDisplacementData(...)       \
              hdf5_copyPeakDisplacementData(__VA_ARGS__)
#define GFAST_hdf5_copyPGDResults(...)       \
              hdf5_copyPGDResults(__VA_ARGS__)
#define GFAST_hdf5_copyWaveform3CData(...)       \
              hdf5_copyWaveform3CData(__VA_ARGS__)

#define GFAST_hdf5_createType_cmtResults(...)       \
              hdf5_createType_cmtResults(__VA_ARGS__)
#define GFAST_hdf5_createType_faultPlane(...)       \
              hdf5_createType_faultPlane(__VA_ARGS__)
#define GFAST_hdf5_createType_ffResults(...)       \
              hdf5_createType_ffResults(__VA_ARGS__)
#define GFAST_hdf5_createType_gpsData(...)       \
              hdf5_createType_gpsData(__VA_ARGS__)
#define GFAST_hdf5_createType_hypocenter(...)       \
              hdf5_createType_hypocenter(__VA_ARGS__)
#define GFAST_hdf5_createType_offsetData(...)       \
              hdf5_createType_offsetData(__VA_ARGS__)
#define GFAST_hdf5_createType_peakDisplacementData(...)       \
              hdf5_createType_peakDisplacementData(__VA_ARGS__)
#define GFAST_hdf5_createType_pgdResults(...)       \
              hdf5_createType_pgdResults(__VA_ARGS__)
#define GFAST_hdf5_createType_waveform3CData(...)       \
              hdf5_createType_waveform3CData(__VA_ARGS__)

#define GFAST_hdf5_initialize(...)       \
              hdf5_initialize(__VA_ARGS__)
#define GFAST_hdf5_memory_freeCMTResults(...)       \
              hdf5_memory_freeCMTResults(__VA_ARGS__)
#define GFAST_hdf5_memory_freeGPSData(...)       \
              hdf5_memory_freeGPSData(__VA_ARGS__)
#define GFAST_hdf5_memory_freePGDData(...)       \
              hdf5_memory_freePGDData(__VA_ARGS__)
#define GFAST_hdf5_memory_freePGDResults(...)       \
              hdf5_memory_freePGDResults(__VA_ARGS__)
#define GFAST_hdf5_memory_freeFaultPlane(...)       \
              hdf5_memory_freeFaultPlane(__VA_ARGS__)
#define GFAST_hdf5_memory_freeFFResults(...)       \
              hdf5_memory_freeFFResults(__VA_ARGS__)
#define GFAST_hdf5_memory_freeOffsetData(...)       \
              hdf5_memory_freeOffsetData(__VA_ARGS__)
#define GFAST_hdf5_memory_freeWaveform3CData(...)       \
              hdf5_memory_freeWaveform3CData(__VA_ARGS__)
#define GFAST_hdf5_updateCMT(...)       \
              hdf5_updateCMT(__VA_ARGS__)
#define GFAST_hdf5_updateFF(...)       \
              hdf5_updateFF(__VA_ARGS__)
#define GFAST_hdf5_updateGetIteration(...)            \
              hdf5_updateGetIteration(__VA_ARGS__)
#define GFAST_hdf5_update_gpsData(...)       \
              hdf5_update_gpsData(__VA_ARGS__)
#define GFAST_hdf5_updateHypocenter(...)       \
              hdf5_updateHypocenter(__VA_ARGS__)
#define GFAST_hdf5_updatePGD(...)            \
              hdf5_updatePGD(__VA_ARGS__)


#ifdef __cplusplus
}
#endif
#endif /* _gfast_hdf5_h__ */
