#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <lapacke.h>
#include <cblas.h>
#include "gfast.h"
/*!
 * @brief This performs the grid-search finite-fault slip inversion
 *        for the given fault planes.
 *
 * @note The fault plane arrays (e.g. fault_xutm, fault_yutm, fault_alt,
 *       length, width, strike, dip, sslip, dslip, sslip_unc, dslip_unc)
 *       are packed such that the istr'th, idip'th element on the ifp'th fault
 *       plane is given ifp*l2 + idip*nstr + istr by where l2 = nstr*ndip.
 *       Similarly, the estimate displacement in the NN, EN, and UN
 *       for the i'th site for the ifp'th fault plane is given by 
 *       ifp*l1 + i where l1 is the number of sites in the inversion. 
 *
 * @param[in] l1               number of sites in the inversion. 
 * @param[in] l2               leading dimension for the fault plane
 *                             information.  this must be nstr*ndip.
 * @param[in] nstr             number of elements along strike in each fault
 *                             plane.
 * @param[in] ndip             number of elements down dip in each fault plane.
 * @param[in] nfp              number of fault planes in grid-search
 * @param[in] verbose          controls verbosity (0 will only report on 
 *                             errors)
 * @param[in] nAvgDisp         the average displacement in the north component
 *                             for the i'th site (m) [l1]
 * @param[in] eAvgDisp         the average displacement in the east component
 *                             for the i'th site (m) [l1]
 * @param[in] uAvgDisp         the average displacemeint in the vertical
 *                             component for the i'th site (m) [l1]
 * @param[in] utmRecvEasting   the UTM easting location (m) of the i'th
 *                             site [l1] 
 * @param[in] utmRecvNorthing  the UTM northing location (m) of the i'th
 *                             site [l1]
 * @param[in] fault_xutm       the UTM x (east) location (m) of the if'th
 *                             fault patch on the ifp'th fault plane [l2*nfp].
 * @param[in] fault_yutm       the UTM y (north) location (m) of the if'th
 *                             fault patch on the ifp'th fault plane [l2*nfp].
 * @param[in] fault_alt        depth (km) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] length           the length (m) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] width            the width (m) of the if'th fault patch on the
 *                             ifp'th fault plane [l2*nfp].
 * @param[in] strike           the strike angle (degrees) of the if'th fault
 *                             patch on the ifp'th fault plane [l2*nfp].
 * @param[in] dip              the dip angle (degrees) of the if'th fault
 *                             patch on the ifp'th fault plane [l2*nfp].
 *
 * @param[out] sslip           slip along strike (m) on the if'th fault patch
 *                             on the ifp'th fault plane [l2*nfp]. 
 * @param[out] dslip           slip down dip (m) on the if'th fault patch
 *                             of the ifp'th fault plane [l2*nfp]. 
 * @param[out] Mw              moment magnitude for the event on the ifp'th
 *                             fault plane [nfp].
 * @param[out] vr              the variance reduction for the linearized slip
 *                             inversion on the ifp'th fault plane [nfp]
 * @param[out] NN              the estimated displacement in the north component
 *                             (m) for the i'th site on the ifp'th fault plane
 *                             [l1*nfp].
 * @param[out] EN              the estimated displacement in the east component
 *                             (m) for the i'th site on the ifp'th fault plane
 *                             [11*nfp].
 * @param[out] UN              the estimated displacement in the the vertical
 *                             component (m) for the i'th site on the ifp'th
 *                             fault plane [l1*nfp].
 *
 * @param[inout] sslip_unc     if not NULL then this is the uncertainty
 *                             (diagonal of the resolution matrix) for the
 *                             slip along strike on the if'th fault patch
 *                             on the ifp'th fault plane [l2*nfp]
 * @param[inout] dslip_unc     if not NULL then this is the uncertainty
 *                             (diagonal of the resolution matrix) for the
 *                             slip down dip on the if'th fault patch
 *                             on the ifp'th fault plane [l2*nfp]
 *  
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @bug The units are unknown on the observations and slips are unknown 
 *
 */
