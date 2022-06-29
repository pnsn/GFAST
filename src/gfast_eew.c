#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "gfast.h"
#include "gfast_core.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"
#include <dirent.h>
#include "dmlibWrapper.h"

#include <time.h>

#define MAX_MESSAGES 100000

static int settb2DataFromGFAST(struct GFAST_data_struct gpsData,
                               struct tb2Data_struct *tb2Data);
/*!
 * @brief poll directory for new messages.
 * @param[in] dirname directory to search for messages
 * @param[out] error code
 * @return contents of message(s?) read from directory
 */
char *check_dir_for_messages(const char *dirname, int *ierr);


/*!
 * @brief GFAST earthquake early warning driver routine
 * First argument assumed to be configuration file name.  Defaults to gfast.props.
 *
 */
int main(int argc, char **argv)
{
  const char *fcnm = "gfast_eew\0";
  char propfilename[PATH_MAX];
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
  double t0, t1;
  const enum opmode_type opmode = REAL_TIME_EEW;
  /*activeMQ variables*/
  char *amqMessage;
  const bool useTopic = true;              /**< ShakeAlert uses topics */
  const bool clientAck = false;            /**< False means set session acknowledge transacations */
  const bool luseListener = false;         /**< C can't trigger so turn this off (remove?) */
  double tstatus, tstatus0, tstatus1;
  static void *amqMessageListener = NULL;  /**< pointer to ShakeAlertConsumer object */
  int ierr, im, msWait, nTracebufs2Read;
  bool lacquire, lnewEvent;
  const int rdwt = 2; // H5 file is read/write
  char errorLogFileName[PATH_MAX];
  char infoLogFileName[PATH_MAX];
  char debugLogFileName[PATH_MAX];
  char warnLogFileName[PATH_MAX];
  bool check_message_dir = false;
  bool USE_AMQ = false;
  bool USE_DMLIB = false;
  int niter = 0;
  int i, i1, i2, is, chunk;
#ifdef GFAST_USE_AMQ
  USE_AMQ = true;
// GFAST_USE_DMLIB defined inside GFAST_USE_AMQ since dmlib won't work without amq
#ifdef GFAST_USE_DMLIB
  USE_DMLIB = true;
#endif
#endif

  // Get program instance for ShakeAlert-approved xml header
  char longprog[PATH_MAX], program_instance[PATH_MAX];

  // Get program name
  strncpy(longprog, argv[0], PATH_MAX - 1);
  char *pch = strrchr(longprog, '/');
  size_t lenshort = &longprog[strlen(longprog)] - pch;
  if (pch != NULL) {
    memcpy(program_instance, pch + 1, lenshort - 1);
    program_instance[lenshort] = '\0';
  } else {
    strncpy(program_instance, argv[0], PATH_MAX - 1);
  }
  
  // Add host name to program_instance
  char name[1000];
  if(gethostname(name, (int)sizeof(name) - 1) == 0) {
    // if eew-bk-dev1.geo.berkeley.edu, shorten to eew-bk-dev1
    char *c = strstr(name, ".");
    if(c != NULL) *c = '\0';
    strcat(program_instance, "@\0");
    strcat(program_instance, name);
  }

  // Initialize
  if (argc>1) {
    strncpy(propfilename,argv[1],PATH_MAX-1);
  } else {
    strcpy(propfilename,"gfast.props\0");
  }

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

  printf("%s Version: %s\n", fcnm, GFAST_VERSION);
  printf("%s: Reading configuration from %s\n", fcnm, propfilename);
  // Read the program properties
  ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
  if (ierr != 0)
    {
      LOG_ERRMSG("%s: Error reading GFAST initialization file: %s\n", fcnm, propfilename);
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

  if (USE_AMQ) {
    // Fire up the listener
    if (props.verbose > 0)
      {
	LOG_INFOMSG("%s: Initializing trigger listener on %s...\n", fcnm,
		    props.activeMQ_props.originTopic);
	LOG_MSG("%s: Initializing trigger listener on %s...", fcnm,
		props.activeMQ_props.originTopic);
      }
    activeMQ_start();  // start library
    amqMessageListener = activeMQ_consumer_initialize(props.activeMQ_props.user,
						      props.activeMQ_props.password,
						      props.activeMQ_props.originURL,
						      props.activeMQ_props.originTopic,
						      props.activeMQ_props.msReconnect,
						      props.activeMQ_props.maxAttempts,
						      useTopic,
						      clientAck,
						      luseListener,
						      props.activeMQ_props.maxMessages,
						      props.verbose,
						      &ierr);
    if (ierr != 0)
      {
	LOG_ERRMSG("%s: Error connecting listener to ActiveMQ\n", fcnm);
	goto ERROR;
      }
    /* dmlib startup */
    if (USE_DMLIB)
      {
        /* start connection */
        ierr = startDestinationConnection(props.activeMQ_props.user,
                                          props.activeMQ_props.password,
                                          props.activeMQ_props.destinationURL,
                                          props.activeMQ_props.msReconnect,
                                          props.activeMQ_props.maxAttempts,
                                          props.verbose);
        if (ierr == 0) {
          LOG_ERRMSG("%s: Attemted to re-initialize activeMQ connection object", fcnm);
        }
        if (ierr < 0) {
          LOG_ERRMSG("%s: Error initializing activeMQ connection object", fcnm);
          goto ERROR;
        }
        /* start heartbeat producer and set to manual heartbeats */
        if ((props.activeMQ_props.hbTopic != NULL) &&
            (strlen(props.activeMQ_props.hbTopic) > 0)) {
          char senderstr[100], *pp;
          int ii;
          strcpy(senderstr, "gfast.");
          ii = strlen(senderstr);
          gethostname(senderstr + ii, 90);   /*append hostname*/
          pp = strchr(senderstr + ii, '.');  /*find . in hostname if any*/
          if (pp != NULL) *pp = '\0';        /*truncate long hostname*/
          if (props.verbose > 0)
            {
              LOG_INFOMSG("%s: Initializing heartbeat sender %s on %s...\n", fcnm,
                          senderstr, props.activeMQ_props.hbTopic);
              LOG_MSG("%s: Initializing heartbeat sender %s on %s...", fcnm,
                      senderstr, props.activeMQ_props.hbTopic);
            }
          ierr = startHBProducer(senderstr,
                                 props.activeMQ_props.hbTopic,
                                 props.activeMQ_props.hbInterval,
                                 props.verbose);
          if (ierr == 0) {
            LOG_ERRMSG("%s: Attemted to re-initialize active HB producer object", fcnm);
          }
          if (ierr < 0) {
            LOG_ERRMSG("%s: Error initializing HB producer object", fcnm);
            goto ERROR;
          }
        }
        /*start message sender*/
        if (props.verbose > 0)
          {
            LOG_INFOMSG("%s: Initializing event sender on %s...\n", fcnm,
                        props.activeMQ_props.hbTopic);
            LOG_MSG("%s: Initializing event sender on %s...", fcnm,
                    props.activeMQ_props.destinationTopic);
          }
        ierr = startEventSender(props.activeMQ_props.destinationTopic);
        if (ierr == 0) {
          LOG_ERRMSG("%s: Attemted to re-initialize active event sender object", fcnm);
        }
        if (ierr < 0) {
          LOG_ERRMSG("%s: Error initializing event sender object", fcnm);
          goto ERROR;
        }
      } /* end of if USE_DMLIB */
  } /* end if USE_AMQ */

  if (strlen(props.SAeventsDir)) {
    message_dir = props.SAeventsDir;
    check_message_dir = true;
  }

  // Initialize PGD
  ierr = core_scaling_pgd_initialize(props.pgd_props, gps_data,
				     &pgd, &pgd_data);
  if (ierr != 0) {   
    LOG_ERRMSG("%s: Error initializing PGD\n", fcnm);
    goto ERROR;
  }
  // Initialize CMT
  ierr = core_cmt_initialize(props.cmt_props, gps_data,
			     &cmt, &cmt_data);
  if (ierr != 0) {   
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
  LOG_INFOMSG("%s: Connecting to earthworm ring %s\n", fcnm, ringInfo.ewRingName);
  LOG_MSG("%s: Connecting to earthworm ring %s", fcnm, ringInfo.ewRingName);
  ierr = traceBuffer_ewrr_initialize(props.ew_props.gpsRingName,
				     10,
				     &ringInfo);
  if (ierr != 0)
    {
      LOG_ERRMSG("%s: Error initializing tracebuf reader\n", fcnm);
      goto ERROR;
    }
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
  t0 = time_timeStamp();
  //unused: t_now = (double) (long) (time_timeStamp() + 0.5);
  //unused: tbeg = t0; 
  tstatus = t0;
  tstatus0 = t0;
  lacquire = true;
  /***************************************************
   * Start of main acquisition loop
   ***************************************************/
  while(lacquire)
    {
      // Initialize the iteration
      amqMessage = NULL;
      // Don't start loop until prop.waitTime has elapsed (default 1 second)
      t1 = time_timeStamp();
      double tloop = t1-t0;
      if (tloop < props.waitTime) {
	continue;
      }
      else if ((props.waitTime>0.0)&&((tloop) >= 2*props.waitTime)) {
	LOG_MSG("== [GFAST main loop took %fs >= 2x%fs waitTimes. not keeping up", tloop, props.waitTime);
      }

      t0 = t1;
      tstatus1 = t0;

      //printf("\n== [Iter:%d t0:%f] ==\n", niter,t0);
      //printf("\n== [GFAST t0:%f] ==\n", t0);
      //LOG_MSG("== [GFAST t0:%f]", t0);
      LOG_MSG("== [GFAST t0:%f Get the msgs off the EW ring]", time_timeStamp());

      if (tstatus1 - tstatus0 > 3600.0)
        {
	  LOG_DEBUGMSG("%s: GFAST has been running for %d hours\n",
                       fcnm, (int) ((tstatus1 - tstatus)/3600.0));
	  tstatus0 = tstatus1;
        } 

      double tbeger = time_timeStamp();
      memory_free8c(&msgs); //ISCL_memory_free__char(&msgs);

      msgs = traceBuffer_ewrr_getMessagesFromRing(MAX_MESSAGES,
                                                  false,
                                                  &ringInfo,
                                                  &nTracebufs2Read,
                                                  &ierr);
      LOG_MSG("== [GFAST t0:%f] getMessages returned nTracebufs2Read:%d", time_timeStamp(), nTracebufs2Read);

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
      LOG_MSG("scrounge %8.4f\n", time_timeStamp() - tbeger);
      tbeger = time_timeStamp();
     
      // Unpackage the tracebuf2 messages
      LOG_MSG("%s", "== unpackTraceBuf2Messages");
      ierr = traceBuffer_ewrr_unpackTraceBuf2Messages(nTracebufs2Read,
						      msgs, &tb2Data);
      memory_free8c(&msgs);
      if (ierr != 0)
        {
	  LOG_ERRMSG("%s: Error unpacking tracebuf2 messages\n", fcnm);
	  goto ERROR;
        }
 
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
      //LOG_MSG("%s returned ierr=%d", "== Update the hdf5 buffers", ierr);
      if (ierr != 0)
        {
	  LOG_ERRMSG("%s: Error setting data in H5 file\n", fcnm);
	  goto ERROR;
        }
      //exit(0);
      //printf("update %8.4f\n", ISCL_time_timeStamp() - tbeger);
      //printf("full %8.4f\n", ISCL_time_timeStamp() - tbeger0);
      // early quit
      
      // Check for an event
      if (USE_AMQ){
	if (props.verbose > 2) {
	  LOG_MSG("%s: Checking Activemq for events", fcnm);
	}
	msWait = props.activeMQ_props.msWaitForMessage;
	amqMessage = GFAST_activeMQ_consumer_getMessage(amqMessageListener, msWait, &ierr);
	if ((props.verbose > 2)&&(amqMessage == NULL)) {
	  LOG_MSG("%s: Activemq returned NULL", fcnm);
	}
      } 
      //else if (check_message_dir) {
      if (amqMessage == NULL && check_message_dir) {
	// Alternatively, check for SA message trigger in message_dir
	amqMessage = check_dir_for_messages(message_dir, &ierr);
	if ((ierr != 0)&&(props.verbose > 2)) {
	  LOG_MSG("check_dir_for_messages returned ierr=%d\n", ierr);
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
	      LOG_ERRMSG("%s: Error parsing the activeMQ trigger message\n",
			 fcnm);
	      LOG_ERRMSG("%s\n", amqMessage);
	      goto ERROR;
            }
	  if (strncasecmp(SA.orig_sys,"gfast",5)==0) 
	    {
	      LOG_MSG("ignoring activeMQ message from gfast: evid=%s orig_sys=%s",SA.eventid,SA.orig_sys);
	    }
	  else 
	    { //don't trigger on gfast messages
	      // If this is a new event we have some file handling to do
	      LOG_MSG("MTH: call newEvent events.nev=%d xml_status.nev=%d\n", events.nev, xml_status.nev);
	      lnewEvent = GFAST_core_events_newEvent(SA, &events, &xml_status);
	      LOG_MSG("%s", "MTH: call newEvent DONE");
	      if (lnewEvent){
		LOG_MSG("NEW event evid:%s lat:%7.3f lon:%8.3f dep:%6.2f mag:%.2f time:%.2f age_now:%.0f",
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
		  eewUtils_setLogFileNames(SA.eventid,props.SAoutputDir,
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
	    }  //end gfast message if/else check
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
      LOG_MSG("== [GFAST t0:%f] Call driveGFAST:", t0);
      ierr = eewUtils_driveGFAST(t1, //currentTime,
                                 program_instance,
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
      if (ierr != 0)
	{
	  LOG_ERRMSG("%s: Error calling GFAST driver!\n", fcnm);
	  goto ERROR; 
	}

      // This whole xmlMessages struct isn't currently necessary since messages are handled and
      // sent in driveGFAST, but keeping for now in case we want to change back - CWU
      // Send the messages where they need to go
      if (xmlMessages.mmessages > 0)
	{
	  for (im=0; im<xmlMessages.nmessages; im++)
	    {
	//       if ((USE_DMLIB) && (xmlMessages.pgdXML[im] != NULL)) {
	// 	sendEventXML(xmlMessages.pgdXML[im]);
	//       }
	//       LOG_MSG("== [GFAST t0:%f] evid:%s pgdXML=[%s]\n",
        //               t0, xmlMessages.evids[im], xmlMessages.pgdXML[im]);
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
  /***************************************************
   * End of main acquisition loop
   ***************************************************/
 ERROR:;
  memory_free8c(&msgs);
  core_events_freeEvents(&events);
  traceBuffer_ewrr_freetb2Data(&tb2Data);
  traceBuffer_ewrr_finalize(&ringInfo);
  if (USE_AMQ)
    {
      activeMQ_consumer_finalize(amqMessageListener);
      if (USE_DMLIB)
        {
          stopHBProducer();
          stopEventSender();
          //stopDestinationConnection();
        }
      activeMQ_stop();
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
  // Close the big logfile
  core_log_closeLog();
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
  const char *fcnm = __func__;
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

