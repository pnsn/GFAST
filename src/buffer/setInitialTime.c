#include <stdio.h>
#include <stdlib.h>
#include "gfast.h"
/*!
 * @brief Sets the initial time in all buffers
 *
 * @param[in] epoch0        intial UTC epochal time (s) to start gps_data 
 *                          traces at
 * @param[inout] gps_data   on input holds the stream_length traces
 *                          on output each trace begins at epoch0
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_buffer__setInitialTime(double epoch0,
                                  struct GFAST_data_struct *gps_data)
{
    int k;
    for (k=0; k<gps_data->stream_length; k++){
        gps_data->data[k].epoch = epoch0;
    }   
    return;
}
