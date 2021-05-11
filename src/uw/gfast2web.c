#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "gfast.h"
#include "gfast_activeMQ.h"
#include "beachball.h"
#include "compearth.h"
//#include "gfast2json.h"
#include "iscl/array/array.h"
#include "iscl/iscl/iscl.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

#define HYPO_STRUCT "/DataStructures/hypocenterStructure"
#define PGD_STRUCT "/DataStructures/pgdResultsStructure"
#define CMT_STRUCT "/DataStructures/cmtResultsStructure"
#define FF_STRUCT "/DataStructures/finiteFaultResultsStructure"
#define TRIGGER_HYPO "triggeringHypocenter"
#define CMT_RES "cmtResults"
#define FF_RES "finiteFaultResults"
#define PGD_RES "pgdResults"

static bool checkMessages(char *evid, char *archiveFile);

struct curlMessage_struct
{
    const char *readptr;  /*!< Pointer to char * string to post. */
    size_t sizeleft;      /*!< Size remaining in message to post. */
};

struct event_struct
{
    char archiveFile[PATH_MAX];
    char evid[128];
    double lastPublished;
};

struct eventList_struct
{
    struct event_struct *events;
    int nevents;
    char pad[4];
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);

int addEvent(const char *evid, const char *archiveFile,
             struct eventList_struct *events);
int deleteEvent(const char *evid, struct eventList_struct *events);
int copyEvents(const struct eventList_struct eventIn,
               struct eventList_struct *eventOut);
int freeEvents(struct eventList_struct *events);
int getEventIndex(const char *evid, const struct eventList_struct events);
bool eventExists(const char *evid, const struct eventList_struct events);
static int parseMessage(const char *message,
                        char *evid, char *archiveFile);
static char *packMessage(const char *evid, const char *archiveFile);
int postJson(const char *msg, const char *url,
             const char *userName, const char *password);
int postImage(const char *fileName, const char *url,
              const char *userName, const char *password);
static void getCMTopt(const struct GFAST_cmtResults_struct cmt,
                      int *iopt, int *depOpt, int *latOpt, int *lonOpt);

void usage(void)
{
	printf("Usage:\n");
	printf(" --filename <h5 filename>\n");
	printf(" -i<interval>\n");
	printf(" -a //Output all intervals\n");
	exit (8);
}

