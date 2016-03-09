#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"

/*!
 * This is a mock GFAST driver module
 */
int main()
{
    char fcnm[] = "xgfast\0";
    char propfilename[] = "gfast.props\0"; /* TODO take from EW config file */
    struct GFAST_props_struct props;
    int ierr;
    // Read the GFAST parameter file
    memset(&props, 0, sizeof(struct GFAST_props_struct));
    ierr = GFAST_paraminit(propfilename, &props);
    if (ierr != 0){
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    //GFAST_CMTgreenF();
    //obspy_rotate_NE2RT();
ERROR:;
    return 0;
}
