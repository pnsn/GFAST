#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/iscl/iscl.h"
#include "iscl/time/time.h"

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
    struct GFAST_offsetData_struct cmt_data, ff_data;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_props_struct props;
    struct GFAST_shakeAlert_struct SA;
    char *amqMessage;
    double t0, t1;
    const enum opmode_type opmode = REAL_TIME_EEW;
    const bool useTopic = true;   // Don't want durable queues
    const bool clientAck = false; // Let session acknowledge transacations
    const bool luseListener = false; // C can't trigger so turn this off
    int ierr, iev;
    bool lacquire, lnewEvent;
    // Initialize 
    ierr = 0;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
/*
    memset(&gps_acquisition, 0, sizeof(struct GFAST_data_struct));
*/
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    ISCL_iscl_init(); // Fire up the computational library
    // Read the program properties
    ierr = GFAST_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading GFAST initialization file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){GFAST_properties_print(props);}
    // Initialize the stations locations/names for the module
    if (props.verbose > 0)
    {   
        log_infoF("%s: Initializing the acquisition...\n", fcnm);
    }
    // Fire up the listener
    if (props.verbose > 0)
    {
        log_infoF("%s: Initializing trigger listener...\n", fcnm);
    }
    ierr = activeMQ_initializeConsumer(props.activeMQ_props.user,
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
    //activeMQ_initialize( );

/*
    // Initialize PGD
    ierr = GFAST_core_scaling_pgd_initialize(props.pgd_props, gps_acquisition,
                                             &pgd, &pgd_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_core_cmt_initialize(props.cmt_props, gps_acquisition,
                                     &cmt, &cmt_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_core_ff_initialize(props.ff_props, gps_acquisition,
                                    &ff, &ff_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
*/
    // Begin the acquisition loop
    amqMessage = NULL;
    t0 = (double) (long) ISCL_time_timeStamp();
    lacquire = true;
    while(lacquire)
    {
        // Initialize the iteration
        amqMessage = NULL;
        // Run through the machine every second
        t1 = (double) (long) ISCL_time_timeStamp();
        if (t1 - t0 < props.waitTime){continue;}
        t0 = t1;
        // Update my buffers
  
        // Check my mail for an event
        amqMessage =
            GFAST_activeMQ_getMessage(props.activeMQ_props.msWaitForMessage,
                                      &ierr);
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
    GFAST_activeMQ_finalize(); 
    ISCL_iscl_finalize();
    if (ierr != 0)
    {
        log_errorF("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
