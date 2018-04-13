#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast.h"
#include "gfast_eewUtils.h"
#include "iscl/iscl/iscl.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"

int main(int argc, char *argv[])
{
    const char *fcnm = "gfast_playback\0";
    char propfilename[PATH_MAX]; // = "gfast.props\0";
    FILE *elarms_xml_file;
    struct GFAST_activeEvents_struct events;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_data_struct gps_data;
    struct GFAST_ffResults_struct ff; 
    struct h5traceBuffer_struct h5traceBuffer;
    struct GFAST_offsetData_struct cmt_data, ff_data;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_props_struct props;
    struct GFAST_shakeAlert_struct SA;
    struct GFAST_xmlMessages_struct xmlMessages;
    char errorLogFileName[PATH_MAX];
    char infoLogFileName[PATH_MAX];
    char debugLogFileName[PATH_MAX];
    char warnLogFileName[PATH_MAX];
    char *elarms_xml_message;
    double currentTime, dtmax, t0sim, tbeg;
    const enum opmode_type opmode = OFFLINE;
    int ierr, im, k, kt, ntsim;
    bool lnewEvent;
    size_t message_length;
    //------------------------------------------------------------------------//
    //
    // Read the input file
    iscl_init();
    if (argc != 2)
    {
        printf("Usage: %s property_file_name\n", fcnm);
        printf("Example: %s gfast.props\n", fcnm);
        return EXIT_FAILURE;
    }
    memset(propfilename, 0, sizeof(propfilename));
    strcpy(propfilename, argv[1]);
    if (!os_path_isfile(propfilename))
    {
        LOG_ERRMSG("%s: Properties file %s doesn't exist\n",
                   fcnm, propfilename);
        return EXIT_FAILURE;
    }
    // Initialize 
    ierr = 0;
    elarms_xml_message = NULL;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
    memset(&h5traceBuffer, 0, sizeof(struct h5traceBuffer_struct)); 
    // Read the properties file
    LOG_INFOMSG("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0) 
    {
        LOG_ERRMSG("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2)
    {
        GFAST_core_properties_print(props);
    }
    // Initialize the buffers
    ierr = GFAST_core_data_initialize(props, &gps_data); 
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }
    // Initialize PGD
    ierr = GFAST_core_scaling_pgd_initialize(props.pgd_props, gps_data,
                                             &pgd, &pgd_data);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_core_cmt_initialize(props.cmt_props, gps_data,
                                     &cmt, &cmt_data);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_core_ff_initialize(props.ff_props, gps_data,
                                    &ff, &ff_data);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Set the trace buffer names and open the HDF5 datafile
    ierr = GFAST_traceBuffer_h5_setTraceBufferFromGFAST(props.bufflen,
                                                        gps_data,
                                                        &h5traceBuffer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error setting the H5 tracebuffer\n", fcnm);
        goto ERROR;
    }
    ierr = GFAST_traceBuffer_h5_initialize(1, true, props.obsdataDir,
                                           props.obsdataFile, &h5traceBuffer);
    if (ierr != 0)
    {
        LOG_ERRMSG("%s: Error initializing H5 traebuffer\n", fcnm);
        goto ERROR;
    }

    // Compute the runtime
    dtmax = 0.0;
    for (k=0; k<gps_data.stream_length; k++)
    {
        dtmax = fmax(dtmax, gps_data.data[k].dt);
    }
    ntsim = (int) (props.synthetic_runtime/dtmax + 0.5); // ignore + 1;
    if (props.verbose > 0)
    {   
        LOG_INFOMSG("%s: Number of time steps in simulation: %d\n",
                    fcnm, ntsim); 
    }
    props.processingTime = fmin(props.processingTime,
                                (double) (ntsim - 1)*dtmax);
    // Read the elarms file once and for all
    elarms_xml_file = fopen(props.eewsfile, "rb");
    fseek(elarms_xml_file, 0L, SEEK_END);
    message_length = (size_t) (ftell(elarms_xml_file));
    rewind(elarms_xml_file);
    elarms_xml_message = (char *)calloc(message_length + 1, sizeof(char));
    if (fread(elarms_xml_message, message_length, 1, elarms_xml_file) == 0)
    {
        LOG_ERRMSG("%s: Error reading xml file\n", fcnm);
        goto ERROR;
    }
    fclose(elarms_xml_file);
    // Make the origin time the start time
    ierr = GFAST_eewUtils_parseCoreXML(elarms_xml_message, -12345.0, &SA);
    if (ierr != 0)
    {   
        LOG_ERRMSG("%s: Error parsing XML message\n", fcnm);
        return ierr;
    }
    t0sim = SA.time;
    tbeg = time_timeStamp();
    // Loop on time steps in simulation
    for (kt=0; kt<ntsim; kt++)
    {
        // Parse the XML message
        if (elarms_xml_message != NULL)
        {
            ierr = GFAST_eewUtils_parseCoreXML(elarms_xml_message, SA_NAN, &SA);
            if (ierr != 0)
            {
                LOG_ERRMSG("%s: Error parsing XML message\n", fcnm);
                return ierr;
            }
            // If this is a new event we have some file handling to do
            lnewEvent = GFAST_core_events_newEvent(SA, &events);
            if (lnewEvent)
            {
                // And the logs
                if (props.verbose > 0)
                {
                    LOG_INFOMSG("%s: New event %s added\n", fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_core_events_printEvents(SA);}
                }
                // Set the log file names
                eewUtils_setLogFileNames(SA.eventid,
                                         errorLogFileName, infoLogFileName,
                                         debugLogFileName, warnLogFileName);
                if (os_path_isfile(errorLogFileName))
                {
                    remove(errorLogFileName);
                }
                if (os_path_isfile(infoLogFileName))
                {
                    remove(infoLogFileName);
                }
                if (os_path_isfile(debugLogFileName))
                {
                   remove(debugLogFileName);
                }
                if (os_path_isfile(warnLogFileName))
                {
                   remove(warnLogFileName);
                }
                // Initialize the HDF5 file
                ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                             SA.eventid,
                                             props.propfilename);
                if (ierr != 0)    
                {
                    LOG_ERRMSG("%s: Error initializing the archive file\n",
                               fcnm);
                    return -1;
                }
            }
        }
        // Compute the current time
        currentTime = t0sim + (double) kt*dtmax;
        ierr = eewUtils_driveGFAST(currentTime,
                                   props,
                                   &events,
                                   &gps_data,
                                   &h5traceBuffer,
                                   &pgd_data,
                                   &cmt_data,
                                   &ff_data,
                                   &pgd,
                                   &cmt,
                                   &ff,
                                   &xmlMessages);
         if (ierr != 0)
         {
             LOG_ERRMSG("%s: Error calling GFAST driver!\n", fcnm);
             break;
         }
         if (xmlMessages.mmessages > 0)
         {
             for (im=0; im<xmlMessages.nmessages; im++)
             {
                 if (xmlMessages.evids[im] != NULL)
                 {
                     free(xmlMessages.evids[im]);
                 }
                 if (xmlMessages.cmtQML[im] != NULL)
                 {
                     free(xmlMessages.cmtQML[im]);
                 }
                 if (xmlMessages.pgdXML[im] != NULL)
                 {
                     free(xmlMessages.pgdXML[im]);
                 }
                 if (xmlMessages.ffXML[im] != NULL)
                 {
                     free(xmlMessages.ffXML[im]);
                 }
             }
             if (xmlMessages.evids != NULL){free(xmlMessages.evids);}
             if (xmlMessages.cmtQML != NULL){free(xmlMessages.cmtQML);}
             if (xmlMessages.ffXML  != NULL){free(xmlMessages.ffXML);}
             if (xmlMessages.pgdXML != NULL){free(xmlMessages.pgdXML);}
             memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
         }
    }
    LOG_INFOMSG("%s: Simultation time: %f\n", fcnm, time_timeStamp() - tbeg);
ERROR:;
    if (elarms_xml_message != NULL){free(elarms_xml_message);}
    core_cmt_finalize(&props.cmt_props,
                      &cmt_data,
                      &cmt);
    core_ff_finalize(&props.ff_props,
                     &ff_data,
                     &ff);
    core_scaling_pgd_finalize(&props.pgd_props,
                              &pgd_data,
                              &pgd);
    core_data_finalize(&gps_data);
    core_properties_finalize(&props);
    core_data_finalize(&gps_data);
    core_events_freeEvents(&events);
    traceBuffer_h5_finalize(&h5traceBuffer);
    iscl_finalize();
    if (ierr != 0)
    {   
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
