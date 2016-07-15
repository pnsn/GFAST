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
    hvl_t srcDepths;
    hvl_t lsiteUsed;
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

#ifdef __cplusplus
extern "C"
{
#endif

int GFAST_HDF5_copyType__peakDisplacementData(enum data2h5_enum job,
                          struct GFAST_peakDisplacementData_struct *pgd_data,
                          struct h5_peakDisplacementData_struct *h5_pgd_data);
int GFAST_HDF5_copyType__pgdResults(enum data2h5_enum job,
                                    struct GFAST_pgdResults_struct *pgd,
                                    struct h5_pgdResults_struct *h5_pgd);
herr_t GFAST_HDF5_createType__peakDisplacementData(hid_t group_id);
herr_t GFAST_HDF5_createType__pgdResults(hid_t group_id);
herr_t GFAST_HDF5_createType__offsetData(hid_t group_id);

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_HDF5_H__ */
