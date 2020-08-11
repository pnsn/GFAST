#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"
#include "iscl/memory/memory.h"

enum logFileType_enum
{
    ERROR_FILE = 1,
    INFO_FILE = 2,
    WARNING_FILE = 3,
    DEBUG_FILE = 4,
    // MTH:
    LOG_FILE = 5
};

static FILE *errorFile = NULL;
static FILE *infoFile = NULL;
static FILE *debugFile = NULL;
static FILE *warningFile = NULL;
// MTH:
static FILE *logFile = NULL;

/*!
 * @brief Closes the log file.
 *
 * @param[in] fileType    Type of file (error, log, debug, info) to open.
 *
 * @result 0 indicates success.
 *
 */
static int core_log_closeLogFile(const enum logFileType_enum fileType)
{
    int ierr = 0;
    if (fileType == ERROR_FILE)
    {   
        if (errorFile != NULL){
          printf("** MTH: close the errorFile");
          LOG_MSG("%s", "** MTH: close the errorFile");
          fclose(errorFile);}
        errorFile = NULL;
    }   
    else if (fileType == INFO_FILE)
    {   
        if (infoFile != NULL){fclose(infoFile);}
        infoFile = NULL;
    }   
    else if (fileType == WARNING_FILE)
    {   
        if (warningFile != NULL){fclose(warningFile);}
        warningFile = NULL;
    }   
    else if (fileType == DEBUG_FILE)
    {   
        if (debugFile != NULL){fclose(debugFile);}
        debugFile = NULL;
    }   
    else if (fileType == LOG_FILE)
    {   
      printf("%s\n", "** fileType==LOG_FILE --> close(logFile)");
      LOG_MSG("%s", "** fileType==LOG_FILE --> close(logFile)");
        if (logFile != NULL){fclose(logFile);}
        logFile = NULL;
    }   
    else
    {   
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Invalid option\n", 
                __FILE__, __func__, __LINE__);
        ierr = 1;
    }
    return ierr;
}
/*!
 * @brief Creates a log file.  If the file exists then it will be over-written.
 *
 * @param[in] fileName    Name of log file.
 * @param[in] fileType    Type of log file (error, debug, info, warning) to open.
 *
 * @result 0 indicates success.
 *
 */
static int core_log_createLogFile(const char *fileName,
                                  const enum logFileType_enum fileType)
{
    char *dirName;
    enum isclError_enum isclError;
    int ierr = 0;
    // Does this file name make sense?
printf("MTH: Inside createLogFile fileName=%s fileType=%d\n", fileName, fileType);
    if (fileName == NULL)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error fileName is NULL\n",
                __FILE__, __func__, __LINE__);
        return -1;
    }
    if (strlen(fileName) < 1)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error fileName is blank\n",
                __FILE__, __func__, __LINE__);
        return -1;
    } 
    // Make sure I don't destroy an existing file handle
    ierr = core_log_closeLogFile(fileType);
    if (ierr != 0)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error closing log file %s\n",
                 __FILE__, __func__, __LINE__, fileName);
        return ierr;
    }
    // Check the output
    dirName = os_dirname(fileName, &isclError); 
    if (!os_path_isdir(dirName))
    {
        isclError = os_makedirs(dirName);
        if (isclError != ISCL_SUCCESS)
        {
            fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Failed to make %s\n",
                    __FILE__, __func__, __LINE__, dirName);
           ierr = 1;
        }
        memory_free8c(&dirName);
    }
//MTH: maybe this is where the free should be ?!
    memory_free8c(&dirName);

    if (ierr != 0){return ierr;}
    // Just point out this file is going to be over-written 
    if (os_path_isfile(fileName))
    {
        fprintf(stdout, "[WARNING]: (%s:%s:line=%d) Overwriting file %s\n",
                 __FILE__, __func__, __LINE__, fileName);
    } 
    // Open the desired file
    if (fileType == ERROR_FILE)
    {
        errorFile = fopen(fileName, "w");
    }
    else if (fileType == INFO_FILE)
    {
        infoFile = fopen(fileName, "w");
    }
    else if (fileType == WARNING_FILE)
    {
        warningFile = fopen(fileName, "w");
    }
    else if (fileType == DEBUG_FILE)
    {
        debugFile = fopen(fileName, "w");
    } 
    else if (fileType == LOG_FILE)
    {
        logFile = fopen(fileName, "w");
    } 
    else
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Invalid option\n", 
                __FILE__, __func__, __LINE__);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Opens a log file for writing.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName    Name of log file.
 * @param[in] fileType    Type of log file (error, debug, info, warning) to open.
 *
 * @result 0 indicates success.
 *
 */