int main(int argc, char *argv[])
{
    const char *fcnm = "gfast2web\0";
    struct GFAST_shakeAlert_struct SA;
    struct eventList_struct events;
    char message[PATH_MAX];
    char evid[128]="1111";
    char archiveFile[PATH_MAX] = "/home/mhagerty/gfast/run_playback/run/tohoku/zold/1111_archive.h5";
    double lastPublished, t0;
    int ierr, iev, i, j;
    bool lhaveEvent, leventExists;
    hid_t h5fl;
    char *cm;
    struct beachballPlot_struct beachball;
    const bool useTopic = false;
    const bool clientAck = false;
    const int port = 61616;
    const char *gfastURL = "https://gfast.pnsn.org";
    const char *gfastUserName = "username";
    const char *gfastPassword = "xyz"; 
    const char *destination = "gfastWebProducts";
    const char *user = "activemq";
    const char *password = "";
    const bool ldebug = true;
    const int ms_wait = 10;
    char cmtFile[PATH_MAX];
    int bogusID;
    void *consumer;
    void *webProduct;
    iscl_init();
    int output_interval;
    bool output_all = false;
    // Initialize curl
    //curl = curl_easy_init();
    //curl_easy_setopt(curl, CURLOPT_URL, "https://gfast.pnsn.org");
    // For debugging purposes create the producer
    //
    printf("argc=%d\n", argc);
    for (i=0; i<argc; i++){
      printf("argv[%d]=%s\n", i, argv[i]);
    }

    //while ((argc > 1) && (argv[argc][0] == '-'))
    for (i=0; i<argc; i++)
    {
      if (argv[i][0] == '-') {
        switch (argv[i][1])
        {
          case 'a':
            printf("-a: Output all intervals\n");
            break;

          case 'i':
            printf("scan int from argv[i+1]=%s\n", argv[i+1]);
            //output_interval = atoi(&argv[i][2]);
            output_interval = atoi(argv[i+1]);
            printf("-i: Output i:[%d] interval\n", output_interval);
            break;

          case '-':
            if (strcmp(argv[i], "--filename") == 0){
              strcpy(archiveFile, argv[i+1]);
              printf("Got --filename = [%s]\n", archiveFile);
            }
            break;

          default:
            printf("Wrong Argument: %s\n", argv[i]);
            usage();
        }
        //++argv;
        //--argc;
      }
    }
    exit(0);

    // Create the ActiveMQ consumer (failover:(tcp://localhost:61616)"
    //if (ldebug){activeMQ_producer_sendMessage( );}
    memset(&SA, 0, sizeof(struct GFAST_shakeAlert_struct));
    memset(&events, 0, sizeof(struct eventList_struct));
    // Begin the acquisition
    //while (true)
    //archiveFile = "/home/mhagerty/gfast/run_playback/run/tohoku/zold/1111_archive.h5";
    puts(archiveFile);
    if (1)
    {
            // Verify the archive file exists
            if (!os_path_isfile(archiveFile))
            {
                fprintf(stderr, "%s: Error archive file %s does not exist\n",
                        fcnm, archiveFile);
                //continue;
            }

        // Open the HDF5 file and read the latest entry
        t0 = time_timeStamp();
        //for (iev=0; iev<events.nevents; iev++)
        //{
            // Get the most recently published group 
            h5fl = H5Fopen(archiveFile, H5F_ACC_RDONLY, H5P_DEFAULT);
char groupName[512], structGroup[512];
int idep, iopt, igroup, kgroup, latOpt, lonOpt, nlocs;
hsize_t dims[1] = {1};
struct h5_cmtResults_struct h5cmt;
struct h5_hypocenter_struct h5hypo;
struct h5_pgdResults_struct h5pgd;
struct h5_ffResults_struct h5ff;
struct h5_gpsData_struct h5gpsData;
struct GFAST_shakeAlert_struct hypo;
struct GFAST_pgdResults_struct pgd;
struct GFAST_cmtResults_struct cmt;
struct GFAST_ffResults_struct ff;
struct GFAST_data_struct gpsData;
struct GFAST_waveform3CData_struct wdata;
char *ccmt, *cdata, *cff, *chypo, *cpgd, *cts;
double *dep, *dip1, *dip2, *Mcmt, *Mpgd, *Meew, *Mff, *rak1, *rak2, *str1, *str2, *times, trigger0;
hid_t cmtDataType, dataSet, dataSpace, dataType, ffDataType, hypoDataType, groupID, memSpace, pgdDataType;
iopt =-1;
            kgroup = hdf5_getMaxGroupNumber(h5fl);
            printf("MTH: kgroup=%d\n", kgroup);
            times = array_set64f(kgroup, __builtin_nan(""), &ierr);
            Mcmt = array_set64f(kgroup,  __builtin_nan(""), &ierr);
            Meew = array_set64f(kgroup,  __builtin_nan(""), &ierr);
            Mff  = array_set64f(kgroup,  __builtin_nan(""), &ierr);
            Mpgd = array_set64f(kgroup,  __builtin_nan(""), &ierr);
            str1 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            str2 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            dip1 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            dip2 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            rak1 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            rak2 = array_set64f(kgroup, __builtin_nan(""), &ierr);
            dep  = array_set64f(kgroup, __builtin_nan(""), &ierr);

            memset(structGroup, 0, 512*sizeof(char));
            memset(groupName, 0, 512*sizeof(char)); 
            memset(structGroup, 0, 512*sizeof(char));
            strcpy(structGroup, "/DataStructures\0");
            if (H5Lexists(h5fl, structGroup, H5P_DEFAULT) < 1)
            {
                LOG_ERRMSG("%s", "Data structure group doesn't exist");
                H5Fclose(h5fl);
                //continue;
            }
            trigger0 = DBL_MAX;
            // Scrounge the magnitude and strike/dip/rake time series
            hypoDataType = H5Topen(h5fl, HYPO_STRUCT, H5P_DEFAULT);
            cmtDataType = H5Topen(h5fl, CMT_STRUCT, H5P_DEFAULT);
            ffDataType = H5Topen(h5fl, FF_STRUCT, H5P_DEFAULT);
            pgdDataType = H5Topen(h5fl, PGD_STRUCT, H5P_DEFAULT);
            for (igroup=1; igroup<=kgroup; igroup++)
            {
                memset(groupName, 0, 512*sizeof(char));
                sprintf(groupName, "/GFAST_History/Iteration_%d", igroup);
                printf("/GFAST_History/Iteration_%d\n", igroup);
                //groupID = H5Gopen2(h5fl, groupName, H5P_DEFAULT);
                memset(&h5hypo, 0, sizeof(struct h5_hypocenter_struct));
                memset(&hypo, 0, sizeof(struct GFAST_shakeAlert_struct));
                memset(&h5pgd, 0, sizeof(struct h5_pgdResults_struct));
                memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
                memset(&h5cmt, 0, sizeof(struct h5_cmtResults_struct));
                memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
                memset(&h5ff, 0, sizeof(struct h5_ffResults_struct));
                memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
                groupID = H5Gopen2(h5fl, groupName, H5P_DEFAULT);
                if (H5Lexists(groupID, TRIGGER_HYPO, H5P_DEFAULT) > 0)
                {
                    dataSet = H5Dopen(groupID, TRIGGER_HYPO, H5P_DEFAULT);
                    dataSpace = H5Dget_space(dataSet);
                    memSpace = H5Screate_simple(1, dims, NULL);
                    H5Dread(dataSet, hypoDataType, memSpace, dataSpace,
                            H5P_DEFAULT, &h5hypo);
                    hdf5_copyHypocenter(COPY_H5_TO_DATA, &hypo, &h5hypo);
                    H5Sclose(memSpace);
                    H5Sclose(dataSpace);
                    H5Dclose(dataSet);
                    Meew[igroup-1] = hypo.mag;
                    times[igroup-1] = (double) (igroup - 1);
                    if (trigger0 == DBL_MAX){trigger0 = hypo.time;}
                    //trigger0 = fmin(trigger0, hypo.time);
                    times[igroup-1] = trigger0 + (double) (igroup - 1);
                }
                else
                {
                    printf("this is strange - no hypo; things will probably break\n");
                }
                if (H5Lexists(groupID, PGD_RES, H5P_DEFAULT) > 0)
                {
                    dataSet = H5Dopen(groupID, PGD_RES, H5P_DEFAULT);
                    dataSpace = H5Dget_space(dataSet);
                    memSpace = H5Screate_simple(1, dims, NULL);
                    H5Dread(dataSet, pgdDataType, memSpace, dataSpace,
                            H5P_DEFAULT, &h5pgd);
                    hdf5_copyPGDResults(COPY_H5_TO_DATA, &pgd, &h5pgd);
                    nlocs = pgd.nlats*pgd.nlons*pgd.ndeps;
                    iopt = array_argmin64f(nlocs, pgd.dep_vr_pgd, &ierr);
                    Mpgd[igroup-1] = pgd.mpgd[iopt];
                    H5Sclose(memSpace);
                    H5Sclose(dataSpace);
                    H5Dclose(dataSet);
                    hdf5_memory_freePGDResults(&h5pgd);
                    core_scaling_pgd_finalizeResults(&pgd);
                }
                if (H5Lexists(groupID, CMT_RES, H5P_DEFAULT) > 0)
                {
                    cmtDataType = H5Topen(h5fl, CMT_STRUCT, H5P_DEFAULT);
                    dataSet = H5Dopen(groupID, CMT_RES, H5P_DEFAULT);
                    dataSpace = H5Dget_space(dataSet);
                    memSpace = H5Screate_simple(1, dims, NULL);
                    H5Dread(dataSet, cmtDataType, memSpace, dataSpace,
                            H5P_DEFAULT, &h5cmt);
                    hdf5_copyCMTResults(COPY_H5_TO_DATA, &cmt, &h5cmt);
                    H5Sclose(memSpace);
                    H5Sclose(dataSpace);
                    H5Dclose(dataSet);
                    nlocs = cmt.nlats*cmt.nlons*cmt.ndeps;
                    getCMTopt(cmt, &iopt, &idep, &latOpt, &lonOpt);
                    Mcmt[igroup-1] = cmt.Mw[iopt]; //printf("%f\n", cmt.Mw[iopt]);
                    str1[igroup-1] = cmt.str1[iopt];
                    dip1[igroup-1] = cmt.dip1[iopt];
                    rak1[igroup-1] = cmt.rak1[iopt];
                    str2[igroup-1] = cmt.str2[iopt];
                    dip2[igroup-1] = cmt.dip2[iopt];
                    rak2[igroup-1] = cmt.rak2[iopt];
                    dep[igroup-1]  = cmt.srcDepths[idep];
                    hdf5_memory_freeCMTResults(&h5cmt);
                    core_cmt_finalizeResults(&cmt);
                }
                if (H5Lexists(groupID, FF_RES, H5P_DEFAULT) > 0)
                {
                    dataSet = H5Dopen(groupID, FF_RES, H5P_DEFAULT);
                    dataSpace = H5Dget_space(dataSet);
                    memSpace = H5Screate_simple(1, dims, NULL);
                    H5Dread(dataSet, ffDataType, memSpace, dataSpace,
                            H5P_DEFAULT, &h5ff);
                    hdf5_copyFFResults(COPY_H5_TO_DATA, &ff, &h5ff);
                    H5Sclose(memSpace);
                    H5Sclose(dataSpace);
                    H5Dclose(dataSet);
                    Mff[igroup-1] = ff.Mw[ff.preferred_fault_plane];
                    hdf5_memory_freeFFResults(&h5ff);
                    core_ff_finalizeResults(&ff);
                }
                //printf("%f %f %f %f\n", Meew[igroup-1], Mpgd[igroup-1], Mcmt[igroup-1], Mff[igroup-1]);
                H5Gclose(groupID);
            }
            H5Tclose(hypoDataType);
            H5Tclose(cmtDataType);
            H5Tclose(ffDataType);
            H5Tclose(pgdDataType);
            // Make json summary
            /*
            cts = gfast2json_packTimeSeriesMetrics(evid, kgroup,
                                                   times,
                                                   Meew, Mpgd, Mcmt, Mff,
                                                   str1, str2, dip1, dip2,
                                                   rak1, rak2, dep);
            */
            memory_free64f(&times);
            memory_free64f(&Mcmt);
            memory_free64f(&Meew);
            memory_free64f(&Mff);
            memory_free64f(&Mpgd);
            memory_free64f(&str1);
            memory_free64f(&str2);
            memory_free64f(&dip1);
            memory_free64f(&dip2);
            memory_free64f(&rak1);
            memory_free64f(&rak2);
            memory_free64f(&dep);


            memset(groupName, 0, 512*sizeof(char));
            sprintf(groupName, "/GFAST_History/Iteration_%d", kgroup);
            groupID = H5Gopen2(h5fl, groupName, H5P_DEFAULT);
            // hypocenter
            memset(&h5hypo, 0, sizeof(struct h5_hypocenter_struct));
            memset(&hypo, 0, sizeof(struct GFAST_shakeAlert_struct));
            dataType = H5Topen(h5fl, HYPO_STRUCT, H5P_DEFAULT);
            dataSet = H5Dopen(groupID, TRIGGER_HYPO, H5P_DEFAULT);
            dataSpace = H5Dget_space(dataSet);
            memSpace = H5Screate_simple(1, dims, NULL);
            H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5hypo);
            hdf5_copyHypocenter(COPY_H5_TO_DATA, &hypo, &h5hypo);
            //chypo = gfast2json_packTriggeringHypocenter(hypo);
            //printf("%s\n", chypo);
            //if (chypo != NULL){free(chypo);}
            H5Sclose(memSpace);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            H5Tclose(dataType);
            // gps data
            if (H5Lexists(groupID, "gpsData\0", H5P_DEFAULT) < 1)
            {
                LOG_WARNMSG("%s", "gpsData does not exists");
            }
            memset(&wdata, 0, sizeof(struct GFAST_waveform3CData_struct));
            memset(&gpsData, 0, sizeof(struct GFAST_data_struct));
            memset(&h5gpsData, 0, sizeof(struct h5_gpsData_struct));
            dataType = H5Topen(h5fl, "/DataStructures/gpsDataStructure\0",
                               H5P_DEFAULT);
            dataSet = H5Dopen(groupID, "gpsData\0", H5P_DEFAULT);
            dataSpace = H5Dget_space(dataSet);
            memSpace = H5Screate_simple(1, dims, NULL);
            H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5gpsData);
            hdf5_copyGPSData(COPY_H5_TO_DATA, &gpsData, &h5gpsData);
            //cdata = gfast2json_packGPSData(gpsData);
            if (cdata != NULL){free(cdata);}
            hdf5_memory_freeGPSData(&h5gpsData);
            H5Sclose(memSpace);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            H5Tclose(dataType);
            printf("MTH: gpsData.stream_length=%d\n", gpsData.stream_length);
            for (i=0;i<gpsData.stream_length; i++){
              wdata = gpsData.data[i];
              printf("%s.%s.%s %8.3f %8.3f %d\n", wdata.netw, wdata.stnm, wdata.chan[0],
                      wdata.sta_lat, wdata.sta_lon, wdata.npts);
              for (j=0; j<10; j++){
                printf("%f ", wdata.ubuff[j]);
              }
              printf("\n");
            }
            // pgd
            memset(&h5pgd, 0, sizeof(struct h5_pgdResults_struct));
            memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
            dataType = H5Topen(h5fl, PGD_STRUCT, H5P_DEFAULT);
            dataSet = H5Dopen(groupID, PGD_RES, H5P_DEFAULT);
            dataSpace = H5Dget_space(dataSet);
            memSpace = H5Screate_simple(1, dims, NULL);
            H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5pgd);
            hdf5_copyPGDResults(COPY_H5_TO_DATA, &pgd, &h5pgd);
            //cpgd = gfast2json_packPGD(evid, gpsData, pgd);
            hdf5_memory_freePGDResults(&h5pgd);
            H5Sclose(memSpace);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            H5Tclose(dataType);
            core_scaling_pgd_finalizeResults(&pgd);
            // cmt
            if (H5Lexists(groupID, CMT_RES, H5P_DEFAULT) < 1)
            {
                LOG_WARNMSG("%s", "cmt not yet computed");
            }
            memset(&h5cmt, 0, sizeof(struct h5_cmtResults_struct));
            memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
            dataType = H5Topen(h5fl, CMT_STRUCT, H5P_DEFAULT);
            dataSet = H5Dopen(groupID, CMT_RES, H5P_DEFAULT);
            dataSpace = H5Dget_space(dataSet);
            memSpace = H5Screate_simple(1, dims, NULL);
            H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5cmt);
            hdf5_copyCMTResults(COPY_H5_TO_DATA, &cmt, &h5cmt);
            //ccmt = gfast2json_packCMT(evid, gpsData, cmt);
            //if (ccmt != NULL){printf("%s\n", ccmt);}
            //if (ccmt != NULL){free(ccmt);}
            iopt = array_argmin64f(cmt.nlats*cmt.nlons*cmt.ndeps, cmt.objfn, &ierr);
            idep =-1;
            getCMTopt(cmt, &iopt, &idep, &latOpt, &lonOpt);
