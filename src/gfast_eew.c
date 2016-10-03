#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/iscl/iscl.h"
#include "iscl/memory/memory.h"
#include "iscl/time/time.h"

#define MAX_MESSAGES 1024

/*!
 * @brief GFAST earthquake early warning driver routine
 *
 */
int main(int argc, char **argv)
{
    const char *fcnm = "gfast_eew\0";
    char propfilename[] = "gfast.props\0";
    struct GFAST_activeEvents_struct events;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_ffResults_struct ff;
    struct h5traceBuffer_struct h5traceBuffer;
    struct GFAST_offsetData_struct cmt_data, ff_data;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_data_struct gps_data;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_props_struct props;
    struct GFAST_shakeAlert_struct SA;
    struct ewRing_struct ringInfo;
    char *msgs;
    char *amqMessage;
    double t0, t1, tbeg;
    const enum opmode_type opmode = REAL_TIME_EEW;
    const bool useTopic = true;   // Don't want durable queues
    const bool clientAck = false; // Let session acknowledge transacations
    const bool luseListener = false; // C can't trigger so turn this off
    int ierr, iev, msWait, nTracebufs2Read;
    bool lacquire, lnewEvent;
    const int rdwt = 2; // H5 file is read/write
    // Initialize 
    ierr = 0;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ringInfo, 0, sizeof(struct ewRing_struct)); 
    memset(&h5traceBuffer, 0, sizeof(struct h5traceBuffer_struct));
    ISCL_iscl_init(); // Fire up the computational library
    // Read the program properties
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading GFAST initialization file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){GFAST_core_properties_print(props);}
    // Initialize the stations locations/names/sampling periods for the module
    if (props.verbose > 0)
    {   
        log_infoF("%s: Initializing the data buffers...\n", fcnm);
    }
    ierr = core_data_initialize(props, &gps_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }
    // Set the trace buffer names and open the HDF5 datafile
    ierr = GFAST_traceBuffer_h5_setTraceBufferFromGFAST(props.bufflen,
                                                        gps_data,
                                                        &h5traceBuffer);
    if (ierr != 0)
    {   
        log_errorF("%s: Error setting the H5 tracebuffer\n", fcnm);
        goto ERROR;
    }
    // Initialize the tracebuffer h5 archive
    ierr = traceBuffer_h5_initialize(rdwt, true, "./\0", "work.h5\0", 
                                     &h5traceBuffer);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing the HDF5 wave file\n", fcnm);
        goto ERROR;
    }
    // Fire up the listener
    if (props.verbose > 0)
    {
        log_infoF("%s: Initializing trigger listener...\n", fcnm);
    }
    ierr = activeMQ_consumer_initialize(props.activeMQ_props.user,
                                        props.activeMQ_props.password,
                                        props.activeMQ_props.originTopic,
                                        props.activeMQ_props.host,
                                        props.activeMQ_props.port,
                                        props.activeMQ_props.msReconnect,
                                        props.activeMQ_props.maxAttempts,
                                        useTopic,
                                        clientAck,
                                        luseListener,
                                        props.verbose);
    if (ierr != 0)
    {
        log_errorF("%s: Error connecting to upstream message queue\n", fcnm);
        goto ERROR;
    }

    // Initialize PGD
    ierr = core_scaling_pgd_initialize(props.pgd_props, gps_data,
                                       &pgd, &pgd_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = core_cmt_initialize(props.cmt_props, gps_data,
                               &cmt, &cmt_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = core_ff_initialize(props.ff_props, gps_data,
                              &ff, &ff_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Connect to the earthworm
char configFile[PATH_MAX];
    ierr = traceBuffer_ewrr_initialize(configFile,
                                       props.ew_props.gpsRingName,
                                       10,
                                       &ringInfo);
    // Flush the buffer
    log_infoF("%s: Flushing ring %s\n", fcnm, ringInfo.ewRingName);
    ierr = traceBuffer_ewrr_flushRing(&ringInfo);
    if (ierr != 0)
    {
        log_errorF("%s: Error flusing the ring\n", fcnm);
        goto ERROR;
    }
    // Set space for messages
    msgs = ISCL_memory_calloc__char(MAX_MESSAGES*MAX_TRACEBUF_SIZ);
    // Begin the acquisition loop
    log_infoF("%s: Beginning the acquisition...\n", fcnm);
    amqMessage = NULL;
    t0 = (double) (long) (ISCL_time_timeStamp() + 0.5);
    tbeg = t0; 
    lacquire = true;
    while(lacquire)
    {
        // Initialize the iteration
        amqMessage = NULL;
        // Run through the machine every second
        t1 = (double) (long) (ISCL_time_timeStamp() + 0.5);
        if (t1 - t0 < props.waitTime){continue;}
        t0 = t1;
printf("start\n");
<<<<<<< HEAD
        // Look through ring for new data 
        ierr = traceBuffer_ewrr_getTraceBuf2Messages(MAX_MESSAGES,
                                                     false,
                                                     &ringInfo,
                                                     &nTracebufs2Read,
                                                     msgs);
        // Update my buffers

printf("end %d %d\n", nTracebufs2Read, ierr);
=======
        // Update my buffers
        msgs = traceBuffer_ewrr_getTraceBuf2Messages(MAX_MESSAGES,
                                                     false,
                                                     &ringInfo,
                                                     &nTracebufs2Read,
                                                     &ierr);
        if (ierr < 0 || msgs == NULL)
        {
            if (ierr ==-1)
            {
                log_errorF("%s: Terminate message received from ring\n", fcnm);
                ierr = 1;
            }
            else if (ierr ==-2)
            {
                log_errorF("%s: Read error encountered on ring\n", fcnm);
                ierr = 1;
            }
            else if (ierr ==-3)
            {
                log_errorF("%s: Ring info structure never initialized\n", fcnm);
                ierr = 1;
            }
            else if (msgs == NULL)
            {
                log_errorF("%s: Message allocation error\n", fcnm);
                ierr = 1;
            }
            goto ERROR;
        }
printf("end %d\n", nTracebufs2Read);
>>>>>>> 8dfd7e593db5fd625a98f42420a867c518ad5e1f
// early quit
 if (t1 - tbeg > 5)
{
printf("premature shut down\n");
break;
} 
        // Unpackage the tracebuf2 messages

        ISCL_memory_free__char(&msgs);
        // Check my mail for an event
        msWait = props.activeMQ_props.msWaitForMessage;
        amqMessage = GFAST_activeMQ_consumer_getMessage(msWait, &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Internal error when getting message\n", fcnm);
            goto ERROR;
        }
        // If there's a message then process it
        if (amqMessage != NULL)
        {
            // Parse the event message 
            ierr = GFAST_eewUtils_parseCoreXML(amqMessage, -12345.0, &SA);
            if (ierr != 0)
            {
                log_errorF("%s: Error parsing the decision module message\n",
                           fcnm);
                log_errorF("%s\n", amqMessage);
                goto ERROR;
            }
            free(amqMessage);
            amqMessage = NULL;
            // If this is a new event we have some file handling to do
            lnewEvent = GFAST_events_newEvent(SA, &events);
            if (lnewEvent)
            {
                ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                             SA.eventid,
                                             propfilename);
                if (ierr != 0)
                {
                    log_errorF("%s: Error initializing the archive file\n",
                               fcnm);
                    goto ERROR;
                }
            }
        }
        // Are there events to process? 
        for (iev=0; iev<events.nev; iev++)
        {
            if (props.verbose > 2)
            {
                log_debugF("%s: Processing event %s\n", fcnm,
                           events.SA[iev].eventid);
            }
            // Read the data from the buffers

            // Apply the waveform processors

            // Update the HDF5 archive

        }
    }
ERROR:;
    traceBuffer_ewrr_finalize(&ringInfo);
    activeMQ_consumer_finalize(); 
    core_cmt_finalize(&props.cmt_props,
                      &cmt_data,
                      &cmt);
    core_ff_finalize(&props.ff_props,
                     &ff_data,
                     &ff);
    core_scaling_pgd_finalize(&props.pgd_props,
                              &pgd_data,
                              &pgd);
    GFAST_core_data_finalize(&gps_data);
    GFAST_core_properties_finalize(&props);
    traceBuffer_h5_finalize(&h5traceBuffer);
    ISCL_memory_free__char(&msgs);
    ISCL_iscl_finalize();
    if (ierr != 0)
    {
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
