#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast.h"

/*!
 * @brief Extracts data from an ElarmS message and fills a shake alert struct
 *        A characteristic ElarmS message will look like:
 *         ID  Year Mo Da HH:MM:SS.SSSSSS Mag     lon      lat
 *        1999 2015-08-24 14:33:43.612000 2.60 -122.2033 46.7320
 *
 * @param[in] buff      null terminated string containing the ElarmS
 *                      string
 * @param[in] verbose   controls verbosity
 *
 * @param[out] SA       shakeAlert event structure (lat, lon, depth, magnitude,
 *                      ID, and time)
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int GFAST_readElarmS_ElarmSMessage2SAStruct(int verbose, char *buff,
                                            struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_readElarmS_ElarmSMessage2SAStruct\0";
    double ev_sec;
    int ev_year, ev_month, ev_dom, ev_hour, ev_isec, ev_min, ev_musec;
    //------------------------------------------------------------------------//
    //
    // Zero out output and ensure message isn't NULL
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    if (buff == NULL){
        if (verbose > 0){
            log_errorF("%s: Error the buffer is NULL\n", fcnm);
        }
        return -1;
    }
    // Parse it
    //      1999 2015-08-24 14:33:43.612000 2.60 -122.2033 46.7320
    sscanf(buff, "%s %04d-%02d-%02d %02d:%02d:%lf %lf %lf %lf",
           SA->eventid,
           &ev_year, &ev_month, &ev_dom,
           &ev_hour, &ev_min, &ev_sec,
           &SA->mag, &SA->lon, &SA->lat);
    // Create the time
    ev_isec = (int) ev_sec;
    ev_musec = (int) ((ev_sec - (double) ev_isec)*1.e6);
    SA->time = time_calendar2epoch2(ev_year, ev_month, ev_dom, ev_hour,
                                    ev_min, ev_isec, ev_musec);
    return 0;
}
//============================================================================//
/*!
 * @brief Reads the ElarmS file into the Shake Alert structure 
 *
 * @param[in] props     GFAST properties structure (verbosity and eewsfile name)
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_readElarmS(struct GFAST_props_struct props)
{
struct GFAST_shakeAlert_struct SA;
    const char *fcnm = "GFAST_readElarmS\0";
    FILE *ew;
    char buffer[128], line[128];
    bool lempty;
    if (!os_path_isfile(props.eewsfile)){
        if (props.verbose > 0){
            log_errorF("%s: Error cannot find EEW file %s\n",
                       fcnm, props.eewsfile);
        }
        return -1;
    }
    // Open file and read until end
    ew = fopen(props.eewsfile, "r");
    memset(buffer, 0, sizeof(buffer));
    lempty = true;
    while (fgets(buffer, 128, ew) != NULL){
        lempty = false;
        memset(buffer, 0, sizeof(buffer));
    }
    if (!lempty){
        memset(line, 0, sizeof(line));
        sscanf(buffer, "%s\n", line);
        GFAST_readElarmS_ElarmSMessage2SAStruct(props.verbose, line, &SA);
    }
    fclose(ew);
    return 0;
}