static int core_log_openLogFile(const char *fileName,
                                const enum logFileType_enum fileType)
{
    char *dirName;
    enum isclError_enum isclError;
    int ierr = 0;
    // Does this file name make sense?
printf("MTH: Inside openLogFile fileName=%s fileType=%d\n", fileName, fileType);
    if (fileName == NULL)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error fileName is NULL\n",
                __FILE__, __func__, __LINE__);
        return -1;
    }
    if (strlen(fileName) < 1)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error fileName is blank\n",
                __FILE__, __func__, __LINE__);
        return -1;
    }
    // Make sure I don't destroy an existing file handle
    ierr = core_log_closeLogFile(fileType);
    if (ierr != 0)
    {
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Error closing log file %s\n",
                 __FILE__, __func__, __LINE__, fileName);
        return ierr;
    }
    // Does the file exist?  If not then give it an output directory
    if (!os_path_isfile(fileName))
    {
        dirName = os_dirname(fileName, &isclError);
        if (!os_path_isdir(dirName))
        {
            isclError = os_makedirs(dirName);
            if (isclError != ISCL_SUCCESS)
            {
                fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Failed to make %s\n",
                        __FILE__, __func__, __LINE__, dirName);
                ierr = 1;
            }
            memory_free8c(&dirName);
        }
    }
    if (ierr != 0){return ierr;}
    // Open the desired file
    if (fileType == ERROR_FILE)
    {   
        errorFile = fopen(fileName, "a");
    }   
    else if (fileType == INFO_FILE)
    {   
        infoFile = fopen(fileName, "a");
    }   
    else if (fileType == WARNING_FILE)
    {   
        warningFile = fopen(fileName, "a");
    }   
    else if (fileType == DEBUG_FILE)
    {   
        debugFile = fopen(fileName, "a");
    }   
    else if (fileType == LOG_FILE)
    {   
        logFile = fopen(fileName, "a");
    }   
    else
    {   
        fprintf(stderr, "[ERROR]: (%s:%s:line=%d) Invalid option\n", 
                __FILE__, __func__, __LINE__);
        ierr = 1;
    }
    return ierr;
}
//============================================================================//
/*!
 * @brief Convenience function to close logs.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeLogs(void)
{
    int ierr = 0;
    ierr += core_log_closeErrorLog();
    ierr += core_log_closeInfoLog();
    ierr += core_log_closeDebugLog();
    ierr += core_log_closeWarningLog();
    return ierr;
}
//============================================================================//
/*!
 * @brief Opens the error log file.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName   Name of error log file to open.
 *
 * @result 0 indicates success.
 *
 */
int core_log_openErrorLog(const char *fileName)
{
    int ierr;
    ierr = core_log_openLogFile(fileName, ERROR_FILE);
    return ierr;
}
/*!
 * @brief Opens the info log file.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName   Name of info log file to open.
 *
 * @result 0 indicates success.
 *
 */
int core_log_openInfoLog(const char *fileName)
{
    int ierr;
    ierr = core_log_openLogFile(fileName, INFO_FILE);
    return ierr;
}
/*!
 * @brief Opens the warning log file.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName   Name of warning log file to open.
 *
 * @result 0 indicates success.
 *
 */
int core_log_openWarningLog(const char *fileName)
{
    int ierr;
    ierr = core_log_openLogFile(fileName, WARNING_FILE);
    return ierr;
}
/*!
 * @brief Opens the debug log file.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName   Name of debug log file to open.
 *
 * @result 0 indicates success.
 *
 */
