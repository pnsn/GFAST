#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_core.h"
#include "iscl/os/os.h"

static void setVarName(const char *group, const char *variable,
                       char *var)
{
    memset(var, 0, 256*sizeof(char));
    sprintf(var, "%s:%s", group, variable);
    return;
}
/*!
 * @brief Reads the PGD properties from the initialization file.
 *
 * @param[in] propfilename   Name of properties file.
 * @param[in] group          Group in ini file.  Likely "PGD".
 * @param[in] verbose        This is the verbosity from the general
 *                           parameters.
 * @param[in] utm_zone       This is the default UTM zone from the
 *                           general parameters.
 *
 * @param[out] pgd_props     PGD scaling properties.
 *
 * @result 0 indicates success.
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_readIni(const char *propfilename,
                             const char *group,
                             const int verbose, const int utm_zone,
                             struct GFAST_pgd_props_struct *pgd_props)
{
    char var[256];
    int ierr;
    dictionary *ini;
    ierr = 1;
    memset(pgd_props, 0, sizeof(struct  GFAST_pgd_props_struct));
    if (!os_path_isfile(propfilename))
    {   
        LOG_ERRMSG("Properties file: %s does not exist", propfilename);
        return ierr;
    }
    ini = iniparser_load(propfilename);
    // Read the properties
    pgd_props->verbose = verbose;
    pgd_props->utm_zone = utm_zone;
    setVarName(group, "dist_tolerance\0", var);
    pgd_props->dist_tol = iniparser_getdouble(ini, var, 6.0);
    if (pgd_props->dist_tol < 0.0)
    {
        LOG_ERRMSG("Error ndistance tolerance %f cannot be negative",
                   pgd_props->dist_tol);
        goto ERROR;
    }
    setVarName(group, "disp_default\0", var);
    pgd_props->disp_def = iniparser_getdouble(ini, var, 0.01);
    if (pgd_props->disp_def <= 0.0)
    {
        LOG_ERRMSG("Error PGD distance default %f must be positive",
                   pgd_props->disp_def);
        goto ERROR;
    }
    setVarName(group, "deltaLatitude\0", var);
    pgd_props->dLat
         = iniparser_getdouble(ini, var, 0.1);
    if (pgd_props->dLat < 0.0)
    {
        LOG_ERRMSG("Error PGD latitude serach %f must be positive",
                   pgd_props->dLat);
        goto ERROR;
    }
    setVarName(group, "deltaLongitude\0", var);
    pgd_props->dLon
         = iniparser_getdouble(ini, var, 0.1);
    if (pgd_props->dLon < 0.0)
    {
        LOG_ERRMSG("Error PGD longitudes %f must be positive", pgd_props->dLon);
        goto ERROR;
    }
    setVarName(group, "nlats_in_pgd_gridSearch\0", var);
    pgd_props->ngridSearch_lats
         = iniparser_getint(ini, var, 1);
    if (pgd_props->ngridSearch_lats < 1)
    {
        LOG_ERRMSG("Error PGD grid search depths %d must be positive",
                   pgd_props->ngridSearch_lats);
        goto ERROR;
    }
    if (pgd_props->ngridSearch_lats%2 == 0)
    {
        LOG_WARNMSG("%s", "Adding 1 point to CMT lat gridsearch");
        pgd_props->ngridSearch_lats
           = pgd_props->ngridSearch_lats + 1;
    }
    setVarName(group, "nlons_in_pgd_gridSearch\0", var);
    pgd_props->ngridSearch_lons
         = iniparser_getint(ini, var, 1);
    if (pgd_props->ngridSearch_lons < 1)
    {
        LOG_ERRMSG("Error PGD grid search depths %d must be positive",
                   pgd_props->ngridSearch_lons);
        goto ERROR;
    }
    if (pgd_props->ngridSearch_lons%2 == 0)
    {
        LOG_WARNMSG("%s", "Adding 1 point to CMT lat gridsearch");
        pgd_props->ngridSearch_lons
           = pgd_props->ngridSearch_lons + 1;
    }
    setVarName(group, "ndepths_in_pgd_gridSearch\0", var);
    pgd_props->ngridSearch_deps
         = iniparser_getint(ini, var, 100);
    if (pgd_props->ngridSearch_deps < 1)
    {
        LOG_ERRMSG("Error PGD grid search depths %d must be positive",
                   pgd_props->ngridSearch_deps);
        goto ERROR;
    }
    setVarName(group, "pgd_window_vel\0", var);
    pgd_props->window_vel = iniparser_getdouble(ini, var, 3.0);
    if (pgd_props->window_vel <= 0.0)
    {
        LOG_ERRMSG("%s", "Error window velocity must be positive!");
        goto ERROR;
    }
    setVarName(group, "pgd_min_sites\0", var);
    pgd_props->min_sites = iniparser_getint(ini, var, 4);
    if (pgd_props->min_sites < 1)
    {
        LOG_ERRMSG("%s", "Error at least one site needed to estimate PGD!");
        goto ERROR;
    }
    ierr = 0;
    ERROR:;
    iniparser_freedict(ini);
    return ierr;
}
