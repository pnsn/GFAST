#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "gfast_hdf5.h"
#include "gfast_core.h"
#ifdef GFAST_USE_INTEL
#include <mkl_cblas.h>
#else
#include <cblas.h>
#endif
#include "iscl/memory/memory.h"

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

//============================================================================//
int hdf5_copyPeakDisplacementData(
    const enum data2h5_enum job,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct h5_peakDisplacementData_struct *h5_pgd_data)
{
    int *lactiveTemp, *lmaskTemp, i, ierr;
    char *stnTemp;
    char *ctemp;
    size_t nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        // Make sure there is something to do
        nsites = (size_t)  pgd_data->nsites;
        if (nsites < 1)
        {
            LOG_ERRMSG("%s", "No sites!");
            ierr = 1;
            return ierr;
        }

        h5_pgd_data->pd.len = nsites;
        h5_pgd_data->pd.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd_data->nsites, pgd_data->pd, 1, h5_pgd_data->pd.p, 1);

        h5_pgd_data->wt.len = nsites;
        h5_pgd_data->wt.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd_data->nsites, pgd_data->wt, 1, h5_pgd_data->wt.p, 1);

        h5_pgd_data->sta_lat.len = nsites;
        h5_pgd_data->sta_lat.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd_data->nsites, pgd_data->sta_lat, 1,
                    h5_pgd_data->sta_lat.p, 1);

        h5_pgd_data->sta_lon.len = nsites;
        h5_pgd_data->sta_lon.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd_data->nsites, pgd_data->sta_lon, 1,
                    h5_pgd_data->sta_lon.p, 1);

        h5_pgd_data->sta_alt.len = nsites;
        h5_pgd_data->sta_alt.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd_data->nsites, pgd_data->sta_alt, 1,
                    h5_pgd_data->sta_alt.p, 1);

        h5_pgd_data->stnm.len = nsites;
        ctemp = (char *)calloc(64*nsites, sizeof(char));

        h5_pgd_data->lactive.len = nsites;
        lactiveTemp = (int *)calloc(nsites, sizeof(int));

        h5_pgd_data->lmask.len = nsites;
        lmaskTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<(int) nsites; i++)
        {
            lactiveTemp[i] = pgd_data->lactive[i];
            lmaskTemp[i] = pgd_data->lmask[i];
            strcpy(&ctemp[i*64], pgd_data->stnm[i]);
            //printf("MTH*: pgd_data->stnm[%d]=%s\n", i, pgd_data->stnm[i]);
        }
        h5_pgd_data->stnm.p = ctemp;
        h5_pgd_data->lactive.p = lactiveTemp;
        h5_pgd_data->lmask.p = lmaskTemp;

        h5_pgd_data->nsites = (int) nsites;
    }
    else if (job == COPY_H5_TO_DATA)
    {
      printf("MTH: copy h5 peakDisplacementData to GFAST struct\n");
        memset(pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));

        // Make sure there is something to do
        nsites = (size_t)  h5_pgd_data->nsites;
      printf("MTH: copy h5 peakDisplacementData nsites=%d\n", nsites);
        if (nsites < 1)
        {
            LOG_ERRMSG("%s", "No sites!");
            ierr = 1;
            return ierr;
        }

        pgd_data->nsites = nsites;

        pgd_data->pd = memory_calloc64f(pgd_data->nsites);
        cblas_dcopy(pgd_data->nsites, h5_pgd_data->pd.p, 1, pgd_data->pd, 1);

        pgd_data->wt = memory_calloc64f(pgd_data->nsites);
        cblas_dcopy(pgd_data->nsites, h5_pgd_data->wt.p, 1, pgd_data->wt, 1);

        pgd_data->sta_lat = memory_calloc64f(pgd_data->nsites);
        cblas_dcopy(pgd_data->nsites, h5_pgd_data->sta_lat.p, 1, pgd_data->sta_lat, 1);

        pgd_data->sta_lon = memory_calloc64f(pgd_data->nsites);
        cblas_dcopy(pgd_data->nsites, h5_pgd_data->sta_lon.p, 1, pgd_data->sta_lon, 1);

        pgd_data->sta_alt = memory_calloc64f(pgd_data->nsites);
        cblas_dcopy(pgd_data->nsites, h5_pgd_data->sta_alt.p, 1, pgd_data->sta_alt, 1);

        //pgd_data->stnm = (char **)calloc(64*pgd_data->nsites, sizeof(char));
        //
        pgd_data->stnm = (char **)calloc((size_t)pgd_data->nsites, sizeof(char *));

        printf("stnm.p add:%u val:%s\n", h5_pgd_data->stnm.p, h5_pgd_data->stnm.p);
        stnTemp = (char *) h5_pgd_data->stnm.p;

        // (pgd_data->nsites, h5_pgd_data->stnm.p, 1, pgd_data->stnm, 1);

      printf("MTH: copy h5 peakDisplacementData lactive/lmask\n");
        lmaskTemp = (int *) h5_pgd_data->lmask.p;
        pgd_data->lmask = memory_calloc8l(pgd_data->nsites);
        for (i=0; i<pgd_data->nsites; i++)
        {
            pgd_data->lmask[i] = (bool) lmaskTemp[i];
            strcpy(pgd_data->stnm[i], stnTemp[i]);
        }
        lactiveTemp = (int *) h5_pgd_data->lactive.p;
        pgd_data->lactive = memory_calloc8l(pgd_data->nsites);
        for (i=0; i<pgd_data->nsites; i++)
        {
            pgd_data->lactive[i] = (bool) lactiveTemp[i];
        }

        /*
        stnTemp = (char *) h5_pgd_data->stnm.p;
        for (i=0; i<pgd_data->nsites; i++)
        {
          printf("stnm:%s\n", (char *) stnTemp[i]);
        }
        */

    }

    else
    {
        LOG_ERRMSG("Invalid job=%d", job);
        ierr = 1;
    }
    printf("MTH: copy h5 peakDisplacementData return ierr=%d\n", ierr);
    return ierr;
}
//============================================================================//
int hdf5_copyOffsetData(const enum data2h5_enum job,
                        struct GFAST_offsetData_struct *offset_data,
                        struct h5_offsetData_struct *h5_offset_data)
{
    char *ctemp;
    int *lactiveTemp, *lmaskTemp, i, ierr;
    size_t nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_offset_data, 0, sizeof(struct h5_offsetData_struct));
        // Make sure there is something to do
        nsites = (size_t) offset_data->nsites;
        if (nsites < 1)
        {
            if (nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            ierr = 1;
            return ierr;
        }

        h5_offset_data->ubuff.len = nsites;
        h5_offset_data->ubuff.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->ubuff, 1,
                    h5_offset_data->ubuff.p, 1); 

        h5_offset_data->nbuff.len = nsites;
        h5_offset_data->nbuff.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->nbuff, 1,
                    h5_offset_data->nbuff.p, 1);

        h5_offset_data->ebuff.len = nsites;
        h5_offset_data->ebuff.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->ebuff, 1,
                    h5_offset_data->ebuff.p, 1);

        h5_offset_data->wtu.len = nsites;
        h5_offset_data->wtu.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->wtu, 1,
                    h5_offset_data->wtu.p, 1); 

        h5_offset_data->wtn.len = nsites;
        h5_offset_data->wtn.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->wtn, 1,
                    h5_offset_data->wtn.p, 1);

        h5_offset_data->wte.len = nsites;
        h5_offset_data->wte.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->wte, 1,
                    h5_offset_data->wte.p, 1);

        h5_offset_data->sta_lat.len = nsites;
        h5_offset_data->sta_lat.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->sta_lat, 1,
                    h5_offset_data->sta_lat.p, 1);

        h5_offset_data->sta_lon.len = nsites;
        h5_offset_data->sta_lon.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->sta_lon, 1,
                    h5_offset_data->sta_lon.p, 1); 

        h5_offset_data->sta_alt.len = nsites;
        h5_offset_data->sta_alt.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(offset_data->nsites, offset_data->sta_alt, 1,
                    h5_offset_data->sta_alt.p, 1); 

        h5_offset_data->stnm.len = nsites;
        ctemp = (char *)calloc(64*nsites, sizeof(char));

        h5_offset_data->lactive.len = nsites;
        lactiveTemp = (int *)calloc(nsites, sizeof(int));

        h5_offset_data->lmask.len = nsites;
        lmaskTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<(int) nsites; i++)
        {
            lactiveTemp[i] = offset_data->lactive[i];
            lmaskTemp[i] = offset_data->lmask[i];
            strcpy(&ctemp[i*64], offset_data->stnm[i]);
        }
        h5_offset_data->stnm.p = ctemp;
        h5_offset_data->lactive.p = lactiveTemp;
        h5_offset_data->lmask.p = lmaskTemp;

        h5_offset_data->nsites = (int) nsites;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        LOG_ERRMSG("%s", "Error not yet done");
        ierr = 1;
    }
    else
    {
        LOG_ERRMSG("%s", "Invalid job");
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies PGD results structure to/from HDF5 PGD results structure
 *
 * @param[in] job         if job = COPY_DATA_TO_H5 then copy pgd -> h5_pgd.
 *                        if job = COPY_H5_TO_DATA then copy h5_pgd -> pgd.
 *
 * @param[in,out] pgd     if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to h5_pgd.
 *                        if job = COPY_H5_TO_DATA then on output this is the
 *                        copied h5_pgd structure.
 * @param[in,out] h5_pgd  if job = COPY_DATA_TO_H5 then on output this is the
 *                        HDF5 version of pgd.
 *                        if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to pgd.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyPGDResults(const enum data2h5_enum job,
                        struct GFAST_pgdResults_struct *pgd,
                        struct h5_pgdResults_struct *h5_pgd)
{
    int *lsiteUsedTemp, i, ierr, nlld;
    size_t ndeps, nlats, nlons, nloc, nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_pgd, 0, sizeof(struct h5_pgdResults_struct));
        // Make sure there is something to do
        ndeps = (size_t) pgd->ndeps;
        nsites = (size_t) pgd->nsites;
        nlats = (size_t) pgd->nlats;
        nlons = (size_t) pgd->nlons;
        if (ndeps < 1 || nsites < 1 || nlats < 1 || nlons < 1)
        {
            if (nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (ndeps < 1){LOG_ERRMSG("%s", "No depths!");}
            if (nlats < 1){LOG_ERRMSG("%s", "No lats!");}
            if (nlons < 1){LOG_ERRMSG("%s", "No lons!");}
            ierr = 1;
            return ierr;
        }
        nloc = ndeps*nlats*nlons;

        h5_pgd->mpgd.len = nloc;
        h5_pgd->mpgd.p = (double *)calloc(nloc, sizeof(double));
        cblas_dcopy((int) nloc, pgd->mpgd, 1, h5_pgd->mpgd.p, 1);

        h5_pgd->mpgd_vr.len = nloc;
        h5_pgd->mpgd_vr.p = (double *)calloc(nloc, sizeof(double));
        cblas_dcopy((int) nloc, pgd->mpgd_vr, 1, h5_pgd->mpgd_vr.p, 1);

        h5_pgd->dep_vr_pgd.len = nloc;
        h5_pgd->dep_vr_pgd.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy((int) nloc, pgd->dep_vr_pgd, 1, h5_pgd->dep_vr_pgd.p, 1);

        h5_pgd->UP.len = nsites*nloc; //ndeps;
        h5_pgd->UP.p = (double *)calloc(nsites*nloc, sizeof(double));
        cblas_dcopy((int) (nsites*nloc), pgd->UP, 1, h5_pgd->UP.p, 1);

        h5_pgd->srdist.len = nsites*nloc; //ndeps;
        h5_pgd->srdist.p = (double *)calloc(nsites*nloc, sizeof(double));
        cblas_dcopy((int) (nsites*nloc), pgd->srdist, 1,
                    h5_pgd->srdist.p, 1);

        h5_pgd->UPinp.len = nsites;
        h5_pgd->UPinp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd->nsites, pgd->UPinp, 1, h5_pgd->UPinp.p, 1);

        h5_pgd->srcDepths.len = ndeps;
        h5_pgd->srcDepths.p = (double *)calloc(ndeps, sizeof(double)); 
        cblas_dcopy(pgd->ndeps, pgd->srcDepths, 1, h5_pgd->srcDepths.p, 1);

        h5_pgd->iqr.len = nloc; //ndeps;
        h5_pgd->iqr.p = (double *)calloc(nloc, sizeof(double));
        cblas_dcopy((int) nloc, pgd->iqr, 1, h5_pgd->iqr.p, 1);

        h5_pgd->lsiteUsed.len = nsites;
        lsiteUsedTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<(int) nsites; i++)
        {
            lsiteUsedTemp[i] = (int) pgd->lsiteUsed[i];
        }
        h5_pgd->lsiteUsed.p = lsiteUsedTemp;

        h5_pgd->ndeps = (int) ndeps;
        h5_pgd->nsites = (int) nsites; 
        h5_pgd->nlats = pgd->nlats;
        h5_pgd->nlons = pgd->nlons;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
        // Make sure there is something to do
        pgd->ndeps = h5_pgd->ndeps;
        pgd->nsites = h5_pgd->nsites;
        pgd->nlats = h5_pgd->nlats;
        pgd->nlons = h5_pgd->nlons;
        if (pgd->ndeps < 1 || pgd->nsites < 1 ||
            pgd->nlats < 1 || pgd->nlons < 1)
        {
            if (pgd->nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (pgd->ndeps < 1){LOG_ERRMSG("%s", "No depths!");}
            if (pgd->nlats < 1){LOG_ERRMSG("%s", "No lats!");}
            if (pgd->nlons < 1){LOG_ERRMSG("%s", "No lons!");}
            ierr = 1;
            return ierr;
        }
        nlld = pgd->nlats*pgd->nlons*pgd->ndeps;

        pgd->mpgd = memory_calloc64f(nlld);
        cblas_dcopy(nlld, h5_pgd->mpgd.p, 1, pgd->mpgd, 1);

        pgd->mpgd_vr = memory_calloc64f(nlld);
        cblas_dcopy(nlld, h5_pgd->mpgd_vr.p, 1, pgd->mpgd_vr, 1);

        pgd->dep_vr_pgd = memory_calloc64f(nlld);
        cblas_dcopy(nlld, h5_pgd->dep_vr_pgd.p, 1, pgd->dep_vr_pgd, 1);

        pgd->UP = memory_calloc64f(pgd->nsites*nlld);
        cblas_dcopy(pgd->nsites*nlld, h5_pgd->UP.p, 1, pgd->UP, 1);

        pgd->srdist = memory_calloc64f(pgd->nsites*nlld);
        cblas_dcopy(pgd->nsites*nlld, h5_pgd->srdist.p, 1,
                    pgd->srdist, 1);

        pgd->UPinp = memory_calloc64f(pgd->nsites);
        cblas_dcopy(pgd->nsites, h5_pgd->UPinp.p, 1, pgd->UPinp, 1);

        pgd->srcDepths = memory_calloc64f(pgd->ndeps);
        cblas_dcopy(pgd->ndeps, h5_pgd->srcDepths.p, 1, pgd->srcDepths, 1);

        pgd->iqr = memory_calloc64f(nlld);
        cblas_dcopy(nlld, h5_pgd->iqr.p, 1, pgd->iqr, 1);

        lsiteUsedTemp = (int *) h5_pgd->lsiteUsed.p;
        pgd->lsiteUsed = memory_calloc8l(pgd->nsites);
        for (i=0; i<pgd->nsites; i++)
        {
            pgd->lsiteUsed[i] = (bool) lsiteUsedTemp[i];
        }
        lsiteUsedTemp = NULL;
    }
    else
    {
        LOG_ERRMSG("Invalid job=%d\n", job);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies hypocenter structure to/from HDF5 hypocenter structure
 *
 * @param[in] job          if job = COPY_DATA_TO_H5 then copy cmt -> h5_cmt.
 *                         if job = COPY_H5_TO_DATA then copy h5_cmt -> cmt.
 *
 * @param[in,out] hypo     if job = COPY_DATA_TO_H5 then on input this is the
 *                         structure to copy to h5_hypo.
 *                         if job = COPY_H5_TO_DATA then on output this is the
 *                         copied h5_hypo structure.
 * @param[in,out] h5_hypo  if job = COPY_DATA_TO_H5 then on output this is the
 *                         HDF5 version of hypo.
 *                         if job = COPY_DATA_TO_H5 then on input this is the
 *                         structure to copy to hypo.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyHypocenter(const enum data2h5_enum job,
                        struct GFAST_shakeAlert_struct *hypo,
                        struct h5_hypocenter_struct *h5_hypo)
{
    int ierr;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_hypo, 0, sizeof(struct h5_hypocenter_struct));
        strncpy(h5_hypo->eventid, hypo->eventid, 128*sizeof(char));
        h5_hypo->lat = hypo->lat;
        h5_hypo->lon = hypo->lon;
        h5_hypo->dep = hypo->dep;
        h5_hypo->mag = hypo->mag;
        h5_hypo->time = hypo->time;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(hypo, 0, sizeof(struct GFAST_shakeAlert_struct));
        memcpy(hypo->eventid, h5_hypo->eventid, 128*sizeof(char));
        hypo->lat = h5_hypo->lat;
        hypo->lon = h5_hypo->lon;
        hypo->dep = h5_hypo->dep;
        hypo->mag = h5_hypo->mag;
        hypo->time = h5_hypo->time;
    }
    else
    {
        LOG_ERRMSG("%s", "Invalid job");
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies CMT results structure to/from HDF5 CMT results structure
 *
 * @param[in] job         if job = COPY_DATA_TO_H5 then copy cmt -> h5_cmt.
 *                        if job = COPY_H5_TO_DATA then copy h5_cmt -> cmt.
 *
 * @param[in,out] cmt     if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to h5_cmt.
 *                        if job = COPY_H5_TO_DATA then on output this is the
 *                        copied h5_cmt structure.
 * @param[in,out] h5_cmt  if job = COPY_DATA_TO_H5 then on output this is the
 *                        HDF5 version of cmt.
 *                        if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to cmt.
 *
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyCMTResults(const enum data2h5_enum job,
                        struct GFAST_cmtResults_struct *cmt,
                        struct h5_cmtResults_struct *h5_cmt)
{
    int *lsiteUsedTemp, i, ierr, ncopy;
    size_t ndeps, nlats, nlons, nlld, nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_cmt, 0, sizeof(struct h5_cmtResults_struct));
        // Make sure there is something to do
        ndeps = (size_t) cmt->ndeps;
        nsites = (size_t) cmt->nsites;
        nlats = (size_t) cmt->nlats;
        nlons = (size_t) cmt->nlons;
        nlld = ndeps*nlats*nlons;
        ncopy = cmt->ndeps*cmt->nlats*cmt->nlons;
        if (ndeps < 1 || nsites < 1 || nlats < 1 || nlons < 1)
        {
            if (nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (ndeps < 1){LOG_ERRMSG("%s", "No depths!");}
            if (nlats < 1){LOG_ERRMSG("%s", "No lats!");}
            if (nlons < 1){LOG_ERRMSG("%s", "No lons!");}
            ierr = 1;
            return ierr;
        }
        h5_cmt->l2.len = nlld; //ndeps;
        h5_cmt->l2.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->l2, 1, h5_cmt->l2.p, 1);

        h5_cmt->pct_dc.len = nlld; //ndeps;
        h5_cmt->pct_dc.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->pct_dc, 1, h5_cmt->pct_dc.p, 1);

        h5_cmt->objfn.len = nlld; //ndeps;
        h5_cmt->objfn.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->objfn, 1, h5_cmt->objfn.p, 1);

        h5_cmt->mts.len = 6*nlld; //ndeps;
        h5_cmt->mts.p = (double *)calloc(6*nlld, sizeof(double));
        cblas_dcopy((int) (6*nlld), cmt->mts, 1, h5_cmt->mts.p, 1);

        h5_cmt->str1.len = nlld; //ndeps;
        h5_cmt->str1.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->str1, 1, h5_cmt->str1.p, 1);

        h5_cmt->str2.len = nlld; //ndeps;
        h5_cmt->str2.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->str2, 1, h5_cmt->str2.p, 1);

        h5_cmt->dip1.len = nlld; //ndeps;
        h5_cmt->dip1.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->dip1, 1, h5_cmt->dip1.p, 1);

        h5_cmt->dip2.len = nlld; //ndeps;
        h5_cmt->dip2.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->dip2, 1, h5_cmt->dip2.p, 1);

        h5_cmt->rak1.len = nlld; //ndeps;
        h5_cmt->rak1.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->rak1, 1, h5_cmt->rak1.p, 1);

        h5_cmt->rak2.len = nlld; //ndeps;
        h5_cmt->rak2.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->rak2, 1, h5_cmt->rak2.p, 1);

        h5_cmt->Mw.len = nlld; //ndeps;
        h5_cmt->Mw.p = (double *)calloc(nlld, sizeof(double));
        cblas_dcopy((int) nlld, cmt->Mw, 1, h5_cmt->Mw.p, 1);

        h5_cmt->srcDepths.len = ndeps;
        h5_cmt->srcDepths.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->srcDepths, 1, h5_cmt->srcDepths.p, 1); 

        h5_cmt->EN.len = nsites*nlld;
        h5_cmt->EN.p = (double *)calloc(nsites*nlld, sizeof(double));
        cblas_dcopy(cmt->nsites*ncopy, cmt->EN, 1, h5_cmt->EN.p, 1); 

        h5_cmt->NN.len = nsites*nlld;
        h5_cmt->NN.p = (double *)calloc(nsites*nlld, sizeof(double));
        cblas_dcopy(cmt->nsites*ncopy, cmt->NN, 1, h5_cmt->NN.p, 1);

        h5_cmt->UN.len = nsites*nlld;
        h5_cmt->UN.p = (double *)calloc(nsites*nlld, sizeof(double));
        cblas_dcopy(cmt->nsites*ncopy, cmt->UN, 1, h5_cmt->UN.p, 1);

        h5_cmt->Einp.len = nsites;
        h5_cmt->Einp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->Einp, 1, h5_cmt->Einp.p, 1); 

        h5_cmt->Ninp.len = nsites;
        h5_cmt->Ninp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->Ninp, 1, h5_cmt->Ninp.p, 1); 

        h5_cmt->Uinp.len = nsites;
        h5_cmt->Uinp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->Uinp, 1, h5_cmt->Uinp.p, 1);

        h5_cmt->lsiteUsed.len = nsites;
        lsiteUsedTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<(int) nsites; i++)
        {
            lsiteUsedTemp[i] = (int) cmt->lsiteUsed[i];
        }
        h5_cmt->lsiteUsed.p = lsiteUsedTemp;


        h5_cmt->opt_indx = cmt->opt_indx;
        h5_cmt->ndeps = (int) ndeps;
        h5_cmt->nsites = (int) nsites;
        h5_cmt->nlats = (int) nlats;
        h5_cmt->nlons = (int) nlons;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(cmt, 0, sizeof(struct GFAST_cmtResults_struct));
        // Make sure there is something to do
        cmt->ndeps = h5_cmt->ndeps;
        cmt->nsites = h5_cmt->nsites;
        cmt->nlats = h5_cmt->nlats;
        cmt->nlons = h5_cmt->nlons;
        if (cmt->ndeps < 1 || cmt->nsites < 1)
        {
            if (cmt->nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (cmt->ndeps < 1){LOG_ERRMSG("%s", "No depths!");}
            ierr = 1;
            return ierr;
        }
        ncopy = cmt->ndeps*cmt->nlats*cmt->nlons;
        cmt->l2 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->l2.p, 1, cmt->l2, 1);

        cmt->pct_dc = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->pct_dc.p, 1, cmt->pct_dc, 1);

        cmt->objfn = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->objfn.p, 1, cmt->objfn, 1);

        cmt->mts = memory_calloc64f(6*ncopy);
        cblas_dcopy(6*ncopy, h5_cmt->mts.p, 1, cmt->mts, 1);

        cmt->str1 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->str1.p, 1, cmt->str1, 1);

        cmt->str2 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->str2.p, 1, cmt->str2, 1);

        cmt->dip1 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->dip1.p, 1, cmt->dip1, 1);

        cmt->dip2 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->dip2.p, 1, cmt->dip2, 1);

        cmt->rak1 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->rak1.p, 1, cmt->rak1, 1);

        cmt->rak2 = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->rak2.p, 1, cmt->rak2, 1);

        cmt->Mw = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->Mw.p, 1, cmt->Mw, 1);

        cmt->srcDepths = memory_calloc64f(ncopy);
        cblas_dcopy(ncopy, h5_cmt->srcDepths.p, 1, cmt->srcDepths, 1);

        cmt->EN = memory_calloc64f(cmt->nsites*ncopy);
        cblas_dcopy(cmt->nsites*ncopy, h5_cmt->EN.p, 1, cmt->EN, 1); 

        cmt->NN = memory_calloc64f(cmt->nsites*ncopy);
        cblas_dcopy(cmt->nsites*ncopy, h5_cmt->NN.p, 1, cmt->NN, 1); 

        cmt->UN = memory_calloc64f(cmt->nsites*ncopy);
        cblas_dcopy(cmt->nsites*ncopy, h5_cmt->UN.p, 1, cmt->UN, 1); 

        cmt->Einp = memory_calloc64f(cmt->nsites);
        cblas_dcopy(cmt->nsites, h5_cmt->Einp.p, 1, cmt->Einp, 1); 

        cmt->Ninp = memory_calloc64f(cmt->nsites);
        cblas_dcopy(cmt->nsites, h5_cmt->Ninp.p, 1, cmt->Ninp, 1); 

        cmt->Uinp = memory_calloc64f(cmt->nsites);
        cblas_dcopy(cmt->nsites, h5_cmt->Uinp.p, 1, cmt->Uinp, 1); 

        lsiteUsedTemp = (int *) h5_cmt->lsiteUsed.p;
        cmt->lsiteUsed = memory_calloc8l(cmt->nsites);
        for (i=0; i<cmt->nsites; i++)
        {
            cmt->lsiteUsed[i] = (bool) lsiteUsedTemp[i];
        }
        lsiteUsedTemp = NULL;
    }
    else
    {
        LOG_ERRMSG("%s", "Invalid job");
        ierr = 1;
    }
    return ierr;
}
//============================================================================//