printf("iopt=%d idep=%d\n", iopt, idep);
printf("cmt.str1:%f cmt.dip1:%f cmt.rak1:%f\n", cmt.str1[iopt], cmt.dip1[iopt], cmt.rak1[iopt]);
            beachball_plotDefaults(&beachball);
/*
*/
            beachball_setMomentTensor(cmt.mts[6*iopt+0], cmt.mts[6*iopt+1], 
                                      cmt.mts[6*iopt+2], cmt.mts[6*iopt+3],
                                      cmt.mts[6*iopt+4], cmt.mts[6*iopt+5],
                                      CE_NED,
                                      &beachball);
            memset(cmtFile, 0, PATH_MAX*sizeof(char)); 
            sprintf(cmtFile, "%s.png", evid); 
            printf("cmtFile: %s.png\n", evid);
            beachball_draw(cmtFile, beachball);
            hdf5_memory_freeCMTResults(&h5cmt);
            H5Sclose(memSpace);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            H5Tclose(dataType);
            core_cmt_finalizeResults(&cmt);
            // ff
            if (H5Lexists(groupID, "finiteFaultResults\0", H5P_DEFAULT) < 1)
            {
                LOG_WARNMSG("%s", "ff not yet computed");
            }
            memset(&h5ff, 0, sizeof(struct h5_ffResults_struct));
            memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
            dataType = H5Topen(h5fl, FF_STRUCT, H5P_DEFAULT);
            dataSet = H5Dopen(groupID, FF_RES, H5P_DEFAULT);
            dataSpace = H5Dget_space(dataSet);
            memSpace = H5Screate_simple(1, dims, NULL);
            H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5ff);
            hdf5_copyFFResults(COPY_H5_TO_DATA, &ff, &h5ff);
            //cff = gfast2json_packFF(evid, gpsData, ff); 
            //if (cff != NULL){printf("%s\n", cff);}
            //if (cff != NULL){free(cff);}
            hdf5_memory_freeFFResults(&h5ff);
            H5Sclose(memSpace);
            H5Sclose(dataSpace);
            H5Dclose(dataSet);
            H5Tclose(dataType);
            // clean up
            core_ff_finalizeResults(&ff);
            printf("MTH: call core_data_finalize gpsData\n");
            core_data_finalize(&gpsData);

            // Final publication until manual review
            /*
            if (t0 - lastPublished > 300.0)
            {
                // Remove event
                memset(evid, 0, 128*sizeof(char));
                strcpy(evid, events.events[iev].evid);
                deleteEvent(evid, &events);
            }
            else if (t0 - lastPublished > 180.0)
            {

            }
            else if (t0 - lastPublished > 60.0)
            {

            }
            // Initial publication
            else if (t0 - lastPublished > 30.0)
            {
 
            }
            */
            H5Gclose(groupID);
            H5Fclose(h5fl);
            // Curl it the interweb
