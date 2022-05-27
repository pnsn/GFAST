/*!
 * @file dmlibWrapper.cpp
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#include <string>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/Connection.h>
#include "DMLib.h"
#include "FiniteFaultMessage.h"
#include "CoreEventInfo.h"
#include "HBProducer.h"
#include "DMMessageSender.h"
#include "dmlibWrapper.h"
#include "gfast_activeMQ.h"
#include "gfast_struct.h"
#include "gfast_core.h"
#include "gfast_enum.h"
#include "gfast_xml.h"
#include "iscl/iscl/iscl_enum.h"
#include "iscl/time/time.h"

/*static variables local to this file*/
namespace {
  static cms::Connection *destinationConnection=NULL;
  static HBProducer *hbproducer=NULL;
  static DMMessageSender *eventsender=NULL;
  static CoreEventInfo *eventmessage=NULL;
  static std::string hbSender="";
  static std::string hbTopic="";
  static int conVerbose=0;
  static int hbVerbose=0;
}

/*
 * compareDist and dist_index are both used to sort pgd observations by distance from source
 * when creating xml message. Needed for adding the 'assoc' attribute correctly.
 */
static int compareDist(const void *x, const void *y);
struct dist_index {
  double dist;
  int indx;
};

int startDestinationConnection(const char AMQuser[],
			       const char AMQpassword[],
			       const char destinationURL[],
			       const int msReconnect,
			       const int maxAttempts,
			       const int verbose=1) {
  conVerbose=verbose;
  char *brokerURI;
  brokerURI = activeMQ_setTcpURIRequest(destinationURL,
					msReconnect, maxAttempts);
  if (destinationConnection!=NULL) {
    printf("%s: connection already exists\n",__func__);
    return 0;
  }
  try
    {
      // Create a connection factory
      if (conVerbose > 2)
	{
	  printf("%s: Setting the broker URI: %s\n",
		 __func__, brokerURI);
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	   cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

      if (conVerbose > 2)
	{
	  printf("%s: Creating connection for username (%s)\n",
		 __func__, AMQuser);
	}
      // Create a connection
      destinationConnection = connectionFactory->createConnection(AMQuser, AMQpassword);
      connectionFactory.reset();
      delete[] brokerURI;
      if (destinationConnection==NULL) return -1;
    }
  catch (cms::CMSException &e)
    {
      printf("%s: Exception encountered creating dmlib connection\n%s",__func__,e.what());
      e.printStackTrace();
      return -1;
    }
  return 1;
}

int stopDestinationConnection() {
  if ( destinationConnection==NULL ) {
    printf("%s: connection already dead\n",__func__);
    return 0;
  } else {
    try {
      /*if ( not destinationConnection->isClosed() {
	  destinationConnection->stop();
	  destinationConnection->close();
	  }*/
      delete destinationConnection;
      destinationConnection=NULL;
    }
    catch (cms::CMSException &e) {
      printf("%s: CMSException encountered closing dmlib destination connection\n%s",__func__,e.what());
      e.printStackTrace();
      return -1;
    }
    catch (exception &e) {
      printf("%s: Exception encountered closing dmlib destination connection\n%s",__func__,e.what());
      return -1;
    }
  }
  return 1;
}

bool isAMQconnected() {
  if (destinationConnection==NULL) {
    return false;
  }
  //vck: add real connected test here and make default -1
  printf("%s: dummy placeholder function call\n",__func__);
  return true;
}

int startEventSender(const char eventtopic[]) {
  if (conVerbose > 2)
    {
      printf("%s: Starting dmlib DMMessageSender on topic: %s\n",
	     __func__, eventtopic);
    }
  if (!isAMQconnected()) {
      printf("%s: Cannot start event sender without activeMQ connection.",__func__);
      return -1;
    }
  if (eventsender != NULL) {
      printf("%s: Event sender already initalized\n",__func__);
      return 0;
    }
  try {
    eventsender = new DMMessageSender(destinationConnection,eventtopic);
  }
  catch (exception &e)
    {
      printf("%s: Encountered Exception creating DMMessageSender\n%s",__func__,e.what());
      return -1;
    }
  try {
    eventsender->run();
  }
  catch (exception &e)
    {
      printf("%s: Encountered Exception running DMMessageSender\n%s",__func__,e.what());
      return -1;
    }
  return 1;
}

int stopEventSender() {
  if (eventsender==NULL) {
    printf("%s: Event sender not running\n",__func__);
    return 0;
  } else {
    delete eventsender;
    eventsender=NULL;
  }
  if (eventmessage!=NULL) {
    delete eventmessage;
    eventmessage=NULL;
  }
  return 1;
}

int sendEventMessage() {
  if (conVerbose > 2)
    {
      printf("%s: Sending event message to activemq topic.\n",
	     __func__);
    }
  if (eventmessage==NULL) {
    printf("%s: No message to send\n",__func__);
    return 0;
  }
  try {
    eventsender->sendMessage(eventmessage);
  }
  catch (exception &e) {
    printf("%s: DMMessageSender error: %s",__func__,e.what());
  }
  if (conVerbose>1) {
    printf("%s: sent GFAST message to activemqfor evid:%s\n",__func__,eventmessage->getID().c_str());
  }
  return 1;
}

int sendEventXML(const char xmlstr[]) {
  if (conVerbose > 2)
    {
      printf("%s: Sending preformatted xml message to event topic.\n",
	     __func__);
    }

  eventsender->sendString(xmlstr);
  if (conVerbose>1) {
    printf("%s: sent xml message to activemq\n",__func__);
  }
  return 1;
}

int startHBProducer(const char sender[],
		    const char hbtopic[],
		    int interval=0,
		    int verbose=1) {
  if (destinationConnection==NULL) {
    printf("%s: Error: AMQ connection must be started before HBProducer\n",__func__);
    return -1;
  }
  if (hbproducer!=NULL) {
    printf("%s: HBProducer already started\n",__func__);
    return 0;
  }
  hbVerbose=verbose;
  hbSender=string(sender);
  hbTopic=string(hbtopic);
  if (hbVerbose > 2) {
    printf("%s: Starting heartbeat producer on topic: %s\n",__func__, hbtopic);
  }
  try {
    hbproducer = new HBProducer(destinationConnection,hbSender,hbTopic,interval);
  }
  catch (exception &e) {
    printf("%s: Encountered Exception creating dmlib HB producer\n%s",__func__,e.what());
    return -1;
  }
  /* this part is not working (steals the program) 
  if (interval > 0) {
    try {
      hbproducer->run();
    }
    catch (exception &e) {
      printf("%s: Encountered Exception in dmlib HB producer\n%s",__func__,e.what());
      return -1;
    }
  }
  */

  return (hbproducer==NULL)?-1:1;
}

int stopHBProducer() {
  if (hbVerbose > 2)
    {
      printf("%s: killing heartbeat producer\n",__func__);
    }

  if (hbproducer==NULL) {
    printf("%s: HB producer not running\n",__func__);
      return 0;
  } else {
    hbproducer->stop();
    delete hbproducer;
    hbproducer=NULL;
  }
  return 1;
}

int sendHeartbeat(){
  std::string timestr;
  if (hbproducer==NULL) {
    printf("%s: HB producer not running\n",__func__);
      return 0;
  }
  hbproducer->sendHeartbeat("","","");
  return 1;
}

int createDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime) {
  //taking defaults for lklihd,type,ver,category,time_stamp,alg_ver,instance,num_stations,ref_id,
  //ref_src,orig_sys,mag_units,mag_uncer_units,lat_units,lat_uncer_units,lon_units,lon_uncer_units,
  //dep_units,dep_uncer_units,o_time_units,o_time_uncer_units

  if (eventmessage!=NULL) {
    printf("%s: DMEventObject already started\n",__func__);
    return 0;
  }
  eventmessage = new CoreEventInfo(GFAST, evid, mag, 0.0, lat, 0.0, lon, 0.0, depth, 0.0, otime, 0.0);
  return 1;
}

int modifyDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime) {
  if (eventmessage==NULL) {
    printf("%s: DMEventObject must exist.  Call createDMEventObject first\n",__func__);
    return 0;
  }
  if (evid!=eventmessage->getID()) {
    printf("%s: event id's do not match. %s!=%s\n",__func__,evid,eventmessage->getID().c_str());
    return -1;
  }
  eventmessage->setMagnitude(mag);
  eventmessage->setLatitude(lat);
  eventmessage->setLongitude(lon);
  eventmessage->setDepth(depth);
  eventmessage->setOriginTime(otime);
  return 1;
}

int deleteDMEventObject(const char evid[]) {
  if (eventmessage==NULL) {
    printf("%s: DMEventObject does not exist.",__func__);
    return 0;
  }
  if (evid!=eventmessage->getID()) {
    printf("%s: event id's do not match. %s!=%s\n",__func__,evid,eventmessage->getID().c_str());
    return -1;
  }
  delete eventmessage;
  eventmessage=NULL;
  return 1;
}

char *dmlibWrapper_createPGDXML(const enum opmode_type mode,
                                const char *alg_vers,
                                const char *instance,
                                const char *message_type, 
                                const int max_assoc_stations,
                                const struct coreInfo_struct *core,
                                const struct GFAST_pgdResults_struct *pgd,
                                const struct GFAST_peakDisplacementData_struct *pgd_data,
                                int *ierr) {

  char *xmlmsg;
  xmlmsg = NULL;
  *ierr = 0;

  struct dist_index *vals;
  vals = (struct dist_index *) calloc((size_t) pgd->nsites, sizeof(struct dist_index));
  // int max_assoc_stations = 6;

  // Convert enum units to char
  char magUnits[32], magUncerUnits[32], latUnits[32], latUncerUnits[32], lonUnits[32],
      lonUncerUnits[32], depthUnits[32], depthUncerUnits[32], origTimeUnits[32],
      origTimeUncerUnits[32];
  __xml_units__enum2string(core->magUnits, magUnits);
  __xml_units__enum2string(core->magUncerUnits, magUncerUnits);
  __xml_units__enum2string(core->latUnits, latUnits);
  __xml_units__enum2string(core->latUncerUnits, latUncerUnits);
  __xml_units__enum2string(core->lonUnits, lonUnits);
  __xml_units__enum2string(core->lonUncerUnits, lonUncerUnits);
  __xml_units__enum2string(core->depthUnits, depthUnits);
  __xml_units__enum2string(core->depthUncerUnits, depthUncerUnits);
  __xml_units__enum2string(core->origTimeUnits, origTimeUnits);
  __xml_units__enum2string(core->origTimeUncerUnits, origTimeUncerUnits);

  // LOG_MSG("%s", "createEventXML - set unit chars");

  // convert to CoreEventInfo MessageCategory
  enum MessageCategory imode;
  if (mode == REAL_TIME_EEW) {
    imode = LIVE;
  } else if (mode == PLAYBACK) {
    imode = TEST;
  } else if (mode == OFFLINE) {
    imode = TEST;
  } else {
    printf("%s\n", "Defaulting to live mode");
    imode = LIVE;
  }

  // convert to CoreEventInfo nudMessageType
  enum nudMessageType itype;
  if (strcmp(message_type, "new") == 0) {
    itype = NEW;
  } else if (strcmp(message_type, "update") == 0) {
    itype = UPDATE;
  } else {
    printf("%s\n", "Message type not recognized! Defaulting to update");
    itype = UPDATE;
  }

  // required to create FiniteFaultMessage
  enum FaultSegment::FaultSegmentShape shape = FaultSegment::UNKNOWN_SEGMENT;

  // Get time stamp for when message is sent
  int rc;
  char cnow[128];
  double now;
  now = time_timeStamp();
  rc = xml_epoch2string(now, cnow);
  if (rc != 0) {
    printf("%s\n", "Error getting time string!");
  }

  FiniteFaultMessage algMessage(GFAST, shape, core->id, core->mag, core->magUncer, core->lat,
      core->latUncer, core->lon, core->lonUncer, core->depth, core->depthUncer, core->origTime,
      core->origTimeUncer, core->likelihood, itype, core->version, imode, cnow, alg_vers,
      instance, core->numStations, magUnits, magUncerUnits, latUnits,
      latUncerUnits, lonUnits, lonUncerUnits, depthUnits, depthUncerUnits, origTimeUnits,
      origTimeUncerUnits);

  // LOG_MSG("%s", "createEventXML - created algMessage");

  // Now add pgd observations to algMessage
  int i, j;
  int scnl_n = 8;
  char obs_sta[scnl_n], obs_net[scnl_n], obs_chan[scnl_n], obs_loc[scnl_n];
  char *token = NULL, *work = NULL;
  bool assoc_flag = false;
  int n_assoc = 0;

  enum ObservationType obs_type = DISPLACEMENT_OBS;
  
  // Later we assume pgd->nsites = pgd_data->nsites and indices correspond
  if (pgd->nsites != pgd_data->nsites) 
    {
      printf("%s: nsites don't match for pgd, pgd_data! %d, %d\n" ,__func__, pgd->nsites,
             pgd_data->nsites);
      *ierr = -1;
      return xmlmsg;
    }

  // Extra step to sort observations by distance, so that the first max_assoc_stations get the
  // assoc_flag = true. This is used by the Solution Aggregator to associate events. Other
  // algorithms might sort by observation value, but in this case we want to be sure to associate
  // with the input location, since GFAST doesn't determine its own event location.
  // Load distance and index to sort pgd obs by distance from source
  for ( i = 0; i < pgd->nsites; i++ ) 
    {
      vals[i].indx = i;
      // srdist has ndep*nsites. Just use the first depth, they should all be relatively the same
      vals[i].dist = pgd->srdist[i];
    }
  
  // Sort by distance
  qsort((void *) vals, (size_t) pgd->nsites, sizeof(struct dist_index), compareDist);

  // Go through the observations in ascending order of source-receiver distance
  for ( j = 0; j < pgd->nsites; j++ ) 
    {
      i = vals[j].indx;
      // skip site if it wasn't used
      if (!pgd->lsiteUsed[i]) { continue; }

      // LOG_MSG("createEventXML - i = %d, setting chars", i);
      // see core/data/readMetaDataFile for similar SNCL parsing
      memset(obs_sta, 0, scnl_n*sizeof(char));
      memset(obs_net, 0, scnl_n*sizeof(char));
      memset(obs_chan, 0, scnl_n*sizeof(char));
      memset(obs_loc, 0, scnl_n*sizeof(char));

      work = (char *)calloc(strlen(pgd_data->stnm[i])+1, sizeof(char));
      strcpy(work, pgd_data->stnm[i]);

      // LOG_MSG("%s", "createEventXML - starting NSCL tokenizing");
      token = strtok(work, ".");
      int i_tok = 0;
      while (token) 
        {
          if (i_tok == 0) { strcpy(obs_net, token); }
          if (i_tok == 1) { strcpy(obs_sta, token); }
          if (i_tok == 2) { strcpy(obs_chan, token); }
          if (i_tok == 3) { strcpy(obs_loc, token); }
          i_tok++;
          token = strtok(NULL, ".");
        }
      // LOG_MSG("%s", "createEventXML - done NSCL tokenizing, freeing work");
      delete work;
      work = NULL;
      // LOG_MSG("%s", "createEventXML - freed work, adding gmobs");
      
      assoc_flag = (n_assoc >= max_assoc_stations) ? false : true;

      // Make sure to convert pgd to cm (from m)
      algMessage.addGMObservation(obs_type,
                                  obs_sta,
                                  obs_net,
                                  obs_chan,
                                  obs_loc,
                                  pgd_data->pd[i] * 100.,
                                  pgd_data->sta_lat[i],
                                  pgd_data->sta_lon[i],
                                  pgd_data->pd_time[i],
                                  "cm\0",
                                  "deg\0",
                                  "deg\0",
                                  "UTC\0",
                                  "\0",
                                  assoc_flag);

      n_assoc++;

      LOG_MSG("createEventXML - added obs=%f, sta=%s, i=%d, j=%d, dist=%f, assoc=%d, n_assoc=%d",
              pgd_data->pd[i] * 100., pgd_data->stnm[i], i, j, vals[j].dist, assoc_flag, n_assoc);
    }

  LOG_MSG("%s", "createEventXML - finished adding gmobs, encoding message");

  // Finally, encode algMessage as xml
  std::string msg_tmp;

  try {
    msg_tmp = eventsender->getEncodedMessage(&algMessage);
  }
  catch (exception &e) {
    printf("%s: DMMessageSender error while encoding: %s",__func__,e.what());
    *ierr = -1;
    return xmlmsg;
  }

  xmlmsg = (char *)calloc(msg_tmp.length() + 1, sizeof(char));
  strncpy(xmlmsg, msg_tmp.c_str(), msg_tmp.length());

  free(token);
  free(vals);
  LOG_MSG("%s", "createEventXML - Returning");
  return xmlmsg;
}

static int compareDist(const void *x, const void *y) {
  const struct dist_index xx = *(const struct dist_index *) x;
  const struct dist_index yy = *(const struct dist_index *) y;

  if (xx.dist < yy.dist) return -1;
  if (xx.dist > yy.dist) return 1;
  return 0;
}
