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
int GFAST_paraminit(char *propfilename, struct GFAST_props_struct *props)
{
    const char *fcnm = "GFAST_paraminit\0";
    const char *s;
    int ierr; 
    dictionary *ini;
    // Require the properties file exists
    ierr =-1;
    memset(props, 0, sizeof(struct GFAST_props_struct));
    if (!os_path_isfile(propfilename)){
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
    s = iniparser_getstring(ini, "general:eewsfile\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find ElarmS message filename!\n", fcnm);
        goto ERROR;
    }else{
        strcpy(props->eewsfile, s);
    }
    s = iniparser_getstring(ini, "general:eewgfile\0", "GFAST_output.txt\0");
    strcpy(props->eewgfile, s);
    s = iniparser_getstring(ini, "general:siteposfile\0", NULL);
    if (s == NULL){
        log_errorF("%s: Could not find site position file!\n", fcnm);
    }else{
        strcpy(props->siteposfile, s);
        //if (!os_path_isfile(props->siteposfile)){
        //    log_errorF("%s: Position file %s does not exist1!\n",
        //               fcnm, props->siteposfile);
        //    goto ERROR;
        //}
    }
    props->bufflen = iniparser_getint(ini, "general:bufflen\0", 300);
    if (props->bufflen < 1){
        log_errorF("%s: Buffer lengths must be positive!\n", fcnm);
        goto ERROR;
    }
    props->synmode = iniparser_getboolean(ini, "general:synmode\0", false);
    if (props->synmode){
        s = iniparser_getstring(ini, "general:syndriver\0", NULL);
        if (s == NULL){
            log_errorF("%s: Synthetic driver file must be specified!\n", fcnm);
            goto ERROR;
        }else{
            strcpy(props->syndriver, s);
            if (!os_path_isfile(props->syndriver)){
                log_errorF("%s: Synthetic driver file %s doesnt exist\n",
                           fcnm, props->syndriver);
                goto ERROR;
            }
        }
    } 
    props->utm_zone = iniparser_getint(ini, "general:utm_zone\0", -12345);
    if (props->utm_zone < 0 || props->utm_zone > 60){
        if (props->utm_zone ==-12345){
            log_infoF("%s: Will estimate UTM zone from hypocenter\n", fcnm);
        }else{
            log_warnF("%s: UTM zone %d is invalid estimating from hypocenter\n",
                   fcnm, props->utm_zone);
            props->utm_zone =-12345;
        } 
    }
    props->verbose = iniparser_getint(ini, "general:verbose\0", 2);
    //------------------------------PGD Parameters----------------------------//

    //----------------------------CMT/FF Parameters---------------------------//

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
