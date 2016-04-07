#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Allocates space for the internal data buffers  
 *
 * @param[in] props         determines the buffer window lengths for 
 *                          this instance of GFAST
 *
 * @param[inout] gps_data   on input has the stream_length GPS sites
 *                          and their sampling periods
 *                          on exit has space allocated and set to NaN's
 *                          for the position (ubuff, nbuff, ebuff) and
 *                          time (tbuff) buffers
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int GFAST_buffer__setBufferSpace(struct GFAST_props_struct props,
                                 struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_buffer__setBufferSpace\0";
    double dt, time_window;
    int i, k, npts;
    // Error handling
    time_window = props.bufflen;
    if (time_window < 0.0){
        log_errorF("%s: Error modeling window must be >= 0.0!\n", fcnm);
        return -1;
    }
    if (gps_data->stream_length < 1){
        log_errorF("%s: There are on streams\n", fcnm);
        return -1;
    }
    // Initialize each site
    for (k=0; k<gps_data->stream_length; k++){
        // Early skip?
        if (gps_data->data[k].lskip_pgd && gps_data->data[k].lskip_cmtff){
            continue;
        }
        // Compute the modeling window
        dt = gps_data->data[k].dt;
        if (dt <= 0.0){
            log_errorF("%s: %s has an invalid sampling period: %f (s)\n",
                       fcnm, gps_data->data[k].site, dt);
            log_errorF("%s: %s will be skipped in processing\n",
                       fcnm, gps_data->data[k].site);
            gps_data->data[k].lskip_pgd = true;
            gps_data->data[k].lskip_cmtff = true;
            continue;
        }
        // Compute the length - normally I'd add one but if the user meant
        // 30 seconds window length at 1 samples per second there would be
        // [0,29] = 30 samples in buffer
        npts = (int) (time_window/dt + 0.5);
        gps_data->data[k].npts = npts;
        // Set the space
        gps_data->data[k].nbuff = GFAST_memory_calloc__double(npts);
        gps_data->data[k].ebuff = GFAST_memory_calloc__double(npts);
        gps_data->data[k].ubuff = GFAST_memory_calloc__double(npts);
        gps_data->data[k].tbuff = GFAST_memory_calloc__double(npts);
        // Make sure the space was allocated
        if (gps_data->data[k].nbuff == NULL ||
            gps_data->data[k].ebuff == NULL ||
            gps_data->data[k].ubuff == NULL ||
            gps_data->data[k].tbuff == NULL){
            log_errorF("%s: Failed to set space for site %s\n",
                       fcnm, gps_data->data[k].site);
            gps_data->data[k].lskip_pgd = true;
            gps_data->data[k].lskip_cmtff = true;
            continue;
        }
        // Fill with NaN's
        for (i=0; i<gps_data->data[k].npts; i++){
            gps_data->data[k].nbuff[i] = NAN;
            gps_data->data[k].ebuff[i] = NAN;
            gps_data->data[k].ubuff[i] = NAN;
            gps_data->data[k].tbuff[i] = NAN;
        }
        // Set with the current time
 
    } // Loop on sites
    return 0;
}
//============================================================================//
/*!
 * @brief Sets the site sampling period at all stations
 * 
 * @param[in] props        holds the GFAST properties 
 *
 * @param[inout] gps_data   
 *
 * @result  0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_buffer__setSiteSamplingPeriod(struct GFAST_props_struct props,
                                        struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_buffer__setSiteSamplingPeriod\0";
    FILE *dtfl;
    char buf[256], site[256];
    double dt;
    int ierr, k;
    bool lfound;
    //------------------------------------------------------------------------//
    //
    // Set the sampling periods to the default
    for (k=0; k<gps_data->stream_length; k++){
        gps_data->data[k].dt = props.dt_default;
    }
    // Initialize from file
    if (props.dt_init == INIT_DT_FROM_FILE){
        if (!os_path_isfile(props.dtfile)){
            if (props.verbose > 1){
                log_warnF("%s: dtfile %s does not exist\n",
                          fcnm, props.dtfile);
                log_warnF("%s: Will set to default %f\n", fcnm,
                          props.dt_default);
            }
        }else{
            // Look for each station in 
            dtfl = fopen(props.dtfile, "r");
            for (k=0; k<gps_data->stream_length; k++){
                // Read until end until we match
                lfound = false;
                memset(buf, 0, sizeof(buf));
                while (fgets(buf, 256, dtfl) != NULL){
                    memset(site, 0, sizeof(site));
                    sscanf(buf, "%s %lf\n", site, &dt);
                    if (strcasecmp(site, gps_data->data[k].site) == 0){
                        if (dt <= 0.0){
                            log_warnF("%s: Input dt %f is invalid set to %f\n",
                                      fcnm, dt, props.dt_default);
                        }else{
                            gps_data->data[k].dt = dt;
                        } 
                        lfound = true;
                        break; 
                    }
                }
                if (props.verbose > 1 && !lfound){
                    log_warnF("%s: Failed to find station %s default to %f\n",
                              fcnm, gps_data->data[k].site,
                              gps_data->data[k].dt);
                }
                rewind(dtfl);
            }
            fclose(dtfl);
        }
    // Initialize from trace buffs 
    }else if (props.dt_init == INIT_DT_FROM_TRACEBUF){
        log_errorF("%s: This is not yet done!\n", fcnm);
        return -1;
    // No idea what to do
    }else{
        if (props.verbose > 1){
            log_warnF("%s: Invalid initialization job %d\n",
                      fcnm, props.dt_init);
            log_warnF("%s: Defaulting all sampling periods to %f (s)\n",
                      fcnm, props.dt_default);
        }
    }
    // Reality check
    ierr = 0;
    for (k=0; k<gps_data->stream_length; k++){
        if (gps_data->data[k].dt <= 0.0){
            log_errorF("%s: Site %s has invalid dt %f\n", fcnm,
                       gps_data->data[k].site, gps_data->data[k].dt);
            if (props.dt_default <= 0.0){
                log_errorF("%s: dt_default %f is invalid\n",
                           fcnm, props.dt_default);
            }
            ierr = ierr + 1;
        }
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Initializes the GPS data structure by determining the number of
 *        sites and their locations
 *
 * @param[in] props       GFAST properties
 *
 * @param[out] gps_data   on successful exit holds the nstream data stream's
 *                        sites names, locations, and sampling periods
 *
 * @result 0 indicates success
 *
 */