int GFAST_FF__faultPlaneGridSearch(int l1, int l2, 
                                   int nstr, int ndip, int nfp,
                                   int verbose,
                                   double *nAvgDisp,
                                   double *eAvgDisp,
                                   double *uAvgDisp,
                                   double *utmRecvEasting,
                                   double *utmRecvNorthing,
                                   double *staAlt,
                                   double *fault_xutm,
                                   double *fault_yutm,
                                   double *fault_alt,
                                   double *length,
                                   double *width,
                                   double *strike,
                                   double *dip,
                                   double *sslip,
                                   double *dslip,
                                   double *Mw,
                                   double *vr,
                                   double *NN,
                                   double *EN,
                                   double *UN,
                                   double *sslip_unc,
                                   double *dslip_unc
                                   )
{
    const char *fcnm = "GFAST_FF__faultPlaneGridSearch\0";
    double *G2, *R, *S, *T, *UD, *UP, *xrs, *yrs, *zrs,
           ds_unc, lampred, len0, ss_unc, st, M0, res, wid0, xden, xnum;
    int i, ierr, ierr1, if_off, ifp, ij, io_off, j,
        mrowsG, mrowsG2, mrowsT, ncolsG, ncolsG2, ncolsT, ng, ng2, nt;
    bool lrmtx, lsslip_unc, ldslip_unc;
    // Initialize
    ierr = 0;
    xrs = NULL;
    yrs = NULL;
    zrs = NULL;
    G2 = NULL;
    UD = NULL;
    UP = NULL;
    T  = NULL;
    S  = NULL;
    R  = NULL;
    lrmtx = false;
    lsslip_unc = false;
    ldslip_unc = false;
    // Error handling
    if (l1 == 0 || l2 == 0 || l2 != nstr*ndip)
    {
        if (l1 == 0){log_errorF("%s: Error no observations\n", fcnm);}
        if (l2 == 0){log_errorF("%s: Error no points in fault plane\n", fcnm);}
        if (l2 != nstr*ndip){
            log_errorF("%s: Error l2 != nstr*ndip %d %d %d!\n",
                       fcnm, nstr, ndip);
        }
        return -1;
    }
    if (utmRecvEasting == NULL || utmRecvNorthing == NULL ||
        fault_xutm == NULL || fault_yutm == NULL ||
        length == NULL || width == NULL || strike == NULL || dip == NULL ||
        sslip == NULL || dslip == NULL)
    {
        if (utmRecvEasting == NULL){
            log_errorF("%s: Error utmRecvEasting is NULL\n", fcnm);
        }
        if (utmRecvNorthing == NULL){
            log_errorF("%s: Error utmRecvNorthing is NULL\n", fcnm);
        }
        if (fault_xutm == NULL){
            log_errorF("%s: Error fault_xutm is NULL\n", fcnm);
        }
        if (fault_yutm == NULL){
            log_errorF("%s: Error fault_yutm is NULL\n", fcnm);
        }
        if (length == NULL){log_errorF("%s: Error length is NULL\n", fcnm);}
        if (width == NULL){log_errorF("%s: Error width is NULL\n", fcnm);}
        if (strike == NULL){log_errorF("%s: Error strike is NULL\n", fcnm);}
        if (dip == NULL){log_errorF("%s: Error dip is NULL\n", fcnm);}
        if (sslip == NULL){log_errorF("%s: Error sslip is NULL\n", fcnm);}
        if (dslip == NULL){log_errorF("%s: Error dslip is NULL\n", fcnm);}
        return -1;
    }
    if (sslip_unc != NULL || dslip_unc != NULL){
        if (sslip_unc != NULL){lsslip_unc = true;}
        if (dslip_unc != NULL){ldslip_unc = true;}
        lrmtx = true;
    } 
    // Compute sizes of matrices in G2 S = UP where G2 = [G; T]
    mrowsG = 3*l1;
    ncolsG = 2*l2;
    mrowsT = 2*l2 + 2*(2*nstr + 2*(ndip - 2));
    mrowsT = 2*ndip*nstr + 2*(2*nstr+2*(ndip - 2));
    ncolsT = 2*l2;
    mrowsG2 = mrowsG + mrowsT;
    ncolsG2 = ncolsG;
    nt = mrowsT*ncolsT;
    ng = mrowsG*ncolsG;
    ng2 = mrowsG2*ncolsG2;
    // Set space
    xrs = GFAST_memory_calloc__double(l1*l2);
    yrs = GFAST_memory_calloc__double(l1*l2);
    zrs = GFAST_memory_calloc__double(l1*l2);
    G2 = GFAST_memory_calloc__double(ng2);
    UD = GFAST_memory_calloc__double(mrowsG2);
    UP = GFAST_memory_calloc__double(mrowsG2);
    T  = GFAST_memory_calloc__double(nt);
    S  = GFAST_memory_calloc__double(ncolsG2);
    if (lrmtx){R = GFAST_memory_calloc__double(ncolsG2*ncolsG2);}
    if (xrs == NULL || yrs == NULL || zrs == NULL ||
        G2 == NULL || UD == NULL || UP == NULL || T == NULL || S == NULL ||
        (lrmtx && R == NULL) ){
        if (xrs == NULL){log_errorF("%s: Error setting space for xrs\n", fcnm);}
        if (yrs == NULL){log_errorF("%s: Error setting space for yrs\n", fcnm);}
        if (zrs == NULL){log_errorF("%s: Error setting space for zrs\n", fcnm);}
        if (G2 == NULL){log_errorF("%s: Error setting space for G2\n", fcnm);}
        if (UD == NULL){log_errorF("%s: Error setting space for UP\n", fcnm);}
        if (UP == NULL){log_errorF("%s: Error setting space for UP\n", fcnm);}
        if (T  == NULL){log_errorF("%s: Error setting space for T\n",  fcnm);}
        if (S  == NULL){log_errorF("%s: Error setting space for S\n",  fcnm);}
        if (lrmtx && R == NULL){
            log_errorF("%s: Error setting space for R\n", fcnm);
        }
        ierr = 5;
        goto ERROR;
    }
    if (verbose > 2){
        log_debugF("%s: Setting RHS...\n", fcnm);
    }
    // Set the RHS
    ierr = GFAST_FF__setRHS(l1, verbose,
                            nAvgDisp, eAvgDisp, uAvgDisp, UD);
    if (ierr != 0){
        log_errorF("%s: Error setting right hand side\n", fcnm);
        ierr = 4;
        goto ERROR;
    }
    // Begin the grid search on fault planes
    ierr = 0;
    if (verbose > 2){
        log_debugF("%s: Beginning search on fault planes...\n", fcnm);
    }
#ifdef __PARALLEL_FF
    #pragma omp parallel for \
     firstprivate(G2, R, S, T, UP, xrs, yrs, zrs) \
     private(ds_unc, i, ierr1, ifp, if_off, ij, io_off, j, \
             lampred, len0, M0, res, ss_unc, st, wid0, xden, xnum) \
     shared(dip, dslip, dslip_unc, EN, fault_alt, fault_xutm, fault_yutm, \
            fcnm, l1, l2, ldslip_unc, length, lrmtx, lsslip_unc, \
            Mw, mrowsG2, mrowsG, ncolsG2, ndip, \
            nfp, ng, ng2, NN, nstr, nt, sslip, sslip_unc, staAlt, strike, \
            vr, UD, UN, utmRecvEasting, utmRecvNorthing, width) \
     reduction(+:ierr) default(none)
#endif
    // Loop on fault planes
    for (ifp=0; ifp<nfp; ifp++){
        // Set the offsets
        if_off = ifp*l2; // Offset the fault plane
        io_off = ifp*l1; // Offset the observations/estimates
        // Null out G2 and regularizer
        memset(G2, 0, ng2*sizeof(double));
        memset(T, 0, nt*sizeof(double));
        if (lrmtx){memset(R, 0, ncolsG2*ncolsG2*sizeof(double));}
        // Compute the site/fault patch offsets
        for (i=0; i<l2; i++){
            for (j=0; j<l1; j++){
                ij = l1*i + j;
                xrs[ij] = utmRecvEasting[j]  - fault_xutm[if_off+i];
                yrs[ij] = utmRecvNorthing[j] - fault_yutm[if_off+i];
                zrs[ij] = fault_alt[if_off+i]*1.e3 + staAlt[j];
            }
        }
        // Compute the forward modeling matrix (which is in row major format)
        ierr1 = GFAST_FF__setForwardModel__okadagreenF(l1, l2,
                                                       xrs, yrs, zrs,
                                                       &strike[if_off],
                                                       &dip[if_off],
                                                       &width[if_off],
                                                       &length[if_off],
                                                       G2);
        if (ierr1 != 0){
            log_errorF("%s: Error setting forward model\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Set the regularizer
        ierr1 = GFAST_FF__setRegularizer(l2,
                                         nstr, //ff->fp[ifp].nstr,
                                         ndip, //ff->fp[ifp].ndip,
                                         nt,
                                         &width[if_off],
                                         &length[if_off],
                                         T);
        if (ierr1 != 0){
            log_errorF("%s: Error setting regulizer\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Compute scale factor for regularizer
        len0 = length[if_off];
        wid0 = width[if_off];
        lampred = 1.0/pow( (double) l2*2.0, 2);
        lampred = lampred/(cblas_dasum(ng, G2, 1)/(double) ng);
        lampred = lampred/4.0*len0*wid0/1.e6;
        // Append lampred*T to G2
        cblas_daxpy(nt, lampred, T, 1, &G2[ng], 1);
        // Solve the least squares problem
        ierr1 = numpy_lstsq__qr(LAPACK_ROW_MAJOR,
                                mrowsG2, ncolsG2, 1, G2, UD,
                                S, NULL, R);
        if (ierr1 != 0){
            log_errorF("%s: Error solving least squares problem\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // From the right matrix compute the covariance matrix: inv(G^T*G)
        // Notice, because G = QR the inner term is G^T G = R^T Q^T Q R = R^T R
        // Thus, we must simply solve C = inv(R^T R) = inv(R) inv(R^T).  Hence
        // it is sufficient to compute inv(R) and multiply it by its transpose.
        // Futhermore, we only retain the diagonals so computing a full matrix
        // matrix multiply is unnecessary
        if (lrmtx){
            ierr1 = LAPACKE_dtrtri(LAPACK_ROW_MAJOR, 'U', 'N', ncolsG2,
                                   R, ncolsG2);
            if (ierr1 != 0){
                log_errorF("%s: Error inverting triangular matrix!\n", fcnm);
                ierr = ierr + 1;
                continue;
            }            
            for (i=0; i<l2; i++){
                ss_unc = cblas_ddot(ncolsG2, &R[2*(i+0)*ncolsG2], 1,
                                             &R[2*(i+0)*ncolsG2], 1);
                ds_unc = cblas_ddot(ncolsG2, &R[2*(i+1)*ncolsG2], 1,
                                             &R[2*(i+1)*ncolsG2], 1);
                if (lsslip_unc){sslip_unc[if_off+i] = ss_unc;} 
                if (ldslip_unc){dslip_unc[if_off+i] = ds_unc;}
            }
        }
        // Compute the forward problem 
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    mrowsG2, ncolsG2, 1.0, G2, ncolsG2, S, 1, 0.0, UP, 1);
        // Compute the variance reduction
        xnum = 0.0;
        xden = 0.0;
        #pragma omp simd reduction(+:xnum, xden)
        for (i=0; i<mrowsG; i++){
            res = UP[i] - UD[i];
            xnum = xnum + res*res;
            xden = xden + UD[i]*UD[i];
        }
        vr[ifp] = (1.0 - xnum/xden)*100.0;
        // Extract the estimates
        #pragma omp simd
        for (i=0; i<l1; i++){
            EN[io_off+i] = UP[3*i+0];
            NN[io_off+i] = UP[3*i+1];
            UN[io_off+i] = UP[3*i+2];
            //Einp[io_off+i] = UD[3*i+0];
            //Ninp[io_off+i] = UD[3*i+1];
            //Uinp[io_off+i] = UD[3*i+2];
         }
         // Extract the slip
         #pragma omp simd
         for (i=0; i<l2; i++){
             sslip[if_off+i] = S[2*i+0];
             dslip[if_off+i] = S[2*i+1];
         }
         // Compute the magnitude
         M0 = 0.0;
         #pragma omp simd reduction(+:M0)
         for (i=0; i<l2; i++){
             st = sqrt( pow(sslip[if_off+i], 2) + pow(dslip[if_off+i], 2) );
             M0 = M0 + 3.e10*st*length[if_off+i]*width[if_off+i];
         }
         Mw[ifp] = 0.0;
         if (M0 > 0.0){Mw[ifp] = (log10(M0*1.e7) - 16.1)/1.5;}
    } // Loop on fault planes 
    if (ierr != 0){
        log_errorF("%s: There was an error in the fault plane grid-search\n",
                   fcnm);
        ierr = 4;
    }else{
        if (verbose > 2){
            log_debugF("%s: Grid-search time: %f\n", fcnm, time); 
        }
    }
ERROR:;
    GFAST_memory_free__double(&xrs);
    GFAST_memory_free__double(&yrs);
    GFAST_memory_free__double(&zrs);
    GFAST_memory_free__double(&G2);
    GFAST_memory_free__double(&UD);
    GFAST_memory_free__double(&UP);
    GFAST_memory_free__double(&T);
    GFAST_memory_free__double(&S);
    GFAST_memory_free__double(&R);
    return ierr;
}
