#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
// MTH:
#include "gfast_core.h"
#include "iscl/iscl/iscl.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"
#include <dirent.h>  // Needed for DIR

#include <time.h>

//#define MAX_MESSAGES 1024
//#define MAX_MESSAGES 183000
//#define MAX_MESSAGES 200000
#define MAX_MESSAGES 80000

static int settb2DataFromGFAST(struct GFAST_data_struct gpsData,
                               struct tb2Data_struct *tb2Data);

char *check_dir_for_messages(const char *dirname, int *ierr);


/*!
 * @brief GFAST earthquake early warning driver routine
 *
 */
int main(int argc, char **argv)
{
    const char *fcnm = "gfast_eew\0";
    char propfilename[] = "gfast.props\0";
    //const char *message_dir = "./events";                // MTH: eventually move this to gfast.props
    char *message_dir; 
    struct GFAST_activeEvents_struct events;
    struct GFAST_activeEvents_xml_status xml_status;

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
    struct GFAST_xmlMessages_struct xmlMessages;
    struct ewRing_struct ringInfo;
    char *msgs;
    char *amqMessage;
    double t0, t1, tbeg, t_now;
    int first_time = 1;
    const enum opmode_type opmode = REAL_TIME_EEW;
    const bool useTopic = true;   // Don't want durable queues
    const bool clientAck = false; // Let session acknowledge transacations
    const bool luseListener = false; // C can't trigger so turn this off
    double tstatus, tstatus0, tstatus1;
    void *messageQueue = NULL;
    int ierr, im, msWait, nTracebufs2Read;
    bool lacquire, lnewEvent;
    const int rdwt = 2; // H5 file is read/write
    char errorLogFileName[PATH_MAX];
    char infoLogFileName[PATH_MAX];
    char debugLogFileName[PATH_MAX];
    char warnLogFileName[PATH_MAX];
    char logFileName[PATH_MAX]="mth_log\0";
    bool check_message_dir = false;
    bool USE_AMQ = false;
    int niter = 0;
    int iev;
    int i, j, i1, i2, is, chunk;
#ifdef GFAST_USE_AMQ
    USE_AMQ = true;
#endif

// MTH: FIX THIS !!!
    USE_AMQ = false;

    // Initialize 
    ierr = 0;
    msgs = NULL;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&xml_status, 0, sizeof(struct GFAST_activeEvents_xml_status));

    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ringInfo, 0, sizeof(struct ewRing_struct)); 
    memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
    memset(&h5traceBuffer, 0, sizeof(struct h5traceBuffer_struct));
    memset(&tb2Data, 0, sizeof(struct tb2Data_struct));
    ISCL_iscl_init(); // Fire up the computational library

    core_log_openLog(logFileName);
    LOG_MSG("%s: Read gfast.props file", fcnm);
    // Read the program properties
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);

