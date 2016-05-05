#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"
#include "sacio.h"

/*!
 * @brief Copies data on the GPS data buffer to arrays for 
 *        processing in the inversions
 *
 * @param[in] job         = 1 -> get data for PGD estimation.
 *                        = 2 -> get data for CMT estimation. 
 *                        = 3 -> get data for finite fault estimation.
 * @param[in] ndmax       size of ubuff, nbuff, ebuff, and tbuff
 * @param[in] nwaves      number of waveforms retained in data_ptr
 * @param[out] ubuff      holds the vertical displacmenet for all waveforms.
 *                        the first sample of the k'th waveform is given by
 *                        data_ptr[k].
 * @param[out] nbuff      holds the north displacmenet for all waveforms.
 *                        the first sample of the k'th waveform is given by
 *                        data_ptr[k].
 * @param[out] ebuff      holds the east displacmenet for all waveforms.
 *                        the first sample of the k'th waveform is given by
 *                        data_ptr[k].
 * @param[out] tbuff      holds the times (s) at which the ubuff, nbuff,
 *                        and ebuff were evaluted at.  the index for the
 *                        first sample of the k'th waveform is is given by
 *                        data_ptr[k]. 
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_buffer__buffer2array(int job,
                               struct GFAST_data_struct *gps_data,
                               int ndmax, int nwaves, int *data_ptr,
                               double *__restrict__ ubuff,
                               double *__restrict__ nbuff,
                               double *__restrict__ ebuff,
                               double *__restrict__ tbuff)
{
    int i, jpt, k, kwave;
    // Initialize
    kwave = 0;
    jpt = 0;
    // PGD
    if (job == 1){
        data_ptr[0] = 0;
        for (k=0; k<gps_data->stream_length; k++){
            if (gps_data->data[k].lskip_pgd){continue;}
            data_ptr[kwave] = data_ptr[kwave] + gps_data->data[k].npts;
            #pragma omp simd
            for (i=0; i<gps_data->data[k].npts; i++){
                ubuff[jpt] = gps_data->data[k].ubuff[i];
                nbuff[jpt] = gps_data->data[k].nbuff[i];
                ebuff[jpt] = gps_data->data[k].ebuff[i];
                jpt = jpt + 1;
            }
            data_ptr[kwave+1] = jpt;
            kwave = kwave + 1;
        }
    // CMT
    }else if (job == 2){
        data_ptr[0] = 0;
        for (k=0; k<gps_data->stream_length; k++){
            if (gps_data->data[k].lskip_cmt){continue;}
            data_ptr[kwave] = data_ptr[kwave] + gps_data->data[k].npts;
            #pragma omp simd
            for (i=0; i<gps_data->data[k].npts; i++){
                ubuff[jpt] = gps_data->data[k].ubuff[i];
                nbuff[jpt] = gps_data->data[k].nbuff[i];
                ebuff[jpt] = gps_data->data[k].ebuff[i]; 
                jpt = jpt + 1;
            }
            data_ptr[kwave+1] = jpt;
            kwave = kwave + 1; 
        }
    // Finite fault
    }else if (job == 3){
        data_ptr[0] = 0;
        for (k=0; k<gps_data->stream_length; k++){
            if (gps_data->data[k].lskip_ff){continue;}
            data_ptr[kwave] = data_ptr[kwave] + gps_data->data[k].npts;
            #pragma omp simd
            for (i=0; i<gps_data->data[k].npts; i++){
                ubuff[jpt] = gps_data->data[k].ubuff[i];
                nbuff[jpt] = gps_data->data[k].nbuff[i];
                ebuff[jpt] = gps_data->data[k].ebuff[i]; 
                jpt = jpt + 1;
            }
            data_ptr[kwave+1] = jpt; 
            kwave = kwave + 1;  
        }
    }
    return 0;
}
//============================================================================//
/*!
 * @brief Reads desired data from SAC files into the gps_data buffer
 *
 * @param[in] job          =1 -> initialize sampling period from SAC files
 *                         =2 -> initialize locations (lat, lon) from SAC files
 *                         =3 -> initialize buffer lengths from SAC files
 *                         =4 -> initialize epoch from SAC files
 *                         =5 -> initialize data from SAC files
 *                         =6 -> initialize everything
 * @param[in] props        holds the data directory and channel prefix (LX)
 *
 * @param[inout] gps_data  on input holds the initialized gps_data structure
 *                         on output holds the desired information corresponding
 *                         to job
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_buffer__readDataFromSAC(int job,
                                  struct GFAST_props_struct props,
                                  struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_buffer__readFromSAC\0";
    enum sac_init_enum
    {
        SET_DT = 1,
        SET_LOC = 2,
        SET_NPTS = 3,
        SET_EPOCH = 4,
        SET_DATA = 5,
        SET_ALL = 6
    };
    char rootfl[PATH_MAX], fname[PATH_MAX], hdr_char[1][16];
    char *argv[11] = {"NPTS\0",
                      "NZYEAR\0", "NZJDAY\0", "NZHOUR\0",
                      "NZMIN\0", "NZSEC\0", "NZMSEC\0",
                      "DELTA\0", "STLA\0", "STLO\0", "STEL\0"};
    double hdr_dble[4];
    int itype[11] = {2,
                     2, 2, 2,
                     2, 2, 2,
                     3, 3, 3, 3};
    int hdr_ints[7];
    int i, ierr, ierr1, ierr2, k, nzyear, nzjday, nzhour, nzmin, nzmusec, nzsec;
    const int nc = 0; // No character header data to read
    const int ni = 7; // Number of itnergers to read
    const int nd = 4; // Number of doubles to read 
    const int argc = nc + ni + nd;
    //------------------------------------------------------------------------//
    if (gps_data == NULL){
        log_errorF("%s: Error NULL pointer detected\n", fcnm);
        return -1;
    }
    if (gps_data->stream_length < 1){
        log_errorF("%s: No data\n", fcnm);
        return -1;
    }
    // Loop on sac files
    for (k=0; k<gps_data->stream_length; k++){
         // This has been masked earlier
         if (gps_data->data[k].lskip_pgd &&
             gps_data->data[k].lskip_cmt &&
             gps_data->data[k].lskip_ff){continue;}
         // Set the file name
         memset(rootfl, 0, sizeof(rootfl));
         strcpy(rootfl, props.syndata_dir);
         if (strlen(rootfl) > 0){
             if (rootfl[strlen(rootfl)-1] != '/'){
                 strcat(rootfl, "/\0");
             }
        }
        if (!os_path_isdir(rootfl)){
            log_errorF("%s: Error data directory %s doesn't exist\n",
                       fcnm, rootfl);
            return -1;
        }
        strcat(rootfl, gps_data->data[k].site);
        strcat(rootfl, ".\0"); 
        strcat(rootfl, props.syndata_pre);
        ierr = 0;
        // Get the header from the Z component 
        ierr2 = 0;
        memset(fname, 0, sizeof(fname));
        strcpy(fname, rootfl);
        strcat(fname, "Z.sac\0");
        ierr1 = sacio_readHeader(fname, argc, argv, itype,
                                 nc, hdr_char,
                                 ni, hdr_ints,
                                 nd, hdr_dble);
        if (ierr1 != 0){
            log_errorF("%s: Error reading header!\n", fcnm);
            ierr2 = ierr2 + 1;
            ierr = ierr + 1;
            continue;
        }
        // Set station sampling period
        if (job == SET_DT || job == SET_ALL){
            gps_data->data[k].dt = hdr_dble[0];
            if (gps_data->data[k].dt < 0.0){
                log_errorF("%s: Invalid sampling period: %f\n",
                           fcnm, gps_data->data[k].dt);
                gps_data->data[k].dt = props.dt_default; 
                log_warnF("%s: Overriding to default\n: %f\n",
                          fcnm, gps_data->data[k].dt);
            }
            if (job == SET_DT){continue;}
        }
        // Set station location
        if (job == SET_LOC || job == SET_ALL){
            gps_data->data[k].sta_lat = hdr_dble[1];
            gps_data->data[k].sta_lon = hdr_dble[2];
            gps_data->data[k].sta_alt = hdr_dble[3];
            if (gps_data->data[k].sta_lat <-90.0 || 
                gps_data->data[k].sta_lat > 90.0){
                log_errorF("%s: Invalid station latitude: %f\n",
                           fcnm, gps_data->data[k].sta_lat);
                gps_data->data[k].lskip_pgd = true;
                gps_data->data[k].lskip_cmt = true;
                gps_data->data[k].lskip_ff = true;
            }
            if (gps_data->data[k].sta_lon < 0.0){
                gps_data->data[k].sta_lon = gps_data->data[k].sta_lon + 360.0;
            }
            if (job == SET_LOC){continue;}
        }
        // Set number of points
        if (job == SET_NPTS || job == SET_ALL){
            gps_data->data[k].npts = hdr_ints[0];
            if (job == SET_NPTS){continue;}
        }
        // Get the epoch
        if (job == SET_EPOCH || job == SET_ALL){
            nzyear = hdr_ints[1];       // year
            nzjday = hdr_ints[2];       // julian day
            nzhour = hdr_ints[3];       // hour
            nzmin  = hdr_ints[4];       // minute
            nzsec  = hdr_ints[5];       // second
            nzmusec = hdr_ints[6]*1000; // millisecond -> microsecond
            gps_data->data[k].epoch
                = time_calendar2epoch(nzyear, nzjday, nzhour,
                                      nzmin, nzsec, nzmusec);
            if (job == SET_EPOCH){continue;}
        }
        // Do we have data?
        if (gps_data->data[k].npts < 1){
            log_errorF("%s: No data points to read for site %s!\n",
                       fcnm, gps_data->data[k].site); 
            ierr = ierr + 1;
            continue;
        }
        // Make sure the buffers aren't allocated 
        if (gps_data->data[k].ubuff != NULL){free(gps_data->data[k].ubuff);}
        if (gps_data->data[k].nbuff != NULL){free(gps_data->data[k].nbuff);}
        if (gps_data->data[k].ebuff != NULL){free(gps_data->data[k].ebuff);}
        if (gps_data->data[k].tbuff != NULL){free(gps_data->data[k].tbuff);}
        // Z
        gps_data->data[k].ubuff = sacio_readData(fname, &gps_data->data[k].npts,
                                                 &ierr1);
        if (ierr1 != 0){ierr2 = ierr2 + 1;}
        // N
        memset(fname, 0, sizeof(fname));
        strcpy(fname, rootfl);
        strcat(fname, "N.sac\0");
        gps_data->data[k].nbuff = sacio_readData(fname, &gps_data->data[k].npts,
                                                 &ierr1);
        if (ierr1 != 0){ierr2 = ierr2 + 1;}
        // E
        memset(fname, 0, sizeof(fname));
        strcpy(fname, rootfl);
        strcat(fname, "E.sac\0");
        gps_data->data[k].ebuff = sacio_readData(fname, &gps_data->data[k].npts,
                                                 &ierr1);
        // Update time
        gps_data->data[k].tbuff
           = GFAST_memory_calloc__double(gps_data->data[k].npts);
        #pragma omp simd
        for (i=0; i<gps_data->data[k].npts; i++){
            gps_data->data[k].tbuff[i] = gps_data->data[k].epoch
                                       + (double) i*gps_data->data[k].dt;
        }
        if (ierr1 != 0){ierr2 = ierr2 + 1;}
        // Set the times
        if (ierr2 > 0){
            log_errorF("%s: Error reading data for site: %s!\n",
                       fcnm, gps_data->data[k].site);
            gps_data->data[k].lskip_pgd = true;
            gps_data->data[k].lskip_cmt = true;
            gps_data->data[k].lskip_ff = true;
            ierr = ierr + 1;
        }
    } // Loop on sac files
    return 0;
}
//============================================================================//
/*!
 * @brief Sets the site names and locations in the GPS data structure 
 *
 * @param[in] props       GFAST properties
 *
 * @param[out] gps_data   on successful exit holds the nstream data stream's
 *                        site names and locations
 *
 * @result 0 indicates success
 *
 */
