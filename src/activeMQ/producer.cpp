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
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include "gfast_activeMQ.h"

using namespace decaf::lang;
using namespace activemq::core;
using namespace activemq::transport;
using namespace activemq::util;
using namespace cms;
using namespace std;

//============================================================================//
//                                 ShakeAlert Producer                        //
//============================================================================//
class ShakeAlertProducer
{
    private:
        // AMQ private variables
        Connection *__connection;
        Session *__session;
        Destination *__destination;
        MessageProducer *__producer;
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
        void initialize(const string username,
                        const string password,
                        const string destination,
                        const string brokerURI,
                        bool useTopic = true,
                        bool sessionTransacted = false,
                        //bool luseListener = false, 
                        int verbose = 0)
        {
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
        //====================================================================//
        /*!
         * @brief Convenience function to determine if the class was
         *        initialized.
         *
         * @result If true then the class was initialized.
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
         * @brief Cleans up the internal variables in the ActiveMQ message
         *        listener by calling the private cleanup function.
         */
        void destroy()
        {
            this->cleanup();
        }
        //====================================================================//
        /*!
         * @brief Starts the message sender.  To use this you must first
         *        initialize the private variables with the initialize()
         *        function.  This is similar to the decision module's 
         *        DMMessageSender.
         */
        void startMessageSender()
        {
            const char *fcnm = "ShakeAlertSender startMessageSender\0";
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
                //    amqConnection->addTransportListener(this);
                }
                if (__verbose > 2){printf("%s: Starting connection\n", fcnm);}
                //__connection->start();
                //__connection->setExceptionListener(this); 
                if (__connection == NULL)
                {
                    printf("%s: Failed to start connection!\n", fcnm);
                    return;
                }
                if (__verbose > 2){printf("%s: Connection set\n", fcnm);}
                // Create a session
                if (!__sessionTransacted)
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
                        printf("%s: Session will acknowledge transaction\n",
                               fcnm);
                    }
                    __session
                    = __connection->createSession(Session::SESSION_TRANSACTED);
                }
                if (__session == NULL)
                {
                    printf("%s: Error session not made\n", fcnm);
                    return;
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
                __producer = __session->createProducer(__destination);
                
                //if (__luseListener){__consumer->setMessageListener(this);}
                if (__verbose > 2)
                {
                    printf("%s: ActiveMQ producer running...\n", fcnm);
                }
                __lconnected = true;

            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            return;
        }
        //====================================================================//
        /*!
         * @brief Sends a text message.
         *
         * @param[in] message    Message to send.
         *
         * @result 0 indicates success.
         *
         */
        int sendMessage(const char *message)
        {
            const char *fcnm = "ShakeAlertSender sendMessage\0";
            if (!__isInitialized)
            {
                printf("%s: Producer not yet initialized\n", fcnm);
                return -1;
            }
            if (!__lconnected)
            {
                printf("%s: Producer not yet connected\n", fcnm);
                return -1;
            }
            if (message == NULL)
            {
                printf("%s: NULL message!\n", fcnm);
                return -1;
            }
            string msg = string(message);
            TextMessage *amqMessage = __session->createTextMessage(msg);
            __producer->send(amqMessage);
            delete amqMessage;
            return 0;
        }
        //====================================================================//
        /*!
         * @brief Sends a blob file message.
         *
         * @param[in] location   Location of blob message (e.g. /scratch/gfast).
         *
         * @result 0 indicates success.
         *
         */
/*
        int sendBlobFileMessage(const char *location)
        {
            string loc = string(location);
            //BlobMessage message = __session->createBlobMessage(new File(loc));
            //__producer->send(message);
            //delete message;
            return 0;
        }
*/
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
                printf("%s: Closing producer...\n", fcnm);
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
            // Free  producer
            try
            {
                if (__producer != NULL){delete __producer;}
            }
            catch (CMSException &e)
            {
                e.printStackTrace();
            }
            __producer = NULL;
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
            __sessionTransacted = false;
            //__luseListener = false;
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
static vector<class ShakeAlertProducer> producer(0);// = 0;
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
extern "C" int activeMQ_producer_initialize(const char AMQuser[],
                                            const char AMQpassword[],
                                            const char AMQdestination[],
                                            const char AMQhostname[],
                                            const int port,
                                            const bool useTopic,
                                            const bool clientAck,
                                            const int verbose)
{
    const char *fcnm = "activeMQ_producer_initialize\0";
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
    // Give the producer a unique name
    //string prodID = IdGenerator().generateId();
    // Set the URI
    brokerURI = activeMQ_producer_setTcpURI(AMQhostname, port);
    // Make sure the library is initialized
    if (!activeMQ_isInit()){activeMQ_start();}
    if (verbose > 0)
    {
        printf("%s: Initializing the producer...\n", fcnm);
    }
    size_t len = producer.size();
    producer.resize(len+1);
    producer[len].initialize(username, password, destination, brokerURI,
                             useTopic, clientAck, verbose);
    producer[len].startMessageSender();
    return static_cast<int>(len);
}
/*!
 * @brief C interface function to destroy the ActiveMQ producer.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" void activeMQ_producer_finalize(void)
{
    size_t len;
    len = producer.size();
    for (size_t i=0; i<len; i++)
    {
        producer[i].destroy();
    }
    producer.resize(0);
    if (activeMQ_isInit()){activeMQ_stop();}
    return;
}
//============================================================================//
/*!
 * @brief C interface function to send a message.
 *
 * @param[in] id        Message broker ID index.  This is a C indexed number
 *                      that starts at 0 and is less than the number of
 *                      producers.
 * @param[in] message   Message to send.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker, ISTI
 *
 */
extern "C" int activeMQ_producer_sendMessage(const int id, const char *message)
{
    const char *fcnm = "activeMQ_producer_sendMessage\0";
    int ierr;
    size_t idl;
    idl = static_cast<size_t>(id);
    if (id < 0 || idl >= producer.size())
    {
        printf("%s: Invalid session ID: %d\n", fcnm, id);
        return -1;
    }
    ierr = producer[idl].sendMessage(message);
    if (ierr != 0)
    {
        printf("%s: Error sending message\n", fcnm);
    }
    return ierr;
}
