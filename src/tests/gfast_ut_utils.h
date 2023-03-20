/**
 * @file gfast_ut_utils.h
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief Helper functions for GFAST unit tests
 */
#include "gtest/gtest.h"

#include "gfast.h"
#include "iscl/memory/memory.h"

// See https://github.com/google/googletest/blob/main/docs/advanced.md#using-a-function-that-returns-an-assertionresult
namespace testing {

// Returns an AssertionResult object to indicate that an assertion has
// succeeded.
AssertionResult AssertionSuccess();

// Returns an AssertionResult object to indicate that an assertion has
// failed.
AssertionResult AssertionFailure();

}

testing::AssertionResult lequal(double a, double b, double tol)
{
    if (a == 0.0 && b == 0.0){return testing::AssertionSuccess();}
    if (fabs(a - b)/fabs(a + b) > tol) {
        return testing::AssertionFailure() << a << " and " << b << " aren't within " << tol << " sig figs";
    }
    return testing::AssertionSuccess() << a << " and " << b << " are within " << tol << " sig figs";
}

// Based on gfast/unit_tests/pgd.c by Ben Baker
int read_pgd_results(const char *filenm,
                            struct GFAST_pgd_props_struct *pgd_props,
                            struct GFAST_peakDisplacementData_struct *pgd_data,
                            struct GFAST_pgdResults_struct *pgd,
                            double *SA_lat, double *SA_lon, double *SA_dep)
{
    FILE *infl;
    char cline[128];
    int i, ierr;
    //------------------------------------------------------------------------//
    ierr = 1;
    infl = fopen(filenm, "r");
    // line 1
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;} 
    sscanf(cline, "%d %d %d %d %lf %lf %lf\n",
           &pgd_data->nsites, &pgd_props->ngridSearch_deps,
           &pgd_props->utm_zone, &pgd_props->min_sites, 
           &pgd_props->dist_tol, &pgd_props->disp_def,
           &pgd_props->window_vel);
    // line 2
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf\n", SA_lat, SA_lon, SA_dep);
    // pgd data
    pgd_data->pd = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->wt = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_lat = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_lon = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->sta_alt = ISCL_memory_calloc__double(pgd_data->nsites);
    pgd_data->lmask   = ISCL_memory_calloc__bool(pgd_data->nsites);
    pgd_data->lactive = ISCL_memory_calloc__bool(pgd_data->nsites);
    for (i=0; i<pgd_data->nsites; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf\n",
               &pgd_data->sta_lat[i], &pgd_data->sta_lon[i],
               &pgd_data->sta_alt[i], &pgd_data->pd[i]);
        pgd_data->wt[i] = 1.0;
        pgd_data->lactive[i] = true;
    }
    // Results + depths in grid search
    pgd->nsites = pgd_data->nsites;
    pgd->ndeps = pgd_props->ngridSearch_deps;
    pgd->nlats = 1;
    pgd->nlons = 1;
    pgd->mpgd    = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->mpgd_vr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->dep_vr_pgd = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->iqr = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->UP = ISCL_memory_calloc__double(pgd->ndeps*pgd->nsites);
    pgd->UPinp = ISCL_memory_calloc__double(pgd->nsites);
    pgd->srcDepths = ISCL_memory_calloc__double(pgd->ndeps);
    pgd->srdist = ISCL_memory_calloc__double(pgd->ndeps*pgd->nsites);
    pgd->lsiteUsed = ISCL_memory_calloc__bool(pgd->nsites);
    // (a) depths
    for (i=0; i<pgd->ndeps; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf\n", &pgd->srcDepths[i]);
    }
    // (b) results
    for (i=0; i<pgd->ndeps; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf\n",
               &pgd->mpgd[i], &pgd->mpgd_vr[i], &pgd->iqr[i]);
    }
    fclose(infl);
    ierr = 0;
ERROR:;
    return ierr;
}

