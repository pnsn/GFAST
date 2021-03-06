#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_core.h"
#include "iscl/os/os.h"

/*!
 * @brief Prints the GFAST properties to the debug file.  Note, there is 
 *        no verbosity check.
 *
 * @param[in] props    GFAST properties structure to write to the debug file
 *
 * @author Ben Baker, ISTI
 *
 */
void core_properties_print(struct GFAST_props_struct props)
{
    const char *lspace = "    \0";
    LOG_DEBUGMSG("\n%s", "GFAST properties");
    if (props.opmode == OFFLINE)
    {
        LOG_DEBUGMSG("%s GFAST site metadata file %s", lspace,
                     props.metaDataFile);  
        LOG_DEBUGMSG("%s GFAST is operating in offline mode", lspace);
        LOG_DEBUGMSG("%s GFAST default sampling period is %f (s)", lspace,
                     props.dt_default);
        if (props.dt_init == INIT_DT_FROM_FILE)
        {
            LOG_DEBUGMSG("%s GFAST will get sampling period from file %s",
                         lspace, props.metaDataFile); 
        }
        else if (props.dt_init == INIT_DT_FROM_DEFAULT)
        {
            LOG_DEBUGMSG("%s GFAST will set GPS sampling periods to %f (s)",
                         lspace, props.dt_default); 
        }
        else if (props.dt_init == INIT_DT_FROM_SAC)
        {
            LOG_DEBUGMSG("%s GFAST will set GPS sampling period from SAC files",
                         lspace);
        }
        if (props.loc_init == INIT_LOCS_FROM_FILE)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from file %s",
                         lspace, props.metaDataFile);
        }
        else if (props.loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from tracebuf",
                         lspace);
        }
        else if (props.loc_init == INIT_LOCS_FROM_SAC)
        {
            LOG_DEBUGMSG("%s GFAST will initialize locations from SAC files",
                         lspace);
        }
        LOG_DEBUGMSG("%s GFAST simulation time (s) %f",
                     lspace, props.synthetic_runtime);
        LOG_DEBUGMSG("%s GFAST observed data directory: %s",
                   lspace, props.obsdataDir);
        LOG_DEBUGMSG("%s GFAST observed data file: %s",
                   lspace, props.obsdataFile);
    }
    else if (props.opmode == PLAYBACK)
    {
        LOG_DEBUGMSG("%s GFAST is operating in playback mode", lspace);
    }
    else if (props.opmode == REAL_TIME_EEW)
    {
        LOG_DEBUGMSG("%s GFAST is operating in EEW real-time mode", lspace);
        LOG_DEBUGMSG("%s GFAST time between iterations is %f (s)", lspace,
                     props.waitTime);
        LOG_DEBUGMSG("%s GFAST site position file %s", lspace,
                     props.metaDataFile);
        LOG_DEBUGMSG("%s GFAST default sampling period is %f (s)", lspace,
                   props.dt_default);
        if (props.opmode == REAL_TIME_EEW)
        {
            LOG_DEBUGMSG("%s GFAST host name: %s", lspace,
                         props.activeMQ_props.host);
            LOG_DEBUGMSG("%s GFAST origin topic: %s",
                         lspace, props.activeMQ_props.originTopic);
            LOG_DEBUGMSG("%s GFAST destination topic: %s",
                         lspace, props.activeMQ_props.destinationTopic);
            LOG_DEBUGMSG("%s Will connect to port %d", lspace,
                         props.activeMQ_props.port);
            if (props.activeMQ_props.maxAttempts > 0)
            {
                LOG_DEBUGMSG("%s Milliseconds before reconnect %d",
                             lspace, props.activeMQ_props.msReconnect); 
                LOG_DEBUGMSG("%s Max number of attempts to connect %d",
                             lspace, props.activeMQ_props.maxAttempts);
            }
            LOG_DEBUGMSG("%s Will wait %d milliseconds for a message",
                         lspace, props.activeMQ_props.msWaitForMessage);
        }
    }
    LOG_DEBUGMSG("%s GFAST buffer length is %f seconds", lspace, props.bufflen);
    if (props.utm_zone ==-12345)
    {
        LOG_DEBUGMSG("%s GFAST will get UTM zone from hypocenters", lspace);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will set UTM zone to %d", lspace,
                   props.utm_zone);
    }
    LOG_DEBUGMSG("%s GFAST verbosity level is %d", lspace, props.verbose);
    LOG_DEBUGMSG("%s GFAST stream file: %s", lspace, props.metaDataFile);
    if (os_path_isfile(props.siteMaskFile))
    {
        LOG_DEBUGMSG("%s GFAST will use site maskfile: %s", lspace,
                     props.siteMaskFile);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will not mask any sites", lspace);
    } 
    LOG_DEBUGMSG("%s GFAST HDF5 archive dir: %s", lspace, props.h5ArchiveDir);
    LOG_DEBUGMSG("%s GFAST will finish processing an event after %f (s)",
               lspace, props.processingTime);
    LOG_DEBUGMSG("%s GFAST will use a default earthquake depth of %f",
               lspace, props.eqDefaultDepth);
    if (props.lh5SummaryOnly)
    {
        LOG_DEBUGMSG("%s GFAST will only write an HDF5 summary", lspace);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will save all steps to HDF5", lspace);
    }
    //--------------------------------pgd-------------------------------------//
    LOG_DEBUGMSG("%s GFAST PGD source receiver distance tolerance %f (km)",
               lspace, props.pgd_props.dist_tol);
    LOG_DEBUGMSG("%s GFAST PGD default distance %f (cm)",
               lspace, props.pgd_props.disp_def);
    LOG_DEBUGMSG("%s GFAST Number of PGD grid search latitudes %d",
               lspace, props.pgd_props.ngridSearch_lats);
    LOG_DEBUGMSG("%s GFAST Number of PGD grid search longitudes %d",
               lspace, props.pgd_props.ngridSearch_lons);
    LOG_DEBUGMSG("%s GFAST Number of PGD grid search depths is %d",
               lspace, props.pgd_props.ngridSearch_deps);
    LOG_DEBUGMSG("%s GFAST PGD data selection velocity is %f (km/s)",
               lspace, props.pgd_props.window_vel);
    LOG_DEBUGMSG("%s GFAST Number of sites required to compute PGD is %d",
               lspace, props.pgd_props.min_sites);
    if (props.pgd_props.ngridSearch_lats > 1)
    {
        LOG_DEBUGMSG("%s GFAST PGD latitude grid spacing %f",
                   lspace, props.pgd_props.dLat); 
    }
    if (props.pgd_props.ngridSearch_lons > 1)
    {
        LOG_DEBUGMSG("%s GFAST PGD longitude grid spacing %f",
                   lspace, props.pgd_props.dLon);
    }
    //--------------------------------cmt-------------------------------------//
    LOG_DEBUGMSG("%s GFAST Number of latitudes in CMT grid search %d",
               lspace, props.cmt_props.ngridSearch_lats);
    LOG_DEBUGMSG("%s GFAST Number of longitudes in CMT grid search %d",
               lspace, props.cmt_props.ngridSearch_lons);
    LOG_DEBUGMSG("%s GFAST Number of depths in CMT grid search  %d",
               lspace, props.cmt_props.ngridSearch_deps);
    LOG_DEBUGMSG("%s GFAST CMT data selection velocity is %f (km/s)",
               lspace, props.cmt_props.window_vel);
    LOG_DEBUGMSG("%s GFAST CMT data averaging window length %f (s)",
               lspace, props.cmt_props.window_avg);
    LOG_DEBUGMSG("%s GFAST Number of sites required to compute CMT is %d",
               lspace, props.cmt_props.min_sites); 
    if (props.cmt_props.ldeviatoric)
    {
        LOG_DEBUGMSG("%s GFAST will apply deviatoric constraint to CMT",
                   lspace);
    }
    else
    {
        LOG_DEBUGMSG("%s GFAST will invert for all 6 MT terms", lspace);
    }
    if (props.cmt_props.ngridSearch_lats > 1)
    {   
        LOG_DEBUGMSG("%s GFAST CMT latitude grid spacing %f",
                   lspace, props.cmt_props.dLat); 
    }   
    if (props.cmt_props.ngridSearch_lons > 1)
    {   
        LOG_DEBUGMSG("%s GFAST CMT longitude grid spacing %f",
                   lspace, props.cmt_props.dLon);
    }
    //--------------------------------ff--------------------------------------//
    LOG_DEBUGMSG("%s GFAST will use %d fault patches along strike",
                lspace, props.ff_props.nstr);
    LOG_DEBUGMSG("%s GFAST will use %d fault patches down-dip",
                lspace, props.ff_props.ndip);
    LOG_DEBUGMSG("%s GFAST Number of sites required to compute FF is %d",
                lspace, props.ff_props.min_sites);
    LOG_DEBUGMSG("%s GFAST finite fault data selection velocity is %f (km/s)",
                lspace, props.ff_props.window_vel);
    LOG_DEBUGMSG("%s GFAST finite fault data averaging window length %f (s)",
                lspace, props.ff_props.window_avg);
    LOG_DEBUGMSG("%s GFAST fault length safety factor %.2f pct",
                lspace, props.ff_props.flen_pct);
    LOG_DEBUGMSG("%s GFAST fault width safety factor %.2f pct",
                lspace, props.ff_props.fwid_pct); 
    LOG_DEBUGMSG("%s", "\n");
    return;
}
