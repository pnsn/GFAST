#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_activeMQ.h"

/*! static variable to keep track of whether activeMQCPP library has been initialized */
static bool isinit = false;

/*! @brief Initialize activeMQCPP library */
void activeMQ_start(void)
{
    if (isinit)
    {
        printf("activeMQ_start: Library already initialized\n");
        return;
    }
    activeMQ_initialize();
    isinit = true;
}

/*! @brief Shut down activeMQCPP library */
void activeMQ_stop(void)
{
    if (!isinit)
    {
        printf("activeMQ_stop: Library already shut down\n");
        return;
    }
    activeMQ_finalize();
    isinit = false;
    return;
}

/*! @brief simple utility function to access activeMQCPP library state */
bool activeMQ_isInit(void)
{
    return isinit;
}
//============================================================================//
/*!
 * @brief Sets the tcp URI from the host name, port number, max milliseconds
 *        for a reconnect, and max number of attempts to connect.
 *
 * @param[in] AMQurl       url pointing to target host and port (e.g. tcp://localhost:61616).
 * @param[in] msReconnect  Number of milliseconds to wait for a reconnect
 *                         attempt.  If 0 or if maxAttempts is 0 then this
 *                         command will be ignored.
 * @param[in] maxAttempts  Number of attempts to connect before giving up
 *                         if 0 this command will be ignored.
 *
 * @result tcp URI request for ActiveMQ connection.
 *
 */
char *activeMQ_setTcpURIRequest(const char *AMQurl,
				const int msReconnect,
				const int maxAttempts)
{
    char *uri = NULL;
    char cwork[4096], cbuff[64];
    memset(cwork,  0, sizeof(cwork));
    memset(cbuff,  0, sizeof(cbuff));
    strcpy(cwork, "failover:(\0");
    strcat(cwork, AMQurl);
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
    uri = (char *)malloc((strlen(cwork)+1)*sizeof(char)); 
    memset(uri, 0, strlen(cwork)+1);
    strcpy(uri, cwork);
    return uri;
}
