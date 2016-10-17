#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Closes the HDF5 file on h5trace
 *
 * @param[in,out] h5trace   on input holds the HDF5 file handle and
 *                          h5traceBuffer properties. 
 *                          on output h5tracebuffer information is reset
 *                          and the underlying HDF5 file with the traces
 *                          is closed.
 *
 * @result 0 indicates success
 * 
 */ 
int traceBuffer_h5_finalize(struct h5traceBuffer_struct *h5trace)
{
    const char *fcnm = "traceBuffer_h5_finalize\0";
    herr_t status;
    int i, ierr;
    //------------------------------------------------------------------------//
    ierr = 0;
    if (!h5trace->linit)
    {
        log_errorF("%s: Error h5trace never initialized\n", fcnm);
        ierr = 1;
        return ierr;
    }
    if (h5trace->ntraces > 0 && h5trace->traces != NULL)
    {
        for (i=0; i<h5trace->ntraces; i++)
        {
            if (h5trace->traces[i].groupName != NULL)
            {
                free(h5trace->traces[i].groupName);
            }
            if (h5trace->traces[i].metaGroupName != NULL)
            {
                free(h5trace->traces[i].metaGroupName);
            }
        }
        free(h5trace->traces);
    }
    if (h5trace->ndtGroups > 0)
    {
        for (i=0; i<h5trace->ndtGroups; i++)
        {
            free(h5trace->dtGroupName[i]);
        }
        free(h5trace->dtGroupName);
        ISCL_memory_free__int(&h5trace->dtPtr);
    }
    status = H5Fclose(h5trace->fileID);
    if (status != 0)
    {
        log_errorF("%s: Error closing file\n", fcnm);
        ierr = 1; 
    }
    memset(h5trace, 0, sizeof(struct h5traceBuffer_struct));
    return ierr;
}