/*
int imsg;
for (imsg=0;imsg<10;imsg++){
  LOG_MSG("%s: This LOG test message:[%d] BEFORE evid is known", fcnm, imsg);
}
*/

    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error reading GFAST initialization file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){GFAST_core_properties_print(props);}
    // Initialize the stations locations/names/sampling periods for the module
    if (props.verbose > 0)
    {
        LOG_INFOMSG("%s: Initializing the data buffers...\n", fcnm);
        LOG_MSG("%s: Initializing the data buffers...", fcnm);
    }

    ierr = core_data_initialize(props, &gps_data);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }
    // Set the trace buffer names and open the HDF5 datafile
    ierr = GFAST_traceBuffer_h5_setTraceBufferFromGFAST(props.bufflen,
                                                        gps_data,
                                                        &h5traceBuffer);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s: Error setting the H5 tracebuffer\n", fcnm);
        goto ERROR;
    }
    // Initialize the tracebuffer h5 archive
    ierr = traceBuffer_h5_initialize(rdwt, true, "./\0", "work.h5\0", 
                                     &h5traceBuffer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing the HDF5 wave file\n", fcnm);
        goto ERROR;
    }
    // Fire up the listener
    if (props.verbose > 0)
    {
        LOG_INFOMSG("%s: Initializing trigger listener...\n", fcnm);
        LOG_MSG("%s: Initializing trigger listener...", fcnm);
    }

    if (USE_AMQ) {
      messageQueue = activeMQ_consumer_initialize(props.activeMQ_props.user,
                                          props.activeMQ_props.password,
                                          props.activeMQ_props.originTopic,
                                          props.activeMQ_props.host,
                                          props.activeMQ_props.port,
                                          props.activeMQ_props.msReconnect,
                                          props.activeMQ_props.maxAttempts,
                                          useTopic,
                                          clientAck,
                                          luseListener,
                                          props.verbose,
                                          &ierr);
      if (ierr != 0)
      {
          LOG_ERRMSG("%s: Error connecting to upstream message queue\n", fcnm);
          goto ERROR;
      }
    }

    if (strlen(props.SAeventsDir)){
        message_dir = props.SAeventsDir;
        check_message_dir = true;
    }

    // Initialize PGD
    ierr = core_scaling_pgd_initialize(props.pgd_props, gps_data,
                                       &pgd, &pgd_data);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = core_cmt_initialize(props.cmt_props, gps_data,
                               &cmt, &cmt_data);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = core_ff_initialize(props.ff_props, gps_data,
                              &ff, &ff_data);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Set up the SNCL's to target
    ierr = settb2DataFromGFAST(gps_data, &tb2Data);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error setting tb2Data\n", fcnm);
        goto ERROR;
    }
    // Connect to the earthworm ring
    ierr = traceBuffer_ewrr_initialize(props.ew_props.gpsRingName,
                                       10,
                                       &ringInfo);
    // Flush the buffer
    LOG_INFOMSG("%s: Flushing ring %s\n", fcnm, ringInfo.ewRingName);
LOG_MSG("%s: Flushing ring %s", fcnm, ringInfo.ewRingName);
    ierr = traceBuffer_ewrr_flushRing(&ringInfo);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error flusing the ring\n", fcnm);
        goto ERROR;
    }
    // Begin the acquisition loop
    LOG_INFOMSG("%s: Beginning the acquisition...\n", fcnm);
LOG_MSG("%s: Beginning the acquisition...", fcnm);
    amqMessage = NULL;
    t0 = (double) (long) (ISCL_time_timeStamp() + 0.5);
    t_now = (double) (long) (ISCL_time_timeStamp() + 0.5);
    tbeg = t0; 
    tstatus = t0;
    tstatus0 = t0;
    lacquire = true;
    while(lacquire)
    {
        // Initialize the iteration
        amqMessage = NULL;
        // Run through the machine every second
        t1 = (double) (long) (ISCL_time_timeStamp() + 0.5);
        if (t1 - t0 < props.waitTime){continue;}
        t0 = t1;
        tstatus1 = t0;

LOG_MSG("== [GFAST t0:%f Get the msgs off the EW ring]", ISCL_time_timeStamp());

        if (tstatus1 - tstatus0 > 3600.0)
        {
            LOG_DEBUGMSG("%s: GFAST has been running for %d hours\n",
                       fcnm, (int) ((tstatus1 - tstatus)/3600.0));
            tstatus0 = tstatus1;
        } 

        double tbeger = ISCL_time_timeStamp();
        double tbeger0 = tbeger;
        // Read my messages off the ring
        memory_free8c(&msgs); //ISCL_memory_free__char(&msgs);

        msgs = traceBuffer_ewrr_getMessagesFromRing(MAX_MESSAGES,
                                                    false,
                                                    &ringInfo,
                                                    &nTracebufs2Read,
                                                    &ierr);
LOG_MSG("== [GFAST t0:%f] getMessages returned nTracebufs2Read:%d", ISCL_time_timeStamp(), nTracebufs2Read);

        if (ierr < 0 || (msgs == NULL && nTracebufs2Read > 0))
        {
            if (ierr ==-1)
            {
                LOG_ERRMSG("%s: Terminate message received from ring\n", fcnm);
                ierr = 1;
            }
            else if (ierr ==-2)
            {
                LOG_ERRMSG("%s: Read error encountered on ring\n", fcnm);
                ierr = 1;
            }
            else if (ierr ==-3)
            {
                LOG_ERRMSG("%s: Ring info structure never initialized\n", fcnm);
                ierr = 1;
            }
            else if (msgs == NULL)
            {
                LOG_ERRMSG("%s: Message allocation error\n", fcnm);
                ierr = 1;
            }
            goto ERROR;
        }

//printf("scrounge %8.4f\n", ISCL_time_timeStamp() - tbeger);
tbeger = ISCL_time_timeStamp();
        // Unpackage the tracebuf2 messages
//LOG_MSG("%s", "== unpackTraceBuf2Messages");
        ierr = traceBuffer_ewrr_unpackTraceBuf2Messages(nTracebufs2Read,
                                                        msgs, &tb2Data);
//LOG_MSG("%s", "== free msgs memory");
        memory_free8c(&msgs);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s: Error unpacking tracebuf2 messages\n", fcnm);
            goto ERROR;
        }
