#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "iscl/os/os.h"
#include "iscl/log/log.h"
#ifdef USE_AMQ
#include "gfast_activeMQ.h"
#endif

/*!
 * @brief Initializes the GFAST properties (parameter) structure
 *
 * @param[in] propfilename   Name of properties file.
 * @param[in] opmode         GFAST operational mode.  Can be OFFLINE
 *                           for playback or REAL_TIME_EEW for real-time
 *                           earthquake early warning.
 *
 * @param[out] props         On successful exit holds the GFAST properties
 *                           structure.
 *
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int core_properties_initialize(const char *propfilename,
                               const enum opmode_type opmode,
                               struct GFAST_props_struct *props)
{
    const char *fcnm = "core_properties_initialize\0";
    const char *s;
    char cwork[PATH_MAX];
    int i, ierr, itemp, lenos;
    dictionary *ini;
    //------------------------------------------------------------------------//
    // Require the properties file exists
    ierr =-1;
    memset(props, 0, sizeof(struct GFAST_props_struct));
    props->opmode = opmode;
    if (!ISCL_os_path_isfile(propfilename))
    {
        log_errorF("%s: Properties file: %s does not exist\n",
                   fcnm, propfilename);
        return ierr;
    }
    // Load the ini file
    ierr = 1;
    ini = iniparser_load(propfilename);
    strcpy(props->propfilename, propfilename);
    //-------------------------GFAST General Parameters-----------------------//
    s = iniparser_getstring(ini, "general:metaDataFile\0",
                            "GFAST_streams.txt\0");
    strcpy(props->metaDataFile, s);
    if (!ISCL_os_path_isfile(props->metaDataFile))
    {
        log_errorF("%s: Cannot find station list %s\n",
                   fcnm, props->metaDataFile);
        return -1;
    }
    s = iniparser_getstring(ini, "general:siteMaskFile\0", NULL);
    if (s != NULL)
    {
        strcpy(props->siteMaskFile, s);
        if (!os_path_isfile(props->siteMaskFile))
        {
            memset(props->siteMaskFile, 0, sizeof(props->siteMaskFile));
        }
    }
    props->bufflen = iniparser_getdouble(ini, "general:bufflen\0", 1800.0);
    if (props->bufflen <= 0.0)
    {
        log_errorF("%s: Buffer lengths must be positive!\n", fcnm);
        goto ERROR;
    }
    if (props->opmode == OFFLINE)
    {
        s = iniparser_getstring(ini, "general:eewsfile\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find decision module XML file!\n", fcnm);
            goto ERROR;
        }
        strcpy(props->eewsfile, s);
        s = iniparser_getstring(ini, "general:observed_data_dir\0", NULL);
        if (s != NULL)
        {
            strcpy(props->obsdataDir, s);
            if (!ISCL_os_path_isdir(props->obsdataDir))
            {
                log_errorF("%s: Observed data directory %s doesn't exist\n",
                           fcnm, props->obsdataDir);
                goto ERROR; 
            }
            if (strlen(props->obsdataDir) == 0)
            {
                strcpy(props->obsdataDir, "./\0");
            }
            else
            {
                if (props->obsdataDir[strlen(props->obsdataDir)-1] != '/')
                {
                    strcat(props->obsdataDir, "/\0");
                }
            }
        }
        else
        {
            strcpy(props->obsdataDir, "./\0");
        }
        s = iniparser_getstring(ini, "general:observed_data_file\0", NULL);
        if (s != NULL)
        {
            strcpy(props->obsdataFile, s);
            memset(cwork, 0, sizeof(cwork));
            strcpy(cwork, props->obsdataDir);
            strcat(cwork, props->obsdataFile);
            if (!ISCL_os_path_isfile(cwork))
            {
                log_errorF("%s: Observed data file %s doesn't exist\n",
                           fcnm, cwork);
                goto ERROR;
            }
        }
        else
        {
            log_errorF("%s: Must specify observed data file!\n", fcnm);
        }
        s = iniparser_getstring(ini, "general:synthetic_data_prefix\0", "LX\0");
    }
    // UTM zone
    props->utm_zone = iniparser_getint(ini, "general:utm_zone\0", -12345);
    if (props->utm_zone < 0 || props->utm_zone > 60)
    {
        if (props->utm_zone !=-12345)
        {
            log_warnF("%s: UTM zone %d is invalid estimating from hypocenter\n",
                   fcnm, props->utm_zone);
            props->utm_zone =-12345;
        } 
    }
    // Verbosity
    props->verbose = iniparser_getint(ini, "general:verbose\0", 2);
    // Sampling period
    props->dt_default = iniparser_getdouble(ini, "general:dt_default\0", 1.0);
    if (props->dt_default <= 0.0)
    {
        log_warnF("%s: Default sampling period %f invalid; defaulting to %f!\n",
                  fcnm, props->dt_init, 1.0);
        props->dt_default = 1.0; 
    }
    itemp = iniparser_getint(ini, "general:dt_init\0", 3);
    props->dt_init = (enum dtinit_type) itemp; //iniparser_getint(ini, "general:dt_init\0", 3);
    if (props->opmode != OFFLINE)
    {
        if (props->dt_init != INIT_DT_FROM_TRACEBUF){
            log_warnF("%s: Obtaining sampling period from tracebuf\n", fcnm);
            props->dt_init = 3;
        }
    }
    if (props->opmode == OFFLINE)
    {
        // Make sure the EEW XML file exists
        if (!ISCL_os_path_isfile(props->eewsfile))
        {
            log_errorF("%s: Cannot find EEW XML file!\n",
                       fcnm, props->eewsfile);
            goto ERROR;
        }
        // Figure out how to initialize sampling period
        if (props->dt_init == INIT_DT_FROM_FILE)
        {
            itemp = iniparser_getint(ini, "general:dt_init\0", 1);
            props->dt_init = (enum dtinit_type) itemp;
            if (s == NULL)
            {
                log_errorF("%s: Must specify metaDataFile!\n", fcnm);
                goto ERROR; 
            }
        }
        else if (props->dt_init == INIT_DT_FROM_SAC)
        {
            props->dt_init = INIT_DT_FROM_SAC;
        }
        else
        {
            if (props->dt_init != INIT_DT_FROM_DEFAULT)
            {
                log_warnF("%s: Setting dt from default\n", fcnm);
                props->dt_init = INIT_DT_FROM_DEFAULT;
            }
        }
    }
    else
    {
        if (props->dt_init != INIT_DT_FROM_TRACEBUF)
        {
            log_warnF("%s: Will get GPS sampling period from trace buffer!\n",
                      fcnm);
            props->dt_init = INIT_DT_FROM_TRACEBUF;
        }
    }
    // Wait time
    props->waitTime = 1.0;
    if (props->opmode == REAL_TIME_EEW)
    {
        props->waitTime = iniparser_getdouble(ini, "general:waitTime\0", 1.0);
        if (props->waitTime < 0.0)
        {
            log_errorF("%s: Invalid wait time %f!\n", fcnm, props->waitTime);
            goto ERROR;
        } 
    }        
    // Location initialization
    itemp = iniparser_getint(ini, "general:loc_init\0", 1);
    props->loc_init = (enum locinit_type) itemp;
    if (props->opmode == OFFLINE)
    {
        if (props->loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
            log_errorF("%s: offline can't initialize locations from tracebuf\n",
                       fcnm);
            goto ERROR;
        }
    }
    // Synthetic runtime
    if (props->opmode == OFFLINE)
    {
        props->synthetic_runtime
           = iniparser_getdouble(ini, "general:synthetic_runtime\0", 0.0); 
    } 
    // Processing time
    props->processingTime
       = iniparser_getdouble(ini, "general:processing_time\0", 300.0);
    if (props->processingTime > props->bufflen)
    {
        log_errorF("%s: Error processing time cannot exceed buffer length\n",
                   fcnm);
        goto ERROR;
    }
    // Default earthquake depth
    props->eqDefaultDepth
        = iniparser_getdouble(ini, "general:default_event_depth\0", 8.0);
    if (props->eqDefaultDepth < 0.0)
    {
        log_errorF("%s: Error default earthquake depth must be positive %f\n",
                   fcnm, props->eqDefaultDepth);
        goto ERROR;
    }
    // H5 archive directory
    s = iniparser_getstring(ini, "general:h5ArchiveDirectory\0", NULL);
    if (s == NULL)
    {
        strcpy(props->h5ArchiveDir, "./\0");
    }
    else
    {
        strcpy(props->h5ArchiveDir, s);
        if (!ISCL_os_path_isdir(props->h5ArchiveDir))
        {
            log_warnF("%s: Archive directory %s doesn't exist\n",
                       fcnm, props->h5ArchiveDir);
            log_warnF("%s: Will use current working directory\n", fcnm);
            memset(props->h5ArchiveDir, 0, sizeof(props->h5ArchiveDir));
            strcpy(props->h5ArchiveDir, "./\0");
        }
    }
    // Only write summary HDF5 files?
    props->lh5SummaryOnly = iniparser_getboolean(ini, "general:H5SummaryOnly\0",
                                                 false);
    // ANSS informaiton
    s = iniparser_getstring(ini, "general:anssNetwork\0", "UW\0");
    strcpy(props->anssNetwork, s);
    lenos = (int) (strlen(props->anssNetwork));
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
    for (i=0; i<lenos; i++)
    {
        putchar(props->anssNetwork[i]);
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    s = iniparser_getstring(ini, "general:anssDomain\0", "anss.org\0"); 
    strcpy(props->anssDomain, s);
    //---------------------------Earthworm parameters-------------------------//
    if (props->opmode == REAL_TIME_EEW)
    {
        s = iniparser_getstring(ini, "earthworm:gpsRingName\0", "WAVE_RING\0");
        strcpy(props->ew_props.gpsRingName, s);
        if (strlen(props->ew_props.gpsRingName) < 1)
        {
            log_warnF("%s: GPS ring name may not be specified\n", fcnm);
        }
        s = iniparser_getstring(ini, "earthworm:moduleName\0", "geojson2ew\0");
        strcpy(props->ew_props.moduleName, s);
        if (strlen(props->ew_props.moduleName) < 1)
        {
            log_errorF("%s: Module name is not specified\n", fcnm);
            goto ERROR;
        }
    }
    // Free the ini file
    iniparser_freedict(ini);
    //------------------------------PGD Parameters----------------------------//
    ierr = core_scaling_pgd_readIni(propfilename, "PGD\0",
                                    props->verbose, props->utm_zone,
                                    &props->pgd_props);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading PGD parameters\n", fcnm);
        goto ERROR;
    }
    //----------------------------CMT Parameters------------------------------//
    ierr = core_cmt_readIni(propfilename, "CMT\0",
                            props->verbose, props->utm_zone,
                            &props->cmt_props); 
    if (ierr != 0)
    {
        log_errorF("%s: Error reading CMT parameters\n", fcnm);
        goto ERROR;
    } 
    //------------------------------FF Parameters-----------------------------//
    ierr = core_ff_readIni(propfilename, "FF\0",
                           props->verbose, props->utm_zone,
                           props->cmt_props.min_sites,
                           &props->ff_props);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading FF parameters\n", fcnm);
        goto ERROR; 
    }
    //---------------------------ActiveMQ Parameters--------------------------//
#ifdef USE_AMQ
    if (props->opmode == REAL_TIME_EEW) 
    {
        ierr = activeMQ_readIni(propfilename, "ActiveMQ",
                                &props->activeMQ_props);
        if (ierr != 0)
        {
            log_errorF("%s: Error reading FF parameters\n", fcnm);
            goto ERROR; 
        }
    } // End check on need for ActiveMQ
#endif
    // Success!
    ierr = 0;
    return ierr;
ERROR:;
    // Free the ini file
    iniparser_freedict(ini);
    return ierr;
}
