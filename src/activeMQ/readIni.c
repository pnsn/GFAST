#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "gfast_activeMQ.h"
#include "fileutils.h"

static void setVarName(const char *group, const char *variable,
                       char *var)
{
    memset(var, 0, 256*sizeof(char));
    sprintf(var, "%s:%s", group, variable);
    return;
}
/*!
 * @brief Reads the ActiveMQ properties from the initialization file.
 *
 * @param[in] propfilename     Name of properties file.
 * @param[in] group            Group in ini file.  Likely "ActiveMQ".
 *
 * @param[out] activeMQ_props  ActiveMQ properties.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int activeMQ_readIni(const char *propfilename,
                     const char *group,
                     struct GFAST_activeMQ_struct *activeMQ_props)
{
    const char *s;
    char var[256];
    int ierr;
    dictionary *ini;
    ierr = 1;
    memset(activeMQ_props, 0, sizeof(struct  GFAST_activeMQ_struct));
    if (!cfileexists(propfilename))
    {
        LOG_ERRMSG("%s: Properties file: %s does not exist\n",
                   __func__, propfilename);
        return ierr;
    }
    ini = iniparser_load(propfilename);
    // Read the properties
    setVarName(group, "host\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        LOG_ERRMSG("%s: Could not find ActiveMQ host!\n", __func__);
        goto ERROR;
    }
    else
    {
        strcpy(activeMQ_props->host, s);
    }
    setVarName(group, "user\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        LOG_ERRMSG("%s: Could not find ActiveMQ user!\n", __func__);
        goto ERROR;
    }
    else
    {
        strcpy(activeMQ_props->user, s);
    }
    setVarName(group, "password\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        LOG_ERRMSG("%s: Could not find password!\n", __func__);
        goto ERROR;
    }
    else
    {
         strcpy(activeMQ_props->password, s);
    }
    setVarName(group, "originTopic\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        LOG_ERRMSG("%s: Could not find activeMQ originTopic\n", __func__);
        goto ERROR;
    }
    else
    {
        strcpy(activeMQ_props->originTopic, s);
    }
    setVarName(group, "destinationTopic\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        LOG_ERRMSG("%s: Could not find ActiveMQ destinationTopic!\n",
                   __func__);
        goto ERROR;
    }
    else
    {
        strcpy(activeMQ_props->destinationTopic, s);
    }
    setVarName(group, "port\0", var);
    activeMQ_props->port = iniparser_getint(ini, var, -12345);
    if (activeMQ_props->port ==-12345)
    {
        LOG_ERRMSG("%s: Could not find activeMQ port\n", __func__);
        goto ERROR;
    }
    setVarName(group, "msReconnect\0", var);
    activeMQ_props->msReconnect = iniparser_getint(ini, var, 500);
    if (activeMQ_props->msReconnect < 0)
    {
        LOG_WARNMSG("%s: Overriding msReconnect to 500\n", __func__);
        activeMQ_props->msReconnect = 500;
    }
    setVarName(group, "maxAttempts\0", var);
    activeMQ_props->maxAttempts = iniparser_getint(ini, var, 5);
    if (activeMQ_props->maxAttempts < 0)
    {
        LOG_WARNMSG("%s: Overriding maxAttempts to 5\n", __func__);
        activeMQ_props->maxAttempts = 5;
    }
    setVarName(group, "msWaitForMessage\0", var);
    activeMQ_props->msWaitForMessage  = iniparser_getint(ini, var, 1);
    if (activeMQ_props->msWaitForMessage < 0)
    {
        LOG_WARNMSG("%s: ActiveMQ could hang indefinitely, overriding to 1\n",
                    __func__);
        activeMQ_props->msWaitForMessage = 1;
    }
    setVarName(group, "maxMessages\0", var);
    activeMQ_props->maxMessages  = iniparser_getint(ini, var, 5);
    if (activeMQ_props->maxMessages <= 0)
    {
        LOG_WARNMSG("%s: ActiveMQ message buffer must be >0, overriding to 5\n",
                    __func__);
        activeMQ_props->maxMessages = 5;
    }
    ierr = 0;
    ERROR:;
    iniparser_freedict(ini);
    return ierr;
}