//printf("%s\n", chypo);
            ierr = postJson(chypo, gfastURL, gfastUserName, gfastPassword);
            if (chypo != NULL)
            {
                free(chypo);
                chypo = NULL;
            }
//printf("%s\n", cpgd);
            ierr = postJson(cpgd, gfastURL, gfastUserName, gfastPassword);
            if (cpgd != NULL)
            {
                free(cpgd);
                cpgd = NULL;
            }
//printf("%s\n", ccmt);
            ierr = postJson(ccmt, gfastURL, gfastUserName, gfastPassword);
            if (ccmt != NULL)
            {
                free(ccmt);
                ccmt = NULL;
            }
//printf("%s\n", cff);
            ierr = postJson(cff,  gfastURL, gfastUserName, gfastPassword);
            if (cff != NULL)
            {
                free(cff);
                cff = NULL;
            }
printf("%s\n", cts);
            ierr = postJson(cts,  gfastURL, gfastUserName, gfastPassword);
            if (cts != NULL)
            {
                free(cts);
                cts = NULL;
            }
            postImage(cmtFile, gfastURL, gfastUserName, gfastPassword);

/*
            curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, "https://gfast.pnsn.org");
            struct curl_httppost *firstItem = NULL;
            struct curl_htttpost *lastItem  = NULL;
            long len = strlen(cpgd);
printf("%ld\n", len);
getchar();
            const char *buffer = cpgd;
            result = curl_formadd(&firstItem, &lastItem,
                                  CURLFORM_BUFFER, "pdgData\0", 
                                  CURLFORM_BUFFERPTR, buffer,
                                  CURLFORM_BUFFERLENGTH, len,
                                  CURLFORM_END);
printf("%d %d\n", result, CURL_FORMADD_INCOMPLETE);
            if (result != 0)
            {
                LOG_ERRMSG("%s", "Error adding cpgd\n");
            }

            if (curl)
            {
printf("do it\n");
                curl_easy_setopt(curl, CURLOPT_HTTPPOST, firstItem);
                res = curl_easy_perform(curl);
                if (res != CURLE_OK)
                {
                    LOG_ERRMSG("%s", curl_easy_strerror(res));
                }
printf("done it\n");
                
            }
            else
            {
                LOG_ERRMSG("%s", "Failed initialize curl");
            }
            curl_easy_cleanup(curl);
*/
        //} // Loop on events
        //break;

    }
    /* MTH: not using this
    freeEvents(&events);
    activeMQ_consumer_finalize(consumer);
    if (ldebug){activeMQ_producer_finalize(webProduct);}
    */
    //curl_easy_cleanup(curl);
    //iscl_finalize();
    return EXIT_SUCCESS;
}

