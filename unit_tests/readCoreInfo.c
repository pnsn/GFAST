#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast.h"

int readCoreInfo_test(void);
/*
int GFAST_alert__parse__shakeAlertXML(const char *message, double SA_NAN,
                                      struct GFAST_shakeAlert_struct *SA)
{
    struct coreInfo_struct core;
    int ierr, length;
    // Initialize
    memset(SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    memset(&core, 0, sizeof(struct coreInfo_struct));
    length = strlen(message);
    if (length == 0){
        printf("%s: Error the message is empty\n", __func__);
        return -1;
    }
    // Extract the pertinent shakeAlert information for GFAST
    ierr = GFAST_xml_read__SACoreInfo(message, SA_NAN, &core);
    if (ierr != 0){
        printf("%s: Error parsing core shakeAlert information\n", __func__);
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

int readCoreInfo_test(void)
{
    FILE *xmlfl;
    struct GFAST_shakeAlert_struct SA;
    char *message = NULL;
    long fsize;
    int ierr;
    // Read the message into memory
    xmlfl = fopen("message.xml", "rb");
    fseek(xmlfl, 0, SEEK_END);
    fsize = ftell(xmlfl);
    rewind(xmlfl);
    message = (char *)calloc((size_t) (fsize + 1), sizeof(char)); // +1 to null terminate
    if (fread(message, (size_t) fsize, 1, xmlfl) == 0){
        printf("%s: Nothing read\n", __func__);
        return EXIT_FAILURE;
    }
    fclose(xmlfl);
    // Parse it
    //ierr = GFAST_alert__parse__shakeAlertXML(message, SA_NAN, &SA);
    ierr = eewUtils_parseCoreXML(message, SA_NAN, &SA);
    free(message);
    if (ierr != 0){
        printf("%s: Error getting shakeAlert info\n", __func__);
        return EXIT_FAILURE;
    }
    // Check it
    if (strcasecmp(SA.eventid, "4557299\0") != 0){
        printf("%s: Couldn't parse ID\n", __func__);
        return EXIT_FAILURE;
    }
    if (fabs(SA.time - 1313104867.753000) > 1.e-4){
        printf("%s: Couldn't parse time\n", __func__);
        return EXIT_FAILURE;
    }
    if (fabs(SA.mag - 3.4) > 1.e-4){
        printf("%s: Couldn't parse magnitude\n", __func__);
        return EXIT_FAILURE;
    }
    if (fabs(SA.lat - 38.8) > 1.e-4){
        printf("%s: Couldn't parse latitude\n", __func__);
        return EXIT_FAILURE;
    }

    if (!(fabs(SA.lon - -122.82) < 1.e-4 || fabs(SA.lon - (-122.82 + 360.0)) < 1.e-4))
    {
        printf("%s: Couldn't parse longitude %f %f\n", __func__, SA.lon, -122.82);
        return EXIT_FAILURE;
    }
    if (fabs(SA.dep - 8.0) > 1.e-4){
        printf("%s: Couldn't parse depth\n", __func__);
        return EXIT_FAILURE;
    }
    //printf("%f %f %f %f %f\n", SA.time, SA.mag, SA.lat, SA.lon, SA.dep);
    printf("%s: Success!\n", __func__);
    return EXIT_SUCCESS;
}
 
