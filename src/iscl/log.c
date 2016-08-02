#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include "gfast_log.h"

static FILE *errorFile = NULL;
static FILE *infoFile = NULL;
static FILE *warnFile = NULL;
static FILE *debugFile = NULL;
static int (*_warnLogF)(const char *, va_list) = NULL;
static int (*_errorLogF)(const char *, va_list) = NULL;
static int (*_debugLogF)(const char *, va_list) = NULL;
static int (*_infoLogF)(const char *, va_list) = NULL;

/*!
 * @brief Example function pointer for initializing an error log.
 *        After defining the error log file name, errorLogFileName,
 *        one can call log_initErrorLog(&__errorToLog); 
 *
 * @note The open mode is "a" so one should take care to delete the 
 *       errorLogFileName prior to using this function.
 *
 * @result 1 indicates success
 *
 */
int __errorToLog(const char fmt[], va_list args)
{
    /*
    static FILE *fd = NULL;
    if (!fd)
    {   
        fd = fopen(errorLogFileName, "a");
        if (!fd)
        {
            perror("__errorToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(fd, fmt, args);
    */
    if (!errorFile)
    {
        errorFile = fopen(errorLogFileName, "a");
        if (!errorFile)
        {
            perror("__errorToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(errorFile, fmt, args);
    //errorFile = fd;
    return 1;
}

/*!
 * @brief Example function pointer for initializing an info log.
 *        After defining the info log file name, infoLogFileName,
 *        one can call log_initErrorLog(&__infoToLog); 
 *
 * @note The open mode is "a" so one should take care to delete the 
 *       infoLogFileName prior to using this function.
 *
 * @result 1 indicate success
 *
 */
int __infoToLog(const char fmt[], va_list args)
{
/*
    static FILE *fd = NULL;
    if (!fd)
    {   
        fd = fopen(infoLogFileName, "a");
        if (!fd)
        {
            perror("__infoToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(fd, fmt, args);
    static FILE *infoFile = NULL;
*/
    if (!infoFile)
    {   
        infoFile = fopen(infoLogFileName, "a");
        if (!infoFile)
        {
            perror("__infoToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(infoFile, fmt, args);
    //infoFile = fd;
    return 1;
}

/*!
 * @brief Example function pointer for initializing an debug log.
 *        After defining the debug log file name, debugLogFileName,
 *        one can call log_initErrorLog(&__debugToLog); 
 *
 * @note The open mode is "a" so one should take care to delete the 
 *       debugLogFileName prior to using this function.
 *
 */
int __debugToLog(const char fmt[], va_list args)
{
/*
    static FILE *fd = NULL;
    if (!fd)
    {   
        fd = fopen(debugLogFileName, "a");
        if (!fd)
        {
            perror("__debugToLog: Error opening file\n");
            return 0;
        }
    }
*/
    if (!debugFile)
    {   
        debugFile = fopen(debugLogFileName, "a");
        if (!debugFile)
        {
            perror("__debugToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(debugFile, fmt, args);
    return 1;
}

/*!
 * @brief Example function pointer for initializing an warning log.
 *        After defining the warning log file name, warnLogFileName,
 *        one can call log_initErrorLog(&__warnToLog);
 *
 * @note The open mode is "a" so one should take care to delete the 
 *       warnLogFileName prior to using this function.
 *
 * @result 1 indicates success
 *
 */
int __warnToLog(const char fmt[], va_list args)
{
/*
    static FILE *fd = NULL;
    if (!fd)
    {   
        fd = fopen(warnLogFileName, "a");
        if (!fd)
        {
            perror("__warnToLog: Error opening file\n");
            return 0;
        }
    }
*/
    if (!warnFile)
    {   
        warnFile = fopen(warnLogFileName, "a");
        if (!warnFile)
        {
            perror("__warnToLog: Error opening file\n");
            return 0;
        }
    }
    vfprintf(warnFile, fmt, args);
    //warnFile = fd;
    return 1;
}

/*!
 * @brief Initializes the warning, error, debug, and info logs
 *
 * @result 1 indicates success
 *
 */
int log_initLogs(int (*outf)(const char *, va_list))
{
    log_initWarnLog(outf);
    log_initErrorLog(outf);
    log_initDebugLog(outf);
    log_initInfoLog(outf);
    return 1;
}

/*!
 * @brief Initializes the error log file
 *
 * @result 1 indicates success
 *
 */
int log_initErrorLog(int (*outf)(const char *, va_list))
{
    if (!outf)
    {
        /* TODO make this configurable at compile time */
        _errorLogF = &vprintf;
        return 1;
    }
    _errorLogF = outf;
    return 1;
}

/*!
 * @brief Initializes the warning log file
 *
 * @result 1 indicates success
 *
 */
int log_initWarnLog(int (*outf)(const char *, va_list))
{
    if (!outf)
    {
        /* TODO make this configurable at compile time */
        _warnLogF = &vprintf;
        return 1;
    }
    _warnLogF = outf;
    return 1;
}

/*!
 * @brief Initializes the debug lof file
 *
 * @result 1 indicates success
 *
 */
int log_initDebugLog(int (*outf)(const char *, va_list))
{
    if (!outf)
    {
        /* TODO make this configurable at compile time */
        _debugLogF = &vprintf;
        return 1;
    }
    _debugLogF = outf;
    return 1;
}

/*!
 * @brief Initializes the info log file
 *
 * @result 1 indicates success
 *
 */
int log_initInfoLog(int (*outf)(const char *, va_list))
{
    if (!outf)
    {
        /* TODO make this configurable at compile time */
        _infoLogF = &vprintf;
        return 1;
    }
    _infoLogF = outf;
    return 1;
}

/*!
 * @brief Writes to the error log file.  If this file is uninitialized
 *        will write to standard out
 *
 * @result 1 indicates success
 *
 */
int log_errorF(const char fmt[], ...)
{
    va_list args;
    int ret;

    if (!_errorLogF)
        log_initErrorLog(NULL);

    va_start(args, fmt);
    ret = _errorLogF(fmt, args);
    va_end(args);
    return ret;
}

/*!
 * @brief Writes to the debug log file.  If this file is uninitialized
 *        will write to standard out
 *
 * @result 1 indicates success
 *
 */
int log_debugF(const char fmt[], ...)
{
    va_list args;
    int ret;

    if (!_debugLogF)
        log_initDebugLog(NULL);

    va_start(args, fmt);
    ret = _debugLogF(fmt, args);
    va_end(args);
    return ret;
}

/*!
 * @brief Writes to the info log file.  If this file is uninitialized
 *        will write to standard out
 *
 * @result 1 indicates success
 *
 */
int log_infoF(const char fmt[], ...)
{
    va_list args;
    int ret;

    if (!_infoLogF)
        log_initInfoLog(NULL);

    va_start(args, fmt);
    ret = _infoLogF(fmt, args);
    va_end(args);
    return ret;
}

/*!
 * @brief Writes to the warning log file.  If this file is uninitialized
 *        will write to standard out
 *
 * @result 1 indicates success
 *
 */
int log_warnF(const char fmt[], ...)
{
    va_list args;
    int ret;

    if (!_warnLogF)
        log_initWarnLog(NULL);

    va_start(args, fmt);
    ret = _warnLogF(fmt, args);
    va_end(args);
    return ret;
}

void log_closeWarnF(void)
{
    if (!warnFile){return;}
    fclose(warnFile);
    warnFile = NULL;
    return;
}

void log_closeInfoF(void)
{
    if (!infoFile){return;}
    fclose(infoFile);
    infoFile = NULL;
    return;
}

void log_closeDebugF(void)
{
    if (!debugFile){return;}
    fclose(debugFile);
    debugFile = NULL;
    return;
}

void log_closeErrorF(void)
{
    if (!errorFile){return;}
    fclose(errorFile);
    errorFile = NULL;
    return;
}
