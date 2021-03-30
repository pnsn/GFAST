#if defined WINNT || defined WIN32 || defined WIN64
#include <windows.h>
#include <limits.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>
#define ISCL_MACROS_SRC 1
#include "iscl/iscl/errorCheckMacros.h"
#include "iscl/os/os.h"

/*!
 * @defgroup os_makedirs makedirs
 * @brief Recursively create directories.
 * @ingroup os
 */
/*!
 * @brief Recursive directory creation function.
 *
 * @param[in] path    Directory tree to make.
 *
 * @result ISCL error code where ISCL_SUCCESS indicates success.
 *
 * @ingroup os_makedirs
 *
 * @author Ben Baker
 *
 */
enum isclError_enum os_makedirs(const char *path)
{
    char *where, *dname, *work, directory[PATH_MAX];
#if defined WINNT || defined WIN32 || defined WIN64
    const char find[] = "\\/";
#else
    const char find[] = "/\0";
#endif
    size_t indx, lenos;
    enum isclError_enum ierr;
    //------------------------------------------------------------------------//
    //
    // Errors 
    ierr = ISCL_SUCCESS;
    if (path == NULL)
    {
        isclPrintError("%s", "Directory name is NULL");
        return ISCL_NULL_PATH;
    }
    lenos = strlen(path);
    if (lenos == 0)
    {
        isclPrintError("%s", "Directory name is empty");
        return ISCL_EMPTY_PATH;
    }
    if (lenos > PATH_MAX - 2)
    {
        isclPrintError("Directory %s is too long", path);
        return ISCL_INVALID_INPUT;
    } 
    // Already exists
    if (os_path_isdir(path)){return ISCL_SUCCESS;}
    // Initialize
    work = (char *) calloc(lenos+2, sizeof(char));
    strcpy(work, path);
    memset(directory, 0, PATH_MAX*sizeof(char));
    dname = work;
#if defined WINNT || defined WIN32 || defined WIN64
    dname[lenos] = '\0'; // No idea what to do
#else 
    dname[lenos] = '/'; // Try to catch the final case
#endif
    where = strpbrk(dname, find);
    while ((where != NULL))
    {
        indx = (size_t) (where - dname);
        lenos = strlen(directory);
        strncat(directory, dname, indx);
        // Windows does nonsense like: C:
#if defined WINNT || defined WIN32 || defined WIN64
        if (directory[strlen(directory) - 1] != ':')
#endif
        {
            // If directory doesn't exist then make it
            if (!os_path_isdir(directory))
            {
                ierr = os_mkdir(directory);
                if (ierr != ISCL_SUCCESS)
                {
                    isclPrintError("Error making subdirectory: %s",
                                   directory);
                    isclPrintError("Error making directory: %s", path);
                    free(dname);
                    return ierr;
                }
            } // End check on if directory exists
        }
        // Add directory delimiter
#if defined WINNT || defined WIN32 || defined WIN64
        strcat(directory, "//\0");
#else
        strcat(directory, "/\0"); 
#endif
        dname = dname + indx + 1;
        where = strpbrk(dname, find);
    } // End while
    // Add name of final subdirectory and make it
    strcat(directory, dname);
    if (!os_path_isdir(directory))
    {
         ierr = os_mkdir(directory);
         if (ierr != ISCL_SUCCESS)
         {
             isclPrintError("Error making directory: %s", directory);
             free(work);
             return ierr;
         }
    }
    // Free space
    dname = NULL;
    free(work);
    return ierr;
}

