#ifndef FILEUTILS_H__
#define FILEUTILS_H__ 1
/*!
 * @defgroup fileutils
 * @brief file-related utilities.
 */

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" 
{
#endif


/*!
 * Check if a file exist using stat() function
 * return 1 if the file exist otherwise return 0
 */
int cfileexists(const char* filename);

/*!
 * Check if a directory exists using stat() function
 * return 1 if the file exist otherwise return 0
 */
int cdirexists(const char* dirname);

#ifdef __cplusplus
}
#endif
#endif /* fileutils_h__ */
