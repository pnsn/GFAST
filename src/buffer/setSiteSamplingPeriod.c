#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief Sets the site sampling period at all stations
 * 
 * @param[in] props        holds the GFAST properties 
 *
 * @param[inout] gps_data   
 *
 * @result  0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int GFAST_buffer__setSiteSamplingPeriod(struct GFAST_props_struct props,
                                        struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_buffer__setSiteSamplingPeriod\0";
    FILE *dtfl;
    char buf[256], site[256];
    double dt; 
    int ierr, k;
    bool lfound;
    //------------------------------------------------------------------------//
    //  
    // Set the sampling periods to the default
    for (k=0; k<gps_data->stream_length; k++){
        gps_data->data[k].dt = props.dt_default;
    }
    // Initialize from file
    if (props.dt_init == INIT_DT_FROM_FILE){
        if (!os_path_isfile(props.dtfile)){
            if (props.verbose > 1){ 
                log_warnF("%s: dtfile %s does not exist\n",
                          fcnm, props.dtfile);
                log_warnF("%s: Will set to default %f\n", fcnm,
                          props.dt_default);
            }
        }else{
            // Look for each station in 
            dtfl = fopen(props.dtfile, "r");
            for (k=0; k<gps_data->stream_length; k++){
                // Read until end until we match
                lfound = false;
                memset(buf, 0, sizeof(buf));
                while (fgets(buf, 256, dtfl) != NULL){
                    memset(site, 0, sizeof(site));
                    sscanf(buf, "%s %lf\n", site, &dt);
                    if (strcasecmp(site, gps_data->data[k].site) == 0){
                        if (dt <= 0.0){
                            log_warnF("%s: Input dt %f is invalid set to %f\n",
                                      fcnm, dt, props.dt_default);
                        }else{
                            gps_data->data[k].dt = dt;
                        }
                        lfound = true;
                        break;
                    }
                }
                if (props.verbose > 1 && !lfound){
                    log_warnF("%s: Failed to find station %s default to %f\n",
                              fcnm, gps_data->data[k].site,
                              gps_data->data[k].dt);
                }
                rewind(dtfl);
            }
            fclose(dtfl);
        }
    // Initialize from trace buffs 
    }else if (props.dt_init == INIT_DT_FROM_TRACEBUF){
        log_errorF("%s: This is not yet done!\n", fcnm);
        return -1;
    // Initialize from SAC file
    }else if (props.dt_init == INIT_DT_FROM_SAC){
        ierr = GFAST_buffer__readDataFromSAC(1, props, gps_data);
        if (ierr != 0){
            log_warnF("%s: Errors initializing sampling period from SAC\n",
                      fcnm);
        }
    // No idea what to do
    }else{
        if (props.verbose > 1){
            log_warnF("%s: Invalid initialization job %d\n",
                      fcnm, props.dt_init);
            log_warnF("%s: Defaulting all sampling periods to %f (s)\n",
                      fcnm, props.dt_default);
        }
    }
    // Reality check
    ierr = 0;
    for (k=0; k<gps_data->stream_length; k++){
        if (gps_data->data[k].dt <= 0.0){
            log_errorF("%s: Site %s has invalid dt %f\n", fcnm,
                       gps_data->data[k].site, gps_data->data[k].dt);
            if (props.dt_default <= 0.0){
                log_errorF("%s: dt_default %f is invalid\n",
                           fcnm, props.dt_default);
            }
            ierr = ierr + 1;
        }
    }
    return ierr;
}
