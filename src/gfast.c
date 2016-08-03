#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast.h"
#include "iscl/log/log.h"

/*!
 * This is a mock GFAST driver module
 */
int main()
{
    char fcnm[] = "GFAST\0";
    char propfilename[] = "gfast.props\0"; /* TODO take from EW config file */
    char *cmtQML, *ffXML, ffXMLflnm[PATH_MAX], cmtQMLflnm[PATH_MAX];
    FILE *elarms_xml_file, *ffXMLfl, *cmtQMLfl;
    struct GFAST_props_struct props;
    struct GFAST_data_struct gps_acquisition;
    struct GFAST_shakeAlert_struct SA;
    struct GFAST_activeEvents_struct events;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_ffResults_struct ff;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_offsetData_struct cmt_data, ff_data;
    char *elarms_xml_message;
    double *latency, mtopt[6], currentTime, dtmax, eventTime, t0sim;
    long message_length;
    int iev, iopt, h5k, k, kt, nsites_cmt, nsites_ff,
        nsites_pgd, nstrdip, ntsim, verbose0;
    int ierr = 0;
    bool lcmt_success, lff_success, lpgd_success,
         ldel_event, lnew_event, lupd_event;
    //------------------------------------------------------------------------//
    // 
    // Initializations
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_acquisition, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    latency = NULL;
    // Read the properties file
    log_infoF("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_properties__init(propfilename, &props);
printf("%f\n", props.synthetic_runtime);
    if (ierr != 0)
    {
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2)
    {
        GFAST_properties__print(props);
    }
    // Initialize the stations locations/names for the module
    if (props.verbose > 0)
    {
        log_infoF("%s: Initializing the acquisition...\n", fcnm);
    }
    ierr =  GFAST_acquisition__init(props, &gps_acquisition);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializating acquisition\n", fcnm);
        goto ERROR;
    }
    // Initialize PGD
    ierr = GFAST_core_scaling_pgd_initialize(props.pgd_props, gps_acquisition,
                                             &pgd, &pgd_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_core_cmt_initialize(props.cmt_props, gps_acquisition,
                                     &cmt, &cmt_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_core_ff_initialize(props.ff_props, gps_acquisition,
                                    &ff, &ff_data);
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Connect to EW trace buffer and ring with ElarmS messages 
    //if (props.opmode != OFFLINE)
    //{
    //    log_infoF("%s: Connecting to Earthworm rings...\n", fcnm);
    //}
    if (props.verbose > 2)
    {
        GFAST_buffer_print__samplingPeriod(gps_acquisition);
    }
    // Connect ActiveMQ for ElarmS messages
    if (props.verbose >= 2)
    {
        log_infoF("%s: Subscribing to ElarmS messages...\n", fcnm);
    }
    //GFAST::Buffer init();
 //GFAST_readElarmS(props);
    // Loop on acquisition
    //while (true)
    //{
    //  // Check for input signals from user
    //
    //  // Get the latest and greatest data onto the buffers
    //
    //  // Check for an alarm
    //
    //  // Alarm detected.  Process it!
    //} // Loop on acquisition
    //------------------------------------------------------------------------//
    //                            GFAST OFFLINE MODE                          //
    //------------------------------------------------------------------------//
    if (props.opmode == OFFLINE)
    {
        if (props.verbose >= 2)
        {
            log_infoF("%s: Beginning simulation...\n", fcnm);
        }
        // Compute the runtime - goal is to keep up with `slowest' data
        dtmax = 0.0;
        for (k=0; k<gps_acquisition.stream_length; k++)
        {
            dtmax = fmax(dtmax, gps_acquisition.data[k].dt);
        }
        ntsim = (int) (props.synthetic_runtime/dtmax + 0.5); // ignore + 1;
        if (props.verbose > 0)
        {
            log_infoF("%s: Number of time steps in simulation: %d\n",
                      fcnm, ntsim); 
        }
        // Set simulation start time to the earliest time in the SAC headers
        t0sim = GFAST_acquisition__getT0FromSAC(props,
                                                gps_acquisition,
                                                &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error setting t0 for simulation!\n", fcnm);
            goto ERROR;
        }
        // Make sure we can finish this event
        props.processingTime 
             = fmin(props.processingTime, (double) (ntsim - 2));
        if (props.verbose > 1)
        {
            log_infoF("%s: Simulation start time is: %lf\n", fcnm, t0sim);
        }
        // Save verbose because it may be toggled on and off in the loop
        verbose0 = props.verbose;
        // Loop on time-steps in simulation
//ntsim = 59;
        for (kt=0; kt<ntsim; kt++)
        {
//kt = 299;
//for (kt=58; kt<59; kt++){
            // Update the time
            currentTime = t0sim + (double) kt;
            // Read the elarmS/shakeAlert XML message file
            elarms_xml_file = fopen(props.eewsfile, "rb");
            fseek(elarms_xml_file, 0L, SEEK_END);
            message_length = ftell(elarms_xml_file);
            rewind(elarms_xml_file);
            elarms_xml_message = (char *)calloc(message_length+1, sizeof(char));
            if (fread(elarms_xml_message, message_length,
                      1, elarms_xml_file) == 0)
            {
                log_errorF("%s: Error reading xml file\n", fcnm);
                goto ERROR;
            }
            fclose(elarms_xml_file);
            // Parse the shakeAlert message into the shakeAlert struture 
            ierr = GFAST_readElarmS__xml(elarms_xml_message, -12345.0,
                                         &SA);
            free(elarms_xml_message);
            elarms_xml_message = NULL;

            /*
            if (kt > 0){props.verbose = 0;}
            ierr = GFAST_readElarmS(props, &SA);
            */
            props.verbose = verbose0;
            if (ierr != 0)
            {
                log_errorF("%s: Error reading shakeAlert message!\n", fcnm);
                continue;
            }
            // Is this a new event?  If so then add it to events
            lnew_event = GFAST_events__newEvent(SA, &events);
            // Set the log file names
            memset(errorLogFileName, 0, sizeof(errorLogFileName));
            strcpy(errorLogFileName, SA.eventid);
            strcat(errorLogFileName, "_error.log\0");
            memset(infoLogFileName, 0, sizeof(infoLogFileName));
            strcpy(infoLogFileName, SA.eventid);
            strcat(infoLogFileName, "_info.log\0");
            memset(debugLogFileName, 0, sizeof(debugLogFileName));
            strcpy(debugLogFileName, SA.eventid);
            strcat(debugLogFileName, "_debug.log\0");
            memset(warnLogFileName, 0, sizeof(warnLogFileName));
            strcpy(warnLogFileName, SA.eventid);
            strcat(warnLogFileName, "_debug.log\0");
            if (lnew_event)
            {
                if (props.verbose > 0)
                {
                    log_infoF("%s: New event %s added\n", fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_events__print(SA);}
                }
                if (os_path_isfile(errorLogFileName)){remove(errorLogFileName);}
                if (os_path_isfile(infoLogFileName)){remove(infoLogFileName);}
                if (os_path_isfile(debugLogFileName)){remove(debugLogFileName);}
                if (os_path_isfile(warnLogFileName)){remove(warnLogFileName);}
                log_initErrorLog(&__errorToLog);
                log_initInfoLog(&__infoToLog);
                log_initDebugLog(&__debugToLog);
                log_initWarnLog(&__warnToLog);
                // Initialize the archive
                if (props.verbose > 0)
                {
                    log_infoF("%s: New event %s added\n", fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_events__print(SA);}
                    log_infoF("%s: Initializing archive\n", fcnm);
                }
                ierr = GFAST_HDF5__initialize(props.h5ArchiveDir,
                                              SA.eventid,
                                              propfilename);
            }
            // Not a new event
            else
            {
                // Has the event been updated?
                lupd_event = GFAST_events__updateEvent(SA, &events, &ierr);
                if (ierr != 0)
                {
                    log_errorF("%s: There was an error updating event %s\n",
                               fcnm, SA.eventid);
                }
                if (props.verbose > 0 && lupd_event)
                {
                    log_infoF("%s: Event %s has been modified\n",
                              fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_events__print(SA);}
                }
            }
            log_initErrorLog(&__errorToLog);
            log_initInfoLog(&__infoToLog);
            log_initDebugLog(&__debugToLog);
            log_initWarnLog(&__warnToLog);
            // Acquire the data
            eventTime = SA.time;
            ierr = GFAST_acquisition__updateFromSAC(props,
                                                    t0sim,
                                                    eventTime,
                                                    currentTime,
                                                    latency,
                                                    &gps_acquisition);
            if (ierr != 0)
            {
                log_errorF("%s: An error was encountered reading the data \n",
                           fcnm);
            }
            // Loop on the events
//printf("%f %d\n", currentTime - SA.time, kt);
            for (iev=0; iev<events.nev; iev++)
            {
                // Extract the peak displacement from the waveform buffer
                nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
                                    props.pgd_props.utm_zone,
                                    props.pgd_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    gps_acquisition,
                                    &pgd_data,
                                    &ierr);
                // Extract the offset for the CMT inversion from the buffer 
                nsites_cmt = GFAST_core_waveformProcessor_offset(
                                    props.cmt_props.utm_zone,
                                    props.cmt_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    gps_acquisition,
                                    &cmt_data,
                                    &ierr);
                // Extract the offset for the FF inversion from the buffer 
                nsites_ff = GFAST_core_waveformProcessor_offset(
                                    props.ff_props.utm_zone,
                                    props.ff_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    gps_acquisition,
                                    &ff_data,
                                    &ierr);
                // Run the PGD scaling
                lpgd_success = false;
                if (nsites_pgd >= props.pgd_props.min_sites)
                {
                    if (props.verbose > 2)
                    {
                        log_infoF("%s: Estimating PGD scaling...\n", fcnm);
                    } 
                    lpgd_success = true;
                    ierr = GFAST_scaling_PGD__driver(props.pgd_props,
                                                     SA.lat, SA.lon, SA.dep,
                                                     pgd_data,
                                                     &pgd);
                    if (ierr != PGD_SUCCESS)
                    {
                        log_errorF("%s: Error computing PGD\n", fcnm);
                        lpgd_success = false;
                    }
                }
//props.verbose = verbose0;
                lcmt_success = false;
                if (nsites_cmt >= props.cmt_props.min_sites)
                {
                    lcmt_success = true;
                    ierr = GFAST_CMT__driver(props.cmt_props,
                                             SA.lat, SA.lon, SA.dep,
                                             cmt_data,
                                             &cmt);
                    if (ierr != CMT_SUCCESS)
                    {
                        log_errorF("%s: Error computing CMT\n", fcnm);
                        lcmt_success = false;
                    }
                }
                // If we got a CMT see if we can run an FF inversion  
                lff_success = false;
                if (lcmt_success && nsites_ff >= props.ff_props.min_sites)
                {
                    ff.nfp = 2;
                    ff.SA_lat = events.SA[iev].lat;
                    ff.SA_lon = events.SA[iev].lon;
                    ff.SA_dep = cmt.srcDepths[cmt.opt_indx];
                    ff.SA_mag = cmt.Mw[cmt.opt_indx];
                    ff.str[0] = cmt.str1[cmt.opt_indx];
                    ff.str[1] = cmt.str2[cmt.opt_indx];
                    ff.dip[0] = cmt.dip1[cmt.opt_indx];
                    ff.dip[1] = cmt.dip2[cmt.opt_indx];
                    lff_success = true;
                    ierr = GFAST_FF__driver(props.ff_props,
                                            SA.lat, SA.lon, SA.dep,
                                            ff_data,
                                            &ff);
                    if (ierr != FF_SUCCESS)
                    {
                        log_errorF("%s: Error computing finite fault\n", fcnm);
                        lff_success = false;
                    }
                }
                // Am I ready to publish this event?
                if (currentTime - SA.time >= props.processingTime)
                {
                    if (props.verbose > 0)
                    {
                        log_infoF("%s: Publishing event: %s\n",
                                  fcnm, SA.eventid);
                    }
                    // Generate the CMT XML message
                    if (lcmt_success)
                    {
                        if (props.verbose > 0)
                        {
                            log_infoF("%s: Making CMT quakeML...\n", fcnm);
                        }
                        memcpy(mtopt, &cmt.mts[cmt.opt_indx], 6*sizeof(double));
                        cmtQML = NULL;
/*
                        cmtQML = GFAST_CMT__makeQuakeML("UW\0",
                                                  "anss.org\0",
                                                  events.SA[iev].eventid,
                                                  events.SA[iev].lat,
                                                  events.SA[iev].lon,
                                                  cmt.srcDepths[cmt.opt_indx],
                                                  events.SA[iev].time,
                                                  mtopt,
                                                  &ierr);
                        if (ierr == 0)
                        {

                            memset(&cmtQMLflnm, 0, sizeof(cmtQMLflnm));
                            strcpy(cmtQMLflnm, events.SA[iev].eventid);
                            strcat(cmtQMLflnm, "_quake.xml\0");
                            cmtQMLfl = fopen(cmtQMLflnm, "w");
                            fprintf(cmtQMLfl, "%s", cmtQML);
                            fclose(cmtQMLfl);
                        }
                        else
                        {
                            log_errorF("%s: Error making CMT QML\n", fcnm);
                            
                        }
*/
                        if (cmtQML != NULL){free(cmtQML);}
                        cmtQML = NULL;
                    }
                    // Generate the finite-fault XML for shakeAlert
                    if (lff_success)
                    {
                        if (props.verbose > 0)
                        {
                            log_infoF("%s: Making FF XML...\n", fcnm);
                        }
                        iopt = ff.preferred_fault_plane;
                        nstrdip = ff.fp[iopt].nstr*ff.fp[iopt].ndip;
/*
                        ffXML = GFAST_FF__makeXML(props.opmode,
                                                  "GFAST\0",
                                                   GFAST_ALGORITHM_VERSION,
                                                   GFAST_INSTANCE,
                                                   "new\0",
                                                   GFAST_VERSION,
                                                   events.SA[iev].eventid,
                                                   events.SA[iev].lat,
                                                   events.SA[iev].lon,
                                                   events.SA[iev].dep,
                                                   events.SA[iev].mag,
                                                   events.SA[iev].time,
                                                   nstrdip,
                                                   ff.fp[iopt].fault_ptr,
                                                   ff.fp[iopt].lat_vtx,
                                                   ff.fp[iopt].lon_vtx,
                                                   ff.fp[iopt].dep_vtx,
                                                   ff.fp[iopt].sslip,
                                                   ff.fp[iopt].dslip,
                                                   ff.fp[iopt].sslip_unc,
                                                   ff.fp[iopt].dslip_unc,
                                                   &ierr);
                        if (ierr == 0)
                        {
                            memset(&ffXMLflnm, 0, sizeof(ffXMLflnm));
                            strcpy(ffXMLflnm, events.SA[iev].eventid);
                            strcat(ffXMLflnm, ".xml\0");
                            ffXMLfl = fopen(ffXMLflnm, "w");
                            fprintf(ffXMLfl, "%s", ffXML);
                            fclose(ffXMLfl);
                        }
                        else
                        {
                            log_errorF("%s: Error making FF XML\n", fcnm);
                        }
*/
                        if (ffXML != NULL){free(ffXML);}
                        ffXML = NULL;
                    }
                    ldel_event = GFAST_events__removeEvent(props.processingTime,
                                                           currentTime,
                                                           props.verbose,
                                                           SA,
                                                           &events);
                    if (ldel_event && props.verbose > 0)
                    {
                        log_infoF("%s: Deleted event %s\n", fcnm, SA.eventid);
                    }
                } // End check on publishing event
                // Get the iteration number in the H5 file
                h5k = 0;
                if (lpgd_success || lcmt_success || lff_success)
                {
                    h5k = GFAST_HDF5__update__getIteration(props.h5ArchiveDir,
                                                           SA.eventid,
                                                           currentTime);
                }
                // Update the archive
                if (lpgd_success)
                {
                    ierr = GFAST_HDF5__update__pgd(props.h5ArchiveDir,
                                                   SA.eventid,
                                                   h5k,
                                                   pgd_data,
                                                   pgd);
                }
                if (lcmt_success)
                {
                    ierr = GFAST_HDF5__update__cmt(props.h5ArchiveDir,
                                                   SA.eventid,
                                                   h5k,
                                                   cmt_data,
                                                   cmt);
                }
                if (lff_success)
                {
                    ierr = GFAST_HDF5__update__ff(props.h5ArchiveDir,
                                                  SA.eventid,
                                                  h5k,
                                                  ff_data,
                                                  ff);
                }
            } // Loop on active events
            log_closeLogs();
        }
/*
        while (true)
        {

        } // Loop on acquisition
*/
    }
ERROR:;
    if (ierr != 0 && props.verbose > 0)
    {
        log_errorF("%s: Errors were encountered\n", fcnm);
    }
    if (props.verbose >= 2)
    {
        log_infoF("%s: Freeing memory...\n", fcnm);
    }
    GFAST_memory_free__double(&latency);
    GFAST_memory_freeEvents(&events);
    GFAST_memory_freeData(&gps_acquisition);
    GFAST_memory_freePGDResults(&pgd);
    GFAST_memory_freeCMTResults(&cmt);
    GFAST_memory_freeFFResults(&ff);
    GFAST_memory_freePGDData(&pgd_data);
    GFAST_memory_freeOffsetData(&cmt_data);
    GFAST_memory_freeOffsetData(&ff_data);
    GFAST_memory_freeProps(&props);
    return ierr;
}