int GFAST_buffer__setSitesAndLocations(struct GFAST_props_struct props,
                                       struct GFAST_data_struct *gps_data)
{
    FILE *sitefile, *streamfile;
    const char *fcnm = "GFAST_buffer__setSitesAndLocations\0";
    char *string[256], buf[1024], site[64];
    char delimit[] = " ,;\t\r\n\v\f";
    double lat, lon;
    int i, ierr, k;
    bool *lfound, ldone, nuse;
    //------------------------------------------------------------------------//
    //
    // Verify there is data
    ierr = 0;
    if (gps_data == NULL){
        log_errorF("%s: Error NULL pointer detected!\n", fcnm);
        return -1;
    }
    if (gps_data->stream_length < 1){
        log_errorF("%s: Error no streams to load!\n", fcnm);
        return -1;
    } 
    // Read the streams for this processing
    lat =-12345.0;
    lon =-12345.0;
    streamfile = fopen(props.streamfile, "r");
    for (k=0; k<gps_data->stream_length; k++){
        memset(&gps_data->data[k], 0,
               sizeof(struct GFAST_collocatedData_struct));
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, 1024, streamfile) == NULL){
            log_errorF("%s: Premature end of stream file!\n", fcnm);
            fclose(streamfile);
            return -1;
        }
        sscanf(buf, "%s\n", site);
        strcpy(gps_data->data[k].site, site);
        gps_data->data[k].sta_lat = lat;
        gps_data->data[k].sta_lon = lon;
    }
    fclose(streamfile);
    // Attach the positions
    lfound = (bool *)calloc(gps_data->stream_length, sizeof(bool));
    if (props.loc_init == INIT_LOCS_FROM_SAC){
        for (k=0; k<gps_data->stream_length; k++){
            lfound[k] = true;
        }
        // Read the positions from the SAC header
        ierr = GFAST_buffer__readDataFromSAC(2, props, gps_data);
        if (ierr != 0){
            for (k=0; k<gps_data->stream_length; k++){
                if (gps_data->data[k].lskip_pgd &&
                    gps_data->data[k].lskip_cmt &&
                    gps_data->data[k].lskip_ff)
                {
                    lfound[k] = false; 
                }
            }
        }
    }else{
        sitefile = fopen(props.siteposfile, "r");
        memset(buf, 0, sizeof(buf));
        while (fgets(buf, 1024, sitefile) != NULL){
            memset(site, 0, sizeof(site));
            i = 0;
            lat =-12345.0;
            lon =-12345.0;
            string[i] = strtok(buf, delimit);
            while (string[i] != NULL){
                if (i == 0){strcpy(site, string[i]);}
                if (i == 8){lat = (double) atof(string[i]);}
                if (i == 9){lon = (double) atof(string[i]);}
                i = i + 1;
                string[i] = strtok(NULL, delimit);
            }
            if (lat <-90.0 || lat > 90.0){
                if (props.verbose > 1){
                    log_warnF("%s: Station lat %f is out of bounds [-90,90]\n",
                              fcnm, lat);
                }
            }
            if (lon < 0.0){lon = lon + 360.0;}
            for (k=0; k<gps_data->stream_length; k++){
                if (strcasecmp(gps_data->data[k].site, site) == 0){
                    gps_data->data[k].sta_lat = lat;
                    gps_data->data[k].sta_lon = lon;
                    lfound[k] = true;
                    break;
                }
            }
            // Are we done?
            ldone = true;
            for (k=0; k<gps_data->stream_length; k++){
                 if (!lfound[k]){
                     ldone = false;
                     break;
                 }
            }
            if (ldone){break;}
            memset(buf, 0, sizeof(buf));
        }
        fclose(sitefile);
    }
    // Did anyone fail?
    nuse = 0;
    for (k=0; k<gps_data->stream_length; k++){
       if (!lfound[k]){
           if (props.verbose > 2){
               log_warnF("%s: Station %s was not found - it will be skipped!\n",
                         fcnm, gps_data->data[k].site);
           }
           gps_data->data[k].lskip_pgd = true;
           gps_data->data[k].lskip_cmt = true;
           gps_data->data[k].lskip_ff = true;
       }else{
           nuse = nuse + 1;
       }
    }
    free(lfound);
    lfound = NULL;
    // Did everyone fail?
    if (nuse == 0){
        log_errorF("%s: There are no sites!\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Prints the site sampling periods to the debug file
 *
 * @param[in] gps_data    GPS data structure with sampling period
 *
 */
void GFAST_buffer_print__samplingPeriod(struct GFAST_data_struct gps_data)
{
    const char *fcnm = "GFAST_buffer_print__samplingPeriod\0";
    const char *lspace = "    \0";
    int k;
    log_debugF("%s: Site sampling periods\n", fcnm);
    for (k=0; k<gps_data.stream_length; k++){
        log_debugF("%s Site %s sampling period: %f (s)\n", 
                   lspace, gps_data.data[k].site, gps_data.data[k].dt);
        if (gps_data.data[k].lcollocated){
            log_debugF("%s Site %s strong motion sampling period %f (s)\n",
                       lspace, gps_data.data[k].site, gps_data.data[k].sm.dt);
        }
    }
    log_debugF("\n");
    return;
}
//============================================================================//
/*!
 * @brief Utility function for printing the location initialization parameters
 *
 * @param[in] gps_data    GPS data with streams containing the site name
 *                        and location which will be printed to the debug
 *                        file
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_buffer_print__locations(struct GFAST_data_struct gps_data)
{
    const char *fcnm = "GFAST_buffer_print__locations\0";
    const char *lspace = "    \0";
    int k;
    log_debugF("\n%s: Location summary:\n", fcnm);
    for (k=0; k<gps_data.stream_length; k++){
        log_debugF("%s Site %s located at (%f,%f)\n",
                   lspace, gps_data.data[k].site,
                   gps_data.data[k].sta_lat, gps_data.data[k].sta_lon);
        if (gps_data.data[k].lskip_pgd){
            log_debugF("%s: Site will be skipped in PGD estimation\n",
                       lspace);
        }
        if (gps_data.data[k].lskip_cmt){
            log_debugF("%s: Site will be skipped in CMT inversion\n",
                       lspace);
        }
        if (gps_data.data[k].lskip_ff){
            log_debugF("%s: Site will be skipped in finite-fault inversion\n",
                       lspace);
        }
    }
    log_debugF("\n");
    return;
}
