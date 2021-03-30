#include <sys/time.h>
#include <stdio.h>
#include "timeutils.h"
#include <time.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/iscl/iscl_enum.h"
#include "gfast_core.h"

/*!
 * @defgroup timeutils
 * @brief time utilities. 
 */

static double tic = 0.0;

/*! @brief Returns the current system time as double */
double time_timeStamp(void)
{
  struct timeval now;
  double dnow;
  gettimeofday(&now, NULL);
  dnow = ( (double) now.tv_usec
	   + (double) now.tv_sec*1000000)*10.e-7;
  return dnow;
}
/*!
 * @brief Begins the time for toc.
 *
 * @ingroup time_tictoc
 *
 * @author Ben Baker, ISTI
 *
 */
void time_tic(void)
{
    tic = time_timeStamp(); 
    return;
}
/*!
 * @brief Returns the time elapsed since calling time_tic.
 * 
 * @result Elapsed execution time in seconds.
 *
 * @ingroup time_tictoc
 *
 * @author Ben Baker, ISTI
 *
 */
double time_toc(void)
{
    double toc;
    toc = time_timeStamp() - tic;
    return toc;
}

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
        LOG_MSG("%s", "Error setting gmtime");
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

// /*!
//  * @brief Converts the time given by the year, julian day, hour, minute, second
//  *        and microsecond to seconds since the epoch where the epoch is the
//  *        first day of 1970.  All times and dates are in UTC. 
//  *
//  * @param[in] nzyear     The year.
//  * @param[in] nzjday     The Julian day in the range [1,366].
//  * @param[in] nzhour     The hour in the range [0,23].
//  * @param[in] nzmin      The minute in the range [0,59].
//  * @param[in] nzsec      The second in the range [0,59].
//  * @param[in] nzmusec    The micro-second.
//  * 
//  * @result Epochal time in UTC seconds since epoch.
//  *
//  * @ingroup time_calendar2epoch
//  *
//  * @author Ben Baker, ISTI
//  *
//  */
// double time_calendar2epoch(const int nzyear, const int nzjday, const int nzhour,
//                            const int nzmin, const int nzsec, const int nzmusec)
// {
//     struct tm t, t0;
//     double epoch;
//     memset(&t, 0, sizeof(struct tm));
//     t.tm_year = nzyear - 1900;  // This is year-1900, so 112 = 2012
//     t.tm_yday = nzjday;
//     t.tm_hour = nzhour;
//     t.tm_mday = nzjday - 1;     // C ignores yday and uses this 
//     t.tm_min = nzmin;
//     t.tm_sec = nzsec;
//     // Time since epoch which is 1970 
//     memset(&t0, 0, sizeof(struct tm));
//     t0.tm_year = 70;  // 1970
//     //epoch = difftime(mktime(&t), mktime(&t0));
//     epoch = difftime(timegm(&t), timegm(&t0));
//     epoch = epoch + (double) (nzmusec)*1.e-6;
//     return epoch;
// }

/*!
 * @brief Converts the time given by the year, month, day of month, hour,
 *        minute, second, and microsecond to seconds since the epoch where
 *        the epoch is the first day of 1970.  All times and dates are in
 *        UTC time.
 *
 * @param[in] nzyear     The year.
 * @param[in] month      The month of year in the range [1,12].
 * @param[in] dom        The day of month in the range [1,31].
 * @param[in] nzhour     The hour in the range of [0,23].
 * @param[in] nzmin      The minute in the range [0,59].
 * @param[in] nzsec      The second in the range [0,59].
 * @param[in] nzmusec    The micro-second.
 * 
 * @result Epochal time in UTC seconds since epoch.
 *
 * @ingroup time_calendar2epoch
 *
 * @author Ben Baker, ISTI
 *
 */ 
double time_calendar2epoch2(const int nzyear, const int month, const int dom,
                            const int nzhour, const int nzmin, const int nzsec,
                            const int nzmusec)
{
    struct tm t, t0; 
    double epoch;
    memset(&t, 0, sizeof(struct tm));
    t.tm_year = nzyear - 1900;  // This is year-1900, so 112 = 2012
    t.tm_hour = nzhour;
    t.tm_mon = month - 1;       // Month of year [0,11]
    t.tm_mday = dom - 1;        // Day of month [0,30] 
    t.tm_min = nzmin;
    t.tm_sec = nzsec;
    // Time since epoch which is 1970 
    memset(&t0, 0, sizeof(struct tm));
    t0.tm_year = 70;  // 1970
    //epoch = difftime(mktime(&t), mktime(&t0));
    epoch = difftime(timegm(&t), timegm(&t0));
    epoch = epoch + (double) (nzmusec)*1.e-6;
    return epoch;
}
