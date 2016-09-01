#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/iscl/iscl.h"

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
    const enum opmode_type opmode = REAL_TIME_EEW;
    const bool useTopic = true;   // Don't want durable queues
    const bool clientAck = false; // Let session acknowledge transacations
    const bool luseListener = false; // C can't trigger so turn this off
    int ierr;
    bool lacquire;
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
        //goto ERROR;
    }
    //activeMQ_initialize( );
    // Begin the acquisition loop
    lacquire = true;
    while(lacquire)
    {
        // Run through the machine every second

        // Update my buffers
  
        // Check my mail for an event
        amqMessage = activeMQ_getMessage(props.activeMQ_props.msWaitForMessage,
                                         &ierr);
        if (amqMessage != NULL && ierr == 0)
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
            lacquire = false;
        } 
    }
ERROR:;
    ISCL_iscl_finalize();
    if (ierr != 0)
    {
        log_errorF("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
