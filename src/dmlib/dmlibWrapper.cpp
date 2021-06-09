/*!
 * @file dmlibWrapper.cpp
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/Connection.h>
#include "DMLib.h"
#include "AlgMessage.h"
#include "CoreEventInfo.h"
#include "HBProducer.h"
#include "dmlibWrapper.h"
#include "DMMessageSender.h"
#include "gfast_activeMQ.h"
#include "gfast_struct.h"

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
    destinationConnection->close();
    delete destinationConnection;
    destinationConnection=NULL;
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
  if (not isAMQconnected()) {
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
    printf("%s: Starting heartbeat prodicer on topic: %s\n",__func__, hbtopic);
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
