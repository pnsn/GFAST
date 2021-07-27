#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/iscl.h"
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/time/time.h"

/*!
 * @defgroup time_epoch2calendar epoch2calendar
 * @brief Converts an epochal time to a calendar date.
 * @ingroup time 
 */
/*!
 * @brief Converts the number of seconds since the epoch to the 
 *        the corresponding year, Julian day, month, day of the month,
 *        hour, minute, second, and microsecond.  All dates and times
 *        are in UTC.
 *
 * @param[in] epoch      Epochal time (UTC seconds since 1970).
 *
 * @param[out] nzyear    Year corresponding to epoch.
 * @param[out] nzjday    Julian day [1,366] corresonding to epoch.
 * @param[out] month     Calendar month [1,12] corresponding to epoch time.
 * @param[out] mday      Day of month [1,31] corresponding to epoch time.
 * @param[out] nzhour    Hour [0,23] corresponding to epoch time.
 * @param[out] nzmin     Minute [0,59] corresponding to epoch time.
 * @param[out] nzsec     Second [0,59] corresponding to epoch time.
 * @param[out] nzmusec   Micro-second corresponding to epoch time.
 *
 * @result ISCL_SUCCESS indicates success.
 *
 * @ingroup time_epoch2calendar
 *
 * @author ISTI
 *
 */
enum isclError_enum
    time_epoch2calendar(const double epoch,
                        int *nzyear, int *nzjday, int *month, int *mday,
                        int *nzhour, int *nzmin, int *nzsec, int *nzmusec)
{
    time_t t = (time_t) epoch;
    struct tm *tmptr = gmtime(&t);
    if (tmptr == NULL)
    {
        isclPrintError("%s", "Error setting gmtime\0");
        return ISCL_GMTIME_FAILURE;
    }
    *nzyear  = (int) tmptr->tm_year;
    *nzjday  = (int) tmptr->tm_yday + 1;
    *month   = (int) tmptr->tm_mon + 1; // Months since january
    *mday    = (int) tmptr->tm_mday;
    *nzhour  = (int) tmptr->tm_hour;
    *nzmin   = (int) tmptr->tm_min;     // 20150227 ALomax - this line was missing
    *nzsec   = (int) tmptr->tm_sec;
    *nzmusec = (int) ((epoch - (double) (long) epoch)*1.e6 + 0.5);
    *nzyear = *nzyear + 1900;
    return ISCL_SUCCESS;
}
