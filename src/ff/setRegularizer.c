#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "gfast.h"
/*!
 * @brief Computes the second order Tikhonov regularizer along
 *        strike and down dip for slip along strike and slip down
 *        dip
 *
 * @param[in] l2       total number of faults in plane (nstr*ndip)
 * @param[in] nstr     number of faults along strike
 * @param[in] ndip     number of faults down dip
 * @param[in] nt       length of T
 * @param[in] width    width of fault patches (km) [l2]
 * @param[in] length   length of fault patches (km) [l2]
 *
 * @param[out] T       second order Tikonov regularizer.  T is dimension
 *                     [2*l2 + 2*(2*nstr + 2*(ndip-2)) x 2*l2] with
 *                     leading dimension 2*l2 stored in row major
 *                     order.
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_FF__setRegularizer(int l2, int nstr, int ndip, int nt,
                             double *__restrict__ width,
                             double *__restrict__ length,
                             double *__restrict__ T)
{
    const char *fcnm = "GFAST_FF__setRegularizer\0";
    double len02i, lnwidi, wid02i;
    int i, indx1, indx2, indx3, indx4, indx5, j, k, l, ldt,
           kndx1, kndx2, kndx3, kndx4, kndx5, m, ntref;
    //------------------------------------------------------------------------//
    //
    // Error handling
    ntref = (2*l2 + 2*(2*nstr + 2*(ndip-2)))*2*l2;
    if (nstr < 1 || ndip < 1 || l2 != nstr*ndip || nt != ntref){
        if (nstr < 1){
            log_errorF("%s: Error no faults along strike\n", fcnm);
        }
        if (ndip < 1){
            log_errorF("%s: Error no faults down dip\n", fcnm);
        }
        if (nstr*ndip != l2){
            log_errorF("%s: Error size inconsistency\n", fcnm);
        }
        if (nt != ntref){
            log_errorF("%s: Error nt is not proper size\n", fcnm);
        }
        return -1;
    }
    if (width == NULL || length == NULL || T == NULL){
        if (width == NULL){
            log_errorF("%s: Error width can't be NULL\n", fcnm);
        }
        if (length == NULL){
            log_errorF("%s: Error length can't be NULL\n", fcnm);
        }
        if (T == NULL){
            log_errorF("%s: Error regularizer T can't be NULL\n", fcnm);
        }
    }
    // Null out regularizer
    #pragma omp simd
    for (i=0; i<nt; i++){
        T[i] = 0.0;
    }
    // Fill in the regularizer
    ldt = 2*l2;
    for (j=0; j<ndip; j++){
        for (i=0; i<nstr; i++){
            for (m=0; m<2; m++){
                l = i*nstr + j;         // Fault patch number
                k = j*nstr*m + i*m + m; // Row number
                wid02i = 1.0/(width[l]*width[l])*1.e6;   // 1/dy^2; km^2 -> m^2
                len02i = 1.0/(length[l]*length[l])*1.e6; // 1/dx^2; km^2 -> m^2
                lnwidi = 1.0/(width[l]*length[l])*1.e6;  // 1/dx/dy; km^2 -> m^2
                indx1 =       j*nstr  + i;     // Grid index
                indx2 =       j*nstr  + i - 1; // Backward along strike
                indx3 =       j*nstr  + i + 1; // Forward along strike 
                indx4 = (j - 1)*nstr + i;      // Backward along dip (up dp)
                indx5 = (j + 1)*nstr + i;      // Forward along dip (down dip)
                kndx1 = k*ldt + 2*indx1 + m;   // T[k,2*index1+m]
                kndx2 = k*ldt + 2*indx2 + m;   // T[k,2*index2+m]
                kndx3 = k*ldt + 2*indx3 + m;   // T[k,2*index3+m]
                kndx4 = k*ldt + 2*indx4 + m;   // T[k,2*index4+m]
                kndx5 = k*ldt + 2*indx5 + m;   // T[k,2*index5+m]
                // Fill s.t. the column numbers always increase 
                if (indx4 >= 0 && indx4 < l2){
                    T[kndx4] = wid02i;
                }
                if (indx2 >= 0 && indx2 < l2){
                    T[kndx2] = len02i;
                }
                if (indx1 >= 0 && indx1 < l2){
                    T[kndx1] =-(2.0*len02i + 2.0*wid02i);
                }
                // Boundary condiiton: NOTE no check on j == 0 
                if (j == ndip - 1 || i == 0 || i == nstr - 1){
                    T[kndx1] = 100.0*lnwidi;
                }
                if (indx3 >= 0 && indx3 < l2){
                    T[kndx3] = len02i;
                }
                if (indx5 >= 0 && indx5 < l2){
                    T[kndx5] = wid02i;
                }
            } // Loop on m
        } // Loop on strike
    } // Loop on dip
    return 0;
}
