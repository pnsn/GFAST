#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

static int splitLine(const char *cline,
                     char netw[64], char stnm[64], char loc[64],  char chan[64],
                     int *iusePGD, int *iuseCMT, int *iuseFF);

/*!
 * @brief Reads the site mask file and if a SNCL is found mutes the
 *        site's channel
 *
 * @param[in] siteMaskFile  name of the site mask file 
 * @param[in] verbose       if > 0 then will report on the masked sites
 *
 * @param[in,out] gps_data  on input contains the sites and SNCL's.
 *                          on output, if the SNCL is located in the
 *                          site mask file, the site is masked from the
 *                          inverserions.
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_data_readSiteMaskFile(const char *siteMaskFile,
                               const int verbose,
                               struct GFAST_data_struct *gps_data)
{
    FILE *infl;
    char cline[1024], netw[64], stnm[64], chan[64], loc[64];
    int i, ierr, iuseCMT, iuseFF, iusePGD, k, nlines;
    // No mask file so nothing to do
    if (!os_path_isfile(siteMaskFile)){return 0;}
    infl = fopen(siteMaskFile, "r");
    nlines = 0;
    while (fgets(cline, 1024, infl) != NULL)
    {
        nlines = nlines + 1;
    }
    if (nlines < 1)
    {
        LOG_WARNMSG("%s", "No sites in maskFile");
        fclose(infl);
        return 0;
    }
    rewind(infl);
    for (i=0; i<nlines; i++)
    {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, 1024, infl) == NULL)
        {
            LOG_ERRMSG("%s", "Premature end of file");
            return -1;
        }
        if (i == 0){continue;}
        if (strlen(cline) == 0)
        {
            LOG_ERRMSG("%s", "Error line is blank!");
            return -1;
        }
        if (cline[strlen(cline)-1] == '\n')
        {
            cline[strlen(cline)-1] = '\0';
        }
        ierr = splitLine(cline,
                         netw, stnm, loc, chan,
                         &iusePGD, &iuseCMT, &iuseFF);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error parsing line!");
            return -1;
        }
        if (iusePGD == 1 && iuseCMT == 1 && iuseFF == 1){continue;}
        // Search for match - this will kill the entire 3C structure
        // if any component matches
        for (k=0; k<gps_data->stream_length; k++)
        {
            if (strcasecmp(gps_data->data[k].netw, netw) == 0 &&
                strcasecmp(gps_data->data[k].stnm, stnm) == 0 &&
                strcasecmp(gps_data->data[k].loc,  loc)  == 0)
            {
                if (strcasecmp(gps_data->data[k].chan[0], chan) == 0 ||
                    strcasecmp(gps_data->data[k].chan[1], chan) == 0 ||
                    strcasecmp(gps_data->data[k].chan[2], chan) == 0 ||
                    strcasecmp(gps_data->data[k].chan[3], chan) == 0 ||
                    strcasecmp(gps_data->data[k].chan[4], chan) == 0 ||
                    strcasecmp(gps_data->data[k].chan[5], chan) == 0)
                {
                    if (verbose > 0)
                    {
                        LOG_INFOMSG("Masking %s.%s.%s.%s",
                                    netw, stnm, chan, loc);
                    }
                    if (iusePGD == 0)
                    {
                        gps_data->data[k].lskip_pgd = true;
                    }
                    if (iuseCMT == 0)
                    {
                        gps_data->data[k].lskip_cmt = true;
                    }
                    if (iuseFF == 0)
                    {
                        gps_data->data[k].lskip_ff = true;
                    }
                }
            }
        } // Loop on streams
    } // Loop on lines in file
    fclose(infl); 
    return 0;
}

static int splitLine(const char *cline,
                     char netw[64], char stnm[64], char loc[64], char chan[64],
                     int *iusePGD, int *iuseCMT, int *iuseFF)
{
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
    memset(stnm, 0, sizeof(char)*64);
    memset(chan, 0, sizeof(char)*64);
    memset(loc,  0, sizeof(char)*64);
    *iusePGD = 1;
    *iuseCMT = 1;
    *iuseFF  = 1;
    token = strtok(work, split);
    while (token)
    {   
        if (i == 0){strcpy(netw, token);}
        if (i == 1){strcpy(stnm, token);}
        if (i == 2){strcpy(loc,  token);}
        if (i == 3){strcpy(chan, token);}
        if (i == 4){*iusePGD = atoi(token);}
        if (i == 5){*iuseCMT = atoi(token);}
        if (i == 6){*iuseFF  = atoi(token);}
        i = i + 1;
        token = strtok(NULL, split);
    }   
    if (i != 7) 
    {   
        LOG_ERRMSG("Failed to split line %d %s\n", i, cline);
        ierr = 1;
    }   
    free(work);
    return ierr;
}

