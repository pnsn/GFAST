#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <cblas.h>

/*!
 * @brief Perform the finite fault inversion on both nodal planes.
 *
 */
int GFAST_FF__driver(struct GFAST_props_struct props,
                     struct GFAST_shakeAlert_struct SA,
                     struct GFAST_data_struct gps_data,
                     struct GFAST_ffResults_struct *ff)
{
    const char *fcnm = "GFAST_FF__driver\0";
    double *G2, *S, *T, *UD, *UP, *xrs, *yrs, *zrs,
           lampred, res, xden, xnum;
    int i, ierr, ierr1, ifp, l1, l2, nf, ng, nt;
    // Initialize
    ierr1 = 0;
    ierr = 0;
    G2 = NULL;
    UD = NULL;
    UP = NULL;
    T  = NULL;
    S  = NULL;
    xrs = NULL;
    yrs = NULL;
    zrs = NULL;
    // Error checks
    if (props.nfp < 1 || props.nfp > 2){
        log_errorF("%s: Error invalid number of nodal planes\n", fcnm);
        return -1;
    }
    // Set space for regularizer
    nf = props.nfp; 
    mrowsG = 3*l1;
    ncolsG = 2*l2;
    mrowsT = 2*l2 + 2*(2*nstr + 2*(ndip - 2));
    ncolsT = 2*l2;
    mrowsG2 = mrowsG + mrowsT;
    ncolsG2 = ncolsG;
    ng = mrowsG*ncolsG;
    nt = mrowsT*ncolsT;
    ng2 = mrowsG2*ncolsG2;
    G2 = GFAST_memory_calloc__double(ng2);
    UD = GFAST_memory_calloc__double(mrowsG2);
    UP = GFAST_memory_calloc__double(mrowsG2);
    T  = GFAST_memory_calloc__double(nt);
    S  = GFAST_memory_calloc__double(ncolsG2);
    if (G == NULL || G2 == NULL || UD == NULL || T == NULL){
        if (G  == NULL){log_errorF("%s: Error setting space for G\n",  fcnm);}
        if (G2 == NULL){log_errorF("%s: Error setting space for G2\n", fcnm);}
        if (UD == NULL){log_errorF("%s: Error setting space for U2\n", fcnm);}
        if (T  == NULL){log_errorF("%s: Error setting space for T\n",  fcnm);}
        ierr = FF_MEMORY_ERROR;
        goto ERROR;
    }
    // Set the hypocenter information
    ff->SA_lat = SA.lat;
    ff->SA_lon = SA.lon;
    ff->SA_dep = SA.dep;
    ff->preferred_fault_plane =-1;
    // Get the strikes, dips, and rakes for plane 1 
    ff->fp[0].str = str1;
    ff->fp[0].dip = dip1;
    ff->fp[0].nobs = l1;
    // Get the strikes, dips, and rakes for plane 2
    ff->fp[1].str = str2;
    ff->fp[1].dip = dip2;
    ff->fp[1].nobs = l1;
    // Set the RHS

    // Loop on the fault planes
#ifdef __PARALLEL_FF
    #pragma omp parallel for \
     firstprivate(G2, S, UP) \
     private(i, ierr1, ifp, res, xden, xnum) \
     shared(ff, l1, l2, mrowsG2, ncolsG2, props, U) \
     reduction(+:ierr) default(none)
#endif
    for (ifp=0; ifp<ff->nfp; ifp++){
        // Null out G2
        memset(G2, 0, ng2*sizeof(double));
        // Mesh the fault plane
        ierr1 = GFAST_FF__meshFaultPlane(ff->SA_lat, ff->SA_lon, ff->SA_dep,
                                         props.ff_flen_pct,
                                         props.ff_fwid_pct,
                                         ff->M, ff->str[ifp], ff->dip[ifp],
                                         ff->fp[ifp].nstr, ff->fp[ifp].ndip,
                                         props.utm_zone, props.verbose,
                                         ff->fp[ifp].fault_lon,
                                         ff->fp[ifp].fault_lat,
                                         ff->fp[ifp].fault_alt,
                                         ff->fp[ifp].strike,
                                         ff->fp[ifp].dip,
                                         ff->fp[ifp].length,
                                         ff->fp[ifp].width);
        if (ierr1 != 0){
            log_errorF("%s: Error meshing fault plane\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the forward modeling matrix (which is in row major format)
        ierr1 = GFAST_FF_setForwardModel__okadagreenF(l1, l2,
                                                      xrs, yrs, zrs, 
                                                      ff->fp[ifp].strike,
                                                      ff->fp[ifp].dip,
                                                      ff->fp[ifp].width,
                                                      ff->fp[ifp].length,
                                                      G2);
        if (ierr1 != 0){
            log_errorF("%s: Error setting forward model\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Set the regularizer
        ierr1 = GFAST_FF__setRegularizer(l2, ff->nstr, ff->ndip, nt,
                                         ff->fp[ifp].width,
                                         ff->fp[ifp].length,
                                         T);
        if (ierr1 != 0){
            log_errorF("%s: Error setting regulizer\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute scale factor for regularizer
        lampred = 1.0/pow( (double) l2*2.0, 2);
        lampred = lampred*cblas_dasum(ng, G2, 1)/(double) ng;
        lampred = lampred/4.0*fp[ifp].length[0]*fp[ifp].width[0]/1.e6;
        // Append lampred*T to G2
        cblas_daxpy(nt, lampred, T, 1, &G2[ng], 1); 
        // Solve the least squares problem
        ierr1 = numpy_lstsq__qr(LAPACK_ROW_MAJOR,
                                mrowsG2, ncolsG2, 1, G2, UD,
                                S);
        if (ierr1 != 0){
            log_errorF("%s: Error solving least squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute the forward problem 
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    mrowsG2, ncolsG2, 1.0, G2, ncolsG2, S, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
        #pragma omp simd reduction(+:xnum, xden)
        for (i=0; i<mrowsG; i++){
            res = UP[i] - U[i];
            xnum = xnum + res*res;
            xden = xden + U[i]*U[i];
        }
        ff->vr[ifp] = (1.0 - xnum/xden)*100.0;
        // Extract the estimates 
        #pragma omp simd
        for (i=0; i<l1; i++){
            ff->fp[ifp].EN[i] = UP[3*i+0];
            ff->fp[ifp].NN[i] = UP[3*i+1];
            ff->fp[ifp].UN[i] = UP[3*i+2];
         }
    } // Loop on fault planes
    if (ierr != 0){
        log_errorF("%s: There were errors detected in the inversion\n", fcnm);
        ierr = FF_;
    }else{
        // Choose a preferred plane
        ff->preferred_plane = 0;
        for (i=1; i<props.nfp; i++){
            if (ff->vr[i] < ff->VR[ff->preferred_plane]){
                ff->preferred_plane = i;
            }
        }
    }
ERROR:;
    GFAST_memory_free__double(&G2);
    GFAST_memory_free__double(&UD);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&T);
    GFAST_memory_free__double(&S);
    return ierr;
}
