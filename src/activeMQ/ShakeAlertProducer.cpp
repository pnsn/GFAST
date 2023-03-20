#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#pragma clang diagnostic ignored "-Wc++11-long-long"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-exception-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Weverything"
#pragma clang diagnostic pop
#endif

#include "gfast_core.h"
#include "ShakeAlertProducer.h"

using namespace decaf::lang;
using namespace activemq::core;
using namespace activemq::transport;
using namespace activemq::util;
using namespace cms;
using namespace std;

ShakeAlertProducer::ShakeAlertProducer() {
  __isInitialized = false;
  return;
}
  
ShakeAlertProducer::~ShakeAlertProducer() {
  this->cleanup();
  return;
}

void ShakeAlertProducer::initialize(const string username,
				    const string password,
				    const string destination,
				    const string brokerURI,
				    bool useTopic = true,
				    bool sessionTransacted = false,
				    int verbose = 0) {
  __connection = NULL;
  __session = NULL;
  __destination = NULL;
  __producer = NULL;
  
  __messageCount = 0;
  __user = username;
  __password = password;
  __destURI = destination;
  __brokerURI = brokerURI; 
  
  __useTopic = useTopic;
  __sessionTransacted = sessionTransacted;
  __verbose = verbose;
  __isInitialized = true;
  __lconnected = false;
  __lhaveMessage = false;
  __textMessage = ""; 
  memset(__pad, 0, sizeof(__pad));
  return;
}

bool ShakeAlertProducer::isInitialized(void)
{
  return __isInitialized;
}

void ShakeAlertProducer::startMessageSender()
{
  const char *fcnm = "ShakeAlertSender startMessageSender\0";
  try
    {
      // Create a connection factory
      if (__verbose > 2)
	{
	  LOG_DEBUGMSG("%s: Setting the broker URI: %s",
		 fcnm, __brokerURI.c_str());
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	      cms::ConnectionFactory::createCMSConnectionFactory(__brokerURI));

      connectionFactory.reset();  //destroy

      if (__verbose > 2)
	{
	  LOG_DEBUGMSG("%s: Creating connection for username (%s)",
		 fcnm, __user.c_str());
	}
      // Create a connection
      __connection = connectionFactory->createConnection(__user.c_str(),
							 __password.c_str());
	
      __connection->setExceptionListener(this);
      if (__connection == NULL)
	{
	  LOG_WARNMSG("%s: Failed to start connection!", fcnm);
	  return;
	}
      
      // Create a session
      if (!__sessionTransacted)
	{
	  if (__verbose > 2)
	    {
	      LOG_DEBUGMSG("%s: Automatic message acknowledgement", fcnm);
	    }
	  __session
	    = __connection->createSession(cms::Session::AUTO_ACKNOWLEDGE);
	}
      else
	{
	  if (__verbose > 2)
	    {
	      LOG_DEBUGMSG("%s: Session will acknowledge transaction",
		     fcnm);
	    }
	  __session
	    = __connection->createSession(cms::Session::SESSION_TRANSACTED);
	}
      if (__session == NULL)
	{
	  LOG_ERRMSG("%s: Error session not made", fcnm);
	  return;
	}
      // Create the destination (topic or queue)
      if (__useTopic)
	{
	  if (__verbose > 2)
	    {
	      LOG_DEBUGMSG("%s: Topic destination is %s",
		     fcnm, __destURI.c_str());
	    }
	  __destination = __session->createTopic(__destURI);
	}
      else
	{
	  if (__verbose > 2)
	    {
	      LOG_DEBUGMSG("%s: Queue destination is %s",
		     fcnm, __destURI.c_str());
	    }
	  __destination = __session->createQueue(__destURI);
	}
      // Create a messageConsumer from the session
      if (__verbose > 2)
	{
	  LOG_MSG("%s: Creating message producer...", fcnm);
	}
      __producer = __session->createProducer(__destination);

      if (__verbose > 2){LOG_DEBUGMSG("%s: Starting connection", fcnm);}
      __connection->start();
      if (__connection == NULL)
	{
	  LOG_WARNMSG("%s: Failed to start connection!", fcnm);
	  return;
	}
      if (__verbose > 2){LOG_DEBUGMSG("%s: Connection set", fcnm);}
                
      if (__verbose > 2)
	{
	  LOG_DEBUGMSG("%s: ActiveMQ producer on-line...", fcnm);
	}
      __lconnected = true;

    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  return;
}

int ShakeAlertProducer::sendMessage(const char *message)
{
  const char *fcnm = "ShakeAlertSender sendMessage\0";
  if (!__isInitialized)
    {
      LOG_ERRMSG("%s: Producer not yet initialized", fcnm);
      return -1;
    }
  if (!__lconnected)
    {
      LOG_ERRMSG("%s: Producer not yet connected", fcnm);
      return -1;
    }
  if (message == NULL)
    {
      LOG_WARNMSG("%s: NULL message!", fcnm);
      return -1;
    }
  string msg = string(message);
  cms::TextMessage *amqMessage = __session->createTextMessage(msg);
  __producer->send(amqMessage);
  delete amqMessage;
  return 0;
}

/*
int ShakeAlertProducer::sendBytesMessage(const unsigned char *cbytes,
					    int bsize)
{
  cms::BytesMessage message = __session->createBytesMessage(cbytes,bsize);
  __producer->send(message);
  delete message;
  return 0;
}
*/

void ShakeAlertProducer::onException(const cms::CMSException& ex AMQCPP_UNUSED)
{
  LOG_ERRMSG("%s",
	  "CMS Exception occurred.  Shutting down client.");
  ex.printStackTrace();
  exit(1); // This looks dangerous
}

void ShakeAlertProducer::cleanup()
{
  const char *fcnm = "cleanup\0";
  if (__verbose > 0)
    {
      LOG_MSG("%s: Closing producer...", fcnm);
    }
  if (!__isInitialized)
    {
      LOG_WARNMSG("%s: Program was never initialized", fcnm);
    }

  // Close the connection
  try
    {
      if (__connection != NULL){
	__connection->close();
      }
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }

  // Free destination 
  try
    {
      if (__destination != NULL){
	delete __destination;
      }
    }
  catch (cms::CMSException& e)
    {
      e.printStackTrace();
    }
  __destination = NULL;
  // Free  producer
  try
    {
      if (__producer != NULL){delete __producer;}
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  __producer = NULL;
  // Close the session
  try
    {
      if (__session != NULL)
	{
	  __session->close();
	}
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  __session = NULL;
  
  delete __connection;
  __connection = NULL;

  // Reset internal variables
  __isInitialized = false;
  __lconnected = false;
  __messageCount = 0;
  __user = "";
  __password = "";
  __brokerURI = "";
  __destURI = "";

  __useTopic = true;
  __sessionTransacted = false;
  __verbose = 0;
  __isInitialized = false;
  __lconnected = false;
  __lhaveMessage = false;
  __textMessage = "";

  return;
}
