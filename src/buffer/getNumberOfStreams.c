#include <stdio.h>
#include <stdlib.h>
#include <gfast.h>
#include "iscl/log/log.h"
/*!
 * @brief Gets the number of streams to initialize
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
