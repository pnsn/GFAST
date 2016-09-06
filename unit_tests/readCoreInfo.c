#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"
#include "iscl/log/log.h"
/*
int GFAST_alert__parse__shakeAlertXML(const char *message, double SA_NAN,
                                      struct GFAST_shakeAlert_struct *SA)
{
    const char *fcnm = "GFAST_alert__parse__shakeAlertXML\0";
    struct coreInfo_struct core;
    int ierr, length;
    // Initialize
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    memset(&core, 0, sizeof(struct coreInfo_struct));
    length = strlen(message);
    if (length == 0){
        printf("%s: Error the message is empty\n", fcnm);
        return -1;
    }
    // Extract the pertinent shakeAlert information for GFAST
    ierr = GFAST_xml_read__SACoreInfo(message, SA_NAN, &core);
    if (ierr != 0){
        printf("%s: Error parsing core shakeAlert information\n", fcnm);
    }else{
        strcpy(SA->eventid, core.id);
        SA->time = core.orig_time;
        SA->lat = core.lat;
        SA->lon = core.lon;
        SA->dep = core.depth;
        SA->mag = core.mag;
    }
    return 0;
}
*/

int readCoreInfo_test()
{
    const char *fcnm = "readCoreInfo_test\0";
    FILE *xmlfl;
    struct GFAST_shakeAlert_struct SA;
    char *message = NULL;
    double SA_NAN = -12345.0;
    long fsize;
    int ierr;
    // Read the message into memory
    xmlfl = fopen("message.xml", "rb");
    fseek(xmlfl, 0, SEEK_END);
    fsize = ftell(xmlfl);
    rewind(xmlfl);
    message = (char *)calloc(fsize + 1, sizeof(char)); // +1 to null terminate
    if (fread(message, fsize, 1, xmlfl) == 0){
        printf("%s: Nothing read\n", fcnm);
        return EXIT_FAILURE;
    }
    fclose(xmlfl);
    // Parse it
    //ierr = GFAST_alert__parse__shakeAlertXML(message, SA_NAN, &SA);
    ierr = eewUtils_parseCoreXML(message, SA_NAN, &SA);
    free(message);
    if (ierr != 0){
        printf("%s: Error getting shakeAlert info\n", fcnm);
        return EXIT_FAILURE;
    }
    // Check it
    if (strcasecmp(SA.eventid, "4557299\0") != 0){
        printf("%s: Couldn't parse ID\n", fcnm);
        return EXIT_FAILURE;
    }
    if (SA.time != 1313104867.753000){
        printf("%s: Couldn't parse time\n", fcnm);
        return EXIT_FAILURE;
    }
    if (SA.mag != 3.4){
        printf("%s: Couldn't parse magnitude\n", fcnm);
        return EXIT_FAILURE;
    }
    if (SA.lat != 38.8){
        printf("%s: Couldn't parse latitude\n", fcnm);
        return EXIT_FAILURE;
    }
    if (!(SA.lon == -122.82 || SA.lon == -122.82 + 360.0)){
        printf("%s: Couldn't parse longitude %f %f\n", fcnm, SA.lon, -122.82);
        return EXIT_FAILURE;
    }
    if (SA.dep != 8.0){
        printf("%s: Couldn't parse depth\n", fcnm);
        return EXIT_FAILURE;
    }
    //printf("%f %f %f %f %f\n", SA.time, SA.mag, SA.lat, SA.lon, SA.dep);
    printf("%s: Success!\n", fcnm);
    return EXIT_SUCCESS;
}
 
