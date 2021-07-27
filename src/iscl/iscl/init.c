#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include "iscl/iscl/iscl.h"
#include "iscl/memory/memory.h"

static bool liscl_init = false;
#ifdef OPENMP
#pragma omp threadprivate (liscl_init)
#endif

/*!
 * @defgroup iscl_init init
 * @brief Initializes the library.
 * @ingroup iscl
 */
/*!
 * @brief Initializes memory in ISCL library.
 *
 * @ingroup iscl_init
 *
 * @author Ben Baker, ISTI
 *
 */
void iscl_init(void)
{
    liscl_init = true;
    //iscl_setNumThreads(1);
    iscl_setVerbosityLevel(ISCL_SHOW_ERRORS);
    return;
}

/*!
 * @brief Private function to determine if ISCL was initialized 
 */
bool __iscl_isinit(void)
{
    return liscl_init;
}

/*!
 * @brief Sets global variable liscl_init to false
 */
void __iscl_uninit(void)
{
    liscl_init = false;
    return;
}
