#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

struct sparseMatrix_coo_struct
{
    double *A;  /*!< Values of sparse matrix [nnz] */
    int *irn;   /*!< Row numbers of non-zeros [nnz] */
    int *jcn;   /*!< Column number of non-zeros [nnz] */
    int nnz;    /*!< Number of non-zeros in COO matrix */
};

static bool lequal(double a, double b, double tol)
{
    if (a == 0.0 && b == 0.0){return true;}
    if (fabs(a - b)/fabs(a + b) > tol){return false;}
    return true;
}

int __read_faultPlane(const char *fname,
                      int *utm_zone,
                      double *SA_lat, double *SA_lon, double *SA_dep,
                      double *flen_pct, double *fwid_pct,
                      double *Mw, double *strike, double *dip,
                      struct GFAST_faultPlane_struct *ff)
{
    FILE *infl;
    char cline[128];
    int i, ierr, l2;
    ierr = 1;
    infl = fopen(fname, "r");
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
    sscanf(cline, "%lf %lf %lf %lf %lf %lf %lf %lf %d %d %d\n",
                   SA_lat, SA_lon, SA_dep,
                   flen_pct, fwid_pct,
                   Mw, strike, dip,
                   &ff->nstr, &ff->ndip, utm_zone);
    l2 = ff->ndip*ff->nstr;
    ff->fault_xutm = ISCL_memory_calloc__double(l2);
    ff->fault_yutm = ISCL_memory_calloc__double(l2);
    ff->fault_alt  = ISCL_memory_calloc__double(l2);
    ff->strike     = ISCL_memory_calloc__double(l2);
    ff->dip        = ISCL_memory_calloc__double(l2);
    ff->length     = ISCL_memory_calloc__double(l2);
    ff->width      = ISCL_memory_calloc__double(l2);
    for (i=0; i<l2; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){goto ERROR;}
        sscanf(cline, "%lf %lf %lf %lf %lf %lf %lf\n",
               &ff->fault_xutm[i], &ff->fault_yutm[i], &ff->fault_alt[i],
               &ff->strike[i], &ff->dip[i], &ff->length[i], &ff->width[i]);
    }
ERROR:;
    fclose(infl);
    ierr = 0;
    return ierr;
}

static int read_results(const char *fname,
                        struct GFAST_ff_props_struct *ff_props,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_ffResults_struct *ff,
                        double *SA_lat, double *SA_lon, double *SA_dep)
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
              calloc(ff->nfp, sizeof(struct GFAST_faultPlane_struct)); 
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

struct sparseMatrix_coo_struct __read_treg(const char *fname, int *ierr)
{
    FILE *infl;
    char cline[128];
    struct sparseMatrix_coo_struct coo;
    int i;
    *ierr = 1;
    infl = fopen(fname, "r");
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){return coo;}
    sscanf(cline, "%d\n", &coo.nnz);
    coo.A = (double *)calloc(coo.nnz, sizeof(double));
    coo.irn = (int *)calloc(coo.nnz, sizeof(double));
    coo.jcn = (int *)calloc(coo.nnz, sizeof(double));
    for (i=0; i<coo.nnz; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, sizeof(cline), infl) == NULL){return coo;}
        sscanf(cline, "%d %d %lf\n", &coo.irn[i], &coo.jcn[i], &coo.A[i]);
    }
    fclose(infl);
    *ierr = 0;
    return coo;
}

double *__read_xyz(const char *fname, int *l1, int *l2, int *ierr)
{
    FILE *infl;
    char cline[128];
    double *pos = NULL;
    int i, indx, j;
    *ierr = *ierr + 1;
    infl = fopen(fname, "r");
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){return NULL;}
    sscanf(cline, "%d %d\n", l1, l2); 
    pos = (double *)calloc(*l1**l2, sizeof(double));
    indx = 0;
    for (i=0; i<*l2; i++)
    {
        for (j=0; j<*l1; j++)
        {
            memset(cline, 0, sizeof(cline));
            if (fgets(cline, sizeof(cline), infl) == NULL){return NULL;}
            sscanf(cline, "%lf\n", &pos[indx]);
            indx = indx + 1;
        }
    }
    fclose(infl);
    *ierr = *ierr - 1;
    return pos;
}

