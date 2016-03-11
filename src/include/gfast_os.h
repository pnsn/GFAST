#include <stdbool.h>
#ifndef __GFAST_OS_OS_H__
#define __GFAST_OS_OS_H__
#ifdef __cplusplus
extern "C" 
{
#endif
/* Determines if pathnm exists */
bool os_path_exists(char *pathnm);
/* Determines if dirnm is an existing directory */
bool os_path_isdir(char *dirnm);
/* Determines if filenm is an existing file */
bool os_path_isfile(char *filenm);
/* Recursively make directories */
int os_makedirs(char *path);
/* Makes a directory */
int os_mkdir(char *dirnm);
#ifdef __cplusplus
}
#endif
#endif /* __GFAST_OS_OS_H__ */
