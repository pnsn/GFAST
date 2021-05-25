#include <stdio.h>
#include <stdlib.h>
#include "iscl/iscl/iscl.h"
#ifdef ISCL_USE_INTEL
#include <mkl_service.h>
#endif

/*!
 * @defgroup iscl_finalize finalize 
 * @brief Cleans any accrued memory.
 * @ingroup iscl
 */
/*!
 * @brief Releases any memory accrued while the ISCL library was run.
 *        This should be used at the end of the user's program.
 *
 * @ingroup iscl_finalize
 *
 * @author Ben Baker, ISTI
 *
 */
void iscl_finalize(void)
{
  __iscl_uninit();
#ifdef ISCL_USE_INTEL
  mkl_free_buffers();
#endif
  return;
}
