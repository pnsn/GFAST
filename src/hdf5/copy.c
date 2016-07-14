#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <cblas.h>
#include "gfast.h"

//============================================================================//
int GFAST_HDF5_copyType__peakDisplacementData(enum data2h5_enum job,
                          struct GFAST_peakDisplacementData_struct *pgdData,
                          struct h5_peakDisplacementData_struct *h5_pgdData)
{
    const char *fcnm = "GFAST_HDF5_copyType__peakDisplacementData\0";
    int *lactiveTemp, *lmaskTemp, i, ierr, nsites;
    char *ctemp;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        nsites = pgdData->nsites;

        h5_pgdData->pd.len = nsites;
        h5_pgdData->pd.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgdData->pd, 1, h5_pgdData->pd.p, 1);

        h5_pgdData->wt.len = nsites;
        h5_pgdData->wt.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgdData->wt, 1, h5_pgdData->wt.p, 1);

        h5_pgdData->sta_lat.len = nsites;
        h5_pgdData->sta_lat.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgdData->sta_lat, 1, h5_pgdData->sta_lat.p, 1);

        h5_pgdData->sta_lon.len = nsites;
        h5_pgdData->sta_lon.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgdData->sta_lon, 1, h5_pgdData->sta_lon.p, 1);

        h5_pgdData->sta_alt.len = nsites;
        h5_pgdData->sta_alt.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgdData->sta_alt, 1, h5_pgdData->sta_alt.p, 1);

        h5_pgdData->stnm.len = 64*nsites;
        ctemp = (char *)calloc(64*nsites, sizeof(char));

        h5_pgdData->lactive.len = nsites;
        lactiveTemp = (int *)calloc(nsites, sizeof(int));

        h5_pgdData->lmask.len = nsites;
        lmaskTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<nsites; i++)
        {
            lactiveTemp[i] = pgdData->lactive[i];
            lmaskTemp[i] = pgdData->lmask[i];
            strcpy(&ctemp[i*64], pgdData->stnm[i]);
        }
        h5_pgdData->stnm.p = ctemp;
        h5_pgdData->lactive.p = lactiveTemp;
        h5_pgdData->lmask.p = lmaskTemp;
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
/*!
 * @brief Copies PGD results structure to/from HDF5 PGD results structure
 *
 * @param[in] job        if job = COPY_DATA_TO_H5 then copy pgd -> h5_pgd
 *                       if job = COPY_H5_TO_DATA then copy h5_pgd -> pgd
 *
 * @param[inout] pgd     if job = COPY_DATA_TO_H5 then on input this is the
 *                       structure to copy to h5_pgd
 *                       if job = COPY_H5_TO_DATA then on output this is the
 *                       copied h5_pgd structure 
 * @param[inout] h5_pgd  if job = COPY_DATA_TO_H5 then on output this is the
 *                       HDF5 version of pgd
 *                       if job = COPY_DATA_TO_H5 then on input this is the
 *                       structure to copy to pgd 
 *
 * @author Ben Baker, ISTI
 *
 * @bug COPY_H5_TO_DATA not yet done
 *
 */
int GFAST_HDF5_copyType__pgdResults(enum data2h5_enum job,
                                    struct GFAST_pgdResults_struct *pgd,
                                    struct h5_pgdResults_struct *h5_pgd)
{
    const char *fcnm = "GFAST_HDF5_copyType__pgdResults\0";
    int *lsiteUsedTemp, i, ierr, ndeps, nsites;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (job == COPY_DATA_TO_H5)
    {
        memset(h5_pgd, 0, sizeof(struct h5_pgdResults_struct));
        // Set space
        ndeps = pgd->ndeps;
        nsites = pgd->nsites;

        h5_pgd->mpgd.len = ndeps;
        h5_pgd->mpgd.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(ndeps, pgd->mpgd, 1, h5_pgd->mpgd.p, 1);

        h5_pgd->mpgd_vr.len = ndeps;
        h5_pgd->mpgd_vr.p = (double *)calloc(ndeps, sizeof(double));
        cblas_dcopy(ndeps, pgd->mpgd_vr, 1, h5_pgd->mpgd_vr.p, 1);

        h5_pgd->UP.len = nsites; 
        h5_pgd->UP.p = (double *)calloc(nsites, sizeof(double));
        cblas_dcopy(nsites, pgd->UP, 1, h5_pgd->UP.p, 1);

        h5_pgd->srcDepths.len = ndeps;
        h5_pgd->srcDepths.p = (double *)calloc(ndeps, sizeof(double)); 
        cblas_dcopy(ndeps, pgd->srcDepths, 1, h5_pgd->srcDepths.p, 1);

        h5_pgd->lsiteUsed.len = nsites;
        lsiteUsedTemp = (int *)calloc(nsites, sizeof(int));
        for (i=0; i<nsites; i++)
        {
            lsiteUsedTemp[i] = (int) pgd->lsiteUsed[i];
        }
        h5_pgd->lsiteUsed.p = lsiteUsedTemp;

        h5_pgd->ndeps = ndeps;
        h5_pgd->nsites = nsites; 
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

