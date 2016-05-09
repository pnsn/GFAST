#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "gfast.h"

static double tic = 0.0;
#ifdef OPENMP
#pragma omp threadprivate(tic)
#endif
/*!
 * @brief Begins the time for toc
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
 * @brief Returns the time elapsed since calling time_tic
 * 
 * @result elapsed execution time in seconds
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
 * @brief Converts the time given by the year, julian day, hour, minute, second
 *        and microsecond to seconds since epoch where the epoch is the first
 *        day of 1970.
 *
 * @param[in] nzyear     year
 * @param[in] nzjday     julian day [1,366]
 * @param[in] nzhour     hour  [1,24]
 * @param[in] nzmin      minute [0,59]
 * @param[in] nzsec      second [0,59]
 * @param[in] nzmusec    micro-second
 * 
 * @result epochal time in seconds since epoch
 *
 * @author Ben Baker, ISTI
 *
 */
double time_calendar2epoch(int nzyear, int nzjday, int nzhour,
                           int nzmin, int nzsec, int nzmusec)
{
   struct tm t, t0;
   double epoch;
   memset(&t, 0, sizeof(t));
   t.tm_year = nzyear - 1900;  // This is year-1900, so 112 = 2012
   t.tm_yday = nzjday;
   t.tm_hour = nzhour;
   t.tm_mday = nzjday - 1;     // C ignores yday and uses this 
   t.tm_min = nzmin;
   t.tm_sec = nzsec;
   // Time since epoch which is 1970 
   memset(&t0, 0, sizeof(t0));
   t0.tm_year = 70;  // 1970
   epoch = difftime(mktime(&t), mktime(&t0));
   epoch = epoch + (double) (nzmusec)*1.e-6;
   return epoch;
}
//============================================================================//
/*!
 * @brief Converts the time given by the year, month, day of month, hour,
 *        minute, second, and microsecond to seconds since epoch where
 *        the epoch is the first day of 1970.
 *
 * @param[in] nzyear     year
 * @param[in] month      month of year [1,12]
 * @param[in] dom        day of month [1,31]
 * @param[in] nzhour     hour  [1,24]
 * @param[in] nzmin      minute [0,59]
 * @param[in] nzsec      second [0,59]
 * @param[in] nzmusec    micro-second
 * 
 * @result epochal time in seconds since epoch
 *
 * @author Ben Baker, ISTI
 *
 */ 
double time_calendar2epoch2(int nzyear, int month, int dom, int nzhour,
                            int nzmin, int nzsec, int nzmusec)
{
   struct tm t, t0;
   double epoch; 
   memset(&t, 0, sizeof(t));
   t.tm_year = nzyear - 1900;  // This is year-1900, so 112 = 2012
   t.tm_hour = nzhour;
   t.tm_mon = month - 1;       // Month of year [0,11]
   t.tm_mday = dom - 1;        // Day of month [0,30] 
   t.tm_min = nzmin;
   t.tm_sec = nzsec;
   // Time since epoch which is 1970 
   memset(&t0, 0, sizeof(t0));
   t0.tm_year = 70;  // 1970
   epoch = difftime(mktime(&t), mktime(&t0));
   epoch = epoch + (double) (nzmusec)*1.e-6;
   return epoch;
}
//============================================================================//
/*!
 * @param[in] epoch      epoch time (seconds since 1970)
 *
 * @param[out] nzyear    year corresponding to epoch
 * @param[out] nzjday    julian day [1,366] corresonding to epoch
 * @param[out] month     calendar month [1,12] corresponding to epoch time
 * @param[out] mday      day of month [1,31] corresponding to epoch time
 * @param[out] nzhour    hour [0,23] corresponding to epoch time
 * @param[out] nzsec     second [0,59] corresponding to epoch time
 * @param[out] nzmusec   micro-second corresponding to epoch time
 *
 * @result 0 indicates success
 *
 * @author ISTI
 *
 */
int time_epoch2calendar(double epoch,
                        int *nzyear, int *nzjday, int *month, int *mday,
                        int *nzhour, int *nzmin, int *nzsec, int *nzmusec)
{
    const char *fcnm = "time_epoch2calendar\0";
    time_t t = (time_t) epoch;
    struct tm *tmptr = gmtime(&t);
    if (tmptr == NULL){
        log_errorF("%s: Error converting time\n", fcnm);
        return -1; 
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
    return 0;
}
//============================================================================//
/*!
 * @brief Computes the epochal time (s) right now since 1970
 *
 * @result UTC epochal time (s) of right now
 *
 * @bug This does not work if case 1970 is not defined on the epoch on this
 *      computer.
 */
double time_currentTime(void)
{
    time_t epoch;
    double now;
    epoch = time(NULL); // Current time
    now = (double) epoch;
    return now;
}
//============================================================================//
/*! 
 * @brief Returns the current system time in seconds.  Useful when timing 
 * a routine.  For example: 
 *
 *    tstart = time_timeStamp();
 *    some code
 *    tend = time_timeStamp();
 *    tsim = tstart - tend; // Time of simulation
 *
 * @result Current system time (seconds)
 *
 */
double time_timeStamp(void)
{
    double time;
#ifdef MPI
    time = MPI_Wtime();
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    time = (double)
           (now.tv_usec + (timerTimeStampType) now.tv_sec * 1000000)*10.e-7;
#endif
    return time;
}
