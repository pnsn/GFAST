#ifndef gfast_activemq_h
#define gfast_activemq_h 1

#ifdef __cplusplus
extern "C"
{
#endif

/* Helps C start and stop the ActiveMQ library */
void activeMQ_start(void);
void activeMQ_stop(void);
/* Private stuff - really not for general use */
void activeMQ_setInit(void);
void activeMQ_setUninit(void);
bool activeMQ_isInit(void);

/* Initialize and finalize activeMQ library */
void activeMQ_initialize(void);
bool activeMQ_isInitialized(void);
void activeMQ_finalize(void);

/* Initialize activeMQ parameters */
int activeMQ_consumer_initialize(const char AMQuser[],
                                 const char AMQpassword[],
                                 const char AMQdestination[],
                                 const char AMQhostname[],
                                 const int port,
                                 const int msReconnect,
                                 const int maxAttempts,
                                 const bool useTopic,
                                 const bool clientAck,
                                 const bool luseListener,
                                 const int verbose);
/* Get a message from activeMQ */
char *activeMQ_consumer_getMessage(const int ms_wait, int *ierr);
/* Get a message from activeMQ listener */
char *activeMQ_consumer_getMessageFromListener(int *ierr);
/* Shut down activeMQ consumer */
void activeMQ_consumer_finalize(void);
/* Shut down activeMQ producer */
void activeMQ_producer_finalize(void);
/* Send a text message */
int activeMQ_producer_sendMessage(const int id, const char *message);
/* Convenience function to set the tcp URI request */
char *activeMQ_setTcpURIRequest(const char *host,
                                const int port,
                                const int msReconnect,
                                const int maxAttempts);

/* Convenience function to set the tcp URI for the producer */
char *activeMQ_producer_setTcpURI(const char *host, const int port);
/* Initialize the ActiveMQ producer */
int activeMQ_producer_initialize(const char AMQuser[],
                                 const char AMQpassword[],
                                 const char AMQdestination[],
                                 const char AMQhostname[],
                                 const int port,
                                 const bool useTopic,
                                 const bool clientAck,
                                 const int verbose);

#ifndef __cplusplus
#define GFAST_activeMQ_consumer_initialize(...)       \
              activeMQ_consumer_initialize(__VA_ARGS__)
#define GFAST_activeMQ_consumer_getMessage(...)       \
              activeMQ_consumer_getMessage(__VA_ARGS__)
#define GFAST_activeMQ_consumer_finalize(...)       \
              activeMQ_consumer_finalize(__VA_ARGS__)
#define GFAST_activeMQ_producer_finalize(...)       \
              activeMQ_producer_finalize(__VA_ARGS__)
#define GFAST_activeMQ_producer_sendMessage(...)       \
              activeMQ_producer_sendMessage(__VA_ARGS__)
#define GFAST_activeMQ_setTcpURIRequest(...)       \
              activeMQ_setTcpURIRequest(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif
#endif /* _gfast_activemq_h__ */
