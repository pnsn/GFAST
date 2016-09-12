#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/iscl/iscl.h"
#include "iscl/time/time.h"

int main()
{
    const char *fcnm = "gfast_playback\0";
    char propfilename[] = "gfast.props\0";
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
    char *elarms_xml_message;
    double currentTime, dtmax, t0sim, tbeg;
    const enum opmode_type opmode = OFFLINE;
    int ierr, im, k, kt, message_length, ntsim;
    //------------------------------------------------------------------------//
    //
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
    ISCL_iscl_init(); // Fire up the computational library
    // Read the properties file
    log_infoF("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0) 
    {
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
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
        log_errorF("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }
    // Initialize PGD
    ierr = GFAST_core_scaling_pgd_initialize(props.pgd_props, gps_data,
                                             &pgd, &pgd_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_core_cmt_initialize(props.cmt_props, gps_data,
                                     &cmt, &cmt_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_core_ff_initialize(props.ff_props, gps_data,
                                    &ff, &ff_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Set the trace buffer names and open the HDF5 datafile
    ierr = GFAST_traceBuffer_h5_setTraceBufferFromGFAST(props.bufflen,
                                                        gps_data,
                                                        &h5traceBuffer);
    if (ierr != 0)
    {
        log_errorF("%s: Error setting the H5 tracebuffer\n", fcnm);
        goto ERROR;
    }
    ierr = GFAST_traceBuffer_h5_initialize(1, true, "./\0",
                                           props.obsdata_file, &h5traceBuffer);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing H5 traebuffer\n", fcnm);
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
        log_infoF("%s: Number of time steps in simulation: %d\n",
                  fcnm, ntsim); 
    }
    props.processingTime = fmin(props.processingTime,
                                (double) (ntsim - 1)*dtmax);
    // Read the elarms file once and for all
    elarms_xml_file = fopen(props.eewsfile, "rb");
    fseek(elarms_xml_file, 0L, SEEK_END);
    message_length = ftell(elarms_xml_file);
    rewind(elarms_xml_file);
    elarms_xml_message = (char *)calloc(message_length+1, sizeof(char));
    if (fread(elarms_xml_message, message_length, 1, elarms_xml_file) == 0)
    {
        log_errorF("%s: Error reading xml file\n", fcnm);
        goto ERROR;
    }
    fclose(elarms_xml_file);
    // Make the origin time the start time
    ierr = GFAST_eewUtils_parseCoreXML(elarms_xml_message, -12345.0, &SA);
    if (ierr != 0)
    {   
        log_errorF("%s: Error parsing XML message\n", fcnm);
        return ierr;
    }
    t0sim = SA.time;
    tbeg = time_timeStamp();
    // Loop on time steps in simulation
    for (kt=0; kt<ntsim; kt++)
    {
        // Compute the current time
        currentTime = t0sim + (double) kt*dtmax;
        ierr = eewUtils_driveGFAST(currentTime,
                                   elarms_xml_message,
                                   &gps_data,
                                   &h5traceBuffer,
                                   props,
                                   &pgd_data,
                                   &cmt_data,
                                   &ff_data,
                                   &events,
                                   &pgd,
                                   &cmt,
                                   &ff,
                                   &xmlMessages);
         if (ierr != 0)
         {
             log_errorF("%s: Error calling GFAST driver!\n", fcnm);
             break;
         }
         if (xmlMessages.mmessages > 0)
         {
             for (im=0; im<xmlMessages.nmessages; im++)
             {
                 if (xmlMessages.evids[im] != NULL)
                 {
                     free(xmlMessages.evids[im]);
                     xmlMessages.evids = NULL;
                 }
                 if (xmlMessages.cmtQML[im] != NULL)
                 {
                     free(xmlMessages.cmtQML[im]);
                     xmlMessages.cmtQML = NULL;
                 }
                 if (xmlMessages.pgdXML[im] != NULL)
                 {
                     free(xmlMessages.pgdXML[im]);
                     xmlMessages.pgdXML = NULL;
                 }
                 if (xmlMessages.ffXML[im] != NULL)
                 {
                     free(xmlMessages.ffXML[im]);
                     xmlMessages.ffXML = NULL;
                 }
             }
             if (xmlMessages.evids == NULL){free(xmlMessages.evids);}
             if (xmlMessages.cmtQML == NULL){free(xmlMessages.cmtQML);}
             if (xmlMessages.ffXML  == NULL){free(xmlMessages.ffXML);}
             if (xmlMessages.pgdXML == NULL){free(xmlMessages.pgdXML);}
             memset(&xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
         }
    }
    log_infoF("%s: Simultation time: %f\n", fcnm, time_timeStamp() - tbeg);
ERROR:;
    if (elarms_xml_message != NULL){free(elarms_xml_message);}
    GFAST_core_cmt_finalize(&props.cmt_props,
                            &cmt_data,
                            &cmt);
    GFAST_core_ff_finalize(&props.ff_props,
                           &ff_data,
                           &ff);
    GFAST_core_scaling_pgd_finalize(&props.pgd_props,
                                    &pgd_data,
                                    &pgd);
    GFAST_core_data_finalize(&gps_data);
    GFAST_core_properties_finalize(&props);
    GFAST_core_data_finalize(&gps_data);
    GFAST_traceBuffer_h5_finalize(&h5traceBuffer);
    ISCL_iscl_finalize();
    if (ierr != 0)
    {   
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
