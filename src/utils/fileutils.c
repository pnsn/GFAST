/*file-related utilities*/
#include "fileutils.h"

/*!
 * Check if a file exist using stat() function
 * return 1 if the file exist otherwise return 0
 */
int cfileexists(const char* filename){
    struct stat sb;
    int exists = stat(filename,&sb);
    if(exists == 0)
        return 1;
    else // -1
        return 0;
}

/*!
 * Check if a directory exists using stat() function
 * return 1 if the file exist otherwise return 0
 */
int cdirexists(const char* dirname){
    struct stat sb;
    int exists = stat(dirname,&sb);
    if(exists == 0 && S_ISDIR(sb.st_mode))
        return 1;
    else // -1
        return 0;
}
