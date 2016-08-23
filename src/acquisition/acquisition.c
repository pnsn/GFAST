#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "gfast.h"
#include "iscl/log/log.h"
#define EPOCH0_NAN -(DBL_MAX + 1.0)
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
        gps_sac_data.data = (struct GFAST_waveformData_struct *)
                            calloc(1,
                                   sizeof(struct GFAST_waveformData_struct));
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
//============================================================================//
/*!
 * @brief Updates the acquisition by reading in all the SAC files and 
 *        filling the gps_acquisition buffer
 *
 * @param[in] props               has relevant information for reading SAC data
 * @param[in] simStartTime        simulation start time
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
                                     double simStartTime,
                                     double eventTime,
                                     double currentTime,
                                     double *latency,
                                     struct GFAST_data_struct *gps_acquisition)
{
    const char *fcnm = "GFAST_acquisition__updateFromSAC\0";
    struct GFAST_data_struct gps_sac_data;
    double dt, t0_read, t1_read, t0_data, t1_data, tlag;
    int i, i0, i1, ierr, ierr1, imax,
        j, j0, j1, k, npadd, npcopy, npts_data, npts_read, shift_left;
    bool latent, linit;
    // Determine if we have the headache of simulating latent data
    latent = false;
    if (latency != NULL){latent = true;} 
    // Loop on the SAC files
    ierr = 0;
    for (k=0; k<gps_acquisition->stream_length; k++){
        // Initialize this channel 
        memset(&gps_sac_data, 0, sizeof(struct GFAST_data_struct)); 
        gps_sac_data.stream_length = 1;
        gps_sac_data.data = (struct GFAST_waveformData_struct *)
                            calloc(1,
                                   sizeof(struct GFAST_waveformData_struct));
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
        // No points indicates trace acquisition has not yet started 
        linit = false;
        if (gps_acquisition->data[k].epoch == EPOCH0_NAN){
            gps_acquisition->data[k].epoch = simStartTime;
            gps_acquisition->data[k].npts = 0; // insert at npts + 1
            linit = true;
        }
        // Extract some numbers to make subsequent code a little shorter
        dt = gps_acquisition->data[k].dt;
        npts_read = gps_sac_data.data[0].npts;
        t0_read = gps_sac_data.data[0].epoch;
        npts_data = gps_acquisition->data[k].npts;
        t0_data = gps_acquisition->data[k].epoch;
        t1_data = t0_data + (double) npts_data*dt;
        // Trim the acquisition so that it only goes up to the current time
        tlag = 0.0;
        t0_read = gps_sac_data.data[0].epoch;
        if (latent){tlag = latency[k];} 
        j0 = 0;
        j1 = (int) ((currentTime - (t0_read + tlag))/dt + 0.5) + 1;
        j1 = fmax(0, j1);         // Don't go before first sample 
        j1 = fmin(npts_read, j1); // Don't extend past last sample
        // The buffer should be large so we can go back in time and correct 
        // any late arriving data.
        if (j1 - j0 > gps_acquisition->data[k].maxpts){
            log_errorF("%s: Make a bigger buffer size %d %d!\n", fcnm,
                       j1 - j0, gps_acquisition->data[k].maxpts);
            return -1;
        }
        // Update
        gps_sac_data.data[0].npts = j1 - j0;
        npts_read = gps_sac_data.data[0].npts;
        t1_read = t0_read + (double) fmax(0, (npts_read - 1))*dt;
        // How many new points are to be added? - remember we want to overwrite
        npcopy = (int) ((t1_read - t0_data)/dt + 0.5) + 1;
        npadd  = (int) ((t1_read - t1_data)/dt + 0.5) + 1;
        if (npcopy == 0){continue;} // Nothing new to add
        // Was the data initialized?
        i0 = 0; // Want the latest and greatest data
        if (linit){i0 = 0;} // Start at first index on initializaiton
        i1 = i0 + npcopy;
        shift_left = i1 + 1 - gps_acquisition->data[k].maxpts; // make space
        if (shift_left > 0){
            if (k == 0){log_warnF("%s: Haven't tested shift yet!\n", fcnm);}
            imax = gps_acquisition->data[k].maxpts - shift_left;
            for (i=0; i<imax; i++){
                if (shift_left + i >= gps_acquisition->data[k].maxpts){
                    log_warnF("%s: segfault skip\n", fcnm);
                    continue;
                }
                gps_acquisition->data[k].ubuff[k]
                   = gps_acquisition->data[k].ubuff[shift_left+i];
                gps_acquisition->data[k].nbuff[k]
                   = gps_acquisition->data[k].nbuff[shift_left+i];
                gps_acquisition->data[k].ebuff[k]
                   = gps_acquisition->data[k].ebuff[shift_left+i];
                gps_acquisition->data[k].tbuff[k]
                   = gps_acquisition->data[k].tbuff[shift_left+i];
            }
            i0 = gps_acquisition->data[k].maxpts - 1 - npcopy;
            i1 = gps_acquisition->data[k].maxpts - 1;
        }
        // Make sure i didn't butcher the indices
        if (i1 - i0 != j1 - j0){
            log_errorF("%s: Error size inconsistency %d %d %d %d %d!\n",
                       fcnm, i0, i1, j0, j1, npcopy);
            getchar();
            return -1;
        }
        // Add it in 
        j = j0;
        for (i=i0; i<i1; i++){
            if (i >= gps_acquisition->data[k].maxpts){
                log_errorF("%s: seg fault on buffer!\n", fcnm);
                return -1;
            }
            if (j > gps_sac_data.data[0].npts){
                log_errorF("%s: seg fault on gps_read %d %d\n", fcnm, j, j0);
            }
            gps_acquisition->data[k].ubuff[i] = gps_sac_data.data[0].ubuff[j];
            gps_acquisition->data[k].nbuff[i] = gps_sac_data.data[0].nbuff[j];
            gps_acquisition->data[k].ebuff[i] = gps_sac_data.data[0].ebuff[j];
            gps_acquisition->data[k].tbuff[i] = gps_sac_data.data[0].tbuff[j];
            j = j + 1;
        } 
        // Update the start and end times
        gps_acquisition->data[k].epoch = gps_acquisition->data[k].tbuff[0]; 
        gps_acquisition->data[k].npts = gps_acquisition->data[k].npts + npadd;
/*
if (k == 0){
printf("%d | %d %d -> %d %d | %d %d\n", gps_acquisition->data[k].npts, j0, j1, i0, i1, npts_read, npadd);
getchar();
}
*/
//        maxTime = (double) gps_acquisition->data[k].npts
//                + gps_acquisition->data[k].dt;
//        (currentTime - maxTime) 
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
    gps_acquisition->data = (struct GFAST_waveformData_struct *)
                            calloc(gps_acquisition->stream_length,
                                   sizeof(struct GFAST_waveformData_struct));
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
    // Set the start times to a really small number to indicate they arent set
    GFAST_buffer__setInitialTime(EPOCH0_NAN, gps_acquisition);
    // Set the buffer space and set to NaN's
    ierr = GFAST_buffer__setBufferSpace(props, gps_acquisition);
    if (ierr != 0){
        log_errorF("%s: Error initializing buffer space!\n", fcnm);
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
