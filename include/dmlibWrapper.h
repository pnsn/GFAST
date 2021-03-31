#ifndef _dmlibWrapper_h
#define _dmlibWrapper_h

#include "gfast_struct.h"

/*!
 * @file dmlibWrapper.h
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#ifdef __cplusplus
extern "C" {
#endif /*ifdef __cplusplus*/
  
  /*!
    @brief Start connection to activemq instance
    @param[in] AMQuser         Authenticating username.
    @param[in] AMQpassword     Authenticating password.
    @param[in] destinationURL  URL of host computer (e.g. tcp://localhost:61616).
    @param[in] msReconnect     Number of milliseconds to wait for a reconnect
                               attempt.  If 0 or if maxAttempts is 0 then this
                               command will be ignored.
    @param[in] maxAttempts     Number of attempts to connect before giving up.
                               If 0 this command will be ignored.
    @param[in] verbose         logging verbosity. default=1
    @return 1 if success, 0 if already connected and -1 for error
    Points new connection to static amqconnection pointer or sets pointer to NULL
    if unsuccessful.
  */
  int startDestinationConnection(const char AMQuser[],
				 const char AMQpassword[],
				 const char destinationURL[],
				 const int msReconnect,
				 const int maxAttempts,
				 const int verbose);
  /*!
    @brief stops active activemq connection
    Stops and destroys connection and sets amqconnection pointer to NULL
    @return 1 if success, 0 if not connected and -1 for error
  */
  int stopDestinationConnection();
  /*!
    @brief tests if connection is started
    @return true if connected, false if not
  */
  bool isDestinationConnected();
  /*!
    @brief instantiates and starts new DMMessageSender for events.
    @return 1 if success, 0 if already connected and -1 for error
  */
  int startEventSender(const char topic[]);
  /*!
    @brief stops active DMMessageSender for events.
    Stops and destroys DMMessageSender object.
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int stopEventSender();
  
  /*!
    @brief sends static event message
    Message must first be created with createDMEventObject
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int sendEventMessage();
  
  /*!
    @brief sends event message as preformatted xml
    @param xmlstr preformatted xml message to send to ShakeAlert
    @return 1 if success, 0 if not initialized and -1 for error (todo. currently dummy)
  */
  int sendEventXML(const char xmlstr[]);

  /*!
    @brief start heartbeat producer
    Points new HBProducer to static hbproducer pointer or sets pointer to NULL
    if unsuccessful.
    @param[in] sender      identifier string for sending instance
    @param[in] hbtopic     target topic for heartbeats
    @param[in] interval    heartbeat interval in seconds (0=manual heartbeats)
    @param[in] verbose     logging verbosity. default=1
    @return 1 if success, 0 if already initialized and -1 for error
  */
  int startHBProducer(const char sender[], const char hbtopic[], int interval, int verbose);
  /*!
    @brief stops active HBProducer
    Stops and destroys HBProducer object and sets amqconnection pointer to NULL
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int stopHBProducer();
  /*!
    @brief sends heartbeat message
    @return 1 if success, 0 if not initialized and -1 for error
  */
  int sendHeartbeat();

  /*!
    @brief create dmlib CoreEventInfo object and store in static variable
    @return 1 for success, 0 if already exists or -1 for error
  */
  int createDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime);

  /*!
    @brief modify existing dmlib CoreEventInfo object
    @return 1 for success, 0 if no object exists or -1 for error
  */
  int modifyDMEventObject(const char evid[], double mag, double lat, double lon, double depth, double otime);
  /*!
    @brief delete stored dmlib CoreEventInfo object
    @return 1 for success, 0 no object exists or -1 for error
  */
  int deleteDMEventObject(const char evid[]);

  
#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#endif /*_dmlibWrapper_h*/
