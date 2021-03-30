#ifndef TIME_UTILS_H__
#define TIME_UTILS_H__ 1

/*!
 * @defgroup time Time
 * @brief Time converters and program timing utilities.
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif

/*! @brief Returns the current system time as double */
double time_timeStamp(void);
  
/*! @brief Matlabesqe tictoc stopwatch start */
void time_tic(void);

/*! @brief Matlabesqe tictoc stopwatch 
 *  @return time since last time_tic() call
 */
double time_toc(void);

/*!
 * @brief Converts the number of seconds since the epoch to the 
 *        the corresponding year, Julian day, month, day of the month,
 *        hour, minute, second, and microsecond.  All dates and times
 *        are in UTC.
 * 
 * @result ISCL_SUCCESS indicates success.
 */
enum isclError_enum
    time_epoch2calendar(const double epoch,
                        int *nzyear, int *nzjday, int *month, int *mday,
                        int *nzhour, int *nzmin, int *nzsec, int *nzmusec);

/*!
 * @brief Converts the time given by the year, month, day of month, hour,
 *        minute, second, and microsecond to seconds since the epoch where
 *        the epoch is the first day of 1970.  All times and dates are in
 *        UTC time.
 * @result Epochal time in UTC seconds since epoch.
 */ 
double time_calendar2epoch2(const int nzyear, const int month, const int dom,
                            const int nzhour, const int nzmin, const int nzsec,
                            const int nzmusec);
  
#ifdef __cplusplus
}
#endif
#endif /* timeutils_h__ */
