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
    //------------------------------PGD Parameters----------------------------//
    props->pgd_props.verbose = props->verbose;
    props->pgd_props.utm_zone = props->utm_zone;
    props->pgd_props.dist_tol = iniparser_getdouble(ini, "PGD:dist_tolerance\0",
                                                    6.0);
    if (props->pgd_props.dist_tol < 0.0)
    {
        log_errorF("%s: Error ndistance tolerance %f cannot be negative\n",
                   fcnm, props->pgd_props.dist_tol);
        goto ERROR;
    }
    props->pgd_props.disp_def = iniparser_getdouble(ini, "PGD:disp_default\0",
                                                    0.01);
    if (props->pgd_props.disp_def <= 0.0)
    {
        log_errorF("%s: Error PGD distance default %f must be positive\n",
                   fcnm, props->pgd_props.disp_def);
        goto ERROR;
    }
    props->pgd_props.dLat
         = iniparser_getdouble(ini, "PGD:deltaLatitude\0", 0.1);
    if (props->pgd_props.dLat < 0.0)
    {
        log_errorF("%s: Error PGD latitude serach %f must be positive\n",
                   fcnm, props->pgd_props.dLat);
        goto ERROR;
    }
    props->pgd_props.dLon
         = iniparser_getdouble(ini, "PGD:deltaLongitude\0", 0.1);
    if (props->pgd_props.dLon < 0.0)
    {
        log_errorF("%s: Error PGD longitudes %f must be positive\n",
                   fcnm, props->pgd_props.dLon);
        goto ERROR;
    }
    props->pgd_props.ngridSearch_lats
         = iniparser_getint(ini, "PGD:nlats_in_pgd_gridSearch\0", 1);
    if (props->pgd_props.ngridSearch_lats < 1)
    {
        log_errorF("%s: Error PGD grid search depths %d must be positive\n",
                   fcnm, props->pgd_props.ngridSearch_lats);
        goto ERROR;
    }
    if ((int) (fmod((double) props->pgd_props.ngridSearch_lats, 2.0)) == 0)
    {
        log_warnF("%s: Adding 1 point to CMT lat gridsearch\n", fcnm);
        props->pgd_props.ngridSearch_lats
           = props->pgd_props.ngridSearch_lats + 1;
    }
    props->pgd_props.ngridSearch_lons
         = iniparser_getint(ini, "PGD:nlons_in_pgd_gridSearch\0", 1);
    if (props->pgd_props.ngridSearch_lons < 1)
    {
        log_errorF("%s: Error PGD grid search depths %d must be positive\n",
                   fcnm, props->pgd_props.ngridSearch_lons);
        goto ERROR;
    }
    if ((int) (fmod((double) props->pgd_props.ngridSearch_lons, 2.0)) == 0)
    {
        log_warnF("%s: Adding 1 point to CMT lat gridsearch\n", fcnm);
        props->pgd_props.ngridSearch_lons
           = props->pgd_props.ngridSearch_lons + 1;
    }
    props->pgd_props.ngridSearch_deps
         = iniparser_getint(ini, "PGD:ndepths_in_pgd_gridSearch\0", 100);
    if (props->pgd_props.ngridSearch_deps < 1)
    {
        log_errorF("%s: Error PGD grid search depths %d must be positive\n",
                   fcnm, props->pgd_props.ngridSearch_deps);
        goto ERROR;
    } 
    props->pgd_props.window_vel = iniparser_getdouble(ini,
                                                "PGD:pgd_window_vel\0", 3.0);
    if (props->pgd_props.window_vel <= 0.0){
        log_errorF("%s: Error window velocity must be positive!\n", fcnm);
        goto ERROR;
    }
    props->pgd_props.min_sites = iniparser_getint(ini, "PGD:pgd_min_sites\0",
                                                  4);
    if (props->pgd_props.min_sites < 1)
    {
        log_errorF("%s: Error at least one site needed to estimate PGD!\n", 
                   fcnm);
        goto ERROR;
    }
    //----------------------------CMT Parameters------------------------------//
    props->cmt_props.verbose = props->verbose;
    props->cmt_props.utm_zone = props->utm_zone;
    props->cmt_props.dLat
         = iniparser_getdouble(ini, "CMT:deltaLatitude\0", 0.1);
    if (props->cmt_props.dLat < 0.0)
    {   
        log_errorF("%s: Error CMT latitude serach %f must be positive\n",
                   fcnm, props->cmt_props.dLat);
        goto ERROR;
    }
    props->cmt_props.dLon
         = iniparser_getdouble(ini, "CMT:deltaLongitude\0", 0.1);
    if (props->cmt_props.dLon < 0.0)
    {   
        log_errorF("%s: Error CMT longitudes %f must be positive\n",
                   fcnm, props->cmt_props.dLon);
        goto ERROR;
    }
    props->cmt_props.ngridSearch_lats
         = iniparser_getint(ini, "CMT:nlats_in_cmt_gridSearch\0", 1);
    if (props->cmt_props.ngridSearch_lats < 1)
    {
        log_errorF("%s: Error CMT grid search lats %d must be positive\n",
                   fcnm, props->cmt_props.ngridSearch_lats);
        goto ERROR;
    }
    if ((int) (fmod((double) props->cmt_props.ngridSearch_lats, 2.0)) == 0)
    {   
        log_warnF("%s: Adding 1 point to CMT lat gridsearch\n", fcnm);
        props->cmt_props.ngridSearch_lats
           = props->cmt_props.ngridSearch_lats + 1;
    }
    props->cmt_props.ngridSearch_lons
         = iniparser_getint(ini, "CMT:nlons_in_cmt_gridSearch\0", 1);
    if (props->cmt_props.ngridSearch_lons < 1)
    {
        log_errorF("%s: Error CMT grid search lons %d must be positive\n",
                   fcnm, props->cmt_props.ngridSearch_lons);
        goto ERROR;
    }
    if ((int) (fmod((double) props->cmt_props.ngridSearch_lons, 2.0)) == 0)
    {
        log_warnF("%s: Adding 1 point to CMT lon gridsearch\n", fcnm);
        props->cmt_props.ngridSearch_lons
           = props->cmt_props.ngridSearch_lons + 1;
    }
    props->cmt_props.ngridSearch_deps
         = iniparser_getint(ini, "CMT:ndepths_in_cmt_gridSearch\0", 100);
    if (props->cmt_props.ngridSearch_deps < 1)
    {
        log_errorF("%s: Error CMT grid search depths %d must be positive\n",
                   fcnm, props->cmt_props.ngridSearch_deps);
        goto ERROR;
    }
    props->cmt_props.min_sites
        = iniparser_getint(ini, "CMT:cmt_min_sites\0", 4);
    if (props->cmt_props.min_sites < 3)
    {
        log_errorF("%s: Error at least two sites needed to estimate CMT!\n",
                   fcnm);
        goto ERROR;
    }
    props->cmt_props.window_vel
        = iniparser_getdouble(ini, "CMT:cmt_window_vel\0", 2.0);
    if (props->cmt_props.window_vel <= 0.0)
    {
        log_errorF("%s: Error window velocity must be positive!\n", fcnm);
        goto ERROR;
    }
    props->cmt_props.window_avg
        = iniparser_getdouble(ini, "CMT:cmt_window_avg\0", 0.0);
    if (props->cmt_props.window_avg < 0.0)
    {   
        log_errorF("%s: Error window average time must be positive!\n", fcnm);
        goto ERROR;
    }
    props->cmt_props.ldeviatoric
        = iniparser_getboolean(ini, "CMT:ldeviatoric_cmt\0", true);
    if (!props->cmt_props.ldeviatoric)
    {
        log_errorF("%s: Error general CMT inversions not yet programmed\n",
                   fcnm);
        goto ERROR;
    }
    //------------------------------FF Parameters-----------------------------//
    props->ff_props.verbose = props->verbose;
    props->ff_props.utm_zone = props->utm_zone;
    props->ff_props.nfp
        = iniparser_getint(ini, "FF:ff_number_of_faultplanes\0", 2);
    if (props->ff_props.nfp != 2)
    {
        log_errorF("%s: Error only 2 fault planes considered in ff\n", fcnm);
        goto ERROR;
    }
    props->ff_props.nstr = iniparser_getint(ini, "FF:ff_nstr\0", 10);
    if (props->ff_props.nstr < 1)
    {
        log_errorF("%s: Error no fault patches along strike!\n", fcnm);
        goto ERROR;
    }
    props->ff_props.ndip = iniparser_getint(ini, "FF:ff_ndip\0", 5);
    if (props->ff_props.ndip < 1)
    {
        log_errorF("%s: Error no fault patches down dip!\n", fcnm);
        goto ERROR;
    }
    props->ff_props.min_sites = iniparser_getint(ini, "FF:ff_min_sites\0", 4);
    if (props->ff_props.min_sites < props->cmt_props.min_sites)
    {
        log_errorF("%s: Error FF needs at least as many sites as CMT\n", fcnm);
        goto ERROR;
    }
    props->ff_props.window_vel
        = iniparser_getdouble(ini, "FF:ff_window_vel\0", 3.0);
    if (props->ff_props.window_vel <= 0.0)
    {
        log_errorF("%s: Error window velocity must be positive!\n", fcnm);
        goto ERROR;
    }
    props->ff_props.window_avg
        = iniparser_getdouble(ini, "FF:ff_window_avg\0", 10.0);
    if (props->ff_props.window_avg <= 0.0)
    {
        log_errorF("%s: Error window average time must be positive!\n", fcnm);
        goto ERROR;
    }
    props->ff_props.flen_pct
        = iniparser_getdouble(ini, "FF:ff_flen_pct\0", 10.0);
    if (props->ff_props.flen_pct < 0.0)
    {
        log_errorF("%s: Error cannot shrink fault length\n", fcnm);
        goto ERROR;
    }
    props->ff_props.fwid_pct
        = iniparser_getdouble(ini, "FF:ff_fwid_pct\0", 10.0);
    if (props->ff_props.fwid_pct < 0.0)
    {
        log_errorF("%s: Error cannot shrink fault width\n", fcnm);
        goto ERROR;
    }
    //---------------------------ActiveMQ Parameters--------------------------//
    if (props->opmode == REAL_TIME_EEW) 
    {
        s = iniparser_getstring(ini, "ActiveMQ:host\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find ActiveMQ host!\n", fcnm);
            goto ERROR;
        }
        else
        {
            strcpy(props->activeMQ_props.host, s);
        }
        s = iniparser_getstring(ini, "ActiveMQ:user\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find ActiveMQ user!\n", fcnm);
            goto ERROR;
        }
        else
        {
            strcpy(props->activeMQ_props.user, s);
        }
        s = iniparser_getstring(ini, "ActiveMQ:password\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find password!\n", fcnm);
            goto ERROR;
        }
        else
        {
             strcpy(props->activeMQ_props.password, s);
        }
        s = iniparser_getstring(ini, "ActiveMQ:originTopic\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find activeMQ originTopic\n", fcnm);
            goto ERROR;
        }
        else
        {
            strcpy(props->activeMQ_props.originTopic, s);
        }
        s = iniparser_getstring(ini, "ActiveMQ:destinationTopic\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Could not find ActiveMQ destinationTopic!\n", fcnm);
            goto ERROR;
        }
        else
        {
            strcpy(props->activeMQ_props.destinationTopic, s); 
        }
        props->activeMQ_props.port = iniparser_getint(ini, "ActiveMQ:port\0",
                                                      -12345);
        if (props->activeMQ_props.port ==-12345)
        {
            log_errorF("%s: Could not find activeMQ port\n", fcnm);
            goto ERROR;
        }
        props->activeMQ_props.msReconnect
             = iniparser_getint(ini, "ActiveMQ:msReconnect\0", 500);
        if (props->activeMQ_props.msReconnect < 0)
        {
            log_warnF("%s: Overriding msReconnect to 500\n", fcnm);
            props->activeMQ_props.msReconnect = 500;
        }
        props->activeMQ_props.maxAttempts
             = iniparser_getint(ini, "ActiveMQ:maxAttempts\0", 5);
        if (props->activeMQ_props.maxAttempts < 0)
        {
            log_warnF("%s: Overriding maxAttempts to 5\n", fcnm);
            props->activeMQ_props.maxAttempts = 5;
        }
        props->activeMQ_props.msWaitForMessage 
             = iniparser_getint(ini, "ActiveMQ:msWaitForMessage\0", 1);
        if (props->activeMQ_props.msWaitForMessage < 0)
        {
            log_warnF("%s: ActiveMQ could hang indefinitely, overriding to 1\n",
                      fcnm);
            props->activeMQ_props.msWaitForMessage = 1;
        }
    } // End check on need for ActiveMQ
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
    //----------------------------RabbitMQ Parameters-------------------------//
/*
    s = iniparser_getstring(ini, "RabbitMQ:RMQhost\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find RMQhost!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->RMQhost, s); 
    }   
    props->RMQport = iniparser_getint(ini, "RabbitMQ:RMQport\0", -12345);
    if (props->RMQport ==-12345){
        log_errorF("%s: Could not find RMQport\n", fcnm);
        goto ERROR;
    }   
    s = iniparser_getstring(ini, "RabbitMQ:RMQtopic\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find RMQtopic!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->RMQtopic, s); 
    }   
    s = iniparser_getstring(ini, "RabbitMQ:RMQuser\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find RMQuser!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->RMQuser, s); 
    }   
    s = iniparser_getstring(ini, "RabbitMQ:RMQpassword\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find RMQpassword!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->RMQpassword, s); 
    }
    s = iniparser_getstring(ini, "RabbitMQ:RMQexchange\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find RMQexchange!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->RMQexchange, s);
    }
*/
    // Success!
    ierr = 0;
ERROR:;
    // Free the ini file
    iniparser_freedict(ini);
    return ierr;
}
