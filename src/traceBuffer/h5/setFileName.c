#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"

/*!
 * @brief Sets the HDF5 trace buffer file name
 *
 * @param[in]h5dir     Name of directory where HDF5 is located or will
 *                     reside.  If null then the current working directory
 *                     will be used.
 * @param[in] h5file   Name of the HDF5 file.  This should have the .h5
 *                     already appended.
 *
 * @result the full path to the HDF5 file
 *
 */
int traceBuffer_h5_setFileName(const char *h5dir,
                               const char *h5file,
                               char h5name[PATH_MAX])
{
    const char *fcnm = "traceBuffer_h5_setFileName\0";
    unsigned long lend;
    memset(h5name, 0, PATH_MAX*sizeof(char));
    if (h5file == NULL)
    {
        log_errorF("%s: Error h5file cannot be NULL\n", fcnm);
        return -1;
    }
    if (strlen(h5file) == 0)
    {
        log_errorF("%s: Error h5file cannot be blank\n", fcnm);
        return -1;
    }
    // Set the directory name
    if (h5dir == NULL)
    {
        strcpy(h5name, "./\0");
    }
    else
    {
        lend = strlen(h5dir);
        if (lend == 0)
        {
            strcpy(h5name, "./\0");
        }
        else
        {
            if (!os_path_isdir(h5dir))
            {
                log_errorF("%s: Directory %s does not exist\n", fcnm, h5dir);
                return -1;
            }
            strcpy(h5name, h5dir);
            if (h5name[lend-1] != '/'){strcat(h5name, "/\0");}
         }
    }
    // Append the file name
    strcat(h5name, h5file);
    return 0;
}
