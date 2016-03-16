#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.hpp"

/*!
 * This is a mock GFAST driver module
 */
int main()
{
    char fcnm[] = "GFAST\0";
    char propfilename[] = "gfast.props\0"; /* TODO take from EW config file */
    struct GFAST_props_struct props;
    struct GFAST_data_struct gps_data;
    int nsites_read;
    int ierr = 0;
    //------------------------------------------------------------------------//
    // 
    // Initializations
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    // Read the properties file
    log_infoF("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_paraminit(propfilename, &props);
    if (ierr != 0){
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){
        GFAST_paraminit_print(props);
    }
    // Initialize the stations locations/names for the module
    if (props.verbose >= 2){
        log_infoF("%s: Initializing locations...\n", fcnm);
    }
    nsites_read = GFAST_locinit(props, &gps_data);
    if (nsites_read < 0){
        log_errorF("%s: Error getting locations!\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    // Connect to EW trace buffer and ring with ElarmS messages 
    if (props.opmode != OFFLINE){
        log_infoF("%s: Connecting to Earthworm rings...\n", fcnm);
    }
    // Initialize the sampling periods
    ierr = GFAST_buffer_setSiteSamplingPeriod(props, &gps_data);
    if (ierr != 0){
        log_errorF("%s: Error setting sampling periods\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){
        GFAST_buffer_print_samplingPeriod(gps_data);
    }
    // Connect ActiveMQ for ElarmS messages
    if (props.verbose >= 2){
        log_infoF("%s: Subscribing to ElarmS messages...\n", fcnm);
    }
 //GFAST_readElarmS(props);
    //GFAST_CMTgreenF();
    //obspy_rotate_NE2RT();
ERROR:;
    if (ierr != 0 && props.verbose > 0){
        log_errorF("%s: Errors were encountered\n", fcnm);
    }
    if (props.verbose >= 2){
        log_infoF("%s: Freeing memory...\n", fcnm);
    }
    GFAST_memory_freeData(&gps_data);
    GFAST_memory_freeProps(&props);
    return ierr;
}