int hdf5_copyFaultPlane(const enum data2h5_enum job,
                        struct GFAST_faultPlane_struct *fp,
                        struct h5_faultPlane_struct *h5_fp)
{
    int *itemp, i, ierr;
    size_t nfp, nfp4, ndip, nsites, nstr;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_fp, 0, sizeof(struct h5_faultPlane_struct));
        // Make sure there is something to do
        nstr = (size_t) fp->nstr;
        ndip = (size_t) fp->ndip;
        nsites = (size_t) fp->maxobs; //(size_t) fp->nsites_used;
        if (nstr < 1 || ndip < 1 || nsites < 1)
        {
            if (nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (ndip < 1){LOG_ERRMSG("%s", "No faults down dip!");}
            if (nstr < 1){LOG_ERRMSG("%s", "No faults along strike!");}
            ierr = 1;
            return ierr;
        }
        nfp = nstr*ndip; // number of fault patches

        nfp4 = 4*nfp;
        h5_fp->lon_vtx.len = nfp4;
        h5_fp->lon_vtx.p = (double *)calloc(nfp4, sizeof(double));
        cblas_dcopy((int) nfp4, fp->lon_vtx, 1, h5_fp->lon_vtx.p, 1);

        h5_fp->lat_vtx.len = nfp4;
        h5_fp->lat_vtx.p = (double *)calloc(nfp4, sizeof(double));
        cblas_dcopy((int) nfp4, fp->lat_vtx, 1, h5_fp->lat_vtx.p, 1);

        h5_fp->dep_vtx.len = (size_t) (4*nfp);
        h5_fp->dep_vtx.p = (double *)calloc(nfp4, sizeof(double));
        cblas_dcopy((int) nfp4, fp->dep_vtx, 1, h5_fp->dep_vtx.p, 1);

        h5_fp->fault_xutm.len = nfp;
        h5_fp->fault_xutm.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->fault_xutm, 1, h5_fp->fault_xutm.p, 1);

        h5_fp->fault_yutm.len = nfp;
        h5_fp->fault_yutm.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->fault_yutm, 1, h5_fp->fault_yutm.p, 1);

        h5_fp->fault_alt.len = nfp;
        h5_fp->fault_alt.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->fault_alt, 1, h5_fp->fault_alt.p, 1);

        h5_fp->strike.len = nfp;
        h5_fp->strike.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->strike, 1, h5_fp->strike.p, 1);

        h5_fp->dip.len = nfp;
        h5_fp->dip.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->dip, 1, h5_fp->dip.p, 1);

        h5_fp->length.len = nfp;
        h5_fp->length.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->length, 1, h5_fp->length.p, 1);

        h5_fp->width.len = nfp;
        h5_fp->width.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->width, 1, h5_fp->width.p, 1);

        h5_fp->sslip.len = nfp;
        h5_fp->sslip.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->sslip, 1, h5_fp->sslip.p, 1);

        h5_fp->dslip.len = nfp;
        h5_fp->dslip.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->dslip, 1, h5_fp->dslip.p, 1);

        h5_fp->sslip_unc.len = nfp;
        h5_fp->sslip_unc.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->sslip_unc, 1, h5_fp->sslip_unc.p, 1);

        h5_fp->dslip_unc.len = nfp;
        h5_fp->dslip_unc.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, fp->dslip_unc, 1, h5_fp->dslip_unc.p, 1);

        h5_fp->EN.len = nsites;
        h5_fp->EN.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->EN, 1, h5_fp->EN.p, 1);

        h5_fp->NN.len = nsites;
        h5_fp->NN.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->NN, 1, h5_fp->NN.p, 1);

        h5_fp->UN.len = nsites;
        h5_fp->UN.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->UN, 1, h5_fp->UN.p, 1);

        h5_fp->Einp.len = nsites;
        h5_fp->Einp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->Einp, 1, h5_fp->Einp.p, 1);

        h5_fp->Ninp.len = nsites;
        h5_fp->Ninp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->Ninp, 1, h5_fp->Ninp.p, 1);

        h5_fp->Uinp.len = nsites;
        h5_fp->Uinp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy((int) nsites, fp->Uinp, 1, h5_fp->Uinp.p, 1);

        h5_fp->fault_ptr.len = nfp + 1;
        itemp = (int *)calloc(nfp+1, sizeof(int));
        for (i=0; i<(int) nfp+1; i++)
        {
            itemp[i] = fp->fault_ptr[i]; 
        } 
        h5_fp->fault_ptr.p = itemp;

        h5_fp->maxobs = fp->maxobs;
        h5_fp->nsites_used = fp->nsites_used;
        h5_fp->nstr = fp->nstr;
        h5_fp->ndip = fp->ndip;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(fp, 0, sizeof(struct GFAST_faultPlane_struct));
        fp->maxobs = h5_fp->maxobs;
        fp->nsites_used = h5_fp->nsites_used;
        fp->nstr = h5_fp->nstr;
        fp->ndip = h5_fp->ndip;
        // Make sure there is something to do
        nstr = (size_t) fp->nstr;
        ndip = (size_t) fp->ndip;
        nsites = (size_t) fp->maxobs; //(size_t) fp->nsites_used;
        if (nstr < 1 || ndip < 1 || nsites < 1) 
        {
            if (nsites < 1){LOG_ERRMSG("%s", "No sites!");}
            if (ndip < 1){LOG_ERRMSG("%s", "No faults down dip!");}
            if (nstr < 1){LOG_ERRMSG("%s", "No faults along strike!");}
            ierr = 1;
            return ierr;
        }
        nfp = nstr*ndip; // number of fault patches
        nfp4 = 4*nfp;
        fp->lon_vtx = memory_calloc64f((int) nfp4);
        cblas_dcopy((int) nfp4, h5_fp->lon_vtx.p, 1, fp->lon_vtx, 1);

        fp->lat_vtx = memory_calloc64f((int) nfp4);
        cblas_dcopy((int) nfp4, h5_fp->lat_vtx.p, 1, fp->lat_vtx, 1);

        fp->dep_vtx = memory_calloc64f((int) nfp4);
        cblas_dcopy((int) nfp4, h5_fp->dep_vtx.p, 1, fp->dep_vtx, 1);

        fp->fault_xutm = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->fault_xutm.p, 1, fp->fault_xutm, 1);

        fp->fault_yutm = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->fault_yutm.p, 1, fp->fault_yutm, 1);

        fp->fault_alt = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->fault_alt.p, 1, fp->fault_alt, 1);

        fp->strike = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->strike.p, 1, fp->strike, 1);

        fp->dip = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->dip.p, 1, fp->dip, 1);

        fp->length = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->length.p, 1, fp->length, 1);

        fp->width = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->width.p, 1, fp->width, 1);

        fp->sslip = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->sslip.p, 1, fp->sslip, 1);

        fp->dslip = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->dslip.p, 1, fp->dslip, 1);

        fp->sslip_unc = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->sslip_unc.p, 1, fp->sslip_unc, 1);

        fp->dslip_unc = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_fp->dslip_unc.p, 1, fp->dslip_unc, 1);

        fp->EN = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->EN.p, 1, fp->EN, 1);

        fp->NN = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->NN.p, 1, fp->NN, 1);

        fp->UN = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->UN.p, 1, fp->UN, 1);

        fp->Einp = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->Einp.p, 1, fp->Einp, 1);

        fp->Ninp = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->Ninp.p, 1, fp->Ninp, 1);

        fp->Uinp = memory_calloc64f((int) nsites);
        cblas_dcopy((int) nsites, h5_fp->Uinp.p, 1, fp->Uinp, 1);

        itemp = h5_fp->fault_ptr.p;
        fp->fault_ptr = memory_calloc32i((int) nfp+1);
        for (i=0; i<(int) nfp+1; i++) 
        {
            fp->fault_ptr[i] = itemp[i];
        }
        itemp = NULL;
    }
    else
    {
        LOG_ERRMSG("%s", "Invalid job");
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies finite fault results structure to/from HDF5 finite fault
 *        results structure.
 *
 * @param[in] job         If job = COPY_DATA_TO_H5 then copy ff -> h5_ff. <br>
 *                        If job = COPY_H5_TO_DATA then copy h5_ff -> ff.
 *
 * @param[in,out] ff      If job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to h5_ff. <br>
 *                        If job = COPY_H5_TO_DATA then on output this is the
 *                        copied h5_ff structure.
 * @param[in,out] h5_ff   If job = COPY_DATA_TO_H5 then on output this is the
 *                        HDF5 version of ff. <br>
 *                        If job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to ff.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyFFResults(const enum data2h5_enum job,
                       struct GFAST_ffResults_struct *ff,
                       struct h5_ffResults_struct *h5_ff)
{
    struct h5_faultPlane_struct *h5_fp = NULL;
    int i, ierr;
    size_t nfp;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_ff, 0, sizeof(struct h5_ffResults_struct));
        nfp = (size_t) ff->nfp;
        if (ff->nfp <= 0)
        {
            LOG_ERRMSG("%s", "Error no fault planes!");
            return -1;
        }

        h5_ff->fp.len = nfp;
        h5_fp = (struct h5_faultPlane_struct *)
                calloc(nfp,  sizeof(struct h5_faultPlane_struct));
        for (i=0; i<(int) nfp; i++)
        {
            ierr = GFAST_hdf5_copyFaultPlane(job, &ff->fp[i], &h5_fp[i]);
        }
        h5_ff->fp.p = h5_fp;

        h5_ff->vr.len = nfp;
        h5_ff->vr.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, ff->vr, 1, h5_ff->vr.p, 1); 

        h5_ff->Mw.len = nfp;
        h5_ff->Mw.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, ff->Mw, 1, h5_ff->Mw.p, 1);

        h5_ff->str.len = nfp;
        h5_ff->str.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, ff->str, 1, h5_ff->str.p, 1);

        h5_ff->dip.len = nfp;
        h5_ff->dip.p = (double *)calloc(nfp, sizeof(double));
        cblas_dcopy((int) nfp, ff->dip, 1, h5_ff->dip.p, 1);

        h5_ff->SA_lat = ff->SA_lat;
        h5_ff->SA_lon = ff->SA_lon;
        h5_ff->SA_dep = ff->SA_dep;
        h5_ff->SA_mag = ff->SA_mag;
        h5_ff->preferred_fault_plane = ff->preferred_fault_plane;
        h5_ff->nfp = ff->nfp;

    }   
    else if (job == COPY_H5_TO_DATA)
    {
        nfp = (size_t) h5_ff->nfp;
        if (nfp <= 0)
        {
            LOG_ERRMSG("%s", "Error no fault planes!");
            return -1;
        }

        ff->SA_lat = h5_ff->SA_lat;
        ff->SA_lon = h5_ff->SA_lon;
        ff->SA_dep = h5_ff->SA_dep;
        ff->SA_mag = h5_ff->SA_mag;
        ff->preferred_fault_plane = h5_ff->preferred_fault_plane;
        ff->nfp = h5_ff->nfp;

        ff->fp = (struct GFAST_faultPlane_struct *)
                 calloc(nfp,  sizeof(struct GFAST_faultPlane_struct));
        h5_fp = h5_ff->fp.p;
        for (i=0; i<(int) nfp; i++)
        {   
            ierr = GFAST_hdf5_copyFaultPlane(job, &ff->fp[i], &h5_fp[i]);
            if (ierr != 0)
            {
                LOG_ERRMSG("Error copying fault plane %d\n", i+1);
                return -1;
            }
        }
        h5_fp = NULL;

        ff->vr = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_ff->vr.p, 1, ff->vr, 1);  

        ff->Mw = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_ff->Mw.p, 1, ff->Mw, 1);

        ff->str = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_ff->str.p, 1, ff->str, 1);

        ff->dip = memory_calloc64f((int) nfp);
        cblas_dcopy((int) nfp, h5_ff->dip.p, 1, ff->dip, 1);
    }
    else
    {
        LOG_ERRMSG("Invalid job=%d", job);
        ierr = 1;
    }
    return ierr;
}