//printf("end %d %8.4f\n", nTracebufs2Read, ISCL_time_timeStamp() - tbeger);
tbeger = ISCL_time_timeStamp();

if (0) {
  for (i=0;i<tb2Data.ntraces;i++){
    if (strcmp(tb2Data.traces[i].stnm, "0001")==0 && strcmp(tb2Data.traces[i].chan, "LYZ")==0) {
      for (chunk=0;chunk<tb2Data.traces[i].nchunks; chunk++){
        i1 = tb2Data.traces[i].chunkPtr[chunk];
        i2 = tb2Data.traces[i].chunkPtr[chunk+1];
        for (is=i1; is<i2; is++) {
          printf("%s.%s.%s.%s time:%f val:%8.2f [tb2Data]\n",
                  tb2Data.traces[i].netw, tb2Data.traces[i].stnm,
                  tb2Data.traces[i].chan, tb2Data.traces[i].loc,
                  tb2Data.traces[i].times[is], (double)tb2Data.traces[i].data[is]);
        }
      }
    }
  }
}

        // Update the hdf5 buffers

//LOG_MSG("%s", "== Update the hdf5 buffers");
        ierr = traceBuffer_h5_setData(t1,
                                      tb2Data,
                                      h5traceBuffer);

        if (ierr != 0)
        {
            LOG_ERRMSG("%s: Error setting data in H5 file\n", fcnm);
            goto ERROR;
        }
//exit(0);
//printf("update %8.4f\n", ISCL_time_timeStamp() - tbeger);
//printf("full %8.4f\n", ISCL_time_timeStamp() - tbeger0);
// early quit

        // Check my mail for an event
        if (USE_AMQ){
          msWait = props.activeMQ_props.msWaitForMessage;
          amqMessage = GFAST_activeMQ_consumer_getMessage(messageQueue, msWait, &ierr);
        }

        // Alternatively, check for SA message trigger in message_dir
        if (check_message_dir) {
          amqMessage = check_dir_for_messages(message_dir, &ierr);
          if (ierr != 0){
//printf("** MTH: check_dir_for_messages return ierr=%d\n", ierr);
ierr=0;
          }
        }

        if (ierr != 0)
        {
            LOG_ERRMSG("%s: Internal error when getting message\n", fcnm);
            goto ERROR;
        }
    // If there's a message then process it
        if (amqMessage != NULL)
        {
LOG_MSG("== [GFAST t0:%f] Got new amqMessage:", t0);
LOG_MSG("%s", amqMessage);
            // Parse the event message 
            ierr = GFAST_eewUtils_parseCoreXML(amqMessage, -12345.0, &SA);
            if (ierr != 0)
            {
                LOG_ERRMSG("%s: Error parsing the decision module message\n",
                          fcnm);
                LOG_ERRMSG("%s\n", amqMessage);
                goto ERROR;
            }
//printf("eventid:%s time:%f lat:%f lon:%f\n", SA.eventid, SA.time, SA.lat, SA.lon);
            // If this is a new event we have some file handling to do
            lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
            if (lnewEvent){
              LOG_MSG("NEW event evid:%s lat:%8.3f lon:%8.3f dep:%5.3f mag:%.2f time:%f age_now:%f",
                      SA.eventid, SA.lat, SA.lon, SA.dep, SA.mag, SA.time, t0 - SA.time);
            }
            else{
              LOG_MSG("This is NOT a new event: evid=%s", SA.eventid);
            }

            if (lnewEvent)
            {
                // And the logs
                if (props.verbose > 0)
                {
                    LOG_INFOMSG("%s: New event %s added\n", fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_core_events_printEvents(SA);}
                }
                // Set the log file names
                eewUtils_setLogFileNames(SA.eventid,
                                        errorLogFileName, infoLogFileName,
                                        debugLogFileName, warnLogFileName);
                if (os_path_isfile(errorLogFileName))
                {
                    remove(errorLogFileName);
                }
                if (os_path_isfile(infoLogFileName))
                {
                    remove(infoLogFileName);
                }
                if (os_path_isfile(debugLogFileName))
                {
                  remove(debugLogFileName);
                }
                if (os_path_isfile(warnLogFileName))
                {
                  remove(warnLogFileName);
                }
                // Initialize the HDF5 file
                ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                            SA.eventid,
                                            props.propfilename);
                if (ierr != 0)
                {
                    LOG_ERRMSG("%s: Error initializing the archive file\n", fcnm);
                    goto ERROR;
                }
            }
            free(amqMessage);
            amqMessage = NULL;
        } // End check on ActiveMQ message

        // Are there events to process?
        if (events.nev < 1){continue;} 
        if (props.verbose > 2)
        {
            LOG_DEBUGMSG("%s: Processing events...\n", fcnm);
        }