double *__read_grns(const char *fname, int *nrows, int *ncols, int *ierr)
{
    FILE *infl;
    char cline[128];
    double *grns = NULL;
    int i, indx, j;
    *ierr = *ierr + 1;
    infl = fopen(fname, "r");
    memset(cline, 0, sizeof(cline));
    if (fgets(cline, sizeof(cline), infl) == NULL){return NULL;}
    sscanf(cline, "%d %d\n", nrows, ncols); 
    grns = (double *)calloc(*nrows**ncols, sizeof(double));
    indx = 0;
    for (i=0; i<*nrows; i++)
    {
        for (j=0; j<*ncols; j++)
        {
            memset(cline, 0, sizeof(cline));
            if (fgets(cline, sizeof(cline), infl) == NULL){return NULL;}
            sscanf(cline, "%lf\n", &grns[indx]);
            indx = indx + 1;
        }
    }   
    fclose(infl);
    *ierr = *ierr - 1;
    return grns;
}
//============================================================================//
/*!
 * @brief Tests the regularization matrix computation for the finite
 *        fault inversion
 *
 * @result EXIT_SUCCESS indicates success
 *
 */
int ff_regularizer_test()
{
    const char *fcnm = "ff_regularizer_test\0";
    const char *fname = "files/treg.txt\0";
    struct sparseMatrix_coo_struct coo;
    double *T, *length, *width;
    int i, ierr, indx, inz, j, l2, mrowsT, ncolsT, nt, nnz;
    const double wid = 51586.88700804;
    const double len = 90754.4164353;
    const double tol = 1.e-6;
    const int nstr = 10;
    const int ndip = 5;
    coo = __read_treg(fname, &ierr);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading file\n", fcnm);
        return EXIT_FAILURE;
    }
    // Set lengths and widths
    l2 = nstr*ndip;
    length = (double *)calloc(l2, sizeof(double));
    width  = (double *)calloc(l2, sizeof(double));
    for (i=0; i<l2; i++)
    {
        length[i] = len;
        width[i] = wid;
    }
    // Compute the regularizer
    mrowsT = 2*ndip*nstr + 2*(2*ndip + nstr - 2); 
    ncolsT = 2*l2;
    nt = mrowsT*ncolsT;
    T = (double *)calloc(nt, sizeof(double));
    ierr = GFAST_core_ff_setRegularizer(l2, nstr, ndip, nt,
                                        width, length, T);
    if (ierr != 0)
    {
        log_errorF("%s: Error computing regularizer\n", fcnm);
        return EXIT_FAILURE;
    }
    nnz = 0;
    for (i=0; i<mrowsT; i++)
    {
        for (j=0; j<ncolsT; j++)
        {
            indx = ncolsT*i + j;
            if (fabs(T[indx]) > 0.0)
            {
                for (inz=0; inz<coo.nnz; inz++)
                {
                    if (coo.irn[inz] == i && coo.jcn[inz] == j)
                    {
                        goto COMPARE;
                    }
                }
                log_errorF("%s: Couldn't find index %d %d\n", fcnm, i, j);
                return EXIT_FAILURE;
COMPARE:;
                if (!lequal(coo.A[inz], T[indx], tol))
                {
                    log_errorF("%s: Differing T %e %e\n",
                               fcnm, coo.A[inz], T[indx]);
                    return EXIT_FAILURE;
                }
                nnz = nnz + 1;
            }
        }
    }
    if (nnz != coo.nnz)
    {
        log_errorF("%s: Differing nnz\n", fcnm);
        return EXIT_FAILURE;
    }
    if (coo.A   != NULL){free(coo.A);}
    if (coo.irn != NULL){free(coo.irn);}
    if (coo.jcn != NULL){free(coo.jcn);}
    if (T != NULL){free(T);}
    if (length != NULL){free(length);}
    if (width != NULL){free(width);}
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
//============================================================================//
/*!
 * @brief Tests the Greens functions computation for finite fault inversion
 *
 * @result EXIT_SUCCESS indicates success
 *
 */ 
