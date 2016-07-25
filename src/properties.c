#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iniparser.h>
#include "gfast.h"
/*!
 * @brief Initializes the GFAST properties (parameter) structure
 *
 * @param[in] propfilename   name of properties file
 *
 * @param[out] props         on successful exit holds the GFAST properties
 *                           structure
 *
 * @author Brendan Crowell (crowellb@uw.edu) and Ben Baker (benbaker@isti.com)
 *
 * @result 0 indicates success
 *
 */
int GFAST_properties__init(char *propfilename, struct GFAST_props_struct *props)
{
    const char *fcnm = "GFAST_properties__init\0";
    const char *s;
    int ierr; 
    dictionary *ini;
    // Require the properties file exists
    ierr =-1;
    memset(props, 0, sizeof(struct GFAST_props_struct));
    if (!os_path_isfile(propfilename))
    {
        log_errorF("%s: Properties file: %s does not exist\n",
                   fcnm, propfilename);
        return ierr;
    }
    // Load the ini file
    ierr = 1;
    ini = iniparser_load(propfilename);
    //-------------------------GFAST General Parameters-----------------------//
    s = iniparser_getstring(ini, "general:streamfile\0", "GFAST_streams.txt\0");
    strcpy(props->streamfile, s);
    if (!os_path_isfile(props->streamfile))
    {
        log_errorF("%s: Cannot find station list %s\n",
                   fcnm, props->streamfile);
        return -1;
    }
    props->opmode = iniparser_getint(ini, "general:opmode\0", 0);
    if (props->opmode < 1 || props->opmode > 3)
    {
        log_errorF("%s: Invalid operation mode %d\n", fcnm, props->opmode); 
    }
    s = iniparser_getstring(ini, "general:eewsfile\0", NULL);
    if (s == NULL)
    {
        log_errorF("%s: Could not find ElarmS message filename!\n", fcnm);
        goto ERROR;
    }
    strcpy(props->eewsfile, s);

    s = iniparser_getstring(ini, "general:eewgfile\0", "GFAST_output.txt\0");
    strcpy(props->eewgfile, s);
    props->bufflen = iniparser_getdouble(ini, "general:bufflen\0", 1800.0);
    if (props->bufflen <= 0.0)
    {
        log_errorF("%s: Buffer lengths must be positive!\n", fcnm);
        goto ERROR;
    }
    if (props->opmode == OFFLINE)
    {
        s = iniparser_getstring(ini, "general:syndriver\0", NULL);
        if (s == NULL) 
        {
            log_errorF("%s: Synthetic driver file must be specified!\n", fcnm);
            goto ERROR;
        }
        strcpy(props->syndriver, s);
        if (!os_path_isfile(props->syndriver))
        {
            log_errorF("%s: Synthetic driver file %s doesnt exist\n",
                       fcnm, props->syndriver);
            goto ERROR;
        }
        s = iniparser_getstring(ini, "general:synthetic_data_directory\0",
                                "./"); 
        if (strlen(s) > 0)
        {
            strcpy(props->syndata_dir, s);
            if (!os_path_isdir(props->syndata_dir))
            {
                log_errorF("%s: Synthetic data directory %s doesn't exist\n",
                           fcnm, props->syndata_dir);
                goto ERROR;
            }
        }
        else
        {
            strcpy(props->syndata_dir, "./\0");
        }
        s = iniparser_getstring(ini, "general:synthetic_data_prefix\0", "LX\0");
        if (strlen(s) > 0){strcpy(props->syndata_pre, s);}
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
    props->dt_init = iniparser_getint(ini, "general:dt_init\0", 3);
    if (props->opmode != OFFLINE)
    {
        if (props->dt_init != INIT_DT_FROM_TRACEBUF){
            log_warnF("%s: Obtaining sampling period from tracebuf\n", fcnm);
            props->dt_init = 3;
        }
    }
    s = iniparser_getstring(ini, "general:dtfile\0", NULL);
    if (props->opmode == OFFLINE)
    {
        // Make sure the EEW XML file exists
        if (!os_path_isfile(props->eewsfile))
        {
            log_errorF("%s: Cannot find EEW XML file!\n",
                       fcnm, props->streamfile);
            goto ERROR;
        }
        // Figure out how to initialize sampling period
        if (props->dt_init == INIT_DT_FROM_FILE)
        {
            props->dt_init = iniparser_getint(ini, "general:dt_init\0", 1);
            if (s == NULL)
            {
                log_errorF("%s: Must specify dtfile!\n", fcnm);
                goto ERROR; 
            }
            strcpy(props->dtfile, s);
            if (!os_path_isfile(props->dtfile))
            {
                log_errorF("%s: Cannot find dtfile %s!\n", fcnm, s);
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
    // Location initialization
    props->loc_init = iniparser_getint(ini, "general:loc_init\0", 1);
    if (props->opmode == OFFLINE)
    {
        if (props->loc_init == INIT_LOCS_FROM_TRACEBUF)
        {
            log_errorF("%s: offline can't initialize locations from tracebuf\n",
                       fcnm);
            goto ERROR;
        }
    }
    if (props->loc_init == INIT_LOCS_FROM_FILE)
    {
        s = iniparser_getstring(ini, "general:siteposfile\0", NULL);
        if (s == NULL)
        {
            log_errorF("%s: Site position file must be defined\n");
            return -1;
        }
        else
        {
            strcpy(props->siteposfile, s);
        }
        if (!os_path_isfile(props->siteposfile))
        {
            log_errorF("%s: Position file %s does not exist!\n",
                       fcnm, props->siteposfile);
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
        if (!os_path_isdir(props->h5ArchiveDir))
        {
            log_warnF("%s: Archive directory %s doesn't exist\n",
                       fcnm, props->h5ArchiveDir);
            log_warnF("%s: Will use current working directory\n", fcnm);
            memset(props->h5ArchiveDir, 0, sizeof(props->h5ArchiveDir));
            strcpy(props->h5ArchiveDir, "./\0");
        }
    }
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
    props->pgd_props.ngridSearch_deps
         = iniparser_getint(ini, "PGD:ndepths_in_pgd_gridSearch\0", 100);
    if (props->pgd_props.ngridSearch_deps < 1){
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
    if (props->pgd_props.min_sites < 1){
        log_errorF("%s: Error at least one site needed to estimate PGD!\n", 
                   fcnm);
        goto ERROR;
    }
    //----------------------------CMT Parameters------------------------------//
    props->cmt_props.verbose = props->verbose;
    props->cmt_props.utm_zone = props->utm_zone;
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
    s = iniparser_getstring(ini, "ActiveMQ:AMQhost\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find AMQhost!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->AMQhost, s);
    }
    props->AMQport = iniparser_getint(ini, "ActiveMQ:AMQport\0", -12345);
    if (props->AMQport ==-12345){
        log_errorF("%s: Could not find AMQport\n", fcnm);
        goto ERROR;
    }
    s = iniparser_getstring(ini, "ActiveMQ:AMQtopic\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find AMQtopic!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->AMQtopic, s); 
    }
    s = iniparser_getstring(ini, "ActiveMQ:AMQuser\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find AMQuser!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->AMQuser, s); 
    }
    s = iniparser_getstring(ini, "ActiveMQ:AMQpassword\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find AMQpassword!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->AMQpassword, s); 
    }
    //----------------------------RabbitMQ Parameters-------------------------//
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
    // Success!
    ierr = 0;
ERROR:;
    // Free the ini file
    iniparser_freedict(ini);
    return ierr;
}
//============================================================================//
/*!
 * @brief Prints the GFAST properties to the debug file.  Note, there is 
 *        no verbosity check.
 *
 * @param[in] props    GFAST properties structure to write to the debug file
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_properties__print(struct GFAST_props_struct props)
{
    const char *fcnm = "GFAST_properties__print\0";
    const char *lspace = "    \0";
    log_debugF("\n%s: GFAST properties\n", fcnm);
    if (props.opmode == OFFLINE)
    {
        log_debugF("%s GFAST site position file %s\n", lspace,
                   props.siteposfile);  
        log_debugF("%s GFAST results file %s\n", lspace, props.eewgfile);
        log_debugF("%s GFAST is operating in offline mode\n", lspace);
        log_debugF("%s GFAST default sampling period is %f (s)\n", lspace,
                   props.dt_default);
        if (props.dt_init == INIT_DT_FROM_FILE)
        {
            log_debugF("%s GFAST will get sampling period from file %s\n",
                       lspace, props.dtfile); 
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
                       lspace, props.siteposfile);
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
        log_debugF("%s GFAST synthetic data directory: %s\n",
                   lspace, props.syndata_dir);
        if (strlen(props.syndata_pre) > 0)
        {
            log_debugF("%s GFAST synthetic data prefix: %s\n", 
                       lspace, props.syndata_pre); 
        }
    }
    else if (props.opmode == PLAYBACK)
    {
        log_debugF("%s GFAST is operating in playback mode\n", lspace);
    }
    else if (props.opmode == REAL_TIME)
    {
        log_debugF("%s GFAST is operating in real-time mode\n", lspace);
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
    log_debugF("%s GFAST stream file: %s\n", lspace, props.streamfile);
    log_debugF("%s GFAST HDF5 archive dir: %s\n", lspace, props.h5ArchiveDir);
    log_debugF("%s GFAST will finish processing an event after %f (s)\n",
               lspace, props.processingTime);
    log_debugF("%s GFAST will use a default earthquake depth of %f\n",
               lspace, props.eqDefaultDepth);
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
