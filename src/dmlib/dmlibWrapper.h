#ifndef _dmlibWrapper_h
#define _dmlibWrapper_h

/*!
 * @file dmlibWrapper.h
 * @brief variables and functions to expose dmlib functionality to GFAST c code
 */

#ifdef __cplusplus
#include "DMLib.h"
#include "AlgMessage.h"
#include "CoreEventInfo.h"
#include "HBProducer.h"

/*static variables local to this file*/
static cms::Connection *amqconnection=NULL;
static HBProducer *hbproducer=NULL;
static char *hbSender;
static int conVerbose=0;
static int hbVerbose=0;

extern "C"
{
#endif /*ifdef __cplusplus*/
  
  /*!
    @brief Start connection to activemq instance
    @param[in] AMQuser         Authenticating username.
    @param[in] AMQpassword     Authenticating password.
    @param[in] AMQhostname     URL of host computer (e.g. computer.abc.def.edu).
    @param[in] port            Port number which is accepting connections on
                               host computer.
    @param[in] msReconnect     Number of milliseconds to wait for a reconnect
                               attempt.  If 0 or if maxAttempts is 0 then this
                               command will be ignored.
    @param[in] maxAttempts     Number of attempts to connect before giving up.
                               If 0 this command will be ignored.
    @param[in] verbose         logging verbosity. default=1
    Points new connection to static amqconnection pointer or sets pointer to NULL
    if unsuccessful.
    @return 1 if success, 0 if already connected and -1 for error
  */
  int startAMQconnection((const char AMQuser[],
				  const char AMQpassword[],
				  const char AMQhostname[],
				  const int port,
				  const int msReconnect,
				  const int maxAttempts,
				  const int verbose=1);
  /*!
    @brief stops active activemq connection
    Stops and destroys connection and sets amqconnection pointer to NULL
    @return 1 if success, 0 if not connected and -1 for error
  */
  int stopAMQconnection();
  /*!
    @brief tests if connection is started
    @return true if connected, false if not
  */
  boolean isAMQconnected();
  /*!
    @brief start heartbeat producer
    Points new HBProducer to static hbproducer pointer or sets pointer to NULL
    if unsuccessful.
    @param[in] sender      identifier string for sending instance
    @param[in] topic       target topic for heartbeats
    @param[in] interval    heartbeat interval in seconds (0=manual heartbeats)
    @param[in] verbose     logging verbosity. default=1
    @return 1 if success, 0 if already initialized and -1 for error
  */
  int startHBProducer(const char *sender, const char *topic, int interval, int verbose);
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
    
#ifdef __cplusplus
}
#endif

#endif /*_dmlibWrapper_h*/
