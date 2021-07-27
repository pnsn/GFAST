#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "iscl/os/os.h"

/*!
 * @defgroup os_path_isfile isfile
 * @brief Determines if a path is a file.
 * @ingroup os
 */
/*! 
 * @brief Tests if filenm is a file.
 * 
 * @param[in] filenm    Name of file to test.
 * 
 * @retval If true then filenm is an existing file.
 * @retval If false then filenm is not a file.
 *
 * @ingroup os_path_isfile
 *
 * @author Ben Baker, ISTI
 *
 */
bool os_path_isfile(const char *filenm)
{
    struct stat info;
    if (filenm == NULL){return false;}
    if (strlen(filenm) == 0){return false;}
    // Doesn't exist
    if (stat(filenm, &info) ==-1)
    {
        return false;
    }
    // Exists -> check it is a file
    else
    {
        if (S_ISREG(info.st_mode))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
