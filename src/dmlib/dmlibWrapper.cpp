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
  static cms::Connection *amqconnection=NULL;
  static HBProducer *hbproducer=NULL;
  static DMMessageSender *eventsender=NULL;
  static CoreEventInfo *eventmessage=NULL;
  static std::string hbSender="";
  static std::string hbTopic="";
  static int conVerbose=0;
  static int hbVerbose=0;
}

int startAMQconnection(const char AMQuser[],
		       const char AMQpassword[],
		       const char AMQhostname[],
		       const int port,
		       const int msReconnect,
		       const int maxAttempts,
		       const int verbose=1) {
  conVerbose=verbose;
  const char *fcnm = "startAMQconnection\0";
  char *brokerURI;
  brokerURI = activeMQ_setTcpURIRequest(AMQhostname, port,
					msReconnect, maxAttempts);
  if (amqconnection!=NULL) {
    printf("%s: connection already exists",fcnm);
    return 0;
  }
  try
    {
      // Create a connection factory
      if (conVerbose > 2)
	{
	  printf("%s: Setting the broker URI: %s\n",
		 fcnm, brokerURI);
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	   cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

      if (conVerbose > 2)
	{
	  printf("%s: Creating connection for username (%s)\n",
		 fcnm, AMQuser);
	}
      // Create a connection
      amqconnection = connectionFactory->createConnection(AMQuser, AMQpassword);
      connectionFactory.reset();
      delete[] brokerURI;
      if (amqconnection==NULL) return -1;
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
      return -1;
    }
  return 1;
}

int stopAMQconnection() {
  const char *fcnm = "stopAMQconnection\0";

  if (amqconnection==NULL) {
    printf("%s: connection already dead",fcnm);
    return 0;
  }
  amqconnection->close();
  delete amqconnection;
  amqconnection=NULL;
  return 1;
}

bool isAMQconnected() {
  const char *fcnm = "isAMQconnected\0";
  if (amqconnection==NULL) {
    return false;
  }
  //vck: add real connected test here and make default -1
  printf("%s: dummy function call",fcnm);
  return true;
}

int startEventSender(const char eventtopic[]) {
  const char *fcnm = "startEventSender\0";
  if (not isAMQconnected()) {
      printf("%s: Cannot start event sender without activeMQ connection.",fcnm);
      return -1;
    }
  if (eventsender != NULL) {
      printf("%s: Event sender already initalized.",fcnm);
      return 0;
    }
  try {
    eventsender = new DMMessageSender(amqconnection,eventtopic);
  }
  catch (exception &e)
    {
      printf("%s: Encountered Exception creating DMMessageSender\n%s",fcnm,e.what());
      return -1;
    }
  return 1;
}

int stopEventSender() {
  const char *fcnm = "stopEventSender\0";
  if (eventsender==NULL) {
    printf("%s: Event sender not running",fcnm);
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
  const char *fcnm = "sendEventMessage\0";
  if (eventmessage==NULL) {
    printf("%s: No message to send",fcnm);
    return 0;
  }
  try {
    eventsender->sendMessage(eventmessage);
  }
  catch (exception &e) {
    printf("%s: DMMessageSender error: %s",fcnm,e.what());
  }
  if (conVerbose>1) {
    printf("%s: sent GFAST message to activemqfor evid:%s",fcnm,eventmessage->getID().c_str());
  }
  return 1;
}

int sendEventXML(const char xmlstr[]) {
  const char *fcnm = "sendEventXML\0";
  eventsender->sendString(xmlstr);
  if (conVerbose>1) {
    printf("%s: sent xml message to activemq",fcnm);
  }
  return 1;
}

int startHBProducer(const char sender[],
		    const char hbtopic[],
		    int interval=0,
		    int verbose=1){
  const char *fcnm = "startHBProducer\0";
  if (amqconnection==NULL) {
    printf("%s: Error: AMQ connection must be started before HBProducer.",fcnm);
    return -1;
  }
  if (hbproducer!=NULL) {
    printf("%s: HBProducer already started.",fcnm);
    return 0;
  }
  hbVerbose=verbose;
  hbSender=string(sender);
  hbTopic=string(hbtopic);
  hbproducer = new HBProducer(amqconnection,hbSender,hbTopic,interval);
  return (hbproducer==NULL)?-1:1;
}

int stopHBProducer(){
  const char *fcnm = "stopHBProducer\0";
  if (hbproducer==NULL) {
    printf("%s: HB producer not running",fcnm);
      return 0;
  }
  delete hbproducer;
  hbproducer=NULL;
  return 1;
}

int sendHeartbeat(){
  const char *fcnm = "sendHeartbeat\0";
  std::string timestr;
  if (hbproducer==NULL) {
    printf("%s: HB producer not running",fcnm);
      return 0;
  }
  hbproducer->sendHeartbeat("","","");
  return 1;
}

int createDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime) {
  //taking defaults for lklihd,type,ver,category,time_stamp,alg_ver,instance,num_stations,ref_id,
  //ref_src,orig_sys,mag_units,mag_uncer_units,lat_units,lat_uncer_units,lon_units,lon_uncer_units,
  //dep_units,dep_uncer_units,o_time_units,o_time_uncer_units
  const char *fcnm = "createDMEventObject\0";

  if (eventmessage!=NULL) {
    printf("%s: DMEventObject already started.",fcnm);
    return 0;
  }
  eventmessage = new CoreEventInfo(GFAST, evid, mag, 0.0, lat, 0.0, lon, 0.0, depth, 0.0, otime, 0.0);
  return 1;
}

int modifyDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime) {
  const char *fcnm = "modifyDMEventObject\0";
  if (eventmessage==NULL) {
    printf("%s: DMEventObject must exist.  Call createDMEventObject first.",fcnm);
    return 0;
  }
  if (evid!=eventmessage->getID()) {
    printf("%s: event id's do not match. %s!=%s",fcnm,evid,eventmessage->getID().c_str());
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
  const char *fcnm = "deleteDMEventObject\0";
  if (eventmessage==NULL) {
    printf("%s: DMEventObject does not exist.",fcnm);
    return 0;
  }
  if (evid!=eventmessage->getID()) {
    printf("%s: event id's do not match. %s!=%s",fcnm,evid,eventmessage->getID().c_str());
    return -1;
  }
  delete eventmessage;
  eventmessage=NULL;
  return 1;
}
