#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"

static int splitLine(const char *cline,
                     char netw[64], char stat[64], char loc[64], char chan[64],
                     double *lat, double *lon, double *elev,
                     double *dt, double *gain,
                     char units[64], char sensorType[64],
                     double *reflat, double *reflon);
/*!
 * @brief Reads the site metadata file.  From this we initialize the site
 *        network, station, channel, and location code, it's position (latitude,
 *        longitude, elevation), sampling period, and gain)
 *
 * @param[in] metaDataFile    name of GPS metadata file
 *
 * @param[out] gps_data       GPS streams with
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */ 
int core_data_readMetaDataFile(const char *metaDataFile,
                               struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "core_data_readMetaDataFile\0";
    FILE *infl;
    char **textfl, **sites, cline[1024],
         site[256], chan[64], chan1[64], loc[64], loc1[64],
         netw[64], netw1[64], stat[64], stat1[64],
         sensorType[64], units[64];
    double gain0[3], dt, elev, gain, lat, lon, reflat, reflon;
    int *lines, i, ierr, j, k, lfound, nlines, ns;
    // Initialize 
    ierr = 0;
    infl = NULL;
    textfl = NULL;
    sites = NULL;
    lines = NULL;
    nlines = 0;
    ns = 0;
    // Require the site file exists
    if (!os_path_isfile(metaDataFile))
    {
        log_errorF("%s: Error site file does not exist!\n", fcnm);
        return -1;
    }
    // Open the file for reading and count the sites
    infl = fopen(metaDataFile, "r");
    while (fgets(cline, 1024, infl) != NULL)
    {
        nlines = nlines + 1;
    }
    if (nlines < 1)
    {
        log_errorF("%s: Error no data read!\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    rewind(infl);
    // Read the text file into memory
    nlines = nlines - 1; // remove the header
    textfl = (char **)calloc(nlines, sizeof(char *));
    sites = (char **)calloc(nlines, sizeof(char *));
    for (i=0; i<nlines; i++){sites[i] = NULL;}
    lines = (int *)calloc(nlines, sizeof(int));
    for (i=0; i<nlines+1; i++)
    {
        memset(cline, 0, sizeof(cline));
        k = i - 1;
        if (fgets(cline, 1024, infl) == NULL)
        {
            ierr = 1;
            log_errorF("%s: Premature end of file\n", fcnm);
            goto ERROR;
        }
        if (i == 0){continue;} // Skip the header
        if (strlen(cline) == 0)
        {
            ierr = 0; 
            log_errorF("%s: Blank line - invalid input!\n", fcnm);
            goto ERROR;
        }
        // Get rid of the end of line
        if (cline[strlen(cline)-1] == '\n'){cline[strlen(cline)-1] = '\0';}
        textfl[k] = (char *)calloc(strlen(cline)+1, sizeof(cline));
        strcpy(textfl[k], cline);
    }
    fclose(infl);
    infl = NULL;
    // Now match up the three component data streams
    ns = 0;
    for (i=0; i<nlines; i++)
    {
        // Get the root name (ignoring the channel orientation) 
        ierr = splitLine(textfl[i],
                         netw, stat, loc, chan,
                         &lat, &lon, &elev,
                         &dt, &gain,
                         units, sensorType,
                         &reflat, &reflon);
        if (ierr != 0)
        {
            log_errorF("%s: Error parsing line!\n", fcnm);
            goto ERROR;
        }
        // Make the site name
        strcpy(site, netw);
        strcat(site, "_\0");
        strcat(site, stat);
        strcat(site, "_\0");
        if (strlen(chan) != 3)
        {
            ierr = 1;
            log_errorF("%s: I don't know how to parse this channel %s\n",
                       fcnm, chan);
            goto ERROR;
        }
        strncat(site, chan, 2);
        strcat(site, "_\0");
        strcat(site, loc);
        // Does this site exist?
        for (k=0; k<ns; k++)
        {
            if (strcasecmp(sites[k], site) == 0){goto NEXT_LINE;}
        }
        ns = ns + 1;
        sites[ns-1] = (char *)calloc(strlen(site)+1, sizeof(char));
        strcpy(sites[ns-1], site);
        // Now verify at least 1 or 3 sites are specified 
        lfound = 0;
        gain0[0] = NAN;
        gain0[1] = NAN;
        gain0[2] = NAN;
        for (j=0; j<nlines; j++)
        {
            ierr = splitLine(textfl[j],
                             netw1, stat1, loc1, chan1,
                             &lat, &lon, &elev,
                             &dt, &gain,
                             units, sensorType,
                             &reflat, &reflon);
            if (ierr != 0)
            {
                log_errorF("%s: Error parsing line!\n", fcnm);
                goto ERROR;
            }
            if (strcasecmp( netw1, netw) == 0 &&
                strcasecmp( stat1, stat) == 0 &&
                strcasecmp( loc1,  loc)  == 0 &&
                strncasecmp(chan1, chan, 2) == 0)
            {
                gain0[lfound] = gain;
                if (chan1[2] == 'Z'){lfound = lfound + 1;}
                if (chan1[2] == 'N'){lfound = lfound + 1;}
                if (chan1[2] == 'E'){lfound = lfound + 1;}
            }
        }
        if (lfound == 1 || lfound == 3)
        {
            lines[gps_data->stream_length] = i;
            gps_data->stream_length = gps_data->stream_length + 1;
            if (lfound == 3)
            {
                if (fabs(gain0[0] - gain0[1]) > 1.e-6 ||
                    fabs(gain0[0] - gain0[2]) > 1.e-6)
                {
                    log_errorF("%s: Error inconstent gain\n", fcnm);
                    goto ERROR;
                }
            }
        }
        else
        {
            log_errorF("%s: Couldn't find other channels %s\n", fcnm, site);
        }
NEXT_LINE:; // Try another site to match
    }
    // Did we get anything?
    if (gps_data->stream_length < 1)
    {
        log_errorF("%s: No three-component sites found!\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    if (ns != gps_data->stream_length)
    {
        log_warnF("%s: Warning - counting problem\n", fcnm);
    }
    gps_data->data = (struct GFAST_waveform3CData_struct *)
                     calloc(gps_data->stream_length,
                            sizeof(struct GFAST_waveform3CData_struct));
    // Now parse the lines
    for (k=0; k<gps_data->stream_length; k++)
    {
        // Parse the line
        ierr = splitLine(textfl[lines[k]],
                         netw, stat, loc, chan,
                         &lat, &lon, &elev,
                         &dt, &gain,
                         units, sensorType,
                         &reflat, &reflon);
        if (ierr != 0)
        {
            log_errorF("%s: Error parsing line!\n", fcnm);
            goto ERROR;
        }
        // Verify the inputs
        if (lat <-90.0 || lat > 90.0)
        {
            log_errorF("%s: Input latitude %f is invalid\n", fcnm, lat);
            ierr = 1;
            goto ERROR;
        }
        if (lon <-180.0 || lon > 360.0)
        {
            log_errorF("%s: Input longitude %f is invalid\n", fcnm, lon);
            ierr = 1;
            goto ERROR;
        }
        if (dt <= 0.0)
        {
            log_errorF("%s: Input sampling period %f is invalid\n", fcnm, dt);
            ierr = 1;
            goto ERROR;
        }
        if (lon < 0.0){lon = lon + 360.0;}
        // And now set the site information
        strcpy(gps_data->data[k].netw, netw);
        strcpy(gps_data->data[k].stnm, stat);
        strncpy(gps_data->data[k].chan[0], chan, 2);
        strcat( gps_data->data[k].chan[0], "Z\0"); 
        strncpy(gps_data->data[k].chan[1], chan, 2);
        strcat( gps_data->data[k].chan[1], "N\0");
        strncpy(gps_data->data[k].chan[2], chan, 2); 
        strcat( gps_data->data[k].chan[2], "E\0");
        strcpy(gps_data->data[k].loc, loc);
        gps_data->data[k].sta_lat = lat;
        gps_data->data[k].sta_lon = lon;
        gps_data->data[k].sta_alt = elev;
        gps_data->data[k].dt = dt; 
        gps_data->data[k].gain[0] = gain;
        gps_data->data[k].gain[1] = gain;
        gps_data->data[k].gain[2] = gain;
        if (gain == 0.0 || dt <= 0.0 || isnan(dt))
        {
            if (gain == 0.0)
            {
                log_warnF("%s: Instrument gain will mute staion\n", fcnm);
            }
            if (dt <= 0.0)
            {
                log_errorF("%s: Sampling period %f is invalid\n", fcnm, dt);
                ierr = 1;
                goto ERROR;
            }
            if (isnan(dt))
            {
                log_errorF("%s: Sampling rate input was zero\n", fcnm);
                ierr = 1;
                goto ERROR;
            }
         } 
        //if (iuse == 0)
        //{
        //    gps_data->data[k].lskip_pgd = true;
        //    gps_data->data[k].lskip_cmt = true;
        //    gps_data->data[k].lskip_ff  = true;
        //}
    } 
ERROR:;
    if (lines != NULL){free(lines);}
    if (textfl != NULL)
    {
       for (i=0; i<nlines; i++)
       {
           if (textfl[i] != NULL){free(textfl[i]);}
       }
       free(textfl);
    }
    if (sites != NULL)
    {
       for (i=0; i<nlines; i++)
       {
           if (sites[i] != NULL){free(sites[i]);}
       }
       free(sites);
    }
    if (infl != NULL){fclose(infl);}
    return ierr;
}

static int splitLine(const char *cline,
                     char netw[64], char stat[64], char loc[64], char chan[64],
                     double *lat, double *lon, double *elev,
                     double *dt, double *gain,
                     char units[64], char sensorType[64],
                     double *reflat, double *reflon)
{
    const char *fcnm = "splitLine\0";
    char *token, *work;
    int i, ierr;
    const char *split = " ";
    // Copy the input
    ierr = 0;
    i = 0;
    work = (char *)calloc(strlen(cline)+1, sizeof(char));
    strcpy(work, cline);
    // Set the outputs
    memset(netw, 0, sizeof(char)*64);
    memset(stat, 0, sizeof(char)*64);
    memset(chan, 0, sizeof(char)*64);
    memset(loc,  0, sizeof(char)*64);
    *lat = 0.0;
    *lon = 0.0;
    *elev = 0.0;
    *dt = 0.0;
    *gain = 0;
    memset(units,      0, sizeof(char)*64);
    memset(sensorType, 0, sizeof(char)*64); 
    *reflat = NAN;
    *reflon = NAN;
    token = strtok(work, split);
    while (token)
    {
        if (i == 0){strcpy(netw, token);}
        if (i == 1){strcpy(stat, token);}
        if (i == 2){strcpy(loc,  token);}
        if (i == 3){strcpy(chan, token);}
        if (i == 4){*lat = (double) atof(token);}
        if (i == 5){*lon = (double) atof(token);}
        if (i == 6){*elev = (double) atof(token);}
        if (i == 7){*dt = 1.0/(double) atof(token);} // actually rate
        if (i == 8){*gain = (double) atof(token);}
        if (i == 9){strcpy(units, token);}
        if (i == 10){strcpy(sensorType, token);}
        if (i == 11){*reflat = (double) atof(token);}
        if (i == 12){*reflon = (double) atof(token);}
        i = i + 1;
        token = strtok(NULL, split);
    }
    if (i != 13 && i != 14)
    {
        log_errorF("%s: Failed to split line %d %s\n", fcnm, i, cline);
        ierr = 1;
    }
    free(work);
    return ierr;
}
