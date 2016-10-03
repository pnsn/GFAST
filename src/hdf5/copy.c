#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <cblas.h>
#include "gfast_hdf5.h"
#include "iscl/log/log.h"

//============================================================================//
int hdf5_copy__peakDisplacementData(
    const enum data2h5_enum job,
    struct GFAST_peakDisplacementData_struct *pgd_data,
    struct h5_peakDisplacementData_struct *h5_pgd_data)
{
    const char *fcnm = "hdf5_copyPeakDisplacementData\0";
    int *lactiveTemp, *lmaskTemp, i, ierr;
    char *ctemp;
    unsigned long nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        // Make sure there is something to do
        nsites = (unsigned long)  pgd_data->nsites;
        if (nsites < 1)
        {
            log_errorF("%s: No sites!\n", fcnm);
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
        }
        h5_pgd_data->stnm.p = ctemp;
        h5_pgd_data->lactive.p = lactiveTemp;
        h5_pgd_data->lmask.p = lmaskTemp;

        h5_pgd_data->nsites = (int) nsites;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        printf("%s: Not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        printf("%s: Invalid job\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
int hdf5_copy__offsetData(const enum data2h5_enum job,
                          struct GFAST_offsetData_struct *offset_data,
                          struct h5_offsetData_struct *h5_offset_data)
{
    const char *fcnm = "hdf5_copy__offsetData\0";
    char *ctemp;
    int *lactiveTemp, *lmaskTemp, i, ierr;
    unsigned long nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_offset_data, 0, sizeof(struct h5_offsetData_struct));
        // Make sure there is something to do
        nsites = (unsigned long) offset_data->nsites;
        if (nsites < 1)
        {
            if (nsites < 1){log_errorF("%s: No sites!\n", fcnm);}
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
        log_errorF("%s: Error not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
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
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int hdf5_copy__pgdResults(const enum data2h5_enum job,
                          struct GFAST_pgdResults_struct *pgd,
                          struct h5_pgdResults_struct *h5_pgd)
{
    const char *fcnm = "hdf5_copyPGDResults\0";
    int *lsiteUsedTemp, i, ierr;
    unsigned long ndeps, nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_pgd, 0, sizeof(struct h5_pgdResults_struct));
        // Make sure there is something to do
        ndeps = (unsigned long) pgd->ndeps;
        nsites = (unsigned long) pgd->nsites;
        if (ndeps < 1 || nsites < 1)
        {
            if (nsites < 1){log_errorF("%s: No sites!\n", fcnm);}
            if (ndeps < 1){log_errorF("%s: No depths!\n", fcnm);}
            ierr = 1;
            return ierr;
        }

        h5_pgd->mpgd.len = ndeps;
        h5_pgd->mpgd.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(pgd->ndeps, pgd->mpgd, 1, h5_pgd->mpgd.p, 1);

        h5_pgd->mpgd_vr.len = ndeps;
        h5_pgd->mpgd_vr.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(pgd->ndeps, pgd->mpgd_vr, 1, h5_pgd->mpgd_vr.p, 1);

        h5_pgd->dep_vr_pgd.len = ndeps;
        h5_pgd->dep_vr_pgd.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(pgd->ndeps, pgd->dep_vr_pgd, 1, h5_pgd->dep_vr_pgd.p, 1);

        h5_pgd->UP.len = nsites*ndeps;
        h5_pgd->UP.p = (double *)calloc(nsites*ndeps, sizeof(double));
        cblas_dcopy(pgd->nsites*pgd->ndeps, pgd->UP, 1, h5_pgd->UP.p, 1);

        h5_pgd->srdist.len = nsites*ndeps;
        h5_pgd->srdist.p = (double *)calloc(nsites*ndeps, sizeof(double));
        cblas_dcopy(pgd->nsites*pgd->ndeps, pgd->srdist, 1,
                    h5_pgd->srdist.p, 1);

        h5_pgd->UPinp.len = nsites;
        h5_pgd->UPinp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(pgd->nsites, pgd->UPinp, 1, h5_pgd->UPinp.p, 1);

        h5_pgd->srcDepths.len = ndeps;
        h5_pgd->srcDepths.p = (double *)calloc(ndeps, sizeof(double)); 
        cblas_dcopy(pgd->ndeps, pgd->srcDepths, 1, h5_pgd->srcDepths.p, 1);

        h5_pgd->iqr.len = ndeps;
        h5_pgd->iqr.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(pgd->ndeps, pgd->iqr, 1, h5_pgd->iqr.p, 1);

        h5_pgd->lsiteUsed.len = nsites;
        lsiteUsedTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<(int) nsites; i++)
        {
            lsiteUsedTemp[i] = (int) pgd->lsiteUsed[i];
        }
        h5_pgd->lsiteUsed.p = lsiteUsedTemp;

        h5_pgd->ndeps = (int) ndeps;
        h5_pgd->nsites = (int) nsites; 
    }
    else if (job == COPY_H5_TO_DATA)
    {
        log_errorF("%s: Error not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
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
 * @author Ben Baker, ISTI
 *
 */
int hdf5_copy__hypocenter(const enum data2h5_enum job,
                          struct GFAST_shakeAlert_struct *hypo,
                          struct h5_hypocenter_struct *h5_hypo)
{
    const char *fcnm = "hdf5_copy__hypocenter\0";
    int ierr;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_hypo, 0, sizeof(struct h5_hypocenter_struct));
        strncpy(h5_hypo->eventid, hypo->eventid, 128);
        h5_hypo->lat = hypo->lat;
        h5_hypo->lon = hypo->lon;
        h5_hypo->dep = hypo->dep;
        h5_hypo->mag = hypo->mag;
        h5_hypo->time = hypo->time;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        memset(hypo, 0, sizeof(struct GFAST_shakeAlert_struct));
        memcpy(hypo->eventid, h5_hypo->eventid, 128);
        hypo->lat = h5_hypo->lat;
        hypo->lon = h5_hypo->lon;
        hypo->dep = h5_hypo->dep;
        hypo->mag = h5_hypo->mag;
        hypo->time = h5_hypo->time;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
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
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int hdf5_copy__cmtResults(const enum data2h5_enum job,
                          struct GFAST_cmtResults_struct *cmt,
                          struct h5_cmtResults_struct *h5_cmt)
{
    const char *fcnm = "hdf5_copy__cmtResults\0";
    int *lsiteUsedTemp, i, ierr;
    unsigned long ndeps, nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_cmt, 0, sizeof(struct h5_cmtResults_struct));
        // Make sure there is something to do
        ndeps = (unsigned long) cmt->ndeps;
        nsites = (unsigned long) cmt->nsites;
        if (ndeps < 1 || nsites < 1)
        {
            if (nsites < 1){log_errorF("%s: No sites!\n", fcnm);}
            if (ndeps < 1){log_errorF("%s: No depths!\n", fcnm);}
            ierr = 1;
            return ierr;
        }
        h5_cmt->l2.len = ndeps;
        h5_cmt->l2.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->l2, 1, h5_cmt->l2.p, 1);

        h5_cmt->pct_dc.len = ndeps;
        h5_cmt->pct_dc.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->pct_dc, 1, h5_cmt->pct_dc.p, 1);

        h5_cmt->objfn.len = ndeps;
        h5_cmt->objfn.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->objfn, 1, h5_cmt->objfn.p, 1);

