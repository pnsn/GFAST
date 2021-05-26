#include <stdio.h>
#include <stdlib.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/iscl/iscl.h"
#ifdef ISCL_USE_INTEL
#include <mkl.h>
#include <ipps.h>
#endif

static enum isclVerbosityLevel_enum verb = ISCL_SHOW_ERRORS;

/*!
 * @defgroup iscl_verbosity verbosity
 * @brief Controls the verbosity of the library.
 * @ingroup iscl
 */
/*!
 * @brief Sets ISCL's verbosity level.
 *
 * @param[in] verbosity   ISCL_SHOW_NONE (0) will not output any messages.
 * @param[in] verbosity   ISCL_SHOW_ERRORS (1) (default) will show error
 *                        messages.
 * @param[in] verbosity   ISCL_SHOW_ERRORS_AND_WARNINGS (2) will show
 *                        errors and warnings.
 * @param[in] verbosity   ISCL_SHOW_ALL (3) will show errors, warnings,
 *                        and intermediate debug information.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup iscl_verbosity
 *
 * @author Ben Baker
 *
 */
enum isclError_enum
    iscl_setVerbosityLevel(const enum isclVerbosityLevel_enum verbosity)
{
    if ((int) verbosity < 0 || (int) verbosity > 3)
    {
        isclPrintError("Invalid verbosity level %d", (int) verbosity);
        return ISCL_INVALID_INPUT;
    } 
    verb = verbosity;
    return ISCL_SUCCESS;
}
/*!
 * @brief Returns the current ISCL verbosity level.
 *
 * @result The verbosity level that ISCL is currently set to.
 *
 * @ingroup iscl_verbosity
 *
 * @author Ben Baker
 *
 */
enum isclVerbosityLevel_enum  iscl_getVerbosityLevel(void)
{
    return verb;
}
