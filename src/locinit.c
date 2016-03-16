#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast.h"

/*!
 * @brief Finds the length of the stream text file
 *
 * @param[in] streamfile     name of streamfile
 * @param[in] verbose        controls verbosity (0 is quiet)
 *
 * @result length (number of lines) in streamfile
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int __GFAST_streamfile_getstreamlength(char *streamfile, int verbose)
{
    const char *fcnm = "__GFAST_getstreamlength\0";
    FILE *f;
    int ch; 
    int len = 0;
    if (!os_path_isfile(streamfile)){
        if (verbose > 0){
            log_errorF("%s: Error cannot find stream file %s\n",
                       fcnm, streamfile);
        }
        return len; 
    }
    // Open the stream file and count the lines
    f = fopen(streamfile, "r");
    while (!feof(f)){
        ch = fgetc(f);
        if (ch == '\n'){len = len + 1;}
    }
    fclose(f); 
    return len;
}
//============================================================================//
/*!
 * @brief Initializes the GPS data structure by determining the number of
 *        sites, their locations, and sampling periods 
 *
 * @param[in] props       GFAST properties
 *
 * @param[out] gps_data   on successful exit holds the nstream data stream's
 *                        sites names, locations, and sampling periods
 *
 * @result number of sites successfully read.  if -1 then there was an error 
 *
 */
int GFAST_locinit(struct GFAST_props_struct props,
                  struct GFAST_data_struct *gps_data)
{
    FILE *sitefile, *streamfile;
    const char *fcnm = "GFAST_locinit\0";
    char *string[256], buf[1024], site[64];
    char delimit[] = " ,;\t\r\n\v\f";
    double lat, lon;
    int i, k, stream_length;
    bool *lfound, ldone, nuse;
    //------------------------------------------------------------------------//
    //
    // Get length of stream file
    gps_data->stream_length = 0;
    stream_length = __GFAST_streamfile_getstreamlength(props.streamfile,
                                                       props.verbose);
    if (stream_length == 0){
        if (props.verbose > 0){
            log_errorF("%s: There are no sites\n", fcnm);
        }
        return -1;
    }
    if (!os_path_isfile(props.siteposfile)){
        if (props.verbose > 0){
            log_errorF("%s: Error cannot find site position file %s\n",
                       fcnm, props.siteposfile);
        }
        return -1;
    }
    // Set the space in the streams
    gps_data->stream_length = stream_length;
    gps_data->data = (struct GFAST_collocatedData_struct *)
                     calloc(gps_data->stream_length,
                            sizeof(struct GFAST_collocatedData_struct));
    // Read the streams for this processing
    lat =-12345.0;
    lon =-12345.0;
    streamfile = fopen(props.streamfile, "r");
    for (k=0; k<stream_length; k++){
        memset(&gps_data->data[k], 0,
               sizeof(struct GFAST_collocatedData_struct));
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, 1024, streamfile) == NULL){
            if (props.verbose > 0){
                log_errorF("%s: Premature end of stream file!\n", fcnm);
                fclose(streamfile);
                return -1;
            }
        }
        sscanf(buf, "%s\n", site);
        strcpy(gps_data->data[k].site, site);
        gps_data->data[k].sta_lat = lat;
        gps_data->data[k].sta_lon = lon;
    }
    fclose(streamfile);
    // Attach the positions
    sitefile = fopen(props.siteposfile, "r");
    lfound = (bool *)calloc(stream_length, sizeof(bool));
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
            if (props.verbose > 0){
                log_warnF("%s: Station latitude %f is out of bounds [-90,90]\n",
                          fcnm, lat);
            }
        }
        if (lon < 0.0){lon = lon + 360.0;}
        for (k=0; k<stream_length; k++){
            if (strcasecmp(gps_data->data[k].site, site) == 0){
                gps_data->data[k].sta_lat = lat;
                gps_data->data[k].sta_lon = lon;
                lfound[k] = true;
                break;
            }
        }
        // Are we done?
        ldone = true;
        for (k=0; k<stream_length; k++){
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
    for (k=0; k<stream_length; k++){
       if (!lfound[k]){
           if (props.verbose >= 2){
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
    if (props.verbose >= 3){
        GFAST_locinit_printLocations(gps_data);
    }
    // Did everyone fail?
    if (nuse == 0){
        log_errorF("%s: There are no sites!\n", fcnm);
    }
    return nuse;
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
void GFAST_locinit_printLocations(struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_locinit_printLocations\0";
    const char *lspace = "    \0";
    int k;
    log_debugF("\n%s: Location summary:\n", fcnm);
    for (k=0; k<gps_data->stream_length; k++){
        log_debugF("%s Site %s located at (%f,%f)\n",
                   lspace, gps_data->data[k].site,
                   gps_data->data[k].sta_lat, gps_data->data[k].sta_lon);
        if (gps_data->data[k].lskip_pgd){
            log_debugF("%s: This site will be skipped in PGD estimation\n",
                       fcnm);
        }
        if (gps_data->data[k].lskip_cmtff){
            log_debugF("%s: This site will be skipped in CMT/FF inversion\n",
                       fcnm);
        }
    }
    log_debugF("\n");
    return;
}
