#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_events.h"
#include "gfast_hdf5.h"
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/time/time.h"

/*!
 * @brief Expert earthquake early warning GFAST driver
 *
 */
int eewUtils_driveGFAST(const double currentTime,
                        const char *xmlMessage,
                        struct GFAST_data_struct *gps_data,
                        struct h5traceBuffer_struct *h5traceBuffer,
                        struct GFAST_props_struct props,
                        struct GFAST_peakDisplacementData_struct *pgd_data,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_activeEvents_struct *events)
{
    const char *fcnm = "eewUtils_driveGFAST\0";
    struct GFAST_data_struct *gps_tempData;
    struct GFAST_shakeAlert_struct SA;
    double t1, t2;
    int ierr, iev, nsites_cmt, nsites_ff, nsites_pgd;
    bool lnewEvent;
    const double SA_NAN =-12345.0; 
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    // Parse the core XML for the hypocenter information 
    ierr = GFAST_eewUtils_parseCoreXML(xmlMessage, SA_NAN, &SA);
    if (ierr != 0)
    {
        log_errorF("%s: Error parsing XML message\n", fcnm);
        return ierr;
    }
    // If this is a new event we have some file handling to do
    lnewEvent = GFAST_events_newEvent(SA, events);
    if (lnewEvent)
    {
        ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                     SA.eventid,
                                     props.propfilename);
        if (ierr != 0)
        {
            log_errorF("%s: Error initializing the archive file\n", fcnm);
            return -1;
        }
    }
    // Loop on the events
    for (iev=0; iev<events->nev; iev++)
    {
        // Get the streams for this event
        memcpy(&SA, &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
        t1 = SA.time;     // Origin time
        t2 = currentTime;
        if (t1 > t2)
        {
            log_warnF("%s: Origin time > currentime? - skipping event %s\n",
                      fcnm, SA.eventid);
            continue;
        }
time_tic();
        // Get the data for this event
        ierr = GFAST_traceBuffer_h5_getData(t1, t2, h5traceBuffer);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting the data for event %s\n",
                       fcnm, SA.eventid);
            continue; 
        }
        // Copy the data onto the buffer
        ierr = GFAST_traceBuffer_h5_copyTraceBufferToGFAST(h5traceBuffer,
                                                           gps_data);
        if (ierr != 0)
        {
            log_errorF("%s: Error copying trace buffer\n", fcnm);
            continue;
        }
        // Process it
        gps_tempData = gps_data;
        // Extract the peak displacement from the waveform buffer
        nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
                                    props.pgd_props.utm_zone,
                                    props.pgd_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    *gps_tempData,
                                    pgd_data,
                                    &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error processing peak displacement\n", fcnm);
            continue;
        }
        // Extract the offset for the CMT inversion from the buffer 
        nsites_cmt = GFAST_core_waveformProcessor_offset(
                                    props.cmt_props.utm_zone,
                                    props.cmt_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    *gps_tempData,
                                    cmt_data,
                                    &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error processing CMT offset\n", fcnm);
            continue;
        }
        // Extract the offset for the FF inversion from the buffer 
        nsites_ff = GFAST_core_waveformProcessor_offset(
                                    props.ff_props.utm_zone,
                                    props.ff_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    *gps_tempData,
                                    ff_data,
                                    &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error processing FF offset\n", fcnm);
            continue;
        }
        gps_tempData = NULL;
    }
    return ierr;
}
