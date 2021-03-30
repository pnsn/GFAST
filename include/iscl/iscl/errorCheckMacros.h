#ifdef ISCL_MACROS_SRC
#ifndef ISCL_ERROR_CHECK_MACROS__
#define ISCL_ERROR_CHECK_MACROS__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
//#include "iscl/iscl/iscl.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifndef isclLogAllocError
#define isclLogAllocError(varName) \
{ \
    if (iscl_getVerbosityLevel() > ISCL_SHOW_NONE) \
    { \
        fprintf(stderr, "%s[ERROR] (file=%s:function=%s:line=%d): ", \
                ANSI_COLOR_RED, __FILE__, __func__, __LINE__); \
        fprintf(stderr, "Error allocating %s%s\n", varName, ANSI_COLOR_RESET); \
    } \
}
#endif

#ifndef isclLogArrayTooSmallError
#define isclLogArrayTooSmallError(varName, n, nmin) \
{ \
    if (iscl_getVerbosityLevel() > ISCL_SHOW_NONE) \
    { \
        fprintf(stderr, "%s[ERROR] (file=%s:function=%s:line=%d): ", \
                ANSI_COLOR_RED, __FILE__, __func__, __LINE__); \
        fprintf(stderr, "Error %s=%d is too small; must be at least %d%s\n", \
                varName, n, nmin, ANSI_COLOR_RESET);  \
    } \
}
#endif


#ifndef isclLogPointerIsNullError
#define isclLogPointerIsNullError(varName) \
{ \
    if (iscl_getVerbosityLevel() > ISCL_SHOW_NONE) \
    { \
        fprintf(stderr, "%s[ERROR] (file=%s:function=%s:line=%d): ", \
                ANSI_COLOR_RED, __FILE__, __func__, __LINE__); \
        fprintf(stderr, "Error %s is NULL%s\n", varName, ANSI_COLOR_RESET); \
    } \
}
#endif

#ifndef isclReturnArrayTooSmallError
#define isclReturnArrayTooSmallError(varName, n, nmin) \
{ \
    if (n < nmin) \
    { \
        isclLogArrayTooSmallError(varName, n, nmin); \
        return ISCL_ARRAY_TOO_SMALL; \
    } \
}
#endif

#ifndef isclReturnInvalidParameter
#define isclReturnInvalidParameter(errorMessage) \
{ \
    if (iscl_getVerbosityLevel() > ISCL_SHOW_NONE) \
    { \
        fprintf(stderr, "%s[ERROR] (%s:function=%s:line=%d): %s%s\n", \
                ANSI_COLOR_RED, __FILE__, __func__, __LINE__, errorMessage, ANSI_COLOR_RESET); \
    } \
    return ISCL_INVALID_INPUT; \
}
#endif

#ifndef isclReturnNullPointerError
#define isclReturnNullPointerError(varName, v) \
{ \
    if (v == NULL) \
    { \
        isclLogPointerIsNullError(varName); \
        return ISCL_NULL_PTR; \
    } \
}
#endif

#ifndef isclSetArrayTooSmallErrorAndReturnNULL
#define isclSetArrayTooSmallErrorAndReturnNULL(varName, n, nmin, ierr) \
{ \
    if (n < nmin) \
    { \
        isclLogArrayTooSmallError(varName, n, nmin); \
        *ierr = ISCL_ARRAY_TOO_SMALL; \
        return NULL; \
    } \
}
#endif

#ifndef isclPrintDebug
#define isclPrintDebug(...) \
{ \
     if (iscl_getVerbosityLevel() >= ISCL_SHOW_ALL) \
     { \
         fprintf(stdout, "%s[DEBUG] (file=%s:function=%s:line=%d) ", \
                 ANSI_COLOR_BLUE, basename(__FILE__), __func__, __LINE__); \
         do{ fprintf(stdout, __VA_ARGS__); } while(false); \
         fprintf(stdout, "%s\n", ANSI_COLOR_RESET); \
     } \
}
#endif

#ifndef isclPrintWarning
#define isclPrintWarning(...) \
{ \
     if (iscl_getVerbosityLevel() >= ISCL_SHOW_ERRORS_AND_WARNINGS) \
     { \
         fprintf(stdout, "%s[WARNING] (file=%s:function=%s:line=%d) ", \
                 ANSI_COLOR_YELLOW, __FILE__, __func__, __LINE__); \
         do{ fprintf(stdout, __VA_ARGS__); } while(false); \
         fprintf(stdout, "%s\n", ANSI_COLOR_RESET); \
     } \
}
#endif

#ifndef isclPrintError
#define isclPrintError(...) \
{ \
    if (iscl_getVerbosityLevel() > ISCL_SHOW_NONE) \
    { \
        fprintf(stderr, "%s[ERROR] (file=%s:function=%s:line=%d) ", \
                ANSI_COLOR_RED, __FILE__, __func__, __LINE__); \
        do{ fprintf(stderr, __VA_ARGS__); } while(false); \
        fprintf(stderr, "%s\n", ANSI_COLOR_RESET); \
    } \
}
#endif

#endif /* ISCL_ERROR_CHECK_MACROS */
#endif /* ISCL_MACROS_SRC */