LOG_DEBUGMSG("%s: MTH: Call driveGFAST DEBUG msg\n", fcnm);
//LOG_MSG("== [GFAST t0:%f] Call driveGFAST:", t0);
        ierr = eewUtils_driveGFAST(t1, //currentTime,
                                   props,
                                   &events,
                                   &gps_data,
                                   &h5traceBuffer,
                                   &pgd_data,
                                   &cmt_data,
                                   &ff_data,
                                   &pgd,
                                   &cmt,
                                   &ff,
                                   &xmlMessages,
                                   &xml_status);
/*
         if (ierr != 0)
         {
             LOG_ERRMSG("%s: Error calling GFAST driver!\n", fcnm);
             goto ERROR; 
         }
*/


/*
         for (iev=0;iev<events.nev;iev++){
printf("GFAST: eventid:%s pgd mag nsites=%d ndeps=%d mpgd[0]=%f\n",
       events.SA[iev].eventid, pgd[iev].nsites, pgd[iev].ndeps, pgd[iev].mpgd[0]);
         }
*/
/*
printf("GFAST: cmt mag nsites=%d ndeps=%d Mw[0]=%f str=%.1f dip=%.1f rake=%.1f\n",
cmt.nsites, cmt.ndeps, cmt.Mw[0], cmt.str1[0], cmt.dip1[0], cmt.rak1[0]);
printf("GFAST: cmt mag nsites=%d ndeps=%d Mw[3]=%f str=%.1f dip=%.1f rake=%.1f\n",
cmt.nsites, cmt.ndeps, cmt.Mw[3], cmt.str1[3], cmt.dip1[3], cmt.rak1[3]);
*/
//printf("GFAST: events.nev=%d xmlMessages.nmessages=%d mmessages=%d\n", 
       //events.nev, xmlMessages.nmessages, xmlMessages.mmessages);
         // Send the messages where they need to go
         if (xmlMessages.mmessages > 0)
         {
             for (im=0; im<xmlMessages.nmessages; im++)
             {
//LOG_MSG("== [GFAST t0:%f] evid:%s pgdXML=[%s]\n", t0,xmlMessages.evids[im], xmlMessages.pgdXML[im]);
//printf("GFAST: evid:%s cmtQML=[%s]\n", xmlMessages.evids[im], xmlMessages.cmtQML[im]);
//printf("GFAST: evid:%s  ffXML=[%s]\n", xmlMessages.evids[im], xmlMessages.ffXML[im]);
                 if (xmlMessages.evids[im] != NULL)
                 {
                     free(xmlMessages.evids[im]);
                 }
                 if (xmlMessages.cmtQML[im] != NULL)
                 {
                     free(xmlMessages.cmtQML[im]);
                 }
                 if (xmlMessages.pgdXML[im] != NULL)
                 {
                     free(xmlMessages.pgdXML[im]);
                 }
                 if (xmlMessages.ffXML[im] != NULL)
                 {
                     free(xmlMessages.ffXML[im]);
                 }
             }
             if (xmlMessages.evids != NULL){free(xmlMessages.evids);}
             if (xmlMessages.cmtQML != NULL){free(xmlMessages.cmtQML);}
             if (xmlMessages.ffXML  != NULL){free(xmlMessages.ffXML);}
             if (xmlMessages.pgdXML != NULL){free(xmlMessages.pgdXML);}

// MTH
         if (ierr != 0)
         {
             LOG_ERRMSG("%s: Error calling GFAST driver!\n", fcnm);
             goto ERROR; 
         }

             memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
//printf("early exit\n");
//break;
//if (events.nev == 0){break;}
         }
    niter ++;
    }
