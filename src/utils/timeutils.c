#include <sys/time.h>
#include <stdio.h>
#include "timeutils.h"

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
