#ifndef _ShakeAlertConsumer_h_
#define _ShakeAlertConsumer_h_

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <queue>
#include <string>
#include "gfast_activeMQ.h"

using namespace std;
/*!
 * @brief GFAST ActiveMQ message consumer class
 * Interface class between GFAST c code and activemqcpp library
 */
class ShakeAlertConsumer : public cms::ExceptionListener,
			   public cms::MessageListener,
			   public activemq::transport::DefaultTransportListener
{
private:
  // AMQ private variables
  cms::Connection *__connection;
  cms::Session *__session;
  cms::Destination *__destination;
  cms::MessageConsumer *__consumer;
  activemq::core::ActiveMQConnection *amqConnection;
  string __user;
  string __password;
  string __destURI;
  string __brokerURI;
  string __textMessage;
  long __messageCount;
  int __verbose;
  bool __useTopic; 
  bool __clientAck;
  bool __luseListener;
  bool __isInitialized;
  bool __lconnected;
  queue<string> __messageBuffer; //!< buffer containing unprocessed messages
  unsigned int __maxMessages;             //!< maximum number of messages in messageBuffer

public:
  /*!
    @brief Simple constructor
  */
  ShakeAlertConsumer();
    
  /*!
    @brief Simple destructor
  */
  ~ShakeAlertConsumer();
  
  /*!
   * @brief Initializes the internal variables for the ActiveMQ message
   *        listener.
   *
   * @param[in] username      Authenticating user name.
   * @param[in] password      Authenticating user password.
   * @param[in] destination   Queue or topic name on the broker.
   * @param[in] brokerURI     URI of message broker.
   * @param[in] useTopic      If true then the message receiver connects
   *                          to a topic (default).
   *                          If false then the message receiver connects
   *                          to a queue.
   * @param[in] clientAck     If true then the session will acknowledge
   *                          a message has been received.
   *                          If false then the session automatically 
   *                          acknowledges a client's receipt of a message
   *                          either when the session has successfully
   *                          returned from a call to receive or when the
   *                          session's message listener has successfully
   *                          processed the message (default).
   *                          If true then the session is transacted
   *                          and the acknowledgement of messages is 
   *                          handled internally.
   * @param[in] luseListener  If false then you must actively check your
   *                          messages (default).
   *                          If true then use a message listener - 
   *                          this is useful when the driving program
   *                          language is threaded and/or uses triggers.
   * @param[in] maxMessages   Maximum number of messages in local queue (default 5).
   * @param[in] verbose       Controls the verbosity - use 0 for reporting
   *                          of errors only. Default=0.
   *
   */
  void initialize(const string username,
		  const string password,
		  const string destination,
		  const string brokerURI,
		  bool useTopic,
		  bool clientAck,
		  bool luseListener,
		  int maxMessages,
		  int verbose);

  /*!
   * @brief Starts the message listener.  To use this you must first
   *        initialize the private variables with the initialize()
   *        function.  This is similar to the decision module's 
   *        DMMessageReceiver.
   */
  void startMessageListener();
  
  /*!
   * @brief Called in asynchronous configurations when new message arrives on listener.
   *        New messages are pushed onto messageBuffer queue.
   *
   * @param[in] message   ActiveMQ Message type which can be NULL.
   * 
   */
  virtual void onMessage(const cms::Message *message);

  /*!
   * @brief Convenience function to determine if the class was
   *        initialized.
   *
   * @result If true then the class was initialized.
   *
   * @author Ben Baker (ISTI)
   *
   */
  bool isInitialized();

  /*!
   * @brief Synchronous poll of ActiveQ topic.
   * Checks if message is on topic and returns after ms_wait milliseconds
   * with or without message.  Can currently handle only one message per call.
   *
   * @param[in] ms_wait  Number of milliseconds to wait for a message
   *                     prior to returning.  If 0 (default) 
   *                     then the function will not wait.
   *
   * @param[out] ierr    0 indicates success.
   *
   * @result number of messages received.
   */
  int pollAMQ(const int ms_wait, int *ierr);

  /* @brief pop oldest message off of messageBuffer stack.
   *
   * @result If NULL there is no message.  Otherwise, this is the NULL
   *         terminated char * message from the AMQ listener.
   *
   */
  char *getMessage();

  /*!
   * @brief In asynchronous implementations, method will be called on exception.
   *        This class is registered as an ExceptionListener with the connection.
   */ 
  virtual void onException(const cms::CMSException& ex AMQCPP_UNUSED);

 private:
  /*!
   * @brief Private function which frees and resets ActiveMQ variables
   *        prior to the final AMQ shutdownLibrary call.
   */
  void cleanup();
};

#endif