int GFAST_buffer__setLocations(struct GFAST_props_struct props,
                               struct GFAST_data_struct *gps_data)
{
    FILE *sitefile, *streamfile;
    const char *fcnm = "GFAST_locinit\0";
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
        log_errorF("%s: Error no NULL pointer detected!\n", fcnm);
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
    sitefile = fopen(props.siteposfile, "r");
    lfound = (bool *)calloc(gps_data->stream_length, sizeof(bool));
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
                log_warnF("%s: Station latitude %f is out of bounds [-90,90]\n",
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
    // Did anyone fail?
    nuse = 0;
    for (k=0; k<gps_data->stream_length; k++){
       if (!lfound[k]){
           if (props.verbose > 2){
               log_warnF("%s: Station %s was not found - it will be skipped!\n",
                         fcnm, gps_data->data[k].site);
           }
           gps_data->data[k].lskip_pgd = true;
           gps_data->data[k].lskip_cmtff = true;
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
 * @brief Gets the number of streams
 *
 * @param[in] streamfile     name of streamfile
 * @param[in] verbose        controls verbosity (< 2 is quiet)
 *
 * @result length (number of lines) in streamfile
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int GFAST_buffer__getNumberOfStreams(struct GFAST_props_struct props)
{
    const char *fcnm = "GFAST_buffer__getNumberOfStreams\0";
    FILE *f; 
    int ch; 
    int len = 0;
    if (!os_path_isfile(props.streamfile)){
        log_errorF("%s: Error cannot find stream file %s\n",
                   fcnm, props.streamfile);
        return len; 
    }   
    // Open the stream file and count the lines
    f = fopen(props.streamfile, "r");
    while (!feof(f)){
        ch = fgetc(f);
        if (ch == '\n'){len = len + 1;} 
    }   
    fclose(f); 
    return len;
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
            log_debugF("%s: This site will be skipped in PGD estimation\n",
                       fcnm);
        }
        if (gps_data.data[k].lskip_cmtff){
            log_debugF("%s: This site will be skipped in CMT/FF inversion\n",
                       fcnm);
        }
    }
    log_debugF("\n");
    return;
}
