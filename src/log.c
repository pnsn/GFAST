#include <stdio.h>
#include <stdarg.h>
#include "gfast_log.h"

static int (*_warnLogF)(const char *, va_list) = NULL;
static int (*_errorLogF)(const char *, va_list) = NULL;
static int (*_debugLogF)(const char *, va_list) = NULL;
static int (*_infoLogF)(const char *, va_list) = NULL;

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
 * @brief Initializes the deubg lof file
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
