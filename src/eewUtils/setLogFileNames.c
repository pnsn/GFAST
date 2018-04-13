#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_eewUtils.h"

void eewUtils_setLogFileNames(const char *eventid,
                              char errorLogFileName[PATH_MAX],
                              char infoLogFileName[PATH_MAX],
                              char debugLogFileName[PATH_MAX],
                              char warnLogFileName[PATH_MAX])
{
    // Set the log file names
    memset(errorLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(errorLogFileName, eventid);
    strcat(errorLogFileName, "_error.log\0");
    memset(infoLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(infoLogFileName, eventid);
    strcat(infoLogFileName, "_info.log\0");
    memset(debugLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(debugLogFileName, eventid);
    strcat(debugLogFileName, "_debug.log\0");
    memset(warnLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(warnLogFileName, eventid);
    strcat(warnLogFileName, "_debug.log\0");
    return;
}
