/*!
 * @file dmlibWrapper.cpp
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#include "dmlibWrapper.h"

extern "C" int startAMQconnection(const char AMQuser[],
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
    printf("%s: connection already exists",fcnm)
      return 0;
  }
  try
    {
      // Create a connection factory
      if (__verbose > 2)
	{
	  printf("%s: Setting the broker URI: %s\n",
		 fcnm, brokerURI);
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	   cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

      if (__verbose > 2)
	{
	  printf("%s: Creating connection for username (%s)\n",
		 fcnm, __user.c_str());
	}
      // Create a connection
      amqconnection = connectionFactory->createConnection(AMQuser, AMQpassword);
      delete connectionFactory;
      delete[] brokerURI;
      if (amqconnection==NULL) return -1;
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
      return -1
    }
  return 1;
}

extern "C" int stopAMQconnection() {
  const char *fcnm = "stopAMQconnection\0";

  if (amqconnection==NULL) {
    printf("%s: connection already dead",fcnm);
    return 0;
  }
  amqconnection_>close();
  delete amqconnection;
  amqconnection=NULL;
  return 1;
}

extern "C" boolean isAMQconnected() {
  const char *fcnm = "isAMQconnected\0";
  if (amqconnection==NULL) {
    return false;
  }
  //vck: add real connected test here and make default -1
  return true;
}

extern "C" int startHBProducer(const char *sender,
			       const char *topic,
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
  hbSender==string(sender);
  hbproducer=HBProducer(amqconnection,hbSender,string(""),string(topic),interval);
  return (hbproducer==NULL)?-1:1;
}

extern "C" int stopHBProducer(){
  const char *fcnm = "stopHBProducer\0";
  if (hbproducer==NULL) {
    printf("%s: HB producer not running",fcnm);
      return 0;
  }
  delete hbproducer;
  hbproducer=NULL;
  return 1;
}

extern "C" int sendHeartbeat(){
  const char *fcnm = "sendHeartbeat\0";
  if (hbproducer==NULL) {
    printf("%s: HB producer not running",fcnm);
      return 0;
  }
  hbproducer->sendHeartbeat(hbsender);
  return 1;
}
