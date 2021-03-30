#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "iscl/os/os.h"

/*!
 * @defgroup os_path_isdir isdir
 * @brief Determines if a path is a directory.
 * @ingroup os
 */
/*! 
 * @brief Tests if dirnm is a directory.
 * 
 * @param[in] dirnm    Name of directory to test.
 *
 * @retval If true true then dirnm is an existing directory.
 * @retval If false then dirnm is not a directory.
 * 
 * @ingroup os_path_isdir
 *
 * @author Ben Baker
 *
 */
bool os_path_isdir(const char *dirnm)
{
    struct stat s;
    int err;
    if (dirnm == NULL){return false;}
    if (strlen(dirnm) == 0){return false;}
    err = stat(dirnm, &s);
    // Doesn't exist
    if (err == -1)
    {
        if (ENOENT == errno)
        {
            return false;
        }
        // Exists
        else
        {
            return true;
        }
    }
    // Exists
    else
    {
        // Test it is a directory
        if (S_ISDIR(s.st_mode))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
