#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"

static FILE *errorFile = NULL;
static FILE *infoFile = NULL;
static FILE *debugFile = NULL;
static FILE *warnFile = NULL;

int core_log_createLogFile(const char *fileName)
{
    if (errorFile != NULL){fclose(errorFile);}
    errorFile = fopen(fileName, "w");
    return 0;
}

void core_log_closeLogFile(void)
{
    fclose(errorFile);
    errorFile = NULL;
    return;
}

/*!
 * @brief Writes an error message to the error log.
 *
 * @param[in] msg    Message to write to the error log file.
 *
 */
void core_log_logErrorMessage(const char *msg)
{
    if (errorFile == NULL)
    {
        fprintf(stderr, "%s\n", msg);
    }
    else
    {
        fprintf(errorFile, "%s\n", msg);
    } 
    return;
}

/*!
 * @brief Writes a warning message to the warning log.
 *
 * @param[in] msg    Message to write to the warning log file.
 *
 */
void core_log_logWarningMessage(const char *msg)
{
    if (msg == NULL){return;}
    if (warnFile == NULL)
    {   
        fprintf(stdout, "%s\n", msg);
    }   
    else
    {   
        fprintf(warnFile, "%s\n", msg);
    }   
    return;
}

/*!
 * @brief Writes an info message to the info log.
 *
 * @param[in] msg    Message to write to the info log file.
 *
 */
void core_log_logInfoMessage(const char *msg)
{
    if (msg == NULL){return;}
    if (infoFile == NULL)
    {
        fprintf(stdout, "%s\n", msg); 
    }
    else
    {
        fprintf(infoFile, "%s\n", msg);
    }
    return;
}

/*!
 * @brief Writes a debug/extra info message to the debug log.
 *
 * @param[in] msg    Message to write to the debug log file.
 *
 */
void core_log_logDebugMessage(const char *msg)
{
    if (msg == NULL){return;}
    if (debugFile == NULL)
    {
        fprintf(stdout, "%s\n", msg);
    }
    else
    {
        fprintf(debugFile, "%s\n", msg);
    }
    return;
}