// Based on gfast/unit_tests/cmt.c by Ben Baker
int read_cmt_results(const char *filenm,
                        struct GFAST_cmt_props_struct *cmt_props,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_cmtResults_struct *cmt,
                        double *SA_lat, double *SA_lon, double *SA_dep)
{
    FILE *infl;
    char cline[128];
    double mxx, myy, mzz, mxy, mxz, myz;
    int i, idep, ierr, ldevi;
    ierr = 1;
    infl = fopen(filenm, "r");
    // line 1
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%d %d %d %d %d %d %d %lf %lf\n",
           &cmt_data->nsites, &cmt_props->ngridSearch_deps,
           &cmt_props->ngridSearch_lats, &cmt_props->ngridSearch_lons,
           &cmt_props->utm_zone, &cmt_props->min_sites, 
           &ldevi, &cmt_props->window_vel,
           &cmt_props->window_avg);
    cmt_props->ldeviatoric = true;
    if (ldevi != 1){cmt_props->ldeviatoric = false;}
    // line 2
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf\n", SA_lat, SA_lon, SA_dep);
    cmt_data->ubuff = memory_calloc64f(cmt_data->nsites);
    cmt_data->ebuff = memory_calloc64f(cmt_data->nsites);
    cmt_data->nbuff = memory_calloc64f(cmt_data->nsites);
    cmt_data->wtu = memory_calloc64f(cmt_data->nsites);
    cmt_data->wte = memory_calloc64f(cmt_data->nsites);
    cmt_data->wtn = memory_calloc64f(cmt_data->nsites);
    cmt_data->sta_lat = memory_calloc64f(cmt_data->nsites);
    cmt_data->sta_lon = memory_calloc64f(cmt_data->nsites);
    cmt_data->sta_alt = memory_calloc64f(cmt_data->nsites);
    cmt_data->lactive = ISCL_memory_calloc__bool(cmt_data->nsites);
    cmt_data->lmask = ISCL_memory_calloc__bool(cmt_data->nsites);
    // cmt data
    for (i=0; i<cmt_data->nsites; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf\n",
               &cmt_data->sta_lat[i], &cmt_data->sta_lon[i],
               &cmt_data->sta_alt[i],
               &cmt_data->nbuff[i], &cmt_data->ebuff[i], &cmt_data->ubuff[i]);
        cmt_data->wtu[i] = 1.0;
        cmt_data->wte[i] = 1.0;
        cmt_data->wtn[i] = 1.0;
        cmt_data->lactive[i] = true;
    }
    // Results + depths in grid search
    cmt->nsites = cmt_data->nsites;
    cmt->ndeps = cmt_props->ngridSearch_deps;
    cmt->nlats = cmt_props->ngridSearch_lats;
    cmt->nlons = cmt_props->ngridSearch_lons;
    cmt->l2 = memory_calloc64f(cmt->ndeps);
    cmt->pct_dc = memory_calloc64f(cmt->ndeps);
    cmt->objfn = memory_calloc64f(cmt->ndeps);
    cmt->mts = memory_calloc64f(cmt->ndeps*6);
    cmt->str1 = memory_calloc64f(cmt->ndeps);
    cmt->str2 = memory_calloc64f(cmt->ndeps);
    cmt->dip1 = memory_calloc64f(cmt->ndeps);
    cmt->dip2 = memory_calloc64f(cmt->ndeps);
    cmt->rak1 = memory_calloc64f(cmt->ndeps);
    cmt->rak2 = memory_calloc64f(cmt->ndeps);
    cmt->Mw = memory_calloc64f(cmt->ndeps);
    cmt->srcDepths = memory_calloc64f(cmt->ndeps);
    cmt->EN = memory_calloc64f(cmt->ndeps*cmt_data->nsites);
    cmt->NN = memory_calloc64f(cmt->ndeps*cmt_data->nsites);
    cmt->UN = memory_calloc64f(cmt->ndeps*cmt_data->nsites);
    cmt->Einp = memory_calloc64f(cmt_data->nsites);
    cmt->Ninp = memory_calloc64f(cmt_data->nsites);
    cmt->Uinp = memory_calloc64f(cmt_data->nsites);
    for (idep=0; idep<cmt->ndeps; idep++)
    {
        // depth
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf\n", &cmt->srcDepths[idep]);
        // moment tensor
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf\n",
               &mxx, &myy, &mzz, &mxy, &mxz, &myz);
        cmt->mts[6*idep+0] = mxx;
        cmt->mts[6*idep+1] = myy;
        cmt->mts[6*idep+2] = mzz;
        cmt->mts[6*idep+3] = mxy;
        cmt->mts[6*idep+4] = mxz;
        cmt->mts[6*idep+5] = myz;
        // rest of information
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf %lf %lf\n",
               &cmt->str1[idep], &cmt->dip1[idep], &cmt->rak1[idep],
               &cmt->str2[idep], &cmt->dip2[idep], &cmt->rak2[idep],
               &cmt->Mw[idep],   &cmt->objfn[idep]);
        if (cmt->rak1[idep] > 180.0){cmt->rak1[idep] = cmt->rak1[idep] - 360.0;}
        if (cmt->rak2[idep] > 180.0){cmt->rak2[idep] = cmt->rak2[idep] - 360.0;}
    }
    fclose(infl);
    ierr = 0;