static bool checkMessages(char *evid, char *archiveFile)
{
    memset(evid, 0, 128*sizeof(char));
    memset(archiveFile, 0, PATH_MAX*sizeof(char));
    strcpy(evid, "2435\0");
    strcpy(archiveFile, "./2435_archive.h5\0");
    return true;
}

//============================================================================//
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct curlMessage_struct *upload = (struct curlMessage_struct *) userp;
    size_t max = size*nmemb;
    size_t copylen;

    if(max < 1){return 0;}
 
    if (upload->sizeleft)
    {   
        copylen = max;
        if (copylen > upload->sizeleft){copylen = upload->sizeleft;}
        memcpy(ptr, upload->readptr, copylen);
        upload->readptr += copylen;
        upload->sizeleft -= copylen;
        return copylen;
    }
    return 0; // No more data left to deliver 
}

static bool setUserPassword(const char *userName, const char *password,
                            char userPassword[GFAST_MAXMSG_LEN])
{
    memset(userPassword, 0, GFAST_MAXMSG_LEN*sizeof(char));
    if (userName != NULL && password != NULL)
    {
        sprintf(userPassword, "%s:%s", userName, password);
        return true;
    }
    return false;
}

int postImage(const char *fileName, const char *url,
              const char *userName, const char *password)
{
    CURL *curl = NULL;
    CURLcode res;
    FILE *fd;
    struct stat fileInfo;
    struct curl_slist *headerList = NULL;
    double uploadSpeed, totalTime;
    static const char header[] = "Content-Type: application/png";
    char userPassword[GFAST_MAXMSG_LEN];
    int ierr;
    ierr = 0;
    // Open file to upload
    if (!os_path_isfile(fileName))
    {
        LOG_ERRMSG("file %s does not exist", fileName);
        return -1;
    }
    fd = fopen(fileName, "rb"); // open file to upload 
    if (fstat(fileno(fd), &fileInfo) != 0)
    {
        LOG_ERRMSG("failed to get file info from %s", fileName);
        fclose(fd);
        return -1;
    }
    // Fire up curl
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK)
    {
        LOG_ERRMSG("curl_global_init() failed: %s\n",
                   curl_easy_strerror(res));
        return -1;
    }
    // Get curl handle
    curl = curl_easy_init();
    headerList = curl_slist_append(headerList, header);
    if (curl)
    {   
        // First set the URL, the target file
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // Set username and password for FTP login
        memset(userPassword, 0, GFAST_MAXMSG_LEN*sizeof(char));
        if (setUserPassword(userName, password, userPassword))
        {
            curl_easy_setopt(curl, CURLOPT_USERPWD, userPassword);
        }
        // HTTP header
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        // upload to this place 
        //curl_easy_setopt(curl, CURLOPT_URL,
        //             "file:///home/dast/src/curl/debug/new");
 
        // tell it to "upload" to the URL 
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
        // set where to read from (on Windows you need to use READFUNCTION too)
        curl_easy_setopt(curl, CURLOPT_READDATA, fd);
 
        // and give the size of the upload (optional) 
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t) fileInfo.st_size);
 
        // enable verbose for easier tracing 
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);
        // Check for errors 
        if (res != CURLE_OK)
        {
            LOG_ERRMSG("curl_easy_perform() failed: %s",
                       curl_easy_strerror(res));
            ierr = 1;
        }
        else
        {
            // now extract transfer info 
            curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &uploadSpeed);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
            LOG_INFOMSG("Speed: %.3f bytes/sec during %.3f seconds\n",
                        uploadSpeed, totalTime);
        }
    }
    else
    {
        LOG_ERRMSG("%s", "Failed to iniitialize curl");
        ierr = 1;
    }
    curl_slist_free_all(headerList);
    // Always clean up
    curl_easy_cleanup(curl);
    // Shut curl down
    curl_global_cleanup();
    fclose(fd);
    return ierr;
}