//============================================================================//
/*!
 * @brief Copies three component waveform data structure to/from HDF5
 *        three component waveform structure.
 *
 * @param[in] job           If job = COPY_DATA_TO_H5 then copy 
 *                          data -> h5_data. <br>
 *                          If job = COPY_H5_TO_DATA then copy h5_data -> data.
 *
 * @param[in,out] data      if job = COPY_DATA_TO_H5 then on input this is the
 *                          structure to copy to h5_data. <br>
 *                          if job = COPY_H5_TO_DATA then on output this is the
 *                          copied h5_data structure.
 * @param[in,out] h5_data   If job = COPY_DATA_TO_H5 then on output this is the
 *                          HDF5 version of data. <br>
 *                          If job = COPY_DATA_TO_H5 then on input this is the
 *                          structure to copy to data.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyWaveform3CData(const enum data2h5_enum job,
                            struct GFAST_waveform3CData_struct *data,
                            struct h5_waveform3CData_struct *h5_data)
{
    char *netw, *stnm, *chan, *loc;
    double nanv[1] = {(double) NAN};
    int ierr, npts;
    int i;
    size_t nalloc;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_data, 0, sizeof(struct h5_waveform3CData_struct));
        npts = data->npts;
        nalloc = (size_t) (MAX(npts, 1));
        h5_data->ubuff.p = (double *)calloc(nalloc, sizeof(double));
        h5_data->nbuff.p = (double *)calloc(nalloc, sizeof(double));
        h5_data->ebuff.p = (double *)calloc(nalloc, sizeof(double));
        h5_data->tbuff.p = (double *)calloc(nalloc, sizeof(double));
        h5_data->gain.p  = (double *)calloc(3, sizeof(double));
        h5_data->ubuff.len = nalloc;
        h5_data->nbuff.len = nalloc;
        h5_data->ebuff.len = nalloc;
        h5_data->tbuff.len = nalloc;
        h5_data->gain.len  = 3;
        if (npts > 0)
        {
          // MTH: test h5py nan read by removing them:
          for (i=0;i<npts;i++){
           data->ubuff[i] = 1.0;
           data->nbuff[i] = 1.0;
           data->ebuff[i] = 1.0;
          }
            cblas_dcopy(npts, data->ubuff, 1, h5_data->ubuff.p, 1);
            cblas_dcopy(npts, data->nbuff, 1, h5_data->nbuff.p, 1);
            cblas_dcopy(npts, data->ebuff, 1, h5_data->ebuff.p, 1);
            cblas_dcopy(npts, data->tbuff, 1, h5_data->tbuff.p, 1);
        }
        else
        {
            cblas_dcopy(1, nanv, 1, h5_data->ubuff.p, 1);
            cblas_dcopy(1, nanv, 1, h5_data->nbuff.p, 1);
            cblas_dcopy(1, nanv, 1, h5_data->ebuff.p, 1);
            cblas_dcopy(1, nanv, 1, h5_data->tbuff.p, 1);
        }
        cblas_dcopy(3, data->gain, 1, h5_data->gain.p, 1);
        h5_data->dt = data->dt;
        h5_data->sta_lat = data->sta_lat;
        h5_data->sta_lon = data->sta_lon;
        h5_data->sta_alt = data->sta_alt;
        h5_data->maxpts = data->maxpts;
        h5_data->npts   = data->npts;
        h5_data->lskip_pgd = data->lskip_pgd;
        h5_data->lskip_cmt = data->lskip_cmt;
        h5_data->lskip_ff  = data->lskip_ff;

        netw = (char *)calloc(64, sizeof(char));
        strcpy(netw, data->netw);
        h5_data->netw.len = 1;
        h5_data->netw.p = netw;

        stnm = (char *)calloc(64, sizeof(char));
        strcpy(stnm, data->stnm);
        h5_data->stnm.len = 1;
        h5_data->stnm.p = stnm;

        // MTH: h5py segfaults reading array of strings
        /*
        chan = (char *)calloc(3*64, sizeof(char));
        strcpy(&chan[0],   data->chan[0]);
        strcpy(&chan[64],  data->chan[1]);
        strcpy(&chan[128], data->chan[2]);
        h5_data->chan.len = 3;
        */
        chan = (char *)calloc(64, sizeof(char));
        strcpy(chan,   data->chan[0]);
        h5_data->chan.len = 1;
        h5_data->chan.p = chan;

        loc = (char *)calloc(64, sizeof(char));
        strcpy(loc, data->loc);
        h5_data->loc.len = 1;
        h5_data->loc.p = loc;

    }
    else if (job == COPY_H5_TO_DATA)
    {

        memset(data, 0, sizeof(struct GFAST_waveform3CData_struct));
        npts = h5_data->npts;
        nalloc = (size_t) (MAX(npts, 1)); 
        data->ubuff = memory_calloc64f((int) nalloc);
        data->nbuff = memory_calloc64f((int) nalloc);
        data->ebuff = memory_calloc64f((int) nalloc);
        data->tbuff = memory_calloc64f((int) nalloc);
        if (npts > 0) 
        {
            cblas_dcopy(npts, h5_data->ubuff.p, 1, data->ubuff, 1);
            cblas_dcopy(npts, h5_data->nbuff.p, 1, data->nbuff, 1);
            cblas_dcopy(npts, h5_data->ebuff.p, 1, data->ebuff, 1);
            cblas_dcopy(npts, h5_data->tbuff.p, 1, data->tbuff, 1);
        }
        else
        {
            cblas_dcopy(1, nanv, 1, data->ubuff, 1);
            cblas_dcopy(1, nanv, 1, data->nbuff, 1);
            cblas_dcopy(1, nanv, 1, data->ebuff, 1);
            cblas_dcopy(1, nanv, 1, data->tbuff, 1);
        }

        cblas_dcopy(3, h5_data->gain.p, 1, data->gain, 1);
        data->dt = h5_data->dt;
        data->sta_lat = h5_data->sta_lat;
        data->sta_lon = h5_data->sta_lon;
        data->sta_alt = h5_data->sta_alt;
        data->maxpts = h5_data->maxpts;
        data->npts   = h5_data->npts;
        data->lskip_pgd = h5_data->lskip_pgd;
        data->lskip_cmt = h5_data->lskip_cmt;
        data->lskip_ff  = h5_data->lskip_ff;

        netw = (char *) h5_data->netw.p;
        strcpy(data->netw, netw);
        netw = NULL;

        stnm = (char *) h5_data->stnm.p;
        strcpy(data->stnm, stnm);
        stnm = NULL;

        chan = h5_data->chan.p; 
        strcpy(data->chan[0], &chan[0]);
        strcpy(data->chan[1], &chan[64]);
        strcpy(data->chan[2], &chan[128]);
        chan = NULL;

        loc = (char *) h5_data->loc.p;
        strcpy(data->loc, loc);
        loc = NULL; 
    }
    else
    {
        LOG_ERRMSG("Invalid job=%d\n", job);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies GPS data structure to/from HDF5 GPS data structure 
 *
 * @param[in] job              if job = COPY_DATA_TO_H5 then copy
 *                             gps_data -> h5_gpsData.
 *                             if job = COPY_H5_TO_DATA then copy
 *                             h5_gpsData -> gps_data
 *
 * @param[in,out] gps_data     if job = COPY_DATA_TO_H5 then on input this is
 *                             the structure to copy to h5_gpsData.
 *                             if job = COPY_H5_TO_DATA then on output this is
 *                             the copied h5_gpsData structure.
 * @param[in,out] h5_gpsData   if job = COPY_DATA_TO_H5 then on output this is
 *                             the HDF5 version of gps_data.
 *                             if job = COPY_DATA_TO_H5 then on input this is
 *                             the structure to copy to gps_data.
 *
 * @result 0 indicates success 
 *
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copyGPSData(const enum data2h5_enum job,
                     struct GFAST_data_struct *gps_data,
                     struct h5_gpsData_struct *h5_gpsData)
{
    struct h5_waveform3CData_struct *h5_data;
    int ierr, k, nstreams;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {   
        memset(h5_gpsData, 0, sizeof(struct h5_gpsData_struct));
        nstreams = gps_data->stream_length;
        if (nstreams < 1 || gps_data->data == NULL)
        {
            LOG_ERRMSG("%s", "Error no streams to copy!");
            ierr = 1;
        }
        h5_data = (struct h5_waveform3CData_struct *)
                  calloc((size_t) nstreams,
                         sizeof(struct h5_waveform3CData_struct));
        h5_gpsData->stream_length = nstreams;
        for (k=0; k<nstreams; k++)
        {
            ierr = GFAST_hdf5_copyWaveform3CData(job,
                                                 &gps_data->data[k],
                                                 &h5_data[k]); 
            // h5_data[].npts == h5_data[].ubuff.len
            /*
            printf("MTH: h5_data[%d] stnm=%s chan=%s sta_lat:%8.3f sta_lon:%8.3f npts:%d ubuff.len:%d ubuff[last]:%9.6f\n", 
                k, h5_data[k].stnm.p, h5_data[k].chan.p, h5_data[k].sta_lat, h5_data[k].sta_lon,
                h5_data[k].npts, h5_data[k].ubuff.len, h5_data[k].ubuff.p[h5_data[k].npts-1]);
            */
            if (ierr != 0)
            {
                LOG_ERRMSG("%s", "Error copying 3C data");
                return ierr;
            }
        }
        h5_gpsData->data.p = h5_data;
        h5_gpsData->data.len = (size_t) nstreams;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(gps_data, 0, sizeof(struct GFAST_data_struct));
        nstreams = h5_gpsData->stream_length;
        gps_data->stream_length = nstreams;
        if (nstreams < 1)
        {
            LOG_ERRMSG("%s", "Error no streams to copy!");
            ierr = 1;
        }
        gps_data->data = (struct GFAST_waveform3CData_struct *)
                         calloc((size_t) nstreams,
                                sizeof(struct GFAST_waveform3CData_struct));
        h5_data = (struct h5_waveform3CData_struct *) h5_gpsData->data.p;
        for (k=0; k<nstreams; k++)
        {
            ierr = GFAST_hdf5_copyWaveform3CData(job,
                                                 &gps_data->data[k],
                                                 &h5_data[k]);
        }
        h5_data = NULL;
    }
    else
    {
        LOG_ERRMSG("Invalid job=%d\n", job);
        ierr = 1;
    }
    return ierr;
}
