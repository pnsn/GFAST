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

#ifdef __cplusplus
}
#endif
#endif /* timeutils_h__ */
