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
#endif
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
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include "gfast_activeMQ.h"
 
using namespace activemq::core;
using namespace activemq::transport;
using namespace cms;
using namespace std;

class ShakeAlertConsumer : public ExceptionListener,
                           public MessageListener,
                           public DefaultTransportListener
{
    private:
        // AMQ private variables
        Connection *__connection;
        Session *__session;
        Destination *__destination;
        MessageConsumer *__consumer;
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
        bool __lhaveMessage;
        char __pad[6];
    //------------------------------------------------------------------------//
    //                              Public Functions                          //
    //------------------------------------------------------------------------//
    public:
        /*!
         * @brief Initializes the internal variables for the ActiveMQ message
         *        listener.
         *
         * @param[in] username      authenticating user name
         * @param[in] password      authenticating user password
         * @param[in] destination   queue or topic name on the broker
         * @param[in] brokerURI     URI of message broker
         * @param[in] useTopic      if true then the message receiver connects
         *                          to a topic (default).
         *                          if false then the message receiver connects
         *                          to a queue.
         * @param[in] clientAck     if true then the session will acknowledge
         *                          a message has been received.
         *                          if false then the session automatically 
         *                          acknowledges a client's receipt of a message
         *                          either when the session has successfully
         *                          returned from a call to receive or when the
         *                          session's message listener has successfully
         *                          processed the message (default).
         *                          if true then the session is transacted
         *                          and the acknowledgement of messages is 
         *                          handled internally.
         * @param[in] luseListener  if false then you must actively check your
         *                          messages (default).
         *                          if true then use a message listener - 
         *                          this is useful when the driving program
         *                          language is threaded and/or uses triggers.
         *                         
         * @param[in] verbose       controls the verbosity - use 0 for reporting
         *                          of errors only
         *
         */
        void initialize(const string username,
                        const string password,
                        const string destination,
                        const string brokerURI,
                        bool useTopic = true,
                        bool clientAck = false,
                        bool luseListener = false, 
                        int verbose = 0)
        {
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
            __verbose = verbose;
            __isInitialized = true;
            __lconnected = false;
            __lhaveMessage = false;
            __textMessage = "";
            memset(__pad, 0, sizeof(__pad));
            return;
        }
        //====================================================================//
        /*!
         * @brief Cleans up the internal variables in the ActiveMQ message
         *        listener by calling the private cleanup function.
         */
        void destroy()
        {
            this->cleanup();
        }
        //====================================================================//
        /*!
         * @brief Starts the message listener.  To use this you must first
         *        initialize the private variables with the initialize()
         *        function.  This is similar to the decision module's 
         *        DMMessageReceiver.
         */
        void startMessageListener()
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
                ActiveMQConnectionFactory *connectionFactory;
                connectionFactory = new ActiveMQConnectionFactory(__brokerURI);
                if (__verbose > 2)
                {
                    printf("%s: Setting username (%s) and password (%s)\n",
                           fcnm, __user.c_str(), __password.c_str());
                }
                if (!__user.empty())
                {
                    connectionFactory->setUsername(__user.c_str());
                }
                if (!__password.empty())
                {
                    connectionFactory->setPassword(__password.c_str());
                }
                // Create a connection
                __connection = connectionFactory->createConnection();
                delete connectionFactory;

                ActiveMQConnection *amqConnection;
                amqConnection
                     = dynamic_cast<ActiveMQConnection *>(__connection);
                if (amqConnection != NULL)
                {
                    amqConnection->addTransportListener(this);
                }
                if (__verbose > 2){printf("%s: Starting connection\n", fcnm);}
                __connection->start();
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
                    = __connection->createSession(Session::AUTO_ACKNOWLEDGE);
                }
                else
                { 
                    if (__verbose > 2)
                    {
                        printf("%s: Client will acknowledge transaction\n",
                               fcnm);
                    }
                    __session
                    = __connection->createSession(Session::SESSION_TRANSACTED);
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
                if (__luseListener){__consumer->setMessageListener(this);}
                if (__verbose > 2)
                {
                    printf("%s: ActiveMQ listener running...\n", fcnm);
                }
                __lconnected = true;
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            return; 
        } // End startMessageListener
        //====================================================================//
        /*!
         * @brief Function for AMQ so that it knows what to do when receiving
         *        a message.  If a message exists then it will be copied into
         *        __textMessage and __lhaveMessage will indicate that it is
         *        available to be copied. 
         *
         * @param[in] message   ActiveMQ Message type which can be NULL
         * 
         */
        virtual void onMessage(const Message *message)
        {
            const char *fcnm = "ShakeAlertConsumer onMessage\0";
            if (!message){return;}
            try
            {
                const TextMessage *textMessage;
                textMessage = dynamic_cast<const TextMessage *>(message);
                if (textMessage != NULL)
                {
                    __textMessage = textMessage->getText();
                    __lhaveMessage = true;
                    if (__verbose > 2)
                    {
                        printf("%s: Message received:\n%s\n",
                               fcnm, __textMessage.c_str());
                    }
                }
                if (__clientAck)
                {
                    message->acknowledge();
                }
                delete textMessage;
                __messageCount = __messageCount + 1;
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            return;
        }
        //====================================================================//
        /*!
         * @brief Convenience function to determine if the class was
         *        initialized
         *
         * @result if true then the class was initialized
         *
         * @author Ben Baker (ISTI)
         *
         */
        bool isInitialized(void)
        {
            return __isInitialized;
        }
        //====================================================================//
        /*!
         * @brief Checks if a message exists, and if yes, returns the message
         *
         * @param[out] ierr    0 indicates success
         *
         * @result If NULL there is no message.  Otherwise, this is the NULL
         *         terminated char * message from the AMQ listener.
         *
         */
        char *getMessageFromListener(int *ierr)
        {
            const char *fcnm = "ShakeAlertConsumer getMessageFromListener\0";
            char *message = NULL;
            size_t lenos;
            *ierr = 0;
            if (!__lconnected)
            {
                printf("%s: The listener isn't connected!\n", fcnm);
                *ierr = 1;
                return message;
            }
            // If I have a message then copy and return it
            if (__lhaveMessage)
            {
                //message = (char *)calloc(strlen(__textMessage.c_str() + 1), 
                //                         sizeof(char));
                lenos = strlen(__textMessage.c_str());
                message =  new char[lenos+1];
                memset(message, 0, lenos+1);
                strcpy(message, __textMessage.c_str());
                __lhaveMessage = false;
                __textMessage = ""; 
            }
            return message;
        }
        //====================================================================//
        /*!
         * @brief Checks if a message exists, and if yes, returns the message
         *
         * @param[in] ms_wait  number of milliseconds to wait for a message
         *                     prior to returning.  if 0 then the function will
         *                     not wait.
         *
         * @param[out] ierr    0 indicates success
         *
         * @result If NULL there is no message.  Otherwise, this is the NULL
         *         terminated char * message from the AMQ listener.
         *
         */
        char *getMessage(const int ms_wait, int *ierr)
        {
            const char *fcnm = "ShakeAlertConsumer getMessage\0";
            const Message *message;
            const TextMessage *textMessage;
            string text;
            char *charMessage = NULL;
            size_t lenos;
            *ierr = 0;
            if (!__lconnected)
            {
                printf("%s: The listener isn't connected!\n", fcnm);
                *ierr = 1;
                return charMessage;
            }
            // Check my mail
            if (ms_wait > 0)
            {
                message = __consumer->receive(ms_wait);
            }
            else
            {
                message = __consumer->receive();
            }
            textMessage = dynamic_cast<const TextMessage *>(message); 
            // If I have a message then copy and return it
            if (textMessage != NULL)
            {
                text = textMessage->getText(); 
                //charMessage = (char *)
                //              calloc(strlen(text.c_str()+1), sizeof(char));
                lenos = strlen(text.c_str());
                charMessage =  new char[lenos+1];
                memset(charMessage, 0, lenos+1);
                strcpy(charMessage, text.c_str());
                text = "";
            }
            delete message;
            return charMessage;
        }
        //====================================================================//
        /*!
         * @brief If something bad happens you see it here as this class 
         *        is also registered as an ExceptionListener (see above
         *        public ExceptionListener) with the connection
         */ 
        virtual void onException(const CMSException& ex AMQCPP_UNUSED)
        {
            printf("CMS Exception occurred.  Shutting down client.\n");
            ex.printStackTrace();
            exit(1); // This looks dangerous
        }
    //---------------------------End public functions-------------------------//

    //------------------------------------------------------------------------//
    //                              Private function                          //
    //------------------------------------------------------------------------//
    private:
        /*!
         * @brief Private function which frees and resets ActiveMQ variables
         *        prior to the final AMQ shutdownLibrary call.
         */
        void cleanup()
        {
            const char *fcnm = "cleanup\0";
            if (__verbose > 0)
            {
                printf("%s: Closing consumer...\n", fcnm);
            }
            if (!__isInitialized)
            {
                printf("%s: Program was never initialized\n", fcnm);
            }
            // Free destination 
            try
            {
                if (__destination != NULL){delete __destination;}
            }
            catch (CMSException& e)
            {
                e.printStackTrace();
            }
            __destination = NULL;
            // Free consumer
            try
            {
                if (__consumer != NULL){delete __consumer;}
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            __consumer = NULL;
            // Close the session
            try
            {
                if (__session != NULL){__session->close();}
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            __session = NULL;
            // Close the connection
            try
            {
                if (__connection != NULL){__connection->close();}
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
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
            __clientAck = false;
            __luseListener = false;
            __verbose = 0;
            __isInitialized = false;
            __lconnected = false;
            __lhaveMessage = false;
            __textMessage = "";
            return;
        }
    //--------------------------End private functions-------------------------//
};

//----------------------------------------------------------------------------//
//                 End the listener class. Begin the C interface              //
//----------------------------------------------------------------------------//
static ShakeAlertConsumer consumer;
//static bool linit_amqlib = false;
/*!
 * @brief C interface function to initialize the ActiveMQ shakeAlert
 *        decision module message listener.  This function must be called
 *        first.
 * @param[in] AMQuser         authenticating username
 * @param[in] AMQpassword     authenticating password 
 * @param[in] AMQdestination  queue or topic name on the broker
 * @param[in] AMQhostname     URL of host computer (e.g. computer.abc.def.edu)
 * @param[in] port            port number which is accepting connections on
 *                            host computer
 * @param[in] msReconnect     number of milliseconds to wait for a reconnect
 *                            attempt.  if 0 or if maxAttempts is 0 then this
 *                            command will be ignored
 * @param[in] maxAttempts     number of attempts to connect before giving up
 *                            if 0 this command will be ignored.
 * @param[in] useTopic        if true then the message receiver connects
 *                            to a topic (default).
 *                            if false then the message receiver connects
 *                            to a queue.
 * @param[in] clientAck       if true then the session will acknowledge
 *                            a message has been received.
 *                            if false then the session automatically 
 *                            acknowledges a client's receipt of a message
 *                            either when the session has successfully
 *                            returned from a call to receive or when the
 *                            session's message listener has successfully
 *                            processed the message (default).
 *                            if true then the session is transacted
 *                            and the acknowledgement of messages is 
 *                            handled internally.
 * @param[in] luseListener    if false then you must actively check your
 *                            messages (default).
 *                            if true then use a message listener - 
 *                            this is useful when the driving program
 *                            language is threaded and/or uses triggers.
 * @param[in] verbose         controls verobosity
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" int activeMQ_consumer_initialize(const char AMQuser[],
                                            const char AMQpassword[],
                                            const char AMQdestination[],
                                            const char AMQhostname[],
                                            const int port,
                                            const int msReconnect,
                                            const int maxAttempts,
                                            const bool useTopic,
                                            const bool clientAck,
                                            const bool luseListener,
                                            const int verbose)
{
    const char *fcnm = "activeMQ_consumer_initialize\0";
    string brokerURI, destination, hostname, password, username;
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
    // Set the URI 
    brokerURI = activeMQ_setTcpURIRequest(AMQhostname, port,
                                          msReconnect, maxAttempts);
    // Make sure the library is initialized
    if (!activeMQ_isInit())
    {
        activeMQ_start();
    }
    ////if (!linit_amqlib)
    //if (!consumer.isInitialized())
    //{
    //    if (verbose > 0)
    //    {
    //        printf("%s: Initializing ActiveMQ library...\n", fcnm);
    //    }
    //    //activemq::library::ActiveMQCPP::initializeLibrary();
    //    //linit_amqlib = true;
    //}
    if (verbose > 0)
    {
        printf("%s: Initializing the consumer...\n", fcnm);
    }
    consumer.initialize(username, password, destination, brokerURI,
                        useTopic, clientAck, luseListener, verbose);
    consumer.startMessageListener();
    return 0;
}
//============================================================================//
/*!
 * @brief C interface function to destroy the ActiveMQ listener
 */
extern "C" void activeMQ_consumer_finalize(void)
{
    //ShakeAlertConsumerClass consumer;
    consumer.destroy();
    if (activeMQ_isInit()){activeMQ_stop();}
    //activemq::library::ActiveMQCPP::shutdownLibrary();
    //linit_amqlib = false;
    return;
}
//============================================================================//
/*!
 * @brief Checks for the presence of a message from ActiveMQ and, if a message
 *        was received, returns it.
 *
 * @param[in] ms_wait  number of milliseconds to wait for a message. if 0
 *                     then it will not wait.
 *
 * @param[out] ierr    0 indicates success (note a successful call can return
 *                     no message)
 *                     1 indicates an internal error has occurred.
 *
 * @result on successful exit either is NULL to indicate there are no messages
 *         or contains a null terminated char * shakeAlert decision module
 *         message
 */
extern "C" char *activeMQ_consumer_getMessage(const int ms_wait, int *ierr)
{
    const char *fcnm = "activeMQ_consumer_getMessage\0";
    char *message = NULL;
    *ierr = 0;
    if (!consumer.isInitialized())
    {
        *ierr = 1;
        printf("%s: Error consumer never initialized\n", fcnm);
        return message;
    }
    message = consumer.getMessage(ms_wait, ierr);
    if (*ierr != 0)
    {
        printf("%s: Error getting message\n", fcnm);
    }
    return message;
}
//============================================================================//
/*!
 * @brief Checks for the presence of a message from ActiveMQ and, if a message
 *        was received, returns it.
 *
 * @param[out] ierr    0 indicates success (note a successful call can return
 *                     no message)
 *                     1 indicates an internal error has occurred.
 *
 * @result on successful exit either is NULL to indicate there are no messages
 *         or contains a null terminated char * shakeAlert decision module
 *         message
 */
extern "C" char *activeMQ_consumer_getMessageFromListener(int *ierr)
{
    const char *fcnm = "activeMQ_consumer_getMessageFromListener\0";
    char *message = NULL;
    *ierr = 0;
    if (!consumer.isInitialized())
    {
        *ierr = 1;
        printf("%s: Error consumer never initialized\n", fcnm);
        return message;
    }
    message = consumer.getMessageFromListener(ierr);
    if (*ierr != 0)
    {   
        printf("%s: Error getting message\n", fcnm);
    }
    return message;
}
//============================================================================//
/*!
 * @brief Sets the tcp URI from the host name, port number, max milliseconds
 *        for a reconnect, and max number of attempts to connect
 *
 * @param[in] host         host name (e.g. mycomputer.abc.def.edu)
 * @param[in] port         port number
 * @param[in] msReconnect  number of milliseconds to wait for a reconnect
 *                         attempt.  if 0 or if maxAttempts is 0 then this
 *                         command will be ignored
 * @param[in] maxAttempts  number of attempts to connect before giving up
 *                         if 0 this command will be ignored.
 *
 * @result tcp URI request for ActiveMQ connection
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
    uri = new char[strlen(cwork)+1]; //(char *)calloc(strlen(cwork)+1, sizeof(cwork));
    memset(uri, 0, strlen(cwork)+1);
    strcpy(uri, cwork);
    return uri;
}

