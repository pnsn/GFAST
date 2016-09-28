#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iniparser.h>
#include "gfast_core.h"
#include "iscl/os/os.h"
#include "iscl/log/log.h"

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
    const char *fcnm = "core_properties_print\0";
    const char *lspace = "    \0";
    log_debugF("\n%s: GFAST properties\n", fcnm);
    if (props.opmode == OFFLINE)
    {
        log_debugF("%s GFAST site metadata file %s\n", lspace,
                   props.metaDataFile);  
        log_debugF("%s GFAST is operating in offline mode\n", lspace);
        log_debugF("%s GFAST default sampling period is %f (s)\n", lspace,
                   props.dt_default);
        if (props.dt_init == INIT_DT_FROM_FILE)
        {
            log_debugF("%s GFAST will get sampling period from file %s\n",
                       lspace, props.metaDataFile); 
        }
        else if (props.dt_init == INIT_DT_FROM_DEFAULT)
        {
            log_debugF("%s GFAST will set GPS sampling periods to %f (s)\n",
                       lspace, props.dt_default); 
        }
        else if (props.dt_init == INIT_DT_FROM_SAC)
        {
            log_debugF("%s GFAST will set GPS sampling period from SAC files\n",
                       lspace);
        }
        if (props.loc_init == INIT_LOCS_FROM_FILE)
        {
            log_debugF("%s GFAST will initialize locations from file %s\n",
                       lspace, props.metaDataFile);
        }
        else if (props.loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
            log_debugF("%s GFAST will initialize locations from tracebuf\n",
                       lspace);
        }
        else if (props.loc_init == INIT_LOCS_FROM_SAC)
        {
            log_debugF("%s GFAST will initialize locations from SAC files\n",
                       lspace);
        }
        log_debugF("%s GFAST simulation time (s) %f\n",
                   lspace, props.synthetic_runtime);
        log_debugF("%s GFAST observed data file: %s\n",
                   lspace, props.obsdata_file);
    }
    else if (props.opmode == PLAYBACK)
    {
        log_debugF("%s GFAST is operating in playback mode\n", lspace);
    }
    else if (props.opmode == REAL_TIME_EEW)
    {
        log_debugF("%s GFAST is operating in EEW real-time mode\n", lspace);
        log_debugF("%s GFAST time between iterations is %f (s)\n", lspace,
                   props.waitTime);
        log_debugF("%s GFAST site position file %s\n", lspace,
                   props.metaDataFile);
        log_debugF("%s GFAST default sampling period is %f (s)\n", lspace,
                   props.dt_default);
        if (props.opmode == REAL_TIME_EEW)
        {
            log_debugF("%s GFAST host name: %s\n", lspace,
                       props.activeMQ_props.host);
            log_debugF("%s GFAST origin topic: %s\n",
                       lspace, props.activeMQ_props.originTopic);
            log_debugF("%s GFAST destination topic: %s\n",
                       lspace, props.activeMQ_props.destinationTopic);
            log_debugF("%s Will connect to port %d\n", lspace,
                       props.activeMQ_props.port);
            if (props.activeMQ_props.maxAttempts > 0)
            {
                log_debugF("%s Milliseconds before reconnect %d\n",
                           lspace, props.activeMQ_props.msReconnect); 
                log_debugF("%s Max number of attempts to connect %d\n",
                           lspace, props.activeMQ_props.maxAttempts);
            }
            log_debugF("%s Will wait %d milliseconds for a message\n",
                       lspace, props.activeMQ_props.msWaitForMessage);
        }
    }
    log_debugF("%s GFAST buffer length is %f seconds\n", lspace, props.bufflen);
    if (props.utm_zone ==-12345)
    {
        log_debugF("%s GFAST will get UTM zone from hypocenters\n", lspace);
    }
    else
    {
        log_debugF("%s GFAST will set UTM zone to %d\n", lspace,
                   props.utm_zone);
    }
    log_debugF("%s GFAST verbosity level is %d\n", lspace, props.verbose);
    log_debugF("%s GFAST stream file: %s\n", lspace, props.metaDataFile);
    if (os_path_isfile(props.siteMaskFile))
    {
        log_debugF("%s GFAST will use site maskfile: %s\n", lspace,
                   props.siteMaskFile);
    }
    else
    {
        log_debugF("%s GFAST will not mask any sites\n", lspace);
    } 
    log_debugF("%s GFAST HDF5 archive dir: %s\n", lspace, props.h5ArchiveDir);
    log_debugF("%s GFAST will finish processing an event after %f (s)\n",
               lspace, props.processingTime);
    log_debugF("%s GFAST will use a default earthquake depth of %f\n",
               lspace, props.eqDefaultDepth);
    if (props.lh5SummaryOnly)
    {
        log_debugF("%s GFAST will only write an HDF5 summary\n", lspace);
    }
    else
    {
        log_debugF("%s GFAST will save all steps to HDF5\n", lspace);
    }
    //--------------------------------pgd-------------------------------------//
    log_debugF("%s GFAST PGD source receiver distance tolerance %f (km)\n",
               lspace, props.pgd_props.dist_tol);
    log_debugF("%s GFAST PGD default distance %f (cm)\n",
               lspace, props.pgd_props.disp_def);
    log_debugF("%s GFAST Number of PGD grid search depths is %d\n",
               lspace, props.pgd_props.ngridSearch_deps);
    log_debugF("%s GFAST PGD data selection velocity is %f (km/s)\n",
               lspace, props.pgd_props.window_vel);
    log_debugF("%s GFAST Number of sites required to compute PGD is %d\n",
               lspace, props.pgd_props.min_sites);
    //--------------------------------cmt-------------------------------------//
    log_debugF("%s GFAST Number of CMT grid search depths is %d\n",
               lspace, props.cmt_props.ngridSearch_deps);
    log_debugF("%s GFAST CMT data selection velocity is %f (km/s)\n",
               lspace, props.cmt_props.window_vel);
    log_debugF("%s GFAST CMT data averaging window length %f (s)\n",
               lspace, props.cmt_props.window_avg);
    log_debugF("%s GFAST Number of sites required to compute CMT is %d\n",
               lspace, props.cmt_props.min_sites); 
    if (props.cmt_props.ldeviatoric)
    {
        log_debugF("%s GFAST will apply deviatoric constraint to CMT\n",
                   lspace);
    }
    else
    {
        log_debugF("%s GFAST will invert for all 6 MT terms\n", lspace);
    }
    //--------------------------------ff--------------------------------------//
    log_debugF("%s GFAST will use %d fault patches along strike\n",
               lspace, props.ff_props.nstr);
    log_debugF("%s GFAST will use %d fault patches down-dip\n",
               lspace, props.ff_props.ndip);
    log_debugF("%s GFAST Number of sites required to compute FF is %d\n",
               lspace, props.ff_props.min_sites);
    log_debugF("%s GFAST finite fault data selection velocity is %f (km/s)\n",
               lspace, props.ff_props.window_vel);
    log_debugF("%s GFAST finite fault data averaging window length %f (s)\n",
               lspace, props.ff_props.window_avg);
    log_debugF("%s GFAST fault length safety factor %.2f %\n",
               lspace, props.ff_props.flen_pct);
    log_debugF("%s GFAST fault width safety factor %.2f %\n",
               lspace, props.ff_props.fwid_pct); 
    log_debugF("\n");
    return;
}