int postJson(const char *msg, const char *url,
             const char *userName, const char *password)
{
    CURL *curl = NULL;
    CURLcode res;
    struct curl_slist *headerList = NULL;
    static const char header[] = "Content-Type: application/json";
    struct curlMessage_struct upload;
    char userPassword[GFAST_MAXMSG_LEN];
    int ierr;
    ierr = 0;
    // Make sure there's something to do
    if (msg == NULL){return 0;}
    if (strlen(msg) == 0){return 0;}
    // Set the message
    memset(&upload, 0, sizeof(struct curlMessage_struct));
    upload.readptr = msg;
    upload.sizeleft = strlen(msg);
    // Fire up curl
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK)
    {
        LOG_ERRMSG("curl_global_init() failed: %s\n",
                   curl_easy_strerror(res));
        return -1;
    }
    // Get curl handle
    curl = curl_easy_init();
    headerList = curl_slist_append(headerList, header);
    if (curl)
    {
        // First set the URL, the target file
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // Set username and password for FTP login
        memset(userPassword, 0, GFAST_MAXMSG_LEN*sizeof(char));
        if (setUserPassword(userName, password, userPassword))
        {
            curl_easy_setopt(curl, CURLOPT_USERPWD, userPassword);
        }
        // HTTP header
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        // size of the POST data 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(msg));
        // pass in a pointer to the data - libcurl will not copy 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg);
/*
        // Specify we want to UPLOAD one byte at a time
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        // We want to use our own data reader function
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback); 
        // Set the pointer to pass to our read function *
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
        // Set the expected upload size
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t) upload.sizeleft);
*/
        // Get verbose debug output please
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);
        // Check for errors 
        if (res != CURLE_OK)
        {
            LOG_ERRMSG("curl_easy_perform() failed: %s",
                       curl_easy_strerror(res));
            ierr = 1;
        } 
    }
    else
    {
        LOG_ERRMSG("%s", "Failed to iniitialize curl");
        ierr = 1;
    }
    curl_slist_free_all(headerList);
    // Always clean up
    curl_easy_cleanup(curl);
    // Shut curl down
    curl_global_cleanup();
    return ierr;
}

