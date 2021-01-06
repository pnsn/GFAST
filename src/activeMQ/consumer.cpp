/*! @file consumer.cpp
 * @brief Wrapper functions to allow c programs to access ShaleAlertConsumer
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
#include "ShakeAlertConsumer.h"
 
//static bool linit_amqlib = false;

/*!
 * @brief C interface function to initialize the ActiveMQ shakeAlert
 *        decision module message listener.  This function must be called
 *        first.
 *
 * @param[in] AMQuser         Authenticating username.
 * @param[in] AMQpassword     Authenticating password.
 * @param[in] AMQdestination  Queue or topic name on the broker.
 * @param[in] AMQhostname     URL of host computer (e.g. computer.abc.def.edu).
 * @param[in] port            Port number which is accepting connections on
 *                            host computer.
 * @param[in] msReconnect     Number of milliseconds to wait for a reconnect
 *                            attempt.  If 0 or if maxAttempts is 0 then this
 *                            command will be ignored.
 * @param[in] maxAttempts     Number of attempts to connect before giving up.
 *                            If 0 this command will be ignored.
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
 * @param[in] luseListener    If false then you must actively check your
 *                            messages (default).
 *                            If true then use a message listener - 
 *                            this is useful when the driving program
 *                            language is threaded and/or uses triggers.
 * @param[in] maxMessages     Max number of messages in local message buffer
 * @param[in] verbose         controls verobosity.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" void *activeMQ_consumer_initialize(const char AMQuser[],
                                              const char AMQpassword[],
                                              const char AMQdestination[],
                                              const char AMQhostname[],
                                              const int port,
                                              const int msReconnect,
                                              const int maxAttempts,
                                              const bool useTopic,
                                              const bool clientAck,
                                              const bool luseListener,
					      const unsigned int maxMessages,
                                              const int verbose,
                                              int *ierr)
{
    const char *fcnm = "activeMQ_consumer_initialize\0";
    ShakeAlertConsumer *consumer = NULL;
    string brokerURI, destination, hostname, password, username;
    *ierr = 0;
    if (AMQuser != NULL)
    {
        username = string(AMQuser);
    }
    else
    {
        username = "";
    }
    if (AMQpassword != NULL)
    {
        password = string(AMQpassword);
    }
    else
    {
        password = "";
    }
    if (AMQhostname != NULL)
    {
        hostname = string(AMQhostname);
    }
    else
    {
        hostname = "";
    }
    if (AMQdestination != NULL)
    {
        destination = string(AMQdestination);
    }
    else
    {
        destination = "";
    }
    // Set the URI 
    char *brokerURIchar;
    brokerURIchar = activeMQ_setTcpURIRequest(AMQhostname, port,
                                                 msReconnect, maxAttempts);
    brokerURI = string(brokerURIchar);
    delete[] brokerURIchar;
    // Make sure the library is initialized
    if (!activeMQ_isInit())
    {
        activeMQ_start();
    }
    if (verbose > 0)
    {
        printf("%s: Initializing the consumer...\n", fcnm);
    }
    consumer = new ShakeAlertConsumer;
    consumer->initialize(username, password, destination, brokerURI,
                         useTopic, clientAck, luseListener, maxMessages,
			 verbose);
    consumer->startMessageListener();
    if (!consumer->isInitialized())
    {
        fprintf(stderr, "%s: Failed to initialize consumer\n", fcnm);
        *ierr = 1;
        delete consumer;
        consumer = NULL;
    }
    brokerURI = "";
    destination = "";
    hostname = "";
    password = "";
    username = "";
    return static_cast<void *> (consumer);
}
//============================================================================//
/*!
 * @brief C interface function to destroy the ActiveMQ listener.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" void activeMQ_consumer_finalize(void *consumerIn)
{
    ShakeAlertConsumer *consumer = NULL;
    consumer = static_cast<ShakeAlertConsumer *> (consumerIn);
    delete consumer; 
    return;
}
//============================================================================//
/*!
 * @brief Polls ActiveMQ for new messages and returns oldest message in local queue.
 *
 * @param[in] ms_wait  Number of milliseconds to wait for a message. If 0
 *                     then it will not wait.
 *
 * @param[out] ierr    0 indicates success (note a successful call can return
 *                     no message). \n
 *                     1 indicates an internal error has occurred.
 *
 * @result On successful exit either is NULL to indicate there are no messages
 *         or contains a null terminated char * shakeAlert decision module
 *         message.
 */
extern "C" char *activeMQ_consumer_getMessage(void *consumerIn,
                                              const int ms_wait, int *ierr)
{
    const char *fcnm = "activeMQ_consumer_getMessage\0";
    char *message = NULL;
    ShakeAlertConsumer *consumer = NULL;
    int nmsg;
    *ierr = 0;
    consumer = static_cast<ShakeAlertConsumer *> (consumerIn);
    if (!consumer->isInitialized())
    {
        *ierr = 1;
        fprintf(stderr, "%s: Error consumer never initialized\n", fcnm);
        consumer = NULL;
        return message;
    }
    nmsg = consumer->pollAMQ(ms_wait, ierr);
    if (nmsg>0) message = consumer->getMessage();
    if (*ierr != 0)
    {
        fprintf(stderr, "%s: Error getting message\n", fcnm);
    }
    consumer = NULL;
    return message;
}
//============================================================================//
/*!
 * @brief Sets the tcp URI from the host name, port number, max milliseconds
 *        for a reconnect, and max number of attempts to connect.
 *
 * @param[in] host         Host name (e.g. mycomputer.abc.def.edu).
 * @param[in] port         Port number.
 * @param[in] msReconnect  Number of milliseconds to wait for a reconnect
 *                         attempt.  If 0 or if maxAttempts is 0 then this
 *                         command will be ignored.
 * @param[in] maxAttempts  Number of attempts to connect before giving up
 *                         if 0 this command will be ignored.
 *
 * @result tcp URI request for ActiveMQ connection.
 *
 */
extern "C" char *activeMQ_setTcpURIRequest(const char *host,
                                           const int port,
                                           const int msReconnect,
                                           const int maxAttempts)
{
    char *uri = NULL;
    char cwork[4096], cbuff[64];
    memset(cwork,  0, sizeof(cwork));
    memset(cbuff,  0, sizeof(cbuff));
    strcpy(cwork, "failover:(tcp://\0");
    strcat(cwork, host);
    strcat(cwork, ":\0");
    sprintf(cbuff, "%d", port);
    strcat(cwork, cbuff);
    strcat(cwork, ")\0");
    // Max milliseconds for reconnect
    if (msReconnect > 0 && maxAttempts > 0)
    {
        memset(cbuff, 0, sizeof(cbuff));
        sprintf(cbuff, "%d", msReconnect);
        strcat(cwork, "?initialReconnectDelay=\0");
        strcat(cwork, cbuff);
    }
    // Max number of attempts
    if (maxAttempts > 0)
    {
        memset(cbuff, 0, sizeof(cbuff));
        sprintf(cbuff, "%d", maxAttempts);
        strcat(cwork, "?startupMaxReconnectAttempts=\0");
        strcat(cwork, cbuff);
    }
    uri = new char[strlen(cwork)+1]; 
    memset(uri, 0, strlen(cwork)+1);
    strcpy(uri, cwork);
    return uri;
}

