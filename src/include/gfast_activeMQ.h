#ifndef __GFAST_ACTIVEMQ_H__
#define __GFAST_ACTIVEMQ_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Initialize activeMQ parameters */
int activeMQ_initializeConsumer(const char AMQuser[],
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
char *activeMQ_getMessage(const int ms_wait, int *ierr);
/* Get a message from activeMQ listener */
char *activeMQ_getMessageFromListener(int *ierr);
/* Shut down activeMQ */
void activeMQ_finalize(void);
/* Start the message listener */
void activeMQ_startMessageListener(void);
/* Convenience function to set the tcp URI request */
char *activeMQ_setTcpURIRequest(const char *host,
                                const int port,
                                const int msReconnect,
                                const int maxAttempts);


#define GFAST_activeMQ_initializeConsumer(...)       \
              activeMQ_initializeConsumer(__VA_ARGS__)
#define GFAST_activeMQ_getMessage(...)       \
              activeMQ_getMessage(__VA_ARGS__)
#define GFAST_activeMQ_finalize(...)       \
              activeMQ_finalize(__VA_ARGS__)
#define GFAST_activeMQ_startMessageListener(...)       \
              activeMQ_startMessageListener(__VA_ARGS__)
#define GFAST_activeMQ_setTcpURIRequest(...)       \
              activeMQ_setTcpURIRequest(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_ACTIVEMQ_H__ */