//============================================================================//

#include <jansson.h>
static int parseMessage(const char *message,
                        char *evid, char *archiveFile)
{
    json_t *jm;
    json_error_t error;
    char *s1, *s2;
    int ierr;
    ierr = 0;
    memset(evid, 0, 128*sizeof(char));
    memset(archiveFile, 0, PATH_MAX*sizeof(char));
    jm = json_loads(message, JSON_DECODE_ANY, &error); 
    if (!jm)
    {
        LOG_ERRMSG("%s", "Error parsing message");
        ierr = 1;
    }
    else
    {
        ierr = json_unpack(jm, "{s:s, s:s}",
                           "EventID", &s1, "ArchiveFile", &s2);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error unpacking message");
            ierr = 1;
        }
        else
        {
            strcpy(evid, s1);
            strcpy(archiveFile, s2);
        }
    }
    json_decref(jm);
    return ierr;
}

static char *packMessage(const char *evid, const char *archiveFile)
{
    json_t *jm;
    char *message = NULL;
    jm = json_pack("{s:s, s:s}",
                  "EventID", evid, "ArchiveFile", archiveFile);
    message = json_dumps(jm, JSON_ENCODE_ANY); 
    json_decref(jm);
    return message;
}

int getEventIndex(const char *evid, const struct eventList_struct events) 
{
    int i;
    for (i=0; i<events.nevents; i++)
    {
        if (strcasecmp(evid, events.events[i].evid) == 0){return i;}
    }
    return -1;
}