int core_log_openDebugLog(const char *fileName)
{
    int ierr;
    ierr = core_log_openLogFile(fileName, DEBUG_FILE);
    return ierr;
}
/*!
 * @brief Opens the big log file.  If the file exists then it will be
 *        appended to.
 *
 * @param[in] fileName   Name of log file to open.
 *
 * @result 0 indicates success.
 *
 */
int core_log_openLog(const char *fileName)
{
    int ierr;
    ierr = core_log_openLogFile(fileName, LOG_FILE);
    return ierr;
}
//============================================================================//
/*!
 * @brief Creates the error log file.  If the file exists then it will be
 *        overwritten.
 *
 * @param[in] fileName   Name of error log file to create.
 *
 * @result 0 indicates success.
 *
 */
int core_log_createErrorLog(const char *fileName)
{
    int ierr;
    ierr = core_log_createLogFile(fileName, ERROR_FILE);
    printf("MTH: create errorlog fileName=%s\n", fileName);
    return ierr;
}
/*!
 * @brief Creates the info log file.  If the file exists then it will be
 *        overwritten.
 *
 * @param[in] fileName   Name of info log file to create.
 *
 * @result 0 indicates success.
 *
 */
int core_log_createInfoLog(const char *fileName)
{   
    int ierr;
    ierr = core_log_createLogFile(fileName, INFO_FILE);
    return ierr;
}
/*!
 * @brief Creates the warning log file.  If the file exists then it will be
 *        overwritten.
 *
 * @param[in] fileName   Name of warning log file to create.
 *
 * @result 0 indicates success.
 *
 */
int core_log_createWarningLog(const char *fileName)
{
    int ierr;
    ierr = core_log_createLogFile(fileName, WARNING_FILE);
    return ierr;
}
/*!
 * @brief Creates the debug log file.  If the file exists then it will be
 *        overwritten.
 *
 * @param[in] fileName   Name of debug log file to create.
 *
 * @result 0 indicates success.
 *
 */
int core_log_createDebugLog(const char *fileName)
{   
    int ierr;
    ierr = core_log_createLogFile(fileName, DEBUG_FILE);
    return ierr;
}
/*!
 * @brief Creates the big log file.  If the file exists then it will be
 *        overwritten.
 *
 * @param[in] fileName   Name of log file to create.
 *
 * @result 0 indicates success.
 *
 */
int core_log_createLog(const char *fileName)
{   
    int ierr;
    ierr = core_log_createLogFile(fileName, LOG_FILE);
    return ierr;
}
//============================================================================//
/*!
 * @brief Closes the error log file.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeErrorLog(void)
{
    int ierr;
    ierr = core_log_closeLogFile(ERROR_FILE);
    return ierr;
}
/*!
 * @brief Closes the info log file.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeInfoLog(void)
{   
    int ierr;
    ierr = core_log_closeLogFile(INFO_FILE);
    return ierr;
}
/*!
 * @brief Closes the warning log file.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeWarningLog(void)
{
    int ierr;
    ierr = core_log_closeLogFile(WARNING_FILE);
    return ierr;
}
/*!
 * @brief Closes the debug log file.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeDebugLog(void)
{   
    int ierr;
    ierr = core_log_closeLogFile(DEBUG_FILE);
    return ierr;
}
/*!
 * @brief Closes the big log file.
 *
 * @result 0 indicates success.
 *
 */
int core_log_closeLog(void)
{   
  printf("**** MTH: close the big LOG_FILE");
  LOG_MSG("%s", "**** MTH: close the big LOG_FILE");
    int ierr;
    ierr = core_log_closeLogFile(LOG_FILE);
    return ierr;
}
//============================================================================//

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
    if (warningFile == NULL)
    {   
        fprintf(stdout, "%s\n", msg);
    }   
    else
    {   
        fprintf(warningFile, "%s\n", msg);
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

/*!
 * @brief Writes a message to the big log.
 *
 * @param[in] msg    Message to write to the log file.
 *
 */
void core_log_logMessage(const char *msg)
{
  if (logFile == NULL) {
    printf("MTH: logFile is NULL!\n");
  }

    if (msg == NULL){return;}
    if (logFile == NULL)
    {
        fprintf(stdout, "%s\n", msg);
    }
    else
    {
        fprintf(logFile, "%s\n", msg);
        fflush(logFile);
    }
    return;
}
