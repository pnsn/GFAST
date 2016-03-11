#include <stdarg.h>

#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* call this before calling any seismic library function to initialize both warning and error to be handled by same function */
int log_initLogs(int (*)(const char s[], va_list));

/* call this before any seismic library function to set the function for error log message handling */
int log_initErrorLog(int (*)(const char s[], va_list));

/* call this before any seismic library function to set the function for warning log message handling */
int log_initWarnLog(int (*)(const char s[], va_list));

/* call this before any seismic library function to set the function for informative log message handling */
int log_initInfoLog(int (*)(const char s[], va_list));

/* call this before any seismic library function to set the function for debugging log message handling */
int log_initDebugLog(int (*)(const char s[], va_list));

/* print formated string to error log. if not inited defaults to stdout. use like printf */
int log_errorF(const char [], ...);

/* print formated string to warning log. if not inited defaults to stdout. use like printf */
int log_warnF(const char [], ...);

/* print formated string to informative log. if not inited defaults to stdout. use like printf */
int log_infoF(const char [], ...);

/* print formated string to debuging log. if not inited defaults to stdout. use like printf */
int log_debugF(const char [], ...);

#ifdef __cplusplus
}
#endif
#endif /* __LOG_H__ */
