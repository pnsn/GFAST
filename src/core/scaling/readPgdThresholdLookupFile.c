#include <stdio.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

/*!
 * @brief Reads the pgd threshold file
 *
 * @param[in] pgdThresholdLookupFile name of the lookup file 
 *
 * @param[in,out] pgd_props   On input contains the pgd props struct, with the
 *                            lookup values unfilled
 *                            On output, the lookup values have been filled
 *                          
 * 
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_readPgdThresholdLookupFile(const char *pgdThresholdLookupFile,
                                            struct GFAST_pgd_props_struct *pgd_props) {
    // Read in file and set sigma lookup values
    FILE *infl;
    char cline[1024];
    int nlines, i;
    // There should be 3 values per line
    double a0, a1;
    int a2;

    pgd_props->n_throttle = 0;
    nlines = 0;

    if (!os_path_isfile(pgdThresholdLookupFile)){
        LOG_WARNMSG("%s does not exist, using 0 pgd threshold values",
            pgdThresholdLookupFile);
        return 0;
    }

    infl = fopen(pgdThresholdLookupFile, "r");
    if (!infl) {
        LOG_ERRMSG("ERROR! Cannot open %s\n", pgdThresholdLookupFile);
        return -1;
    }

    // Count number of lines
    while (fgets(cline, 1024, infl) != NULL) {
        nlines = nlines + 1;
    }

    if (nlines < 1) {
        LOG_WARNMSG("No values in pgdThresholdLookupFile: %s", pgdThresholdLookupFile);
        fclose(infl);
        return 0;
    }

    if (nlines > MAX_THROTTLING_THRESHOLDS) {
        LOG_WARNMSG("nlines=%d in %s is greater than MAX_THROTTLING_THRESHOLDS! Setting to %d\n",
                    nlines, pgdThresholdLookupFile, MAX_THROTTLING_THRESHOLDS);
        nlines = MAX_THROTTLING_THRESHOLDS;
    }

    // Actually read and parse the lines this time
    rewind(infl);
    for (i = 0; i < nlines; i++) {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, 1024, infl) == NULL)
        {
            LOG_ERRMSG("Premature end of file %s", pgdThresholdLookupFile);
            return -1;
        }
        if (cline[strlen(cline)-1] == '\n')
        {
            cline[strlen(cline)-1] = '\0';
        }
        // There should be 3 values per line
        if (sscanf(cline, "%lf %lf %d",
                   &a0, &a1, &a2) != 3) {
            LOG_ERRMSG("Invalid data in %s line %d: %s\n", pgdThresholdLookupFile, i, cline);
            fclose(infl);
            return -1;
        }
        pgd_props->throttle_time_threshold[i] = a0;
        pgd_props->throttle_pgd_threshold[i] = a1;
        pgd_props->throttle_num_stations[i] = a2;
    }
    pgd_props->n_throttle = nlines;

    fclose(infl);
    return 0;
}


//   int throttle_num_stations[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold number of stations to send a message */
//   double throttle_pgd_threshold[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold pgd value (cm) */
//   double throttle_time_threshold[MAX_THROTTLING_THRESHOLDS]; /*!< Threshold time value (s) */
//   int n_throttle;             /*!< number of throttle criteria. num_stations, pgd_threshold,
//                                 time_threshold should be the same length */