#ifndef OS_OS_H__
#define OS_OS_H__ 1
#include <stdbool.h>
#include "iscl/iscl/iscl.h"
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*!
 * @defgroup os OS 
 * @brief Operating system functions for file and directory handling.
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif
/* Gets the name of the file in a path */
char *os_basename(const char *path, enum isclError_enum *ierr);
enum isclError_enum os_basename_work(const char *path, char baseName[PATH_MAX]);
/* Gets the directory leading to a file */
char *os_dirname(const char *path, enum isclError_enum *ierr);
enum isclError_enum os_dirname_work(const char *path, char dirName[PATH_MAX]);
/* Determines if pathnm exists */
bool os_path_exists(const char *pathnm);
/* Determines if dirnm is an existing directory */
bool os_path_isdir(const char *dirnm);
/* Determines if filenm is an existing file */
bool os_path_isfile(const char *filenm);
/* Recursively make directories */
enum isclError_enum os_makedirs(const char *path);
/* Makes a directory */
enum isclError_enum os_mkdir(const char *dirnm);

//#ifdef ISCL_LONG_NAMES
#define ISCL_os_path_exists(...) os_path_exists(__VA_ARGS__)
#define ISCL_os_path_isdir(...)  os_path_isdir(__VA_ARGS__)
#define ISCL_os_path_isfile(...) os_path_isfile(__VA_ARGS__)
#define ISCL_os_makedirs(...)    os_makedirs(__VA_ARGS__)
#define ISCL_os_mkdir(...)       os_mkdir(__VA_ARGS__)
//#endif

#ifdef __cplusplus
}
#endif
#endif /* __OS_OS_H__ */