        h5_cmt->mts.len = 6*ndeps;
        h5_cmt->mts.p = (double *)calloc(6*ndeps, sizeof(double));
        cblas_dcopy(6*cmt->ndeps, cmt->mts, 1, h5_cmt->mts.p, 1);

        h5_cmt->str1.len = ndeps;
        h5_cmt->str1.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->str1, 1, h5_cmt->str1.p, 1);

        h5_cmt->str2.len = ndeps;
        h5_cmt->str2.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->str2, 1, h5_cmt->str2.p, 1);

        h5_cmt->dip1.len = ndeps;
        h5_cmt->dip1.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->dip1, 1, h5_cmt->dip1.p, 1);

        h5_cmt->dip2.len = ndeps;
        h5_cmt->dip2.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->dip2, 1, h5_cmt->dip2.p, 1);

        h5_cmt->rak1.len = ndeps;
        h5_cmt->rak1.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->rak1, 1, h5_cmt->rak1.p, 1);

        h5_cmt->rak2.len = ndeps;
        h5_cmt->rak2.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->rak2, 1, h5_cmt->rak2.p, 1);

        h5_cmt->Mw.len = ndeps;
        h5_cmt->Mw.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->Mw, 1, h5_cmt->Mw.p, 1);

        h5_cmt->srcDepths.len = ndeps;
        h5_cmt->srcDepths.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(cmt->ndeps, cmt->srcDepths, 1, h5_cmt->srcDepths.p, 1); 

        h5_cmt->EN.len = nsites*ndeps;
        h5_cmt->EN.p = (double *)calloc(nsites*ndeps, sizeof(double));
        cblas_dcopy(cmt->nsites*cmt->ndeps, cmt->EN, 1, h5_cmt->EN.p, 1); 

        h5_cmt->NN.len = nsites*ndeps;
        h5_cmt->NN.p = (double *)calloc(nsites*ndeps, sizeof(double));
        cblas_dcopy(cmt->nsites*cmt->ndeps, cmt->NN, 1, h5_cmt->NN.p, 1);

        h5_cmt->UN.len = nsites*ndeps;
        h5_cmt->UN.p = (double *)calloc(nsites*ndeps, sizeof(double));
        cblas_dcopy(cmt->nsites*cmt->ndeps, cmt->UN, 1, h5_cmt->UN.p, 1);

        h5_cmt->Einp.len = nsites;
        h5_cmt->Einp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->EN, 1, h5_cmt->Einp.p, 1); 

        h5_cmt->Ninp.len = nsites;
        h5_cmt->Ninp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->NN, 1, h5_cmt->Ninp.p, 1); 

        h5_cmt->Uinp.len = nsites;
        h5_cmt->Uinp.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(cmt->nsites, cmt->UN, 1, h5_cmt->Uinp.p, 1);

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
    }
    else if (job == COPY_H5_TO_DATA)
    {
        log_errorF("%s: Error not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//

int hdf5_copy__faultPlane(const enum data2h5_enum job,
                          struct GFAST_faultPlane_struct *fp,
                          struct h5_faultPlane_struct *h5_fp)
{
    const char *fcnm = "hdf5_copy__faultPlane\0";
    int *itemp, i, ierr, ndip, nfp, nsites, nstr;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_fp, 0, sizeof(struct h5_faultPlane_struct));
        // Make sure there is something to do
        nstr = fp->nstr;
        ndip = fp->ndip;
        nsites = fp->nsites_used;
        if (ndip < 1 || nsites < 1 || nstr < 1)
        {
            if (nsites < 1){log_errorF("%s: No sites!\n", fcnm);}
            if (ndip < 1){log_errorF("%s: No faults down dip!\n", fcnm);}
            if (nstr < 1){log_errorF("%s: No faults along strike!\n", fcnm);}
            ierr = 1;
            return ierr;
        }
        nfp = nstr*ndip; // number of fault patches

        h5_fp->lon_vtx.len = (unsigned long) (4*nfp);
        h5_fp->lon_vtx.p = (double *)
                           calloc((unsigned long) (4*nfp), sizeof(double));
        cblas_dcopy(4*nfp, fp->lon_vtx, 1, h5_fp->lon_vtx.p, 1);

        h5_fp->lat_vtx.len = (unsigned long) (4*nfp);
        h5_fp->lat_vtx.p = (double *)
                           calloc((unsigned long) (4*nfp), sizeof(double));
        cblas_dcopy(4*nfp, fp->lat_vtx, 1, h5_fp->lat_vtx.p, 1);

        h5_fp->dep_vtx.len = (unsigned long) (4*nfp);
        h5_fp->dep_vtx.p = (double *)
                           calloc((unsigned long) (4*nfp), sizeof(double));
        cblas_dcopy(4*nfp, fp->dep_vtx, 1, h5_fp->dep_vtx.p, 1);

        h5_fp->fault_xutm.len = (unsigned long) nfp;
        h5_fp->fault_xutm.p = (double *)
                              calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->fault_xutm, 1, h5_fp->fault_xutm.p, 1);

        h5_fp->fault_yutm.len = (unsigned long) nfp;
        h5_fp->fault_yutm.p = (double *)
                              calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->fault_yutm, 1, h5_fp->fault_yutm.p, 1);

        h5_fp->fault_alt.len = (unsigned long) nfp;
        h5_fp->fault_alt.p = (double *)
                             calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->fault_alt, 1, h5_fp->fault_alt.p, 1);

        h5_fp->strike.len = (unsigned long) nfp;
        h5_fp->strike.p = (double *)calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->strike, 1, h5_fp->strike.p, 1);

        h5_fp->dip.len = (unsigned long) nfp;
        h5_fp->dip.p = (double *)
                       calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->dip, 1, h5_fp->dip.p, 1);

        h5_fp->length.len = (unsigned long) nfp;
        h5_fp->length.p = (double *)
                          calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->length, 1, h5_fp->length.p, 1);

        h5_fp->width.len = (unsigned long) nfp;
        h5_fp->width.p = (double *)
                         calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->width, 1, h5_fp->width.p, 1);

        h5_fp->sslip.len = (unsigned long) nfp;
        h5_fp->sslip.p = (double *)
                         calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->sslip, 1, h5_fp->sslip.p, 1);

        h5_fp->dslip.len = (unsigned long) nfp;
        h5_fp->dslip.p = (double *)
                         calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->dslip, 1, h5_fp->dslip.p, 1);

        h5_fp->sslip_unc.len = (unsigned long) nfp;
        h5_fp->sslip_unc.p = (double *)
                             calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->sslip_unc, 1, h5_fp->sslip_unc.p, 1);

        h5_fp->dslip_unc.len = (unsigned long) nfp;
        h5_fp->dslip_unc.p = (double *)
                             calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, fp->dslip_unc, 1, h5_fp->dslip_unc.p, 1);

        h5_fp->EN.len = (unsigned long) nsites;
        h5_fp->EN.p = (double *)
                      calloc((unsigned long) nsites, sizeof(double));
        cblas_dcopy(nsites, fp->EN, 1, h5_fp->EN.p, 1);

        h5_fp->NN.len = (unsigned long) nsites;
        h5_fp->NN.p = (double *)
                      calloc((unsigned long) nsites, sizeof(double));
        cblas_dcopy(nsites, fp->NN, 1, h5_fp->NN.p, 1);

        h5_fp->UN.len = (unsigned long) nsites;
        h5_fp->UN.p = (double *)
                      calloc((unsigned long) nsites, sizeof(double));
        cblas_dcopy(nsites, fp->UN, 1, h5_fp->UN.p, 1);

        h5_fp->Einp.len = (unsigned long) nsites;
        h5_fp->Einp.p = (double *)
                        calloc((unsigned long) nsites, sizeof(double));
        cblas_dcopy(nsites, fp->Einp, 1, h5_fp->Einp.p, 1);

        h5_fp->Ninp.len = (unsigned long) nsites;
        h5_fp->Ninp.p = (double *)
                        calloc((unsigned long) nsites, sizeof(double));
        cblas_dcopy(nsites, fp->Ninp, 1, h5_fp->Ninp.p, 1);

        h5_fp->Uinp.len = (unsigned long) (nsites);
        h5_fp->Uinp.p = (double *)
                        calloc((unsigned long) (nsites), sizeof(double));
        cblas_dcopy(nsites, fp->Uinp, 1, h5_fp->Uinp.p, 1);

        h5_fp->fault_ptr.len = (unsigned long) (nfp + 1);
        itemp = (int *)calloc((unsigned long) (nfp+1), sizeof(int));
        for (i=0; i<nfp+1; i++)
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
        log_errorF("%s: Error not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Copies finite fault results structure to/from HDF5 finite fault
 *        results structure
 *
 * @param[in] job         if job = COPY_DATA_TO_H5 then copy ff -> h5_ff.
 *                        if job = COPY_H5_TO_DATA then copy h5_ff -> ff.
 *
 * @param[in,out] ff      if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to h5_ff.
 *                        if job = COPY_H5_TO_DATA then on output this is the
 *                        copied h5_ff structure.
 * @param[in,out] h5_ff   if job = COPY_DATA_TO_H5 then on output this is the
 *                        HDF5 version of ff.
 *                        if job = COPY_DATA_TO_H5 then on input this is the
 *                        structure to copy to ff.
 *
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int hdf5_copy__ffResults(const enum data2h5_enum job,
                         struct GFAST_ffResults_struct *ff,
                         struct h5_ffResults_struct *h5_ff)
{
    const char *fcnm = "hdf5_copyFFResults\0";
    struct h5_faultPlane_struct *h5_fp = NULL;
    int i, ierr, nfp;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_ff, 0, sizeof(struct h5_ffResults_struct));
        nfp = ff->nfp;
        if (nfp < 0)
        {
            log_errorF("%s: Error no fault planes!\n", fcnm);
            return -1;
        }

        h5_ff->fp.len = (unsigned long) nfp;
        h5_fp = (struct h5_faultPlane_struct *)
                calloc((unsigned long) nfp,
                       sizeof(struct h5_faultPlane_struct));
        for (i=0; i<nfp; i++)
        {
            ierr = GFAST_hdf5_copy__faultPlane(job, &ff->fp[i], &h5_fp[i]);
        }
        h5_ff->fp.p = h5_fp;

        h5_ff->vr.len = (size_t) nfp;
        h5_ff->vr.p = (double *)calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, ff->vr, 1, h5_ff->vr.p, 1); 

        h5_ff->Mw.len = (size_t) nfp;
        h5_ff->Mw.p = (double *)calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, ff->Mw, 1, h5_ff->Mw.p, 1);

        h5_ff->str.len = (size_t) nfp;
        h5_ff->str.p = (double *)calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, ff->str, 1, h5_ff->str.p, 1);

        h5_ff->dip.len = (size_t) nfp;
        h5_ff->dip.p = (double *)calloc((unsigned long) nfp, sizeof(double));
        cblas_dcopy(nfp, ff->dip, 1, h5_ff->dip.p, 1);

        h5_ff->SA_lat = ff->SA_lat;
        h5_ff->SA_lon = ff->SA_lon;
        h5_ff->SA_dep = ff->SA_dep;
        h5_ff->SA_mag = ff->SA_mag;
        h5_ff->preferred_fault_plane = ff->preferred_fault_plane;
        h5_ff->nfp = ff->nfp;

    }   
    else if (job == COPY_H5_TO_DATA)
    {
        log_errorF("%s: not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
        ierr = 1;
    }
    return ierr;
}

