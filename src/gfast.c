#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfast.h"

//extern "C"
int GFAST_FF__xml__write(int mode,
                         char *orig_sys,
                         char *evid,
                         double Mw, 
                         double SA_lat,
                         double SA_lon,
                         double SA_depth,
                         double SA_time,
                         int nseg,
                         int *fptr,
                         double *lat_vtx,
                         double *lon_vtx,
                         double *dep_vtx,
                         double *ss,
                         double *ds);


/*!
 * This is a mock GFAST driver module
 */
int main()
{
    char fcnm[] = "GFAST\0";
    char propfilename[] = "gfast.props\0"; /* TODO take from EW config file */
    struct GFAST_props_struct props;
    struct GFAST_data_struct gps_acquisition;
    struct GFAST_shakeAlert_struct SA;
    struct GFAST_activeEvents_struct events;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_ffResults_struct ff;
    double *latency, currentTime, dtmax, eventTime, t0sim;
    int iev, k, kt, ntsim, verbose0;
    int ierr = 0;
    bool ldel_event, lnew_event, lupd_event;
    //------------------------------------------------------------------------//
    // 
    // Initializations
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_acquisition, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    latency = NULL;
    // Read the properties file
    log_infoF("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_properties__init(propfilename, &props);
printf("%f\n", props.synthetic_runtime);
    if (ierr != 0){
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2){
        GFAST_properties__print(props);
    }
    // Initialize the stations locations/names for the module
    if (props.verbose > 0){
        log_infoF("%s: Setting the acquisition...\n", fcnm);
    }
    ierr =  GFAST_acquisition__init(props, &gps_acquisition);
    if (ierr != 0){
        log_errorF("%s: Error initializating acquisition\n", fcnm);
        goto ERROR;
    }
    // Initialize PGD
    ierr = GFAST_scaling_PGD__init(props.pgd_props, gps_acquisition, &pgd);
    if (ierr != 0){
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_CMT__init(props, gps_acquisition, &cmt);
    if (ierr != 0){
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_FF__init(props, gps_acquisition, &ff);
    if (ierr != 0){
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }
    // Connect to EW trace buffer and ring with ElarmS messages 
    //if (props.opmode != OFFLINE){
    //    log_infoF("%s: Connecting to Earthworm rings...\n", fcnm);
    //}
    if (props.verbose > 2){
        GFAST_buffer_print__samplingPeriod(gps_acquisition);
    }
    // Connect ActiveMQ for ElarmS messages
    if (props.verbose >= 2){
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
    if (props.opmode == OFFLINE){
        if (props.verbose >= 2){
            log_infoF("%s: Beginning simulation...\n", fcnm);
        }
        // Compute the runtime - goal is to keep up with `slowest' data
        dtmax = 0.0;
        for (k=0; k<gps_acquisition.stream_length; k++){
            dtmax = fmax(dtmax, gps_acquisition.data[k].dt);
        }
        ntsim = (int) (props.synthetic_runtime/dtmax + 0.5); // ignore + 1;
        if (props.verbose > 0){
            log_infoF("%s: Number of time steps in simulation: %d\n",
                      fcnm, ntsim); 
        }
        // Set simulation start time to the earliest time in the SAC headers
        t0sim = GFAST_acquisition__getT0FromSAC(props,
                                                gps_acquisition,
                                                &ierr);
        if (ierr != 0){
            log_errorF("%s: Error setting t0 for simulation!\n", fcnm);
            goto ERROR;
        }
        // Make sure we can finish this event
        props.processingTime 
             = fmin(props.processingTime, (double) (ntsim - 2));
        if (props.verbose > 1){
            log_infoF("%s: Simulation start time is: %lf\n", fcnm, t0sim);
        }
        // Save verbose because it may be toggled on and off in the loop
        verbose0 = props.verbose;
        // Loop on time-steps in simulation
//ntsim = 59;
        for (kt=0; kt<ntsim; kt++){
//for (kt=58; kt<59; kt++){
            // Update the time
            currentTime = t0sim + (double) kt;
            // Read the elarmS file
            if (kt > 0){props.verbose = 0;}
            ierr = GFAST_readElarmS(props, &SA);
            props.verbose = verbose0;
            if (ierr != 0){
                log_errorF("%s: Error reading shakeAlert message!\n", fcnm);
                continue;
            }
            // Is this a new event?  If so then add it to events
            lnew_event = GFAST_events__newEvent(SA, &events);
            if (lnew_event){
                if (props.verbose > 0){
                    log_infoF("%s: New event %s added\n", fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_events__print(SA);}
                }
            }else{
                // Has the event been updated?
                lupd_event = GFAST_events__updateEvent(SA, &events, &ierr);
                if (ierr != 0){
                    log_errorF("%s: There was an error updating event %s\n",
                               fcnm, SA.eventid);
                } 
                if (props.verbose > 0 && lupd_event){
                    log_infoF("%s: Event %s has been modified\n",
                              fcnm, SA.eventid);
                    if (props.verbose > 2){GFAST_events__print(SA);}
                }
            }
            // Acquire the data
            eventTime = SA.time;
            ierr = GFAST_acquisition__updateFromSAC(props,
                                                    t0sim,
                                                    eventTime,
                                                    currentTime,
                                                    latency,
                                                    &gps_acquisition);
            if (ierr != 0){
                log_errorF("%s: An error was encountered reading the data \n",
                           fcnm);
            }
            // Loop on the events
//printf("%f %d\n", currentTime - SA.time, kt);
            for (iev=0; iev<events.nev; iev++){
                // Run the PGD scaling 
//props.verbose = 0;
                ierr = GFAST_scaling_PGD__driver(props.pgd_props,
                                                 events.SA[iev],
                                                 gps_acquisition,
                                                 &pgd);
//props.verbose = verbose0;
                ierr = GFAST_CMT__driver(props,
                                         events.SA[iev],
                                         gps_acquisition,
                                         &cmt);
                // If we got a CMT see if we can run an MT inversion  
                if (ierr == 0){
                    ff.nfp = 2;
                    ff.SA_lat = events.SA[iev].lat;
                    ff.SA_lon = events.SA[iev].lon;
                    ff.SA_dep = cmt.srcDepths[cmt.opt_indx];
                    ff.SA_mag = cmt.Mw[cmt.opt_indx];
                    ff.str[0] = cmt.str1[cmt.opt_indx];
                    ff.str[1] = cmt.str2[cmt.opt_indx];
                    ff.dip[0] = cmt.dip1[cmt.opt_indx];
                    ff.dip[1] = cmt.dip2[cmt.opt_indx];
ff.SA_mag = 9.07945726;
ff.str[1] = 219.96796844;
ff.dip[1] = 69.27746075;
ff.str[0] = 116.78477424;
ff.dip[0] = 58.91674731;
                    ierr = GFAST_FF__driver(props, events.SA[iev],
                                            gps_acquisition, &ff);

/*
int GFAST_FF__xml__write(int mode,
                         char *orig_sys,
                         char *evid,
                         double Mw, 
                         double SA_lat,
                         double SA_lon,
                         double SA_depth,
                         double SA_time,
                         int nseg,
                         int *fptr,
                         double *lat_vtx,
                         double *lon_vtx,
                         double *dep_vtx,
                         double *ss,
                         double *ds);
*/
/*
int iopt = ff.preferred_fault_plane;
ierr = GFAST_FF__xml__write(props.opmode,
                            "ElarmsS\0",
                            events.SA[iev].eventid,
                            events.SA[iev].mag,
                            events.SA[iev].lat,
                            events.SA[iev].lon,
                            events.SA[iev].dep,
                            events.SA[iev].time,
                            ff.fp[iopt].nstr*ff.fp[iopt].ndip,
                            ff.fp[iopt].fault_ptr,
                            ff.fp[iopt].lat_vtx,
                            ff.fp[iopt].lon_vtx,
                            ff.fp[iopt].dep_vtx,
                            ff.fp[iopt].sslip,
                            ff.fp[iopt].dslip);
*/

                }
                // Am I ready to publish this event?
                if (currentTime - SA.time >= props.processingTime){
                    if (props.verbose > 0){
                        log_infoF("%s: Publishing event: %s\n", fcnm, SA.eventid);
                    }
                    ldel_event = GFAST_events__removeEvent(props.processingTime,
                                                           currentTime,
                                                           props.verbose,
                                                           SA,
                                                           &events);
                    if (ldel_event && props.verbose > 0){
                        log_infoF("%s: Deleted event %s\n", fcnm, SA.eventid);
                    }
                }
            } // Loop on active events
        }
/*
        while (true)
        {

        } // Loop on acquisition
*/
    }
ERROR:;
    if (ierr != 0 && props.verbose > 0){
        log_errorF("%s: Errors were encountered\n", fcnm);
    }
    if (props.verbose >= 2){
        log_infoF("%s: Freeing memory...\n", fcnm);
    }
    GFAST_memory_free__double(&latency);
    GFAST_memory_freeEvents(&events);
    GFAST_memory_freeData(&gps_acquisition);
    GFAST_memory_freePGDResults(&pgd);
    GFAST_memory_freeCMTResults(&cmt);
    GFAST_memory_freeFFResults(&ff);
    GFAST_memory_freeProps(&props);
    return ierr;
}
