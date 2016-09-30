#ifndef _gfast_activemq_h__
#define _gfast_activemq_h__ 1

#ifdef __cplusplus
extern "C"
{
#endif

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
int activeMQ_producer_sendMessage(const char *message);
/* Convenience function to set the tcp URI request */
char *activeMQ_setTcpURIRequest(const char *host,
                                const int port,
                                const int msReconnect,
                                const int maxAttempts);

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