int ff_greens_test()
{
    const char *fcnm = "ff_greens_test\0";
    const char *xrsfl = "files/xrs.txt\0";
    const char *yrsfl = "files/yrs.txt\0";
    const char *zrsfl = "files/zrs.txt\0";
    const char *grnsfl1 = "files/rtok.txt\0";
    const char *grnsfl2 = "files/rtok_dip0.txt\0";
    const double str = 113.43955911;
    const double dp1 = 57.75180433;
    const double wid = 51586.88700804;
    const double len = 90754.4164353;
    const double tol = 1.e-6;
    double *dip, *Gmat, *grns1, *grns2, *length, *strike, *width, *xrs, *yrs, *zrs;
    int i, ierr, l1, l2, nrows, ncols;
    ierr = 0;
    xrs = __read_xyz(xrsfl, &l1, &l2, &ierr);
    yrs = __read_xyz(yrsfl, &l1, &l2, &ierr);
    zrs = __read_xyz(zrsfl, &l1, &l2, &ierr);
    grns1 = __read_grns(grnsfl1, &nrows, &ncols, &ierr);
    grns2 = __read_grns(grnsfl2, &nrows, &ncols, &ierr); 
    if (ierr != 0)
    {
        log_errorF("%s: Error reading files\n", fcnm);
        return EXIT_FAILURE;
    }
    if (nrows != 3*l1)
    {
        log_errorF("%s: Lost count on rows\n", fcnm);
        return EXIT_FAILURE;
    }
    if (ncols != 2*l2)
    {
        log_errorF("%s: Lost cont on columns\n", fcnm);
        return EXIT_FAILURE;
    }
    // Set the strike, dip, width, and length 
    strike = (double *)calloc(l1*l2, sizeof(double));
    dip = (double *)calloc(l1*l2, sizeof(double));
    width = (double *)calloc(l1*l2, sizeof(double));
    length = (double *)calloc(l1*l2, sizeof(double));
    Gmat = (double *)calloc(nrows*ncols, sizeof(double));
    for (i=0; i<l1*l2; i++)
    {
        strike[i] = str;
        dip[i] = dp1;
        width[i] = wid;
        length[i] = len;
    }
    ierr = GFAST_core_ff_setForwardModel__okadagreenF(l1, l2,
                                                  xrs, yrs, zrs,
                                                  strike, dip,
                                                  width, length,
                                                  Gmat);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting forward model 1\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<nrows*ncols; i++)
    {
         //if (fabs(Gmat[i] - grns1[i])/fabs(Gmat[i] + grns1[i]) > 1.e-6)
         if (!lequal(Gmat[i], grns1[i], tol))
         {
             log_errorF("%s: Error with grns1 %e %e %e\n",
                        fcnm, Gmat[i], grns1[i],
                        fabs( (Gmat[i] - grns1[i])/grns1[i] ));
             return EXIT_FAILURE;
         }
    }
    // dip = 0.0
    for (i=0; i<l1*l2; i++)
    {
        dip[i] = 0.0;
    }
    ierr = GFAST_core_ff_setForwardModel__okadagreenF(l1, l2, 
                                                  xrs, yrs, zrs,
                                                  strike, dip,
                                                  width, length,
                                                  Gmat);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting forward model 2\n", fcnm);
        return EXIT_FAILURE;
    }
    for (i=0; i<nrows*ncols; i++)
    {   
         //if (fabs(Gmat[i] - grns2[i])/fabs(Gmat[i] + grns2[i]) > 1.e-6)
         if (!lequal(Gmat[i], grns2[i], tol))
         {
             log_errorF("%s: Error with grns2 %e %e\n",
                        fcnm, Gmat[i], grns2[i]);
             return EXIT_FAILURE;
         }
    }
    free(Gmat);
    free(grns1);
    free(grns2);
    free(dip);
    free(strike);
    free(width);
    free(length);
    free(xrs);
    free(yrs);
    free(zrs);
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
//============================================================================//
int ff_meshPlane_test()
{
    const char *fcnm = "ff_meshPlane_test\0";
    const char *fname[2] = {"files/final_fp1.maule.txt\0",
                            "files/final_fp2.maule.txt\0"};
    struct GFAST_faultPlane_struct ff, ff_ref;
    double SA_lat, SA_lon, SA_dep, Mw, flen_pct, fwid_pct, strike, dip;
    int i, ierr, ifp, l2, utm_zone;
    int verbose = 0;
    const int nfp = 2;
    for (ifp=0; ifp<nfp; ifp++)
    {
        memset(&ff_ref, 0, sizeof(ff_ref));
        memset(&ff, 0, sizeof(ff));
        ierr = __read_faultPlane(fname[ifp],
                                 &utm_zone,
                                 &SA_lat, &SA_lon, &SA_dep,
                                 &flen_pct, &fwid_pct,
                                 &Mw, &strike, &dip,
                                 &ff_ref);
        if (ierr != 0)
        {
            log_errorF("%s: Error reading faultPlane\n", fcnm);
            return EXIT_FAILURE;
        }
        ff.nstr = ff_ref.nstr;
        ff.ndip = ff_ref.ndip;
        l2 = ff.nstr*ff.ndip;
        ff.fault_ptr = ISCL_memory_calloc__int(l2+1);
        ff.lat_vtx = ISCL_memory_calloc__double(4*l2);
        ff.lon_vtx = ISCL_memory_calloc__double(4*l2);
        ff.dep_vtx = ISCL_memory_calloc__double(4*l2);
        ff.fault_xutm = ISCL_memory_calloc__double(l2);
        ff.fault_yutm = ISCL_memory_calloc__double(l2);
        ff.fault_alt = ISCL_memory_calloc__double(l2);
        ff.strike = ISCL_memory_calloc__double(l2);
        ff.dip = ISCL_memory_calloc__double(l2); 
        ff.length = ISCL_memory_calloc__double(l2);
        ff.width = ISCL_memory_calloc__double(l2);
        ierr = GFAST_core_ff_meshFaultPlane(SA_lat,
                                        SA_lon,
                                        SA_dep,
                                        flen_pct,
                                        fwid_pct,
                                        Mw,
                                        strike,
                                        dip,
                                        ff.nstr,
                                        ff.ndip,
                                        utm_zone,
                                        verbose,
                                        ff.fault_ptr,
                                        ff.lat_vtx,
                                        ff.lon_vtx,
                                        ff.dep_vtx,
                                        ff.fault_xutm,
                                        ff.fault_yutm,
                                        ff.fault_alt,
                                        ff.strike,
                                        ff.dip,
                                        ff.length,
                                        ff.width);
        for (i=0; i<l2; i++)
        {
            if (!lequal(ff.fault_xutm[i], ff_ref.fault_xutm[i], 1.e-4))
            {
                log_errorF("%s: Error computing xutm %f %f\n", fcnm,
                           ff.fault_xutm[i], ff_ref.fault_xutm[i]);
                return EXIT_FAILURE;
            } 
            if (!lequal(ff.fault_yutm[i], ff_ref.fault_yutm[i], 1.e-4))
            {
                log_errorF("%s: Error computing yutm %f %f\n", fcnm,
                           ff.fault_yutm[i], ff_ref.fault_yutm[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(ff.fault_alt[i], ff_ref.fault_alt[i], 1.e-4))
            {
                log_errorF("%s: Error computing alt %f %f\n", fcnm,
                           ff.fault_alt[i], ff_ref.fault_alt[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(ff.strike[i], ff_ref.strike[i], 1.e-4))
            {
                log_errorF("%s: Error computing strike %f %f\n", fcnm,
                           ff.strike[i], ff_ref.strike[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(ff.dip[i], ff_ref.dip[i], 1.e-4))
            {
                log_errorF("%s: Error computing dip %f %f\n", fcnm,
                           ff.dip[i], ff_ref.dip[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(ff.width[i]*1.e-3, ff_ref.width[i], 1.e-4))
            {
                log_errorF("%s: Error computing width %f %f\n", fcnm,
                           ff.width[i]*1.e-3, ff_ref.width[i]);
                return EXIT_FAILURE;
            }
            if (!lequal(ff.length[i]*1.e-3, ff_ref.length[i], 1.e-4))
            {
                log_errorF("%s: Error computing strike %f %f\n", fcnm,
                           ff.length[i]*1.e-3, ff_ref.length[i]);
                return EXIT_FAILURE;
            }
        }
        GFAST_core_ff_finalize__faultPlane(&ff_ref);
        GFAST_core_ff_finalize__faultPlane(&ff);
    }
    return EXIT_SUCCESS;
}
//============================================================================//

int ff_inversion_test()
{
    const char *fcnm = "ff_inversion_test\0";
    const char *fname = "files/final_ff.maule.txt\0";
    struct GFAST_ff_props_struct ff_props;
    struct GFAST_offsetData_struct ff_data;
    struct GFAST_ffResults_struct ff_ref, ff;
    double SA_lat, SA_lon, SA_dep;
    int i, ierr, j, l2;
    memset(&ff_props, 0, sizeof(ff_props));
    memset(&ff_data, 0, sizeof(ff_data));
    memset(&ff_ref, 0, sizeof(ff_ref));
    memset(&ff, 0, sizeof(ff));
    ff_props.verbose = 0;
    ierr = read_results(fname,
                        &ff_props,
                        &ff_data,
                        &ff_ref,
                        &SA_lat, &SA_lon, &SA_dep);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading results\n", fcnm);
        return EXIT_FAILURE;
    }
    // allocate space for fault
    ff.nfp = ff_ref.nfp;
    ff.fp = (struct GFAST_faultPlane_struct *)
              calloc(ff.nfp, sizeof(struct GFAST_faultPlane_struct)); 
    ff.SA_lat = ff_ref.SA_lat;
    ff.SA_lon = ff_ref.SA_lon;
    ff.SA_dep = ff_ref.SA_dep;
    ff.SA_mag = ff_ref.SA_mag;
    SA_lat = ff.SA_lat;
    SA_lon = ff.SA_lon;
    SA_dep = ff.SA_dep;
    ff.vr = ISCL_memory_calloc__double(ff.nfp);
    ff.Mw = ISCL_memory_calloc__double(ff.nfp); 
    ff.str = ISCL_memory_calloc__double(ff.nfp);
    ff.dip = ISCL_memory_calloc__double(ff.nfp);
    for (i=0; i<ff.nfp; i++)
    {
        ff.fp[i].maxobs = ff_data.nsites;
        ff.fp[i].nstr = ff_props.nstr;
        ff.fp[i].ndip = ff_props.ndip;
        ff.str[i] = ff_ref.str[i];
        ff.dip[i] = ff_ref.dip[i];
        l2 = ff_props.nstr*ff_props.ndip;
        ff.fp[i].lon_vtx = ISCL_memory_calloc__double(4*l2);
        ff.fp[i].lat_vtx = ISCL_memory_calloc__double(4*l2);
        ff.fp[i].dep_vtx = ISCL_memory_calloc__double(4*l2);
        ff.fp[i].fault_xutm = ISCL_memory_calloc__double(l2);
        ff.fp[i].fault_yutm = ISCL_memory_calloc__double(l2);
        ff.fp[i].fault_alt  = ISCL_memory_calloc__double(l2);
        ff.fp[i].strike     = ISCL_memory_calloc__double(l2);
        ff.fp[i].dip        = ISCL_memory_calloc__double(l2);
        ff.fp[i].length     = ISCL_memory_calloc__double(l2);
        ff.fp[i].width      = ISCL_memory_calloc__double(l2);
        ff.fp[i].sslip = ISCL_memory_calloc__double(l2);
        ff.fp[i].dslip = ISCL_memory_calloc__double(l2);
        ff.fp[i].sslip_unc = ISCL_memory_calloc__double(l2);
        ff.fp[i].dslip_unc = ISCL_memory_calloc__double(l2);
        ff.fp[i].Uinp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].UN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].Ninp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].NN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].Einp = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].EN   = ISCL_memory_calloc__double(ff.fp[i].maxobs);
        ff.fp[i].fault_ptr = ISCL_memory_calloc__int(l2+1);
    }
    ierr = eewUtils_driveFF(ff_props,
                            SA_lat, SA_lon, SA_dep,
                            ff_data, &ff);
    if (ierr != 0)
    {
        log_errorF("%s: Error inverting ff\n", fcnm);
        return EXIT_FAILURE;
    }
    l2 = ff_props.nstr*ff_props.ndip;
    for (j=0; j<ff.nfp; j++)
    {
        if (!lequal(ff.Mw[j], ff_ref.Mw[j], 1.e-3))
        {
            log_errorF("%s: Mw is incorrect %d %f %f\n", fcnm, j,
                       ff.Mw[j], ff_ref.Mw[j]);
        }
        if (!lequal(ff.vr[j], ff_ref.vr[j], 1.e-3))
        {
            log_errorF("%s: vr is incorrect %d %f %f\n", fcnm, j,
                       ff.vr[j], ff_ref.vr[j]);
        }
    }
    for (i=0; i<l2; i++)
    {
        for (j=0; j<ff.nfp; j++)
        {
            if (!lequal(ff.fp[j].dslip[i], ff_ref.fp[j].dslip[i], 1.e-3))
            {
                log_errorF("%s: Error computing dslip %d %f %f\n", fcnm, j,
                           ff.fp[j].dslip[i], ff_ref.fp[j].dslip[i]); 
                return EXIT_FAILURE;
            }
            if (!lequal(ff.fp[j].sslip[i], ff_ref.fp[j].sslip[i], 1.e-3))
            {
                log_errorF("%s: Error computing sslip %d %f %f\n", fcnm, j,
                           ff.fp[j].sslip[i], ff_ref.fp[j].sslip[i]); 
                return EXIT_FAILURE;
           }
        }
    }
    GFAST_core_ff_finalize__offsetData(&ff_data);
    GFAST_core_ff_finalize__ffResults(&ff_ref);
    GFAST_core_ff_finalize__ffResults(&ff);
    log_infoF("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
