#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <limits.h>
#include <math.h>
#include "gfast.h"

/*!
 * @brief Sets the HDF5 archive filename
 *
 * @param[in] adir    archive directory.  if NULL then the archive will be
 *                    written to the current working directory
 * @param[in] evid    event ID
 *
 * @param[out] fname  name of HDF5 archive file
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int GFAST_HDF5__setFileName(const char *adir,
                            const char *evid, 
                            char fname[PATH_MAX])
{
    const char *fcnm = "GFAST_HDF5__setFileName\0";
    int ierr, lenos;
    ierr = 0;
    memset(fname, 0, PATH_MAX*sizeof(char));
    if (evid == NULL)
    {   
        log_errorF("%s: Error event ID cannot be NULL\n", fcnm);
        return -1; 
    }   
    if (strlen(evid) == 0)
    {   
        log_errorF("%s: Error evid is not defined\n", fcnm);
        return -1; 
    }   
    // Set the archive directory
    if (adir == NULL)
    {
        strcpy(fname, "./\0");
    }
    else
    {
        lenos = strlen(adir);
        if (lenos == 0)
        {
            strcpy(fname, "./\0");
        }
        else
        {
            // Require the directory exists - if not make it
            if (!os_path_isdir(adir))
            {
                ierr = os_makedirs(adir);
                if (ierr != 0)
                {
                    log_errorF("%s: Failed making directory %s\n", fcnm, adir);
                    return -1;
                }
            }
            strcpy(fname, adir);
            lenos = strlen(fname);
            // Add a slash
            if (fname[lenos-1] != '/'){strcat(fname, "/\0");}
        }
    }
    // Add the event id
    strcat(fname, evid);
    // Add the file name
    strcat(fname, "_archive.h5\0");
    return 0;
}
