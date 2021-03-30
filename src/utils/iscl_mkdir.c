#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/os/os.h"

/*!
 * @defgroup os_mkdir mkdir
 * @brief Creates a directory.
 * @ingroup os
 */
/*! 
 * Makes a directory named dirnm with full permissions.
 *
 * @param[in] dirnm    Name of directory to make.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup os_mkdir
 *
 * @author Ben Baker
 *
 */
enum isclError_enum os_mkdir(const char *dirnm)
{
    int ierr;
    if (dirnm == NULL)
    {
        isclPrintError("Directory name is NULL");
        return ISCL_NULL_PATH;
    }
    if (strlen(dirnm) == 0)
    {
        isclPrintError("Directory name is empty");
        return ISCL_EMPTY_PATH;
    }
#if defined WINNT || defined WIN32 || defined WIN64
    ierr = mkdir(dirnm);
#else
    ierr = mkdir(dirnm, 0777);
#endif
    if (ierr != 0)
    {
        isclPrintError("Error making directory %s", dirnm);
        return ISCL_SYS_FAILURE;
    }   
    return ISCL_SUCCESS;
}