ERROR:;
    return ierr;
}

// Based on gfast/unit_tests/ff.c by Ben Baker
int read_ff_results(const char *fname,
                        struct GFAST_ff_props_struct *ff_props,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_ffResults_struct *ff)
{
    FILE *infl;
    char cline[128];
    int i, ierr, l2; 
    ierr = 1;
    infl = fopen(fname, "r");
    // line 1
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%d %d %d %d %d %lf %lf %lf %lf\n",
                   &ff_props->nstr, &ff_props->ndip, &ff_props->utm_zone,
                   &ff_props->nfp,  &ff_props->min_sites,
                   &ff_props->window_vel, &ff_props->window_avg,
                   &ff_props->flen_pct, &ff_props->fwid_pct);
    l2 = ff_props->nstr*ff_props->ndip;
    // line 2
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf %lf\n", &ff->SA_lat, &ff->SA_lon,
                                       &ff->SA_dep, &ff->SA_mag);
    // line 3
    ff->nfp = ff_props->nfp;
    ff->fp  = (struct GFAST_faultPlane_struct *)
              calloc((size_t) ff->nfp, sizeof(struct GFAST_faultPlane_struct)); 
    ff->vr  = ISCL_memory_calloc__double(ff->nfp);
    ff->Mw  = ISCL_memory_calloc__double(ff->nfp);
    ff->str = ISCL_memory_calloc__double(ff->nfp);
    ff->dip = ISCL_memory_calloc__double(ff->nfp);
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf %lf\n",
           &ff->str[0], &ff->str[1], &ff->dip[0], &ff->dip[1]);
    // line 4
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%d\n", &ff_data->nsites);
    ff_data->ubuff = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->ebuff = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->nbuff = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->wtu = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->wte = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->wtn = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->sta_lat = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->sta_lon = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->sta_alt = ISCL_memory_calloc__double(ff_data->nsites);
    ff_data->lactive = ISCL_memory_calloc__bool(ff_data->nsites);
    ff_data->lmask = ISCL_memory_calloc__bool(ff_data->nsites);
    // ff data
    for (i=0; i<ff_data->nsites; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf\n",
               &ff_data->sta_lat[i], &ff_data->sta_lon[i],
               &ff_data->sta_alt[i],
               &ff_data->nbuff[i], &ff_data->ebuff[i], &ff_data->ubuff[i]);
        ff_data->wtu[i] = 1.0;
        ff_data->wte[i] = 1.0;
        ff_data->wtn[i] = 1.0;
        ff_data->lactive[i] = true;
    }
    // allocate space for fault
    for (i=0; i<ff->nfp; i++)
    {
        ff->fp[i].maxobs = ff_data->nsites;
        ff->fp[i].nstr = ff_props->nstr;
        ff->fp[i].ndip = ff_props->ndip;
        ff->fp[i].sslip = ISCL_memory_calloc__double(l2);
        ff->fp[i].dslip = ISCL_memory_calloc__double(l2);
    }
    // ff results vr + mag
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf %lf\n",
           &ff->vr[0], &ff->vr[1], &ff->Mw[0], &ff->Mw[1]);
    // ff results slip models
    for (i=0; i<l2; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf\n",
               &ff->fp[0].dslip[i], &ff->fp[0].sslip[i],
               &ff->fp[1].dslip[i], &ff->fp[1].sslip[i]);
    }
    fclose(infl);
    ierr = 0;
ERROR:;
    return ierr;
}
