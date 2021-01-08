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

#include "ShakeAlertConsumer.h"
 
ShakeAlertConsumer::ShakeAlertConsumer() {
  return;
}
  
ShakeAlertConsumer::~ShakeAlertConsumer() {
  this->cleanup();
  return;
}
  
void ShakeAlertConsumer::initialize(const string username,
				    const string password,
				    const string destination,
				    const string brokerURI,
				    bool useTopic = true,
				    bool clientAck = false,
				    bool luseListener = false,
				    int maxMessages = 5,
				    int verbose = 0) {
  __connection = NULL;
  __session = NULL;
  __destination = NULL;
  __consumer = NULL;
  
  __messageCount = 0;
  __user = username;
  __password = password;
  __destURI = destination;
  __brokerURI = brokerURI; 
  
  __useTopic = useTopic;
  __clientAck = clientAck;
  __luseListener = luseListener;
  __maxMessages = (maxMessages>0)?maxMessages:5; //error trap
  __verbose = verbose;
  __isInitialized = true;
  __lconnected = false;
  __textMessage = "";
  memset(__pad, 0, sizeof(__pad));
  return;
}

void ShakeAlertConsumer::startMessageListener()
{
  const char *fcnm = "ShakeAlertConsumer startMessageListener\0";
  try
    {
      // Create a connection factory
      if (__verbose > 2)
	{
	  printf("%s: Setting the broker URI: %s\n",
		 fcnm, __brokerURI.c_str());
	}
      auto_ptr<cms::ConnectionFactory> connectionFactory(
	   cms::ConnectionFactory::createCMSConnectionFactory(__brokerURI));

      if (__verbose > 2)
	{
	  printf("%s: Creating connection for username (%s)\n",
		 fcnm, __user.c_str());
	}
      // Create a connection
      __connection = connectionFactory->createConnection(__user.c_str(),
							 __password.c_str());
      //delete connectionFactory;

      activemq::core::ActiveMQConnection *amqConnection;
      amqConnection =
	dynamic_cast<activemq::core::ActiveMQConnection *>(__connection);
      if (amqConnection != NULL)
	{
	  amqConnection->addTransportListener(this);
	}
      
      __connection->setExceptionListener(this); 
      if (__connection == NULL)
	{
	  printf("%s: Failed to start connection!\n", fcnm);
	  return;
	}
      if (__verbose > 2){printf("%s: Connection set\n", fcnm);}

      // Create a session
      if (!__clientAck)
	{
	  if (__verbose > 2)
	    {   
	      printf("%s: Automatic message acknowledgement\n", fcnm);
	    }   
	  __session
	    = __connection->createSession(cms::Session::AUTO_ACKNOWLEDGE);
	}
      else
	{ 
	  if (__verbose > 2)
	    {
	      printf("%s: Client will acknowledge transaction\n",
		     fcnm);
	    }
	  __session
	    = __connection->createSession(cms::Session::SESSION_TRANSACTED);
	}
      // Create the destination (topic or queue)
      if (__useTopic)
	{
	  if (__verbose > 2)
	    {
	      printf("%s: Topic destination is %s\n",
		     fcnm, __destURI.c_str());
	    }
	  __destination = __session->createTopic(__destURI);
	}
      else
	{
	  if (__verbose > 2)
	    {
	      printf("%s: Queue destination is %s\n",
		     fcnm, __destURI.c_str());
	    }
	  __destination = __session->createQueue(__destURI);
	}
      // Create a messageConsumer from the session
      if (__verbose > 2)
	{
	  printf("%s: Creating message consumer...\n", fcnm);
	}
      __consumer = __session->createConsumer(__destination);
      if (__luseListener){
	__consumer->setMessageListener(this);
      }
      if (__verbose > 2){printf("%s: Starting connection\n", fcnm);}
      __connection->start();
      if (__connection == NULL)
	{
	  fprintf(stderr, "%s: Failed to start connection!\n", fcnm);
	  return;
	}
      if (__verbose > 2){printf("%s: Connection set\n", fcnm);}
      if (__verbose > 2)
	{
	  printf("%s: ActiveMQ consumer on-line...\n", fcnm);
	}
      __lconnected = true;
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  return; 
} // End startMessageListener

void ShakeAlertConsumer::onMessage(const cms::Message *message)
{
  const char *fcnm = "ShakeAlertConsumer onMessage\0";
  string newmsg;
  if (message == NULL){return;} //shouldn't happen
  try
    {
      const cms::TextMessage *textMessage;
      textMessage = dynamic_cast<const cms::TextMessage *>(message);
      if (textMessage != NULL)
	{
	  newmsg = textMessage->getText();
	  // check if queue full and if so clean old messages
	  int ndel=__maxMessages-__messageBuffer.size()+1;
	  while (ndel>0) {
	    if (__verbose > 2) {
	      printf("%s: delete old msg to make space in messageBuffer", fcnm);
	    }
	    __messageBuffer.pop();
	  }
	  __messageBuffer.push(newmsg);
	  if (__verbose > 2) {
	    printf("%s: Message received:\n%s\n",
		   fcnm, newmsg.c_str());
	  }
	}
      if (__clientAck)
	{
	  message->acknowledge();
	}
      delete textMessage;
      __messageCount = __messageCount + 1;
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  return;
}

bool ShakeAlertConsumer::isInitialized()
{
  return __isInitialized;
}

int ShakeAlertConsumer::pollAMQ(const int ms_wait, int *ierr)
{
  const char *fcnm = "ShakeAlertConsumer pollAMQ\0";
  const cms::Message *message;
  const cms::TextMessage *textMessage;
  string text;
  *ierr = 0;
  if (__luseListener) { //set for asynchronous. Return n messages in buffer
    return __messageBuffer.size();
  }
  if (! __lconnected)
    {
      printf("%s: The listener isn't connected!\n", fcnm);
      *ierr = 1;
      return 0;
    }
  // Check my mail
  do {
    if (ms_wait > 0)
      {
	message = __consumer->receive(ms_wait);
      }
    else
      {
	message = __consumer->receiveNoWait();
      }
    textMessage = dynamic_cast<const cms::TextMessage *>(message); 
    // If I have a message then push to messageBuffer
    if (textMessage != NULL)
      {
	text = textMessage->getText();
	if (__messageBuffer.size()==__maxMessages) {
	  if (__verbose > 2) {
	    printf("%s: delete old msg to make space in messageBuffer", fcnm);
	  }
	  __messageBuffer.pop();
	}
	__messageBuffer.push(text);
      }
  }
  while (textMessage != NULL);
  delete message;
  return __messageBuffer.size();
}

char *ShakeAlertConsumer::getMessage()
{
  char *charMessage = NULL;
  string msgstr;
  size_t lenos;
  
  if (! __messageBuffer.empty()) {
    msgstr=__messageBuffer.front();
    lenos = strlen(msgstr.c_str());
    charMessage =  static_cast<char *>(calloc(lenos+1, sizeof(char)));
    memset(charMessage, 0, lenos+1);
    strcpy(charMessage, msgstr.c_str());
    __messageBuffer.pop();
  }
  return charMessage;
}

void ShakeAlertConsumer::onException(const cms::CMSException& ex AMQCPP_UNUSED)
{
  fprintf(stderr, "%s",
	  "CMS Exception occurred.  Shutting down client.\n");
  ex.printStackTrace();
  exit(1); // This looks dangerous
}

void ShakeAlertConsumer::cleanup()
{
  const char *fcnm = "cleanup\0";
  
  if (!__isInitialized)
    {
      printf("%s: Program was never initialized\n", fcnm);
    }
  if (__verbose > 0)
    {
      printf("%s: Closing consumer...\n", fcnm);      
    }
  // Close the connection
  try
    {
      if (__connection != NULL){__connection->close();}
    }
  catch (cms::CMSException &e)
    {
      e.printStackTrace();
    }
  // Close the session
  try
    {
      if (__session != NULL){__session->close();}
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
	__destination = NULL;
	delete __consumer;
	__consumer = NULL;
	delete __session;
	__session = NULL;
      }
    }
  catch (cms::CMSException& e)
    {
      e.printStackTrace();
    }
  // Reset internal variables
  __isInitialized = false;
  __lconnected = false;
  __messageCount = 0;
  __user = "";
  __password = "";
  __brokerURI = "";
  __destURI = "";

  __useTopic = true;
  __clientAck = false;
  __luseListener = false;
  __verbose = 0;
  __isInitialized = false;
  __lconnected = false;
  __textMessage = "";
  return;
}
