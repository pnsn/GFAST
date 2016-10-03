#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include "gfast.h"
#include "iscl/log/log.h"

/*!
 * @brief Updates a data buffer with data from t0 to t0 + (npts - 1)*dt
 *        with the data in a buffer which spans time
 *        t0_buf to t0_buf + (npbuf - 1)*dt_buf.
 *        If the input buffer extends past the limits of the data buffer
 *        then the data closest to the end of buffer will be retained in
 *        data, i.e. the most recent data.
 * 
 * @param[in] maxdat   maximum number of points allowed in data
 * @param[in] verbose  controls verbosity.   if less than 2 will only
 *                     report on errors.
 * @param[in] npbuf    number of samples in update buffer
 * @param[in] dt       sampling period (s) of data
 * @param[in] gapval   default value to insert into any gaps
 * @param[in] t0_buf   start time (s) of update array
 * @param[in] dt_buf   sampling period (s) of update array (this must be 
 *                     equivalent to dt)
 * @param[inout] npts  corresponds to the number of points held in the 
 *                     data array.  thus, the time of the final sample 
 *                     of data is given by t0 + (npts - 1)*dt.
 * @param[inout] t0    corresponds to the start time (s) of the first index
 *                     of data.
 * @param[inout] data  on input holds the current data buffer.
 *                     on output contains as much new data as possible from
 *                     buf [maxdat]
 * @param[inout] work  workspace for copying data.  if not NULL then this
 *                     must be of length maxdat.  
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_buffer__updateBuffer(int maxdat, int verbose,
                               int npbuf, double dt,
                               double gapval,
                               double t0_buf, double dt_buf,
                               double *__restrict__ buf,
                               int *npts, double *t0,
                               double *__restrict__ data,
                               double *__restrict__ work)
{
    const char *fcnm = "GFAST_buffer__updateBuffer\0";
    double t1, t1_buf;
    int i, i0, j0, npadd, npcopy, nrem, nshift;
    bool lwork;
    // Errors
    if (dt <= 0.0){
        log_errorF("%s: Error sampling period must be positive\n", fcnm);
        return -1;
    }
    if (dt != dt_buf){
        log_errorF("%s: Error sampling periods are unequal\n", fcnm);
        return -1;
    }
    // There's nothing to copy
    if (npbuf == 0){return 0;}
    // The buffer has not been initialized or this is a straight copy because
    // we have exceeded the buffer bounds
    t1_buf = t0_buf + (double) (npbuf - 1)*dt;
    if (*npts == 0 || t1_buf < *t0){
        i0 = fmax(npbuf - maxdat, 0);
        *t0 = t0_buf + (double) i0*dt;
        npadd = npbuf - i0;
        #pragma omp simd
        for (i=0; i<npadd; i++){
            data[i] = buf[i0+i];
        }
        *npts = npadd;
        return 0;
    } 
    npcopy = (int) ((t1_buf - *t0)/dt + 0.5) + 1;
    t1 = *t0 + (double) (*npts - 1)*dt;
    // The buffer fits into the tail of data
    if (npcopy <= maxdat){
        // A gap exists
        j0 = *npts;
        if (t0_buf > t1 + dt){
            if (verbose > 2){
                log_debugF("%s: A gap exists\n", fcnm);
            }
            npadd = (int) ((t0_buf - t1 - dt)/dt + 0.5);
            #pragma omp simd
            for (i=0; i<npadd; i++){
                data[j0+i] = gapval;
            }
            i0 = 0;
        }else{
            i0 = (int) ((t1 - t0_buf)/dt + 0.5) + 1;
            i0 = fmin(i0, npbuf-1);
            npadd = (int) ((t1_buf - t1)/dt + 0.5);
        }
        // Fill the rest in
        #pragma omp simd
        for (i=0; i<npadd; i++){
            data[j0+i] = buf[i0+i];
        }
        *npts = *npts + npadd;
        return 0;
    // The buffer does not fit into data and a shift must be applied
    }else{
        // Figure out if i can use workspace
        lwork = false;
        if (work != NULL){lwork = true;}
        // Copy the shifted memory
        nshift = npcopy - maxdat;
        // There's a chance nshift exceeds what the data can hold
        if (nshift > maxdat){
            i0 = npbuf - maxdat;
            npcopy = maxdat;
            for (i=0; i<npcopy; i++){
                data[i] = buf[i0+i];
            }
            *t0 = t0_buf + (double) i0*dt;
            *npts = maxdat;
            return 0;
        }
        // Otherwise copy shifted memory from smaller to larger buffer
        nrem = *npts - nshift;
        if (lwork){
            #pragma omp simd
            for (i=0; i<nrem; i++){
                work[i] = data[nshift+i];
            }
        }else{
            for (i=0; i<nrem; i++){
                data[i] = data[nshift+i];
            }
        }
        // Add the tail
        i0 = (int) ((t1 - t0_buf)/dt + 0.5) + 1;
        i0 = fmin(i0, npbuf - 1);
        t1 = t1 + (double) (nshift - 1)*dt;
        npadd = npbuf - i0;
        if (lwork){
            #pragma omp simd
            for (i=0; i<npadd; i++){
                work[nrem+i] = buf[i0+i];
            }
        }else{
            for (i=0; i<npadd; i++){
                data[nrem+i] = buf[i0+i];
            }
        }
        *npts = maxdat;
        *t0 = *t0 + (double) nshift*dt;
        if (lwork){
            #pragma omp simd
            for (i=0; i<maxdat; i++){
                data[i] = work[i];             
            }
        }
    }
    return 0;
}

/*
int main()
{
    const char *fcnm = "__circularBuffer_tester\0";
    int maxdat = 49;
    int nacq = 2*maxdat;
    int npbuf = 10;
    int verbose = 5;
    int i, iacq, ierr, j, jacq, npts;
    int nt = fmax(npbuf/2,1); //5;
    double gapval = NAN;
    double *work, *buf, *data, dt, dt_buf, t0, t0_buf, targ;
    work = NULL;
    buf  = (double *)calloc(npbuf, sizeof(double));
    data = (double *)calloc(maxdat, sizeof(double));
//work = (double *)calloc(fmax(maxdat, npbuf), sizeof(double));
    dt = 1.0;
    t0 = 0.0;
    t0_buf = 0.0;
    dt_buf = dt;
    npts = 0;
    jacq = 0;
    for (iacq=0; iacq<nacq; iacq=iacq+nt){
        // Fill up the acquisition
        for (i=0; i<npbuf; i++){
            buf[i] = (double) (nt*jacq + i)*dt;
        }
        t0_buf = buf[0]; //(double) iacq*dt;
        ierr = GFAST_buffer__updateBuffer(maxdat, verbose,
                                          npbuf, dt,
                                          gapval,
                                          t0_buf, dt_buf,
                                          buf,
                                          &npts, &t0,
                                          data, work);
        if (ierr != 0){
            printf("Error encountered %d\n", iacq);
            goto ERROR; 
        }
        if (t0 != data[0]){
            printf("%s: Error lost track of time %f %f\n", fcnm, t0, data[0]);
            ierr = 1;
            goto ERROR;
        }
        // Check the data
        if (data[npts-1] != buf[npbuf-1]){
for (i=0; i<npts; i++){printf("%f\n", data[i]);}
printf("\n");
for (i=0; i<npbuf; i++){printf("%f\n", buf[i]);}
            printf("%s: Failed to copy entire array! %d %f %f\n", fcnm,
                   npts, data[npts-1], buf[npbuf-1]);
            ierr = 1;
            goto ERROR;
        }
        for (i=0; i<npts; i++){
            targ = data[npts-1] - (double) (npts - 1 - i)*dt;
            if (data[i] != targ){
                printf("%s: Buffer is out of sync %f %f\n", fcnm, data[i], targ);
                ierr = 1;
                goto ERROR;
            }
        }
        jacq = jacq + 1;
    } 
ERROR:;
    free(buf);
    free(data);
    if (work != NULL){free(work);}
    if (ierr != 0){
        return EXIT_FAILURE;
    }else{
        return EXIT_SUCCESS;
    }
}
*/
