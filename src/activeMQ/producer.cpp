/*! @file producer.cpp
 * @brief Wrapper functions to allow c programs to access ShaleAlertProducer
 * c++ class.
 */
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include "gfast_activeMQ.h"
#include "ShakeAlertProducer.h"

//using namespace decaf::lang;
//using namespace activemq::core;
//using namespace activemq::transport;
//using namespace activemq::util;
//using namespace cms;
//using namespace std;

//static ShakeAlertProducer producer;
//static bool linit_amqlib = false;

/*!
 * @brief Sets the tcp URI for the producer.
 *
 * @param[in] host       Host name (e.g. mycomputer.abc.def.edu).
 * @param[in] port       Port number.
 *
 * @result tcp URI request for ActiveMQ connection.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" char *activeMQ_producer_setTcpURI(const char *host, const int port)
{
  char *uri = NULL;
  uri = activeMQ_setTcpURIRequest(host, port, -1, -1);
  return uri;
}

/*!
 * @brief C interface function to initialize the ActiveMQ producer. 
 *        This function must be called prior to any other functions in this
 *        class.
 *
 * @param[in] AMQuser         Authenticating username.
 * @param[in] AMQpassword     Authenticating password.
 * @param[in] AMQdestination  Queue or topic name on the broker.
 * @param[in] AMQhostname     URL of host computer (e.g. computer.abc.def.edu)
 * @param[in] port            port number which is accepting connections on
 *                            host computer.
 * @param[in] useTopic        If true then the message receiver connects
 *                            to a topic (default).
 *                            If false then the message receiver connects
 *                            to a queue.
 * @param[in] clientAck       If true then the session will acknowledge
 *                            a message has been received.
 *                            If false then the session automatically 
 *                            acknowledges a client's receipt of a message
 *                            either when the session has successfully
 *                            returned from a call to receive or when the
 *                            session's message listener has successfully
 *                            processed the message (default).
 *                            If true then the session is transacted
 *                            and the acknowledgement of messages is 
 *                            handled internally.
 * @param[in] verbose         Controls verobosity.  0 is quiet.
 *
 * @result Session ID number.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" void *activeMQ_producer_initialize(const char AMQuser[],
                                              const char AMQpassword[],
                                              const char AMQdestination[],
                                              const char AMQhostname[],
                                              const int port,
                                              const bool useTopic,
                                              const bool clientAck,
                                              const int verbose,
                                              int *ierr)
{
  const char *fcnm = "activeMQ_producer_initialize\0";
  ShakeAlertProducer *producer = NULL;
  string brokerURI, destination, hostname, password, username;
  *ierr = 0;
  if (AMQuser != NULL)
    {
      username = AMQuser;
    }
  else
    {
      username = ""; 
    }
  if (AMQpassword != NULL)
    {
      password = AMQpassword;
    }
  else
    {
      password = "";
    }
  if (AMQhostname != NULL)
    {
      hostname = AMQhostname;
    }
  else
    {
      hostname = "";
    }
  if (AMQdestination != NULL)
    {
      destination = AMQdestination;
    }
  else
    {
      destination = "";
    }
  // Give the producer a unique name
  //string prodID = IdGenerator().generateId();
  // Set the URI
  char *brokerURIchar;
  brokerURIchar = activeMQ_producer_setTcpURI(AMQhostname, port);
  brokerURI = string(brokerURIchar);
  delete[] brokerURIchar;
  // Make sure the library is initialized
  if (!activeMQ_isInit()){activeMQ_start();}
  if (verbose > 0)
    {
      printf("%s: Initializing the producer...\n", fcnm);
    }
  producer = new ShakeAlertProducer;
  producer->initialize(username, password, destination, brokerURI,
		       useTopic, clientAck, verbose); 
  producer->startMessageSender();
  if (!producer->isInitialized())
    {
      printf("%s: Failed to initialize the producer\n", fcnm);
      *ierr = 1;
    }
  brokerURI = "";
  return static_cast<void *> (producer);
}

/*!
 * @brief C interface function to destroy the ActiveMQ producer.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" void activeMQ_producer_finalize(void *producerIn)
{
  ShakeAlertProducer *producer = NULL;
  producer = static_cast<ShakeAlertProducer *> (producerIn); 
  delete producer;
  return;
}

/*!
 * @brief C interface function to send a message.
 *
 * @param[in] producerIn   Handle to the ActiveMQ message broker.
 * @param[in] message      Message to send.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" int activeMQ_producer_sendMessage(void *producerIn,
                                             const char *message)
{
  const char *fcnm = "activeMQ_producer_sendMessage\0";
  ShakeAlertProducer *producer = NULL;
  int ierr;
  producer = static_cast<ShakeAlertProducer *> (producerIn); 
  ierr = producer->sendMessage(message);
  producer = NULL;
  if (ierr != 0)
    {
      printf("%s: Error sending message\n", fcnm);
    }
  return ierr;
}
