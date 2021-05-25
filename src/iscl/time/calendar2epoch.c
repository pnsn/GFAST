#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "iscl/time/time.h"

#ifndef __USE_MISC
time_t timegm(struct tm *tm);
#endif

/*!
 * @defgroup time_calendar2epoch calendar2epoch
 * @brief Converts a calendar date to epochal time.
 * @ingroup time 
 */
/*!
 * @brief Converts the time given by the year, julian day, hour, minute, second
 *        and microsecond to seconds since the epoch where the epoch is the
 *        first day of 1970.  All times and dates are in UTC. 
 *
 * @param[in] nzyear     The year.
 * @param[in] nzjday     The Julian day in the range [1,366].
 * @param[in] nzhour     The hour in the range [0,23].
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
double time_calendar2epoch(const int nzyear, const int nzjday, const int nzhour,
                           const int nzmin, const int nzsec, const int nzmusec)
{
    struct tm t, t0;
    double epoch;
    memset(&t, 0, sizeof(struct tm));
    t.tm_year = nzyear - 1900;  // This is year-1900, so 112 = 2012
    t.tm_yday = nzjday;
    t.tm_hour = nzhour;
    t.tm_mday = nzjday - 1;     // C ignores yday and uses this 
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
//============================================================================//
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