//============================================================================//
/*!
 * @brief Copies three component waveform data structure to/from HDF5
 *        three component waveform structure 
 *
 * @param[in] job           if job = COPY_DATA_TO_H5 then copy data -> h5_data.
 *                          if job = COPY_H5_TO_DATA then copy h5_data -> data
 *
 * @param[in,out] data      if job = COPY_DATA_TO_H5 then on input this is the
 *                          structure to copy to h5_data.
 *                          if job = COPY_H5_TO_DATA then on output this is the
 *                          copied h5_data structure.
 * @param[in,out] h5_data   if job = COPY_DATA_TO_H5 then on output this is the
 *                          HDF5 version of data.
 *                          if job = COPY_DATA_TO_H5 then on input this is the
 *                          structure to copy to data.
 *
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int hdf5_copy__waveform3CData(const enum data2h5_enum job,
                              struct GFAST_waveform3CData_struct *data,
                              struct h5_waveform3CData_struct *h5_data)
{
    const char *fcnm = "hdf5_copy__waveform3CData\0";
    char *netw, *stnm, *chan, *loc;
    double nanv[1] = {(double) NAN};
    int ierr, npts;
    size_t nalloc;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_data, 0, sizeof(struct h5_waveform3CData_struct));
        npts = data->npts;
        nalloc = (unsigned long) (fmax(npts, 1));
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
        strcpy(netw, data->stnm);
        h5_data->stnm.len = 1;
        h5_data->stnm.p = stnm;

        chan = (char *)calloc(3*64, sizeof(char));
        strcpy(&chan[0],   data->chan[0]);
        strcpy(&chan[64],  data->chan[1]);
        strcpy(&chan[128], data->chan[2]);
        h5_data->chan.len = 3;
        h5_data->chan.p = chan;

        loc = (char *)calloc(64, sizeof(char));
        strcpy(loc, data->loc);
        h5_data->loc.len = 1;
        h5_data->loc.p = loc;

    }
    else if (job == COPY_H5_TO_DATA)
    {
        log_errorF("%s: not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
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
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int hdf5_copy__gpsData(const enum data2h5_enum job,
                       struct GFAST_data_struct *gps_data,
                       struct h5_gpsData_struct *h5_gpsData)
{
    const char *fcnm = "hdf5_copy__gpsData\0";
    struct h5_waveform3CData_struct *h5_data;
    int ierr, k, nstreams;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {   
        memset(h5_gpsData, 0, sizeof(struct h5_gpsData_struct));
        nstreams = gps_data->stream_length;
        if (nstreams < 1)
        {
            log_errorF("%s: Error no streams to copy!\n", fcnm);
            ierr = 1;
        }
        h5_data = (struct h5_waveform3CData_struct *)
                  calloc((unsigned long) nstreams,
                         sizeof(struct h5_waveform3CData_struct));
        h5_gpsData->stream_length = nstreams;
        for (k=0; k<nstreams; k++)
        {
            ierr = GFAST_hdf5_copy__waveform3CData(job,
                                                   &gps_data->data[k],
                                                   &h5_data[k]); 
            if (ierr != 0)
            {
                log_errorF("%s: Error copying 3C data\n", fcnm);
                return ierr;
            }
        }
        h5_gpsData->data.p = h5_data;
        h5_gpsData->data.len = (size_t) nstreams;
    }
    else if (job == COPY_H5_TO_DATA)
    {
        log_errorF("%s: not yet done\n", fcnm);
        ierr = 1;
    }
    else
    {
        log_errorF("%s: Invalid job\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
