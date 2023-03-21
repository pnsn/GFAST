#include <stdio.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

/*!
 * @brief Reads the raw sigma threshold file
 *
 * @param[in] rawSigmaThresholdLookupFile name of the lookup file 
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
int core_scaling_readRawSigmaThresholdLookupFile(const char *rawSigmaThresholdLookupFile,
                                                 struct GFAST_pgd_props_struct *pgd_props) {
    // Read in file and set sigma lookup values
    FILE *infl;
    char cline[1024];
    int nlines, i;
    // There should be 3 values in one line: usigma, nsigma, esigma
    double usigma, nsigma, esigma;

    // Initialize to -1, which means they won't be used
    pgd_props->u_raw_sigma_threshold = -1;
    pgd_props->n_raw_sigma_threshold = -1;
    pgd_props->e_raw_sigma_threshold = -1;
    nlines = 0;

    if (!os_path_isfile(rawSigmaThresholdLookupFile)){
        LOG_WARNMSG("%s does not exist, will ignore raw sigmas in pd calculations!",
            rawSigmaThresholdLookupFile);
        return 0;
    }

    infl = fopen(rawSigmaThresholdLookupFile, "r");
    if (!infl) {
        LOG_ERRMSG("ERROR! Cannot open %s\n", rawSigmaThresholdLookupFile);
        return -1;
    }

    // Count number of lines
    while (fgets(cline, 1024, infl) != NULL) {
        nlines = nlines + 1;
    }

    if (nlines != 1) {
        LOG_ERRMSG("rawSigmaThresholdLookupFile %s requires 1 line, has %d",
            rawSigmaThresholdLookupFile, nlines);
        fclose(infl);
        return -1;
    }

    // Actually read and parse the lines this time
    rewind(infl);
    for (i = 0; i < nlines; i++) {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, 1024, infl) == NULL)
        {
            LOG_ERRMSG("Premature end of file %s", rawSigmaThresholdLookupFile);
            return -1;
        }
        if (cline[strlen(cline)-1] == '\n')
        {
            cline[strlen(cline)-1] = '\0';
        }
        // There should be 3 values in the line
        if (sscanf(cline, "%lf %lf %lf",
                   &usigma, &nsigma, &esigma) != 3) {
            LOG_ERRMSG("Invalid data in %s line %d: %s\n", rawSigmaThresholdLookupFile, i, cline);
            fclose(infl);
            return -1;
        }
        pgd_props->u_raw_sigma_threshold = usigma;
        pgd_props->n_raw_sigma_threshold = nsigma;
        pgd_props->e_raw_sigma_threshold = esigma;
    }

    fclose(infl);
    return 0;
}
