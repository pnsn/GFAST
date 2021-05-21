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

//int getEventIndex(const char *evid, const struct eventList_struct events);

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
    //struct eventList_struct events;
    char message[PATH_MAX];
    char evid[128]="1111";
    char archiveFile[PATH_MAX];

    double lastPublished, t0;
    int ierr, iev, i, j, k;
    bool lhaveEvent, leventExists;
    hid_t h5fl;
    char *cm;
    struct beachballPlot_struct beachball;

    char cmtFile[PATH_MAX];
    int bogusID;
    void *consumer;
    void *webProduct;
    iscl_init();
    int output_interval;
    bool output_all = false;
    char temp[64];

    int indx0;
    double u0, n0, e0, peakDisplacement_i;

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

    // Verify the archive file exists
    if (!os_path_isfile(archiveFile))
    {
        fprintf(stderr, "%s: Error archive file %s does not exist\n",
                fcnm, archiveFile);
        exit(8);
        //continue;
    }

    // Open the HDF5 file and read the latest entry
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

struct GFAST_peakDisplacementData_struct pgd_data;
struct h5_peakDisplacementData_struct h5pgd_data;
memset(&pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));
memset(&h5pgd_data, 0, sizeof(struct h5_peakDisplacementData_struct));

char *ccmt, *cdata, *cff, *chypo, *cpgd, *cts;
double *dep, *dip1, *dip2, *Mcmt, *Mpgd, *Meew, *Mff, *rak1, *rak2, *str1, *str2, *times, trigger0;
hid_t cmtDataType, dataSet, dataSpace, dataType, ffDataType, hypoDataType, groupID, memSpace, pgdDataType;
iopt =-1;

    kgroup = hdf5_getMaxGroupNumber(h5fl);
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

    // MTH:
    igroup = output_interval;
    igroup = 18;
    igroup = 120;

    memset(groupName, 0, 512*sizeof(char));
    sprintf(groupName, "/GFAST_History/Iteration_%d", igroup);
    groupID = H5Gopen2(h5fl, groupName, H5P_DEFAULT);

    /*
    hid_t attr;
    int32 attr_data;
    attr=H5Aopen(dataset, "epoch", H5P_DEFAULT);
    H5Aread(attr,H5T_INT32,attr_data);
    H5Aclose(attr);
    */

    hid_t attr;
    double epoch;
    dataSet   = H5Dopen2("/GFAST_History", "Iteration_120", H5P_DEFAULT);
    attr=H5Aopen(dataSet, "epoch", H5P_DEFAULT);
    H5Aread(attr, H5T_IEEE_F64LE, epoch);
    H5Aclose(attr);
    H5Dclose(dataSet);
    printf("MTH: got epoch:%ld\n", epoch);

    // hypocenter
    memset(&h5hypo, 0, sizeof(struct h5_hypocenter_struct));
    memset(&hypo, 0, sizeof(struct GFAST_shakeAlert_struct));
    dataType  = H5Topen(h5fl, HYPO_STRUCT, H5P_DEFAULT);
    dataSet   = H5Dopen(groupID, TRIGGER_HYPO, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    memSpace  = H5Screate_simple(1, dims, NULL);
    H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5hypo);
    hdf5_copyHypocenter(COPY_H5_TO_DATA, &hypo, &h5hypo);

    printf("igroup:%d hypo eventid:%s lat:%8.2f lon:%8.2f dep:%5.2f mag:%4.2f time:%f\n",
        igroup, hypo.eventid, hypo.lat, hypo.lon, hypo.dep, hypo.mag, hypo.time);

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
    dataType  = H5Topen(h5fl, "/DataStructures/gpsDataStructure\0", H5P_DEFAULT);
    dataSet   = H5Dopen(groupID, "gpsData\0", H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    memSpace  = H5Screate_simple(1, dims, NULL);
    H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5gpsData);
    hdf5_copyGPSData(COPY_H5_TO_DATA, &gpsData, &h5gpsData);

    hdf5_memory_freeGPSData(&h5gpsData);
    H5Sclose(memSpace);
    H5Sclose(dataSpace);
    H5Dclose(dataSet);
    H5Tclose(dataType);


    // peakDisplacementData
    if (H5Lexists(groupID, "pgdData", H5P_DEFAULT) > 0)
    {
        dataType  = H5Topen(h5fl, "/DataStructures/peakDisplacementDataStructure\0", H5P_DEFAULT);
        dataSet   = H5Dopen(groupID, "pgdData", H5P_DEFAULT);
        dataSpace = H5Dget_space(dataSet);
        memSpace  = H5Screate_simple(1, dims, NULL);
        H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5pgd_data);
        hdf5_copyPeakDisplacementData(COPY_H5_TO_DATA, &pgd_data, &h5pgd_data);

        // Print out raw GPS data for PGD active sites
        for (i=0; i<pgd_data.nsites; i++){
          if (pgd_data.lactive[i]){
            printf("i:%4d scnl:%s lat:%8.3f lon:%8.3f wt:%.1f active:%d pd:%12.4e\n", 
                    i, pgd_data.stnm[i], pgd_data.sta_lat[i], pgd_data.sta_lon[i],
                    pgd_data.wt[i], pgd_data.lactive[i], pgd_data.pd[i]);


            for (k=0;k<gpsData.stream_length; k++){
                wdata = gpsData.data[k];
                memset(temp, 0, sizeof(temp));
                sprintf(temp, "%s.%s.%s.%s", wdata.netw, wdata.stnm, wdata.chan[0], wdata.loc);
                if (strcmp(pgd_data.stnm[i], temp) == 0) {

                  indx0 = 0;
                  u0 = wdata.ubuff[indx0];
                  n0 = wdata.nbuff[indx0];
                  e0 = wdata.ebuff[indx0];

                  if (isnan(u0) || isnan(n0) || isnan(e0)){
                    for (j=indx0; j<wdata.npts; j++){
                      if (!isnan(wdata.ubuff[j]) && !isnan(wdata.nbuff[j]) && !isnan(wdata.ebuff[j])){
                          indx0 = j;
                          u0 = wdata.ubuff[indx0];
                          n0 = wdata.nbuff[indx0];
                          e0 = wdata.ebuff[indx0];
                          LOG_MSG("Search leader for t0:  nMax:%d indx0:%d u0:%f n0:%f e0:%f",
                                  wdata.npts, indx0, u0, n0, e0);
                          break;
                      }
                    }
                  }

                  printf("nscl:%s lat:%8.3f lon:%8.3f npts:%d indx0:%d pd:%f\n",
                         temp, wdata.sta_lat, wdata.sta_lon, wdata.npts, indx0, pgd_data.pd[i]);

                  printf("%10s %9s %9s %9s %9s\n", "tbuff", "ubuff", "nbuff", "ebuff", "PGD");
                  for (j=0; j<wdata.npts; j++){
                    peakDisplacement_i = sqrt( pow(wdata.ubuff[j] - u0, 2)
                                             + pow(wdata.nbuff[j] - n0, 2)
                                             + pow(wdata.ebuff[j] - e0, 2));

                    printf("%.3f %9.6f %9.6f %9.6f %9.6f\n", wdata.tbuff[j], wdata.ubuff[j], 
                        wdata.nbuff[j], wdata.ebuff[j], peakDisplacement_i);
                  }

                 break;
                }
            }
          }
        }
        hdf5_memory_freePGDData(&h5pgd_data);
        H5Sclose(memSpace);
        H5Sclose(dataSpace);
        H5Dclose(dataSet);
        H5Tclose(dataType);
        //core_scaling_pgd_finalizeData(&pgd_data);
    }

    exit(0);

    // pgdResults
    memset(&h5pgd, 0, sizeof(struct h5_pgdResults_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    dataType = H5Topen(h5fl, PGD_STRUCT, H5P_DEFAULT);
    dataSet = H5Dopen(groupID, PGD_RES, H5P_DEFAULT);
    dataSpace = H5Dget_space(dataSet);
    memSpace = H5Screate_simple(1, dims, NULL);
    H5Dread(dataSet, dataType, memSpace, dataSpace, H5P_DEFAULT, &h5pgd);
    hdf5_copyPGDResults(COPY_H5_TO_DATA, &pgd, &h5pgd);

    //printf("MTH: pgdResults.nsites=%d\n", pgd.nsites);
    //for (i=0; i<pgd.nsites; i++){
      //printf("lsiteUsed[%i]: %d\n", i, pgd.lsiteUsed[i]);
    //}
    //cpgd = gfast2json_packPGD(evid, gpsData, pgd);
    hdf5_memory_freePGDResults(&h5pgd);
    H5Sclose(memSpace);
    H5Sclose(dataSpace);
    H5Dclose(dataSet);
    H5Tclose(dataType);
    //core_scaling_pgd_finalizeResults(&pgd);

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
            printf("MTH: call core_data_finalize gpsData DONE\n");


            H5Gclose(groupID);
            H5Fclose(h5fl);


    /* MTH: not using this
    freeEvents(&events);
    activeMQ_consumer_finalize(consumer);
    if (ldebug){activeMQ_producer_finalize(webProduct);}
    */
    //curl_easy_cleanup(curl);
    //iscl_finalize();
    return EXIT_SUCCESS;
}


/*
int getEventIndex(const char *evid, const struct eventList_struct events) 
{
    int i;
    for (i=0; i<events.nevents; i++)
    {
        if (strcasecmp(evid, events.events[i].evid) == 0){return i;}
    }
    return -1;
}
*/



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
