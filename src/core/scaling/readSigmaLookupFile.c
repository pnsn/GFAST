#include <stdio.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

/*!
 * @brief Reads the sigma lookup table
 *
 * @param[in] sigmaLookupFile name of the lookup file 
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
int core_scaling_readSigmaLookupFile(const char *sigmaLookupFile,
                                     struct GFAST_pgd_props_struct *pgd_props) {
    // Read in file and set sigma lookup values
    FILE *infl;
    char cline[1024];
    int nlines, i;
    // There should be 8 values per line, we only need the first and last
    double a0, a1, a2, a3, a4, a5, a6, a7;

    pgd_props->n99 = 0;
    nlines = 0;

    if (!os_path_isfile(sigmaLookupFile)){
        LOG_WARNMSG("%s does not exist, using 0 sigma values", sigmaLookupFile);
        return 0;
    }

    infl = fopen(sigmaLookupFile, "r");
    if (!infl) {
        LOG_ERRMSG("ERROR! Cannot open %s\n", sigmaLookupFile);
        return -1;
    }

    // Count number of lines
    while (fgets(cline, 1024, infl) != NULL) {
        nlines = nlines + 1;
    }

    if (nlines < 1) {
        LOG_WARNMSG("No values in sigmaLookupFile: %s", sigmaLookupFile);
        fclose(infl);
        return 0;
    }

    if (nlines > MAX_SIGMA_LOOKUP_VALUES) {
        LOG_WARNMSG("nlines=%d in %s is greater than MAX_SIGMA_LOOKUP_VALUES! Setting to %d\n",
                    nlines, sigmaLookupFile, MAX_SIGMA_LOOKUP_VALUES);
        nlines = MAX_SIGMA_LOOKUP_VALUES;
    }

    // Actually read and parse the lines this time
    rewind(infl);
    for (i = 0; i < nlines; i++) {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, 1024, infl) == NULL)
        {
            LOG_ERRMSG("Premature end of file %s", sigmaLookupFile);
            return -1;
        }
        if (cline[strlen(cline)-1] == '\n')
        {
            cline[strlen(cline)-1] = '\0';
        }
        // There should be 8 values per line
        if (sscanf(cline, "%lf %lf %lf %lf %lf %lf %lf %lf",
                   &a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7) != 8) {
            LOG_ERRMSG("Invalid data in %s line %d: %s\n", sigmaLookupFile, i, cline);
            fclose(infl);
            return -1;
        }
        pgd_props->t99[i] = a0;
        pgd_props->m99[i] = a7;
    }
    pgd_props->n99 = nlines;

    fclose(infl);
    return 0;
}