#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast.h"
/*!
 * @brief Allocates space for the internal data buffers  
 *
 * @param[in] props         determines the buffer window lengths for 
 *                          this instance of GFAST
 *
 * @param[inout] gps_data   on input has the stream_length GPS sites
 *                          and their sampling periods
 *                          on exit has space allocated and set to NaN's
 *                          for the position (ubuff, nbuff, ebuff) and
 *                          time (tbuff) buffers
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
int GFAST_buffer__setBufferSpace(struct GFAST_props_struct props,
                                 struct GFAST_data_struct *gps_data)
{
    const char *fcnm = "GFAST_buffer__setBufferSpace\0";
    double dt, time_window;
    int i, k, maxpts;
    // Error handling
    time_window = props.bufflen;
    if (time_window < 0.0){
        log_errorF("%s: Error modeling window must be >= 0.0!\n", fcnm);
        return -1; 
    }
    if (gps_data->stream_length < 1){ 
        log_errorF("%s: There are on streams\n", fcnm);
        return -1;
    }
    // Initialize each site
    for (k=0; k<gps_data->stream_length; k++){
        // Early skip?
        if (gps_data->data[k].lskip_pgd &&
            gps_data->data[k].lskip_cmt &&
            gps_data->data[k].lskip_ff){
            continue;
        }
        // Compute the modeling window
        dt = gps_data->data[k].dt;
        if (dt <= 0.0){
            log_errorF("%s: %s has an invalid sampling period: %f (s)\n",
                       fcnm, gps_data->data[k].site, dt);
            log_errorF("%s: %s will be skipped in processing\n",
                       fcnm, gps_data->data[k].site);
            gps_data->data[k].lskip_pgd = true;
            gps_data->data[k].lskip_cmt = true;
            gps_data->data[k].lskip_ff = true;
            continue;
        }
        // Compute the length - normally I'd add one but if the user meant
        // 30 seconds window length at 1 samples per second there would be
        // [0,29] = 30 samples in buffer
        maxpts = (int) (time_window/dt + 0.5);
        gps_data->data[k].maxpts = maxpts;
        gps_data->data[k].npts = 0; // No points yet acquired in acquisition
        // Set the space
        gps_data->data[k].nbuff = GFAST_memory_calloc__double(maxpts);
        gps_data->data[k].ebuff = GFAST_memory_calloc__double(maxpts);
        gps_data->data[k].ubuff = GFAST_memory_calloc__double(maxpts);
        gps_data->data[k].tbuff = GFAST_memory_calloc__double(maxpts);
        // Make sure the space was allocated
        if (gps_data->data[k].nbuff == NULL ||
            gps_data->data[k].ebuff == NULL ||
            gps_data->data[k].ubuff == NULL ||
            gps_data->data[k].tbuff == NULL){
            log_errorF("%s: Failed to set space for site %s\n",
                       fcnm, gps_data->data[k].site);
            gps_data->data[k].lskip_pgd = true;
            gps_data->data[k].lskip_cmt = true;
            gps_data->data[k].lskip_ff = true;
            continue;
        }
        // Fill with NaN's
        #pragma omp simd
        for (i=0; i<gps_data->data[k].maxpts; i++){
            gps_data->data[k].nbuff[i] = NAN;
            gps_data->data[k].ebuff[i] = NAN;
            gps_data->data[k].ubuff[i] = NAN;
            gps_data->data[k].tbuff[i] = NAN;
        }
    } // Loop on sites
    return 0;
}
