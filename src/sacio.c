#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "gfast.h"
#include "sacio.h"

/*! 
 * @brief This is a routine for reading SAC headers from the SAC file flname
 * 
 * @param[in] flname     file name of which to read
 * @param[in] argc       number of arguments in header to read (nc + ni + nd)
 * @param[in] argv       list of SAC header variables to read [argc]
 * @param[in] itype      1 -> variable to write is type char
 *                       2 -> variable to read is type integer
 *                       3 -> variable to read is type float 
 *                       [argc]
 * @param[in] nc         number of character variables to read
 * @param[in] hdr_char   char header values to read from SAC header [nc]
 * @param[in] ni         number of integer variables to read
 * @param[in] hdr_ints   integer header values to read from SAC header [ni]
 * @param[in] nd         number of double variables to read
 * @param[in] hdr_dble   double header values to read from float SAC header [nd]
 *
 * @result < 0 there was an error
 *         = 0 if success
 *         > 0 some header variables may not have been read
 *
 * @author Ben Baker, ISTI
 *
 */
int sacio_readHeader(char *flname, int argc, char *argv[], int *itype,
                     int nc, char hdr_char[][16],
                     int ni, int *hdr_ints,
                     int nd, double *hdr_dble)
{
    const char *fcnm = "sacio_readHeader\0";
    char k8[9], k16[17]; // 8 and 16 are max space for a character 
                         // Extra space allows loops to not segfault
    float f4;
    int i, iwarn, lens, i4, nerr, k, k1, k2, k3, lenk;
    //------------------------------------------------------------------------//
    if (argc < 1){return 0;}
    if (nc + ni + nd != argc){
        log_warnF("%s: Size inconsistency\n", fcnm);
    } 
    // Require file exists
    if (!os_path_isfile(flname)){
        log_errorF("%s: Error SAC file %s does not exist\n", fcnm, flname);
        return -1;
    }
    // Read the header
    rsach(flname, &nerr, strlen(flname));
    if (nerr != 0) {
        log_errorF("%s: Error reading SAC header!\n", fcnm);
        return -1;
    }
    // Now get desired header info
    iwarn = 0; 
    k1 = 0;
    k2 = 0;
    k3 = 0;
    for (i = 0; i < argc; i++) {
        lens = strlen(argv[i]);
        if (itype[i] == 1) {    // Character info
            //strncpy(hdr_char[k1], "                ", 16);
            memset(hdr_char[k1],0,16);
            if (strcasecmp(argv[i], "KEVNM\0") == 0) {
                lens = 16;
                memset(k16,0,16);
                getkhv(argv[i], k16, &nerr, lens, lenk);
                if (nerr == 0){
                    // SAC doesn't NULL terminate strings
                    for (k=lens-1; k>=0; k--){
                        if (!isspace(k16[k])){
                            lens = k+1;
                            break;
                        }
                    }
                    strncpy(hdr_char[k1], k16, lens);
                }else{ // Error reading
                    strcpy(hdr_char[k1],"-12345\0");
                }
            }else{
                lenk = 8;
                memset(k8,0,8);
                getkhv(argv[i], k8, &nerr, lens, lenk);
                if (nerr == 0){
                    // SAC doesn't NULL terminate strings
                    for (k=lens-1; k>=0; k--){
                        if (!isspace(k8[k])){
                            lens = k+1;
                            break;
                        }
                    }
                    strncpy(hdr_char[k1], k8, lens);
                }else{ // Error reading
                    strcpy(hdr_char[k1],"-12345\0");
                }
            }
            k1 = k1 + 1;
        } else if (itype[i] == 2) {     // Integer info
            hdr_ints[k2] = 0;
            getnhv(argv[i], &i4, &nerr, lens);
            if (nerr == 0) {
                hdr_ints[k2] = i4;
            }else{ // Error reading
                hdr_ints[k2] =-12345;
            }
            k2 = k2 + 1;
        } else if (itype[i] == 3) {     // Float info
            hdr_dble[k3] = 0.0;
            getfhv(argv[i], &f4, &nerr, lens);
            if (nerr == 0) {
                hdr_dble[k3] = (double) f4;
            }else{
                hdr_dble[k3] =-12345.0;
            }
            k3 = k3 + 1;
        } else {
            log_errorF("%s: Error unknown header type!\n", fcnm);
            return -1; // Will likely cause a segmentation fault
        }
        if (nerr != 0) {
            iwarn = iwarn + 1;
        }
    }
    return iwarn;
}
//============================================================================//
/*!
 * @brief Reads SAC data from file
 *
 * @param[in] flname    name of SAC file to read
 *
 * @param[out] npts     number of points in trace
 * @param[out] ierr     0 indicates success
 *
 * @result seismic data trace [npts]
 *
 * @author Ben Baker, ISTI
 *
 */