ERROR:;
// Close the big logfile
    core_log_closeLog();
    memory_free8c(&msgs);
    core_events_freeEvents(&events);
    traceBuffer_ewrr_freetb2Data(&tb2Data);
    traceBuffer_ewrr_finalize(&ringInfo);
    if (USE_AMQ){
    	activeMQ_consumer_finalize(messageQueue); 
    }
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
    iscl_finalize();
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
        LOG_ERRMSG("%s: Error no data to copy\n", fcnm);
        return -1;
    }
    if (tb2Data->linit)
    {
        LOG_ERRMSG("%s: Error tb2Data already set\n", fcnm);
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
        LOG_ERRMSG("%s: Lost count %d %d\n", fcnm, it, tb2Data->ntraces);
        return -1;
    }
    tb2Data->linit = true;
    return 0;
}

char *check_dir_for_messages(const char *dirname, int *ierr)
{
  int i;
  struct dirent *de; // Pointer for directory entry
  // opendir() returns a pointer of DIR type.
  DIR *dr = opendir(dirname);
  char *ext;
  char *fullpath;
  char *buffer;
  long length = 0;
  FILE *f;
  char *message = NULL;
  int nevents=0;
  size_t dirlen = strlen(dirname);

  *ierr = 1;

  if (dr == NULL) // opendir returns NULL if couldn't open directory
  {
   printf("Could not open directory:%s\n", dirname );
   return NULL;
  }

  // MTH: This is currently set up to expect only 1 .xml file in dir at a time,
  //      but could easily be modified to handle more
  while ((de = readdir(dr)) != NULL)
  {
    if (de->d_name[0] != '.')
    {
      ext = strrchr(de->d_name, '.');
      //printf("%s ext=[%s]\n", de->d_name, ext);
      if (ext && !strcmp(ext+1, "xml")) {
        //printf("%s ext=[%s] len=%ld\n", de->d_name, ext, strlen(de->d_name));

        /* + 2 because of the '/' and the terminating 0 */
        fullpath = malloc(dirlen + strlen(de->d_name) + 2);
        sprintf(fullpath, "%s/%s", dirname, de->d_name);

        f = fopen(fullpath, "r");
        if (f)
        {
          fseek(f, 0, SEEK_END);
          length = ftell(f);
          fseek(f, 0, SEEK_SET);
          buffer = (char *)malloc((length+1)*sizeof(char));
          if (buffer)
          {
            fread(buffer,sizeof(char), length, f);
          }
          fclose(f);
          buffer[length] = '\0';
          message = buffer;
        }
        else {
          printf("Error reading from file:%s\n", fullpath);
          //return NULL;
          goto ERROR;
        }

        // Now remove the event file we just read:
        *ierr = remove(fullpath);
        if (*ierr != 0) {
          LOG_ERRMSG("gfast_eew: Unable to delete file:%s\n", fullpath);
        }

        free(fullpath);
        nevents++;
      }
    }
  }
  closedir(dr);
  //printf("Return nevents=%d\n" , nevents);
  *ierr = 0;
  return message;

ERROR:;
  closedir(dr);
  return NULL;
}