bool eventExists(const char *evid, const struct eventList_struct events)
{
    int indx;
    indx = getEventIndex(evid, events);
    if (indx >= 0){return true;}
    return false;
}

int freeEvents(struct eventList_struct *events)
{
    if (events->events != NULL){free(events->events);}
    memset(events, 0, sizeof(struct eventList_struct));
    return 0;
}

int copyEvents(const struct eventList_struct eventIn,
               struct eventList_struct *eventOut)
{
    int i;
    memset(eventOut, 0, sizeof(struct eventList_struct));
    eventOut->nevents = eventIn.nevents;
    if (eventIn.nevents > 0)
    {
        eventOut->events = (struct event_struct *)
                           calloc((size_t) eventIn.nevents,
                                  sizeof(struct event_struct));
        for (i=0; i<eventIn.nevents; i++)
        {
            strcpy(eventOut->events[i].evid, eventIn.events[i].evid);
            strcpy(eventOut->events[i].archiveFile,
                   eventIn.events[i].archiveFile);
            eventOut->events[i].lastPublished = eventIn.events[i].lastPublished;
        }
    }
    return 0;
}

int deleteEvent(const char *evid, struct eventList_struct *events)
{
    struct eventList_struct eventsWork;
    int i, indx, j;
    // Get the event index
    indx = getEventIndex(evid, *events);
    if (indx < 0)
    {
        LOG_ERRMSG("Event %s does not exist", evid);
        return 0;
    } 
    // Make a copy
    copyEvents(*events, &eventsWork);
    freeEvents(events);
    // Update the number of events
    events->nevents = eventsWork.nevents - 1;
    // If any events remain then selectively copy them 
    j = 0;
    if (events->nevents > 0)
    {
        events->events = (struct event_struct *)
                         calloc((size_t) events->nevents,
                                sizeof(struct event_struct));
        for (i=0; i<eventsWork.nevents; i++)
        {
            if (i == indx){continue;}
            strcpy(events->events[j].evid, eventsWork.events[i].evid);
            strcpy(events->events[j].archiveFile,
                   eventsWork.events[i].archiveFile);
            events->events[j].lastPublished = eventsWork.events[i].lastPublished;
            j = j + 1;
        }
    }
    freeEvents(&eventsWork);
    return 0;
}

int addEvent(const char *evid, const char *archiveFile,
             struct eventList_struct *events)
{
    struct eventList_struct eventsWork; 
    int i, indx;
    // Get the event index
    indx = getEventIndex(evid, *events);
    if (indx >= 0)
    {
        LOG_WARNMSG("Event %s already exists", evid);
        return 0;
    }
    // Make a copy
    copyEvents(*events, &eventsWork);
    freeEvents(events);
    events->nevents = eventsWork.nevents + 1;
    events->events = (struct event_struct *)
                     calloc((size_t) events->nevents,
                            sizeof(struct event_struct));
    for (i=0; i<eventsWork.nevents; i++)
    {
        strcpy(events->events[i].evid, eventsWork.events[i].evid);
        events->events[i].lastPublished = eventsWork.events[i].lastPublished;
    }
    strcpy(events->events[eventsWork.nevents].evid, evid);
    strcpy(events->events[eventsWork.nevents].archiveFile, archiveFile);
    freeEvents(&eventsWork);
    return 0;
}

static void getCMTopt(const struct GFAST_cmtResults_struct cmt,
                      int *iopt, int *depOpt, int *latOpt, int *lonOpt)
{
    int id, ilat, ilon, ierr;
    *depOpt =-1;
    *latOpt =-1;
    *lonOpt =-1;
    *iopt = array_argmin64f(cmt.nlats*cmt.nlons*cmt.ndeps, cmt.objfn, &ierr);
    for (ilon=0; ilon<cmt.nlons; ilon++)
    {
        for (ilat=0; ilat<cmt.nlats; ilat++)
        {
            for (id=0; id<cmt.ndeps; id++)
            {
                if (ilon*cmt.nlats*cmt.ndeps + ilat*cmt.ndeps + id == *iopt)
                {
                    *depOpt = id; 
                    *latOpt = ilat;
                    *lonOpt = ilon;
                }
            }
        }
    }
    return;
} 


#ifdef __clang__
#pragma clang diagnostic pop
#endif