double *sacio_readData(char *flname, int *npts, int *ierr)
{
    const char *fcnm = "sacio_readData\0";
    double *data = NULL;
    float *y, beg, del;
    int i, max, nlen, nerr;
    //------------------------------------------------------------------------//
    //
    // Require file exists
    *ierr = 0;
    if (!os_path_isfile(flname)){
        log_errorF("%s: Error SAC file %s does not exist\n", fcnm, flname);
        *ierr = 1;
        return data;
    }
    // Get number of data points
    nerr = 0;
    rsach(flname, &nerr, strlen(flname));
    if (nerr != 0){ 
        log_errorF("%s: Error reading SAC header\n", fcnm);
        *ierr = 1;
        return data;
    }   
    getnhv("NPTS", npts, &nerr, strlen("NPTS\0"));
    if (nerr != 0){ 
        log_errorF("%s: Error getting number of points\n", fcnm);
        *ierr = 1;
        return data; 
    }
    nlen = *npts;
    max = *npts;
    // Set workspace
    y = (float *)calloc(*npts, sizeof(float));
    if (y == NULL){
        log_errorF("%s: Error allocating workspace\n", fcnm);
        *ierr = 1;
        return data;
    }
    // Read
    rsac1(flname, y, &nlen, &beg, &del, &max, &nerr, strlen(flname));
    if (nerr != 0) {
        log_errorF("%s: There was an error reading the SAC file: %s\n",
                   fcnm, flname);
        if (y != NULL){free(y);}
        if (data != NULL){free(data);}
        *ierr = 1;
        return data;
    }
    // Copy single precision to double precision
    data = (double *)calloc(nlen, sizeof(double));
    if (data == NULL){
        log_errorF("%s: Error allocating output\n", fcnm);
        *ierr = 1;
        return data; 
    }
    for (i = 0; i<nlen; i++) {
        data[i] = (double) y[i];
    }
    free(y);
    return data;
}
//============================================================================//
/*! 
 * @brief This is a routine for writing SAC headers and data to file
 *        flname
 * 
 * @param[in] flname    file name of which to write
 * @param[in] argc      number of arguments in header to write (nc + ni + nd)
 * @param[in] argv      list of SAC header variables to write [argc]
 * @param[in] nc        number of character header variables to write
 * @param[in] hdr_char  char values to write to SAC header [nc]
 * @param[in] ni        number of integer header variables to write
 * @param[in] hdr_ints  integer values to write to SAC header [ni]
 * @param[in] nd        number of double header varialbes to write
 * @param[in] hdr_dble  double values to write to float SAC header [nd]
 * @param[in] itype     1 -> variable to write is type char
 *                      2 -> variable to write is type integer
 *                      3 -> variable to write is type float 
 *                      [argc]
 * @param[in] npts      number of data points
 * @param[in] data      data to write [npts]
 *
 * @result 0 if success
 *
 * @author Ben Baker, ISTI
 *
 */
int sacio_writeTrace(char *flname, int argc, char *argv[], int *itype,
                     int nc, char hdr_char[][16],
                     int ni, int *hdr_ints,
                     int nd, double *hdr_dble,
                     int npts, double *data)
{
    const char *fcnm = "sacio_writeTrace\0";
    char *sacvar, *kvar;
    float *y, *x, f4, dt;
    int i, i4, lens, lenk, nerr, k1, k2, k3, iwarn, npts_use;
    //------------------------------------------------------------------------//
    //
    // Basic check
    if (flname == NULL){
        log_errorF("%s: Output filename undefined\n", fcnm);
        return -1;
    }
    if (nc + ni + nd != argc){
        log_warnF("%s: Size inconsistency\n", fcnm);
    }
    // Make a new header
    newhdr();
    dt = -1.0;
    k1 = 0;
    k2 = 0;
    k3 = 0, iwarn = 0, npts_use = -1;
    for (i = 0; i < argc; i++){
        sacvar = strdup(argv[i]);
        lens = strlen(sacvar);
        if (itype[i] == 1) {
            kvar = strdup(hdr_char[k1]);
            lenk = strlen(kvar);
            setkhv(sacvar, kvar, &nerr, lens, lenk);
            if (nerr != 0) {
                log_errorF("%s: Error setting header variable! (a)",fcnm);
                return -1;
            }
            free(kvar);
            k1 = k1 + 1;
        }else if (itype[i] == 2){
            i4 = hdr_ints[k2];
            if (strcasecmp(sacvar, "NPTS\0") == 0) {
                i4 = fmin(npts, i4);
                npts_use = npts;
            }
            setnhv(sacvar, &i4, &nerr, lens);
            if (nerr != 0) {
                log_errorF("%s: Error setting header variable! (b)",fcnm);
                return -1;
            }
            k2 = k2 + 1;
        }else if (itype[i] == 3){
            f4 = (float) hdr_dble[k3];
            if (strcasecmp(sacvar, "DELTA\0") == 0) {
                dt = f4;
            }
            setfhv(sacvar, &f4, &nerr, lens);
            if (nerr != 0) {
                log_errorF("%s: Error setting header variable! (c)",fcnm);
                return -1;
            }
            k3 = k3 + 1;
        }else{
            log_warnF("%s: Warning unknown header type %d!  Skipping...\n",
                      fcnm, itype[i]);
        }
        if (nerr != 0) {
            iwarn = iwarn + 1;
        }
        free(sacvar);
    }
    if (npts_use == -1) {
        npts_use = npts;
        i4 = npts_use;
        setnhv("NPTS", &i4, &nerr, strlen("NPTS"));
    }
    if (npts_use != npts) {
        log_warnF("%s: Warning number of points may differ!\n", fcnm);
    }
    if (dt == -1.0) {
        log_errorF("%s: Error you must specify a sampling rate!\n", fcnm);
        return -1;
    }
    // Create the time series
    x = (float *)calloc(npts_use, sizeof(float));
    y = (float *)calloc(npts_use, sizeof(float));
    for (i = 0; i < npts_use; i++) {
        x[i] = dt * (float) i;
        y[i] = (float) data[i];
    }
    wsac0(flname, x, y, &nerr, strlen(flname));
    if (nerr != 0) {
        log_errorF("%s: Error calling wsac0: %d!\n",fcnm,nerr);
        return -1;
    }
    free(x);
    free(y);
    x = NULL;
    y = NULL;
    return iwarn;
}
