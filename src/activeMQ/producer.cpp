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
#include "gfast_activeMQ.h"

using namespace activemq::core;
using namespace activemq::transport;
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
        // Parameters for AMQ initialization
        string __user;
        string __password;
        string __destURI;
        string __brokerURI;
        bool __useTopic;
        bool __sessionTransacted;
        //bool __luseListener;
        // Private variables used when listening for messages
        string __textMessage;
        int __verbose;
        long __messageCount;
        bool __isInitialized;
        bool __lconnected;
        bool __lhaveMessage;
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
            //__luseListener = luseListener;
            __verbose = verbose;
            __isInitialized = true;
            __lconnected = false;
            __lhaveMessage = false;
            __textMessage = ""; 
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
         * @brief Sends a text message
         *
         * @param[in] message    message to send
         *
         * @result 0 indicates success
         *
         */
        int sendMessage(const char *message)
        {
            const char *fcnm = "ShakeAlertSender sendMessage\0";
            if (message == NULL)
            {
                printf("%s: NULL message!\n", fcnm);
                return -1;
            }
            string msg = string(message);
            TextMessage *textMessage =  __session->createTextMessage(msg);
            __producer->send(textMessage);
            delete message;
            return 0;
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