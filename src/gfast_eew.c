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

static int settb2DataFromGFAST(struct GFAST_data_struct gpsData,
                               struct tb2Data_struct *tb2Data);

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
    struct tb2Data_struct tb2Data;
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
    memset(&tb2Data, 0, sizeof(struct tb2Data_struct));
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
    // Set up the SNCL's to target
    ierr = settb2DataFromGFAST(gps_data, &tb2Data);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting tb2Data\n", fcnm);
        goto ERROR;
    }
    // Connect to the earthworm
    msgs = NULL;
    ierr = traceBuffer_ewrr_initialize(props.ew_props.gpsRingName,
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
double tbeger = ISCL_time_timeStamp();
        // Update my buffers
        ISCL_memory_free__char(&msgs);
        msgs = traceBuffer_ewrr_getTraceBuf2Messages(MAX_MESSAGES,
                                                     false,
                                                     &ringInfo,
                                                     &nTracebufs2Read,
                                                     &ierr);
        if (ierr < 0 || (msgs == NULL && nTracebufs2Read > 0))
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
        if (nTracebufs2Read == 0)
        {
            log_warnF("%s: No data acquired - skipping\n", fcnm);
            continue;
        }
printf("scrounge %8.4f\n", ISCL_time_timeStamp() - tbeger);
tbeger = ISCL_time_timeStamp();
        // Unpackage the tracebuf2 messages
        ierr = traceBuffer_ewrr_unpackTraceBuf2Messages(nTracebufs2Read,
                                                        msgs, &tb2Data);
        if (ierr != 0)
        {
            log_errorF("%s: Error unpacking tracebuf2 messages\n", fcnm);
            goto ERROR;
        }
printf("end %d %8.4f\n", nTracebufs2Read, ISCL_time_timeStamp() - tbeger);
// early quit
 if (t1 - tbeg > 5)
{
printf("premature shut down\n");
break;
} 
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
    ISCL_memory_free__char(&msgs);
    traceBuffer_ewrr_freetb2Data(&tb2Data);
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
    ISCL_iscl_finalize();
    if (ierr != 0)
    {
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
//============================================================================//
/*!
 * @brief Sets the tb2Data structure and desired SNCL's from the input gpsData
 *
 * @param[in] gpsData      holds the GPS SNCL's GFAST is interested in
 *
 * @param[out] tb2Data     on output has space allocated and has a target
 *                         list of SNCL's for message reading from the 
 *                         earthworm data ring 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
static int settb2DataFromGFAST(struct GFAST_data_struct gpsData,
                               struct tb2Data_struct *tb2Data)
{
    const char *fcnm = "settb2DataFromH5TraceBuffer\0";
    int i, it, k;
    if (gpsData.stream_length == 0)
    {
        log_errorF("%s: Error no data to copy\n", fcnm);
        return -1;
    }
    if (tb2Data->linit)
    {
        log_errorF("%s: Error tb2Data already set\n", fcnm);
        return -1;
    }
    tb2Data->ntraces = 3*gpsData.stream_length;
    tb2Data->traces = (struct tb2Trace_struct *)
                      calloc( (size_t) tb2Data->ntraces,
                              sizeof(struct tb2Trace_struct) );
    it = 0;
    for (k=0; k<gpsData.stream_length; k++)
    {
        for (i=0; i<3; i++)
        {
            strcpy(tb2Data->traces[it].netw, gpsData.data[k].netw);
            strcpy(tb2Data->traces[it].stnm, gpsData.data[k].stnm);
            strcpy(tb2Data->traces[it].chan, gpsData.data[k].chan[i]);
            strcpy(tb2Data->traces[it].loc,  gpsData.data[k].loc); 
            it = it + 1;
        }
    }
    if (it != tb2Data->ntraces)
    {
        log_errorF("%s: Lost count\n", fcnm);
        return -1;
    }
    tb2Data->linit = true;
    return 0;
}
