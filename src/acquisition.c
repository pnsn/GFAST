#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "gfast.h"

/*!
 * @brief Gets the acquisition start time from the minimum epochal time
 *        in the SAC traces
 */
double GFAST_acquisition__getT0FromSAC(struct GFAST_props_struct props,
                                       struct GFAST_data_struct gps_acquisition,
                                       int *ierr)
{
    const char *fcnm = "GFAST_acquisition__getT0FromSAC\0";
    struct GFAST_data_struct gps_sac_data;
    double t0;
    int ierr1, k;
    *ierr = 0;
    t0 = DBL_MAX;
    for (k=0; k<gps_acquisition.stream_length; k++){
        // Initialize this channel 
        memset(&gps_sac_data, 0, sizeof(struct GFAST_data_struct)); 
        gps_sac_data.stream_length = 1;
        gps_sac_data.data = (struct GFAST_collocatedData_struct *)
                            calloc(1,
                                   sizeof(struct GFAST_collocatedData_struct));
        strcpy(gps_sac_data.data->site, gps_acquisition.data[k].site);
        // Get the time and make sure it is in bounds
        ierr1 = GFAST_buffer__readDataFromSAC(4, props, &gps_sac_data);
        if (ierr1 != 0){
            *ierr = *ierr + 1;
            log_errorF("%s: Error reading header!\n", fcnm);
            goto ERROR;
        }
        t0 = fmin(t0, gps_sac_data.data[0].epoch);
ERROR:; // Error reading the data
        GFAST_memory_freeData(&gps_sac_data);
    }
    if (t0 == DBL_MAX){
        log_errorF("%s: Error getting t0!\n", fcnm);
        *ierr = 1;
    }
    return t0;
}
/*!
 * @brief Updates the acquisition by reading in all the SAC files and 
 *        filling the gps_acquisition buffer
 *
 * @param[in] props               has relevant information for reading SAC data
 * @param[in] eventTime           event epochal UTC origin time (s) 
 * @param[in] currentTime         current epochal UTC time (s) to which the
 *                                acquisition is to be updated to
 * @param[in] latency             latency (s) for each site in acquisition
 *                                [gps_acquisition->stream_length] 
 * @param[inout] gps_acquisition  on input holds space for acquisition update.
 *                                on output holds the 
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_acquisition__updateFromSAC(struct GFAST_props_struct props,
                                     double eventTime,
                                     double currentTime,
                                     double *latency,
                                     struct GFAST_data_struct *gps_acquisition)
{
    const char *fcnm = "GFAST_acquisition__updateFromSAC\0";
    struct GFAST_data_struct gps_sac_data;
    int ierr, ierr1, k;
    bool latent;
    // Determine if we have the headache of simulating latent data
    latent = false;
    if (latency != NULL){latent = true;} 
    // Loop on the SAC files
    ierr = 0;
    for (k=0; k<gps_acquisition->stream_length; k++){
        // Initialize this channel 
        memset(&gps_sac_data, 0, sizeof(struct GFAST_data_struct)); 
        gps_sac_data.stream_length = 1;
        gps_sac_data.data = (struct GFAST_collocatedData_struct *)
                            calloc(1,
                                   sizeof(struct GFAST_collocatedData_struct));
        strcpy(gps_sac_data.data->site, gps_acquisition->data[k].site);
        // Get the time and make sure it is in bounds
        ierr1 = GFAST_buffer__readDataFromSAC(4, props, &gps_sac_data);
        if (ierr1 != 0){
            ierr = ierr + 1;
            log_errorF("%s: Error reading header!\n", fcnm);
            goto ERROR;
        }
         
        // Read the data 
        ierr1 = GFAST_buffer__readDataFromSAC(6, props, &gps_sac_data);
        if (ierr1 != 0){
            ierr = ierr + 1;
            log_errorF("%s: Error reading data!\n", fcnm);
            goto ERROR;
        }
        // From the SAC data determine where to insert
         
ERROR:; // Error reading the data
        GFAST_memory_freeData(&gps_sac_data);
    }
    return 0;
}
//============================================================================//
/*!
 * @brief Initializes the GPS data acquisition
 *
 * @param[in] props             GFAST properties pertinent to instantiating the
 *                              `real-time' data acquistion 
 *
 * @param[out] gps_acquisition  on successful output holds space for data
 *                              acquiring real-time data
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_acquisition__init(struct GFAST_props_struct props,
                            struct GFAST_data_struct *gps_acquisition)
{
    const char *fcnm = "GFAST_acquisition__init\0";
    int ierr;
    // Get the sites to be used
    ierr = 0;
    if (props.verbose > 0){log_infoF("%s: Initializing locations...\n", fcnm);}
    gps_acquisition->stream_length = GFAST_buffer__getNumberOfStreams(props);
    if (gps_acquisition->stream_length <= 0){
        log_errorF("%s: Error no streams to initialize\n", fcnm);
        return -1; 
    }
    // Set their names and locations
    gps_acquisition->data = (struct GFAST_collocatedData_struct *)
                            calloc(gps_acquisition->stream_length,
                                   sizeof(struct GFAST_collocatedData_struct));
    ierr = GFAST_buffer__setSitesAndLocations(props, gps_acquisition);
    if (ierr != 0){
        log_errorF("%s: Error getting site locations!\n", fcnm);
        return -1;
    }
    // Set the sampling periods
    ierr = GFAST_buffer__setSiteSamplingPeriod(props, gps_acquisition);
    if (ierr != 0){
        log_errorF("%s: Error setting sampling periods\n", fcnm);
        return -1;
    }
    //------------------------------------------------------------------------//
    //              Special cases for real-time, playback and offline         //
    //------------------------------------------------------------------------//
    // Initialize real-time feed
    if (props.opmode == REAL_TIME){
        log_errorF("%s: Real-time feed not yet done!\n", fcnm);
        return -1;
    // Initialize offline SAC feed
    }else if (props.opmode == OFFLINE){
        // Set the sampling periods
        ierr = GFAST_buffer__setSiteSamplingPeriod(props, gps_acquisition);
        if (ierr != 0){ 
            log_errorF("%s: Error setting sampling periods\n", fcnm);
            return -1;
        }
    // Earthworm playback
    }else if (props.opmode == PLAYBACK){
        log_errorF("%s: This operation is not yet done!\n", fcnm);
        return -1;
    // I don't know what you want from me
    }else{
        log_errorF("%s: Invalid operation mode %d\n", fcnm, props.opmode);
        return -1;
    }
    return ierr;
}
