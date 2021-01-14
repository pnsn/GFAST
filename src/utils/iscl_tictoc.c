#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef OPENMP
#include <omp.h>
#endif
#include "iscl_time.h"

static double tic = 0.0;
#ifdef OPENMP
#pragma omp threadprivate(tic)
#endif
/*!
 * @defgroup time_tictoc tictoc
 * @brief Routines that are useful for timing functions. 
 * @ingroup time 
 */
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
