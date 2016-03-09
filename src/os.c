#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "gfast.h"

/*! 
 * @brief Tests if a path exists
 * 
 * @param[in] pathnm    name of path for which function tests existence 
 *
 * @result true -> path exists
 *         false -> path does not exist
 *
 * @author Ben Baker, ISTI
 *
 */
bool os_path_exists(char *pathnm)
{
    struct stat s;
    int err;
    if (pathnm == NULL){return false;}
    if (strlen(pathnm) == 0){return false;}
    err = stat(pathnm,&s);
    if (err == -1){
        return false;
    }else{ //Exists
        return true;
    }   
}
//============================================================================//
/*! 
 * @brief Tests if dirnm is a directory 
 * 
 * @param[in] dirnm    name of directory to test 
 *
 * @result true -> dirnm is an existing directory 
 *         false -> dirnm is not a directory
 * 
 */
bool os_path_isdir(char *dirnm)
{
    struct stat s;
    int err;
    if (dirnm == NULL){return false;}
    if (strlen(dirnm) == 0){return false;}
    err = stat(dirnm,&s);
    if (err == -1){
        if (ENOENT == errno) { //Doesn't exist
            return false;
        }else{ // It exists
            return true;
        }
    }else{ //Exists
        if (S_ISDIR(s.st_mode)){
            return true;
        }else{ //Exists but its a file
            return false;
        }
    }   
}
//============================================================================//
/*! 
 * @brief Tests if filenm is a file
 * 
 * @param[in] filenm    name of file to test 
 * 
 * @result  true  -> filenm is an existing file
 *          false -> filenm is not a file
 *
 * @author Ben Baker, ISTI
 *
 */
bool os_path_isfile(char *filenm)
{
    struct stat info;
    if (filenm == NULL){return false;}
    if (strlen(filenm) == 0){return false;}
    if (stat(filenm, &info) ==-1){ // Doesn't exist
        return false;
    }else{// Exists
        if (S_ISREG(info.st_mode)){
            return true;
        }else{
            return false;
        }
    }
}
//============================================================================//
/*!
 * @brief Recursive directory creation function
 *
 * @param[in] path    directory tree to make
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int os_makedirs(char *path)
{
    const char *fcnm = "os_makedirs\0";
    char *temp, dir[PATH_MAX];
    int i, ierr, lenos;
    // Early returns
    if (path == NULL){return -1;}
    if (strlen(path) == 0){return -1;}
    // Set workspace
    temp = (char *)calloc(strlen(path)+2, sizeof(char));
    strcpy(temp, path);
    lenos = strlen(temp);
    temp[lenos] = '/'; // Try to catch the final case
    // Loop and make a directory whenever we encounter a "/"
    for (i=1; i<lenos; i++){
        // Make a directory (provided it doesn't exists)
        if (temp[i] == '/'){
            memset(dir, 0, sizeof(dir));
            strncpy(dir, temp, i); 
            // Verify this isn't already a directory
            if (!os_path_isdir(dir)){
                // Make it
                ierr = os_mkdir(dir);
                if (ierr != 0){
                    log_errorF("%s: Error making subdirectory: %s\n",
                               fcnm, dir);
                    log_errorF("%s: Error making directory: %s\n",
                               fcnm, temp);
                    goto ERROR;
                }
            }
        }
    }
    // Ensure everything is done
    if (!os_path_isdir(temp)){
        ierr = os_mkdir(temp);
        if (ierr != 0){
            log_errorF("%s: Error making directory: %s\n", fcnm, temp);
        }
    }
ERROR:;
    free(temp);
    temp = NULL;
    return ierr;
}
//============================================================================//
/*! 
 * Makes a directory named dirnm with full permissions 
 *
 * @param[in] dirnm    name of directory to make
 *
 * @result 0 if success
 *
 * @author Ben Baker, ISTI
 *
 */
int os_mkdir(char *dirnm)
{
    const char *fcnm = "os_mkdir\0";
    int ierr;
    if (dirnm == NULL){return -1;}
    if (strlen(dirnm) == 0){return -1;}
    ierr = mkdir(dirnm, 0777);
    if (ierr != 0){ 
        log_errorF("%s: Error making directory: %s\n", fcnm, dirnm);
        return -1; 
    }   
    return 0;
}

