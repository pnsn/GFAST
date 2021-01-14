#if defined WINNT || defined WIN32 || defined WIN64
#include <window.h>
#endif
#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include "iscl_time.h"
#ifdef MPI
#include <mpi.h>
#endif

/*!
 * @defgroup time_timeStamp timeStamp
 * @brief Returns the current system in seconds.
 * @ingroup time 
 */
/*! 
 * @brief Returns the current system time in seconds.  Useful when timing 
 *        a routine.  For example: 
 *
 *    tstart = time_timeStamp();
 *    some code
 *    tend = time_timeStamp();
 *    tsim = tstart - tend; // Time of simulation
 *
 * @result Current system time (seconds).
 *
 * @ingroup time_timeStamp
 *
 */
double time_timeStamp(void)
{
    double time;
#ifdef MPI
    time = MPI_Wtime();
#else
#if defined WINNT || defined WIN32 || defined WIN64
    FILETIME ft; 
    uint64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = (((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime)/10;
    time = (double) t*1.e-6; 
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    time = (double)
           ( (timerTimeStampType) now.tv_usec
           + (timerTimeStampType) now.tv_sec*1000000)*10.e-7;
#endif
#endif
    return time;
}
