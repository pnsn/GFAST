#ifndef _ShakeAlertProducer_h_
#define _ShakeAlertProducer_h_

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/util/Config.h>
#include <activemq/commands/ActiveMQBlobMessage.h>
#include <activemq/util/IdGenerator.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <decaf/lang/Runnable.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include "gfast_activeMQ.h"

using namespace std;

/*!
 * @brief GFAST ActiveMQ message producer class
 * Interface class between GFAST c code and activemqcpp library
 */
class ShakeAlertProducer : public cms::ExceptionListener
{
private:
  cms::Connection *__connection;
  cms::Session *__session;
  cms::Destination *__destination;
  cms::MessageProducer *__producer;
  string __user;
  string __password;
  string __destURI;
  string __brokerURI;
  string __textMessage;
  size_t __messageCount;
  int __verbose;
  bool __useTopic;
  bool __sessionTransacted;
  bool __isInitialized;
  bool __lconnected;
  bool __lhaveMessage;
  char __pad[7];

public:
  /*!
    @brief Simple constructor
  */
  ShakeAlertProducer();
    
  /*!
    @brief Simple destructor
  */
  ~ShakeAlertProducer();
  
  /*********************************************************************/
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
   * @param[in] sessionTransacted  Look this up.  Default=False
   * @param[in] verbose       Controls the verbosity - use 0 for reporting
   *                          of errors only.  Default=0.
   */
  void initialize(const string username,
		  const string password,
		  const string destination,
		  const string brokerURI,
		  bool useTopic,
		  bool sessionTransacted,
		  int verbose);

  /*!
   * @brief Convenience function to determine if the class was
   *        initialized.
   *
   * @result If true then the class was initialized.
   *
   * @author Ben Baker (ISTI)
   *
   */
  bool isInitialized(void);

  /*!
   * @brief Starts the message sender.  To use this you must first
   *        initialize the private variables with the initialize()
   *        function.  This is similar to the decision module's 
   *        DMMessageSender.
   */
  void startMessageSender();

  /*!
   * @brief Sends a text message.
   *
   * @param[in] message    Message to send.
   *
   * @result 0 indicates success.
   *
   */
  int sendMessage(const char *message);

  /*
  / *!
   * @brief Sends a bytes message.
   *
   * @param[in] cbytes pointer to bytes array.
   * @param[in] bsize length of cbytes
   *
   * @result 0 indicates success.
   *
   * /
  int sendBytesMessage(const unsigned char *cbytes, int bsize);
  */

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
