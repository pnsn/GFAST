#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"

/*!
 * @brief Initializes the GPS data streams
 *
 * @param[in] props      holds the GFAST properties
 *
 * @param[out] gps_data  this should have been pre-initialized to NULL.
 *                       on successful output holds the GPS data structure,
 *                       metadata such (station SNCL's, locations, instrument
 *                       sampling period) and requisite memory.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_data_initialize(struct GFAST_props_struct props,
                         struct GFAST_data_struct *gps_data)
{
    int ierr, k, mpts;
    // Get the sites to be used
    ierr = 0;
    if (props.verbose > 0){LOG_DEBUGMSG("%s", "Initializing metadata...");}
    ierr = core_data_readMetaDataFile(props.metaDataFile,
                                      props.metaDataNetworks,
                                      props.n_networks,
                                      gps_data);
    if (ierr != 0)
    {
        LOG_ERRMSG("Error reading sites file: %s", props.metaDataFile);
        return -1;
    }
    if (os_path_isfile(props.siteMaskFile))
    {
        ierr = core_data_readSiteMaskFile(props.siteMaskFile,
                                          props.verbose, gps_data);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error reading site mask file");
            return -1;
        }
    }
    // Set the buffer lengths
    for (k=0; k<gps_data->stream_length; k++)
    {
        mpts = (int) (props.bufflen/gps_data->data[k].dt + 0.5) + 1;
        gps_data->data[k].maxpts = mpts;
        gps_data->data[k].ubuff = memory_calloc64f(mpts);
        gps_data->data[k].nbuff = memory_calloc64f(mpts);
        gps_data->data[k].ebuff = memory_calloc64f(mpts);
        gps_data->data[k].tbuff = memory_calloc64f(mpts);
    }
    return 0;
}
