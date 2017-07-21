#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iniparser.h>
#include "gfast_activeMQ.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"

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
 * @param[in] profilename      Name of properties file.
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
    const char *fcnm = "activeMQ_readIni\0";
    const char *s;
    char var[256];
    int ierr;
    dictionary *ini;
    ierr = 1;
    memset(activeMQ_props, 0, sizeof(struct  GFAST_activeMQ_struct));
    if (!os_path_isfile(propfilename))
    {
        log_errorF("%s: Properties file: %s does not exist\n",
                   fcnm, propfilename);
        return ierr;
    }
    ini = iniparser_load(propfilename);
    // Read the properties
    setVarName(group, "host\0", var);
    s = iniparser_getstring(ini, var, NULL);
    if (s == NULL)
    {
        log_errorF("%s: Could not find ActiveMQ host!\n", fcnm);
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
        log_errorF("%s: Could not find ActiveMQ user!\n", fcnm);
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
        log_errorF("%s: Could not find password!\n", fcnm);
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
        log_errorF("%s: Could not find activeMQ originTopic\n", fcnm);
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
        log_errorF("%s: Could not find ActiveMQ destinationTopic!\n", fcnm);
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
        log_errorF("%s: Could not find activeMQ port\n", fcnm);
        goto ERROR;
    }
    setVarName(group, "msReconnect\0", var);
    activeMQ_props->msReconnect = iniparser_getint(ini, var, 500);
    if (activeMQ_props->msReconnect < 0)
    {
        log_warnF("%s: Overriding msReconnect to 500\n", fcnm);
        activeMQ_props->msReconnect = 500;
    }
    setVarName(group, "maxAttempts\0", var);
    activeMQ_props->maxAttempts = iniparser_getint(ini, var, 5);
    if (activeMQ_props->maxAttempts < 0)
    {
        log_warnF("%s: Overriding maxAttempts to 5\n", fcnm);
        activeMQ_props->maxAttempts = 5;
    }
    setVarName(group, "msWaitForMessage\0", var);
    activeMQ_props->msWaitForMessage  = iniparser_getint(ini, var, 1);
    if (activeMQ_props->msWaitForMessage < 0)
    {
        log_warnF("%s: ActiveMQ could hang indefinitely, overriding to 1\n",
                  fcnm);
        activeMQ_props->msWaitForMessage = 1;
    }
    ierr = 0;
    ERROR:;
    iniparser_freedict(ini);
    return ierr;
}
