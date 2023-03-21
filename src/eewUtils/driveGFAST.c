#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gfast.h"
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_hdf5.h"
#include "gfast_traceBuffer.h"
#include "dmlibWrapper.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/time/time.h"

/*!
 * @brief writes xml message to flat file
 * Need to poke at this more.
 * @param[in] dirname output directory
 * @param[in] eventid
 * @param[in] msg_type message type
 * @param[in] message xml message
 * @param[in] interval write interval
 * @param[in] interval_in_mins is interval in minutes?
 * @return status code.
 */

int eewUtils_writeXML(const char *dirname, const char *eventid, const char *msg_type, const char *message, int interval, bool interval_in_mins);

bool check_mins_against_intervals(struct GFAST_props_struct props,
                                  int mins,
                                  char * eventid,
                                  char * suffix,
                                  char * xml,
                                  bool * interval_complete,
                                  double age);

/*!
 * @brief Fills a given coreInfo_struct with the appropriate information
 * @param[in] evid Event ID
 * @param[in] version Event version number
 * @param[in] SA_lat Event latitude
 * @param[in] SA_lon Event longitude
 * @param[in] SA_depth Event depth
 * @param[in] SA_mag Event magnitude
 * @param[in] SA_time Event origin time (UTC)
 * @param[in] num_stations Number of stations contributing
 * @param[out] core struct to fill with information
 * @return status code.
 */
int fill_core_event_info(const char *evid,
                         const int version,
                         const double SA_lat,
                         const double SA_lon,
                         const double SA_depth,
                         const double SA_mag,
                         const double SA_time,
                         const int num_stations,
                         struct coreInfo_struct *core);

bool send_xml_filter(const struct GFAST_props_struct *props,
                     const struct GFAST_shakeAlert_struct *SA,
                     const struct GFAST_pgdResults_struct *pgd,
                     const struct GFAST_peakDisplacementData_struct *pgd_data,
                     const struct coreInfo_struct *core,
                     const double age_of_event);

/*!
 * @brief Expert earthquake early warning GFAST driver.
 *
 * @param[in] currentTime        Current epochal time (UTC seconds)
 * @param[in] props              Holds the GFAST properties.
 * @param[in] program_instance   Program instance name (e.g. gfast_eew at eew-uw-dev1)
 * @param[in,out] events         The input event list.  If there are no
 *                               events this function will immediately return.
 *                               On output, if an event has expired then it
 *                               will be popped from the list.
 * @param[in,out] gps_data       Holds the GPS streams to be used in the
 *                               inversions.
 * @param[in,out] h5traceBuffer  Holds the requisite information for reading.
 * @param[in,out] pgd_data       Workspace for the PGD data in the PGD
 *                               inversion.
 * @param[in,out] cmt_data       Workspace for the offset data in the CMT 
 *                               inversion.
 * @param[in,out] ff_data        Workspace for the offset data in the finite
 *                               fault inversion.
 * @param[in,out] pgd            Workspace for the PGD inversion.
 * @param[in,out] cmt            Workspace for the CMT inversion.
 * @param[in,out] ff             Workspace for the finite fault inversion.
 *
 * @param[out] xmlMessages       Contains the XML messages for all events for
 *                               the PGD and finite fault for activeMQ to
 *                               forward onto shakeAlert as well as the CMT
 *                               quakeML.
 * @param[in,out] xml_status     On input holds current active events and the
 *                               previous message versions. On output, holds the
 *                               updated message versions.
 * 
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * 
 */
int eewUtils_driveGFAST(const double currentTime,
                        const char *program_instance,
                        struct GFAST_props_struct props,
                        struct GFAST_activeEvents_struct *events,
                        struct GFAST_data_struct *gps_data,
                        struct h5traceBuffer_struct *h5traceBuffer,
                        struct GFAST_peakDisplacementData_struct *pgd_data,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_pgdResults_struct *pgd,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_ffResults_struct *ff,
                        struct GFAST_xmlMessages_struct *xmlMessages,
                        struct GFAST_activeEvents_xml_status *xml_status)
{
    struct GFAST_shakeAlert_struct SA;
#ifndef ENABLE_PLOG
    char errorLogFileName[PATH_MAX], infoLogFileName[PATH_MAX], 
         debugLogFileName[PATH_MAX], warnLogFileName[PATH_MAX];
#endif
    char *cmtQML, *ffXML, *pgdXML;
    double t1, t2, age_of_event, t_time, t_time0, t_loop, t_event;
    int mins;
    float secs;
    int h5k, ierr, iev, ipf, nPop, nRemoved,
        nsites_cmt, nsites_ff, nsites_pgd,
        nstrdip, pgdOpt;
    bool lcmtSuccess, lffSuccess, lfinalize, lpgdSuccess;

    const char *fcnm = "driveGFAST\0";
    //------------------------------------------------------------------------//
    //
    // Nothing to do 
    ierr = 0;
    if (events->nev <= 0){return 0;}

    // Set memory for XML messages
    memset(xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
    xmlMessages->mmessages = events->nev;
    xmlMessages->nmessages = 0;
    xmlMessages->evids  = (char **)
        calloc((size_t) xmlMessages->mmessages, sizeof(char *));
    xmlMessages->cmtQML = (char **)
        calloc((size_t) xmlMessages->mmessages, sizeof(char *));
    xmlMessages->ffXML  = (char **)
        calloc((size_t) xmlMessages->mmessages, sizeof(char *));
    xmlMessages->pgdXML = (char **)
        calloc((size_t) xmlMessages->mmessages, sizeof(char *));
    nPop = 0;
    t_loop = time_timeStamp();
    ///////////////////////////////////////////////////////////////////////////
    // Loop on the events
    ///////////////////////////////////////////////////////////////////////////
    for (iev=0; iev<events->nev; iev++) {
        t_event = time_timeStamp();
        // Get the streams for this event
        memcpy(&SA, &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
        t1 = SA.time;     // Origin time
        t2 = currentTime;
        age_of_event = (t2 - t1);
        LOG_MSG("%s: Starting event time:%lf evid:%s [age_of_event=%f]",
            fcnm, t2, SA.eventid, age_of_event);

        // Skip event if the times doen't make sense
        if (t1 > t2) {
            LOG_WARNMSG("Origin time > currentTime? - skipping event %s [Timing: %.3fs for event]",
                SA.eventid, time_timeStamp() - t_event);
            continue;
        }

        // Used for determining when to output results (if props.output_interval_mins[0] != 0)
        mins = (int)floor(age_of_event/60.);
        secs = age_of_event - 60.*mins;

        lfinalize = false;
        // Exit processing this event if it is beyond the processing time
        if (age_of_event >= props.processingTime) {
            LOG_MSG("%s: time:%lf evid:%s has expired --> finalize [Timing: %.3fs for event]",
                fcnm, t2, SA.eventid, time_timeStamp() - t_event);
            nPop = nPop + 1;
            lfinalize = true;
            // LOG_MSG("%s: Call core_log_closeLogs() before early exit from loop",
            //     fcnm);
            // core_log_closeLogs();
            continue;
        }

        // Set the log file names. Comment out in advance of providing compile
        // option to use these (for NOAA) or plog (for ShakeAlert) - CWU
#ifndef ENABLE_PLOG
        eewUtils_setLogFileNames(SA.eventid,props.SAoutputDir,
            errorLogFileName, infoLogFileName,
            debugLogFileName, warnLogFileName);
        core_log_openErrorLog(errorLogFileName);
        core_log_openInfoLog(infoLogFileName);
        core_log_openWarningLog(warnLogFileName);
        core_log_openDebugLog(debugLogFileName);
#endif
        
        ///////////////////////////////////////////////////////////////////////////
        // Retrieve data from h5 buffer
        ///////////////////////////////////////////////////////////////////////////
        t_time0 = time_timeStamp();
        // Get the data for this event
        ierr = GFAST_traceBuffer_h5_getData(t1, t2, h5traceBuffer);
        if (ierr != 0) {
            LOG_ERRMSG("%s: Error getting the data for event %s --> continue [Timing: %.3fs for event]",
                fcnm, SA.eventid, time_timeStamp() - t_event);
            continue; 
        }
        // Copy the data onto the buffer
        ierr = GFAST_traceBuffer_h5_copyTraceBufferToGFAST(h5traceBuffer, gps_data);
        if (ierr != 0) {
            LOG_ERRMSG("Error copying trace buffer, evid %s [Timing: %.3fs for event]",
                SA.eventid, time_timeStamp() - t_event);
            continue;
        }
        LOG_DEBUGMSG("Buffer handling [Timing: %.3fs]", time_timeStamp() - t_time0);
        // End data buffer handling
      
        ///////////////////////////////////////////////////////////////////////////
        // Extract pgd, cmt, ff values
        ///////////////////////////////////////////////////////////////////////////
        t_time0 = time_timeStamp();
        if (props.pgd_props.do_pgd) {
            LOG_MSG("%s", "Get peakDisp");
            t_time = time_timeStamp();
            // Extract the peak displacement from the waveform buffer
            nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
                &(props.pgd_props),
                SA.lat,
                SA.lon,
                SA.dep,
                SA.time,
                *gps_data,
                pgd_data,
                &ierr);
            LOG_MSG("%s returned ierr=%d nsites_pgd=%d [Timing: %.3fs]", "Get peakDisp", ierr,
                nsites_pgd, time_timeStamp() - t_time);
            if (ierr != 0) {
                LOG_ERRMSG("Error processing peak displacement [Timing: %.3fs for event]",
                    time_timeStamp() - t_event);
                continue;
            }
        } else {
            LOG_MSG("%s", "Skipping PGD processing - do_pgd is false");
        }
        if (props.cmt_props.do_cmt) {
            LOG_MSG("%s", "Get Offset for CMT");
            t_time = time_timeStamp();
            // Extract the offset for the CMT inversion from the buffer 
            nsites_cmt = GFAST_core_waveformProcessor_offset(
                props.cmt_props.utm_zone,
                props.cmt_props.window_vel,
                SA.lat,
                SA.lon,
                SA.dep,
                SA.time,
                *gps_data,
                cmt_data,
                &ierr);
            LOG_MSG("%s returned ierr=%d nsites_cmt=%d [Timing: %.3fs]", "Get Offset for CMT", ierr,
                nsites_cmt, time_timeStamp() - t_time);
            if (ierr != 0) {
                LOG_ERRMSG("Error processing CMT offset [Timing: %.3fs for event]",
                    time_timeStamp() - t_event);
                continue;
            }
        } else {
            LOG_MSG("%s", "Skipping CMT processing - do_cmt is false");
        }
        if (props.ff_props.do_ff) {
            LOG_MSG("%s", "Get Offset for FF");
            t_time = time_timeStamp();
            // Extract the offset for the FF inversion from the buffer 
            nsites_ff = GFAST_core_waveformProcessor_offset(
                props.ff_props.utm_zone,
                props.ff_props.window_vel,
                SA.lat,
                SA.lon,
                SA.dep,
                SA.time,
                *gps_data,
                ff_data,
                &ierr);
            LOG_MSG("%s returned ierr=%d nsites_ff=%d [Timing: %.3fs]", "Get Offset for FF", ierr,
                nsites_ff, time_timeStamp() - t_time);
            if (ierr != 0) {
                LOG_ERRMSG("Error processing FF offset [Timing: %.3fs for event]",
                    time_timeStamp() - t_event);
                continue;
            }
        } else {
            LOG_MSG("%s", "Skipping FF processing - do_ff is false");
        }
        LOG_DEBUGMSG("Value extractions [Timing: %.3fs]", time_timeStamp() - t_time0);
        // End value extraction
        
        ///////////////////////////////////////////////////////////////////////////
        // Run pgd, cmt, ff inversions
        ///////////////////////////////////////////////////////////////////////////
        lpgdSuccess = false;
        lcmtSuccess = false;
        lffSuccess = false;
        t_time0 = time_timeStamp();
        // Run the PGD scaling
        if (props.pgd_props.do_pgd && 
            nsites_pgd >= props.pgd_props.min_sites)
        {
            if (props.verbose > 2) {
                LOG_INFOMSG("Estimating PGD scaling for %s...", SA.eventid);
            }
            lpgdSuccess = true;
            LOG_MSG("Call drivePGD eventid=%s", SA.eventid);
            t_time = time_timeStamp();
            ierr = eewUtils_drivePGD(props.pgd_props,
                SA.lat, SA.lon, SA.dep, age_of_event,
                *pgd_data,
                pgd);
            LOG_MSG("drivePGD returned ierr=%d [Timing: %.3fs]",
                ierr, time_timeStamp() - t_time);
            if (ierr != PGD_SUCCESS) {
                LOG_ERRMSG("%s", "Error computing PGD");
                lpgdSuccess = false;
            }
        }
        // Run the CMT inversion
        if (props.cmt_props.do_cmt && 
            nsites_cmt >= props.cmt_props.min_sites)
        {
            if (props.verbose > 2) {
                LOG_INFOMSG("Estimating CMT for %s...", SA.eventid);
            }
            lcmtSuccess = true;
            LOG_MSG("%s", "calling driveCMT");
            t_time = time_timeStamp();
            ierr = eewUtils_driveCMT(props.cmt_props,
                SA.lat, SA.lon, SA.dep,
                *cmt_data,
                cmt);
            LOG_MSG("driveCMT returned ierr=%d [Timing: %.3fs]",
                ierr, time_timeStamp() - t_time);
            if (ierr != CMT_SUCCESS || cmt->opt_indx < 0) {
                LOG_ERRMSG("%s", "Error computing CMT");
                lcmtSuccess = false;
            }
        }
        // If we got a CMT see if we can run a finite fault inversion  
        if (props.ff_props.do_ff &&
            lcmtSuccess &&
            nsites_ff >= props.ff_props.min_sites)
        {
            if (props.verbose > 2) {
                LOG_INFOMSG("Estimating finite fault for %s...", SA.eventid);
            }
            ff->SA_lat = events->SA[iev].lat;
            ff->SA_lon = events->SA[iev].lon;
            ff->SA_dep = cmt->srcDepths[cmt->opt_indx]; // TODO make cmt->opt_dep
            ff->SA_mag = cmt->Mw[cmt->opt_indx];
            ff->str[0] = cmt->str1[cmt->opt_indx];
            ff->str[1] = cmt->str2[cmt->opt_indx];
            ff->dip[0] = cmt->dip1[cmt->opt_indx];
            ff->dip[1] = cmt->dip2[cmt->opt_indx];
            lffSuccess = true;
            LOG_MSG("%s", "calling driveFF");
            t_time = time_timeStamp();
            ierr = eewUtils_driveFF(props.ff_props,
                SA.lat,
                SA.lon,
                *ff_data,
                ff);
            LOG_MSG("driveFF returned ierr=%d [Timing: %.3fs]",
                ierr, time_timeStamp() - t_time);
            if (ierr != FF_SUCCESS) {
                LOG_ERRMSG("%s", "Error computing finite fault");
                lffSuccess = false;
            }
        }
        LOG_DEBUGMSG("Inversions [Timing: %.3fs]", time_timeStamp() - t_time0);
        // End inversions

        // Finalize?
        pgdXML = NULL;
        cmtQML = NULL;
        ffXML = NULL;
        
        // anticipate last iteration so that data are archived to h5
        if (t2 - t1 >= props.processingTime - props.dt_default) {
            lfinalize = true;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Make xml messages
        ///////////////////////////////////////////////////////////////////////////
        t_time0 = time_timeStamp();
        LOG_MSG("driveGFAST: make XML msgs: lpgdSuccess=%d lcmtSuccess=%d lffSuccess=%d",
            lpgdSuccess, lcmtSuccess, lffSuccess);
        xml_status->SA_status[iev].version += 1;
        char *message_type = (xml_status->SA_status[iev].version==0)?"new\0":"update\0";
        char sversion[6];
        snprintf(sversion,6,"%d",xml_status->SA_status[iev].version);

        // Fill coreInfo_struct to pass to makeXML for pgd and ff
        struct coreInfo_struct core;
        memset(&core, 0, sizeof(struct coreInfo_struct));
        ierr = fill_core_event_info(SA.eventid, xml_status->SA_status[iev].version, SA.lat,
            SA.lon, SA.dep, SA.mag, SA.time, 0, &core);
              
        // Make the PGD xml
        if (props.pgd_props.do_pgd && lpgdSuccess) {
            if (props.verbose > 2) {
                LOG_DEBUGMSG("%s", "Generating pgd XML");
            }
            // Change depth, mag to match optimal pgd (by variance reduction)
            pgdOpt = array_argmax64f(pgd->ndeps, pgd->dep_vr_pgd, &ierr);
            core.depth = pgd->srcDepths[pgdOpt];
            core.mag = pgd->mpgd[pgdOpt];
            core.magUncer = pgd->mpgd_sigma[pgdOpt];
            core.numStations = nsites_pgd;

#ifdef GFAST_USE_DMLIB
            //   Encode xml with dmlib
            LOG_MSG("%s", "driveGFAST: CWU_TEST dmlib encoding");
            pgdXML = dmlibWrapper_createPGDXML(
                currentTime,
                props.opmode,
                GFAST_VERSION,
                program_instance,
                message_type,
                props.pgd_props.max_assoc_stations,
                &core,
                pgd,
                pgd_data,
                &ierr);
#else
            pgdXML = eewUtils_makeXML__pgd(
                props.opmode,
                "GFAST\0",
                GFAST_VERSION,
                program_instance,
                message_type,
                sversion,
                &core,
                &ierr);
#endif
            if (ierr != 0) {
                LOG_ERRMSG("%s", "Error generating PGD XML");
                if (pgdXML != NULL) {
                    free(pgdXML);
                    pgdXML = NULL;
                }
            }
            xmlMessages->pgdXML[xmlMessages->nmessages] = pgdXML;
            
            // MTH: This is just a sanity check, could be done anywhere:
            if (strcmp(SA.eventid, xml_status->SA_status[iev].eventid) != 0) {
                LOG_WARNMSG("Mismatch between SA.eventid=%s and xml_status.SA_status[%d].eventid=%s --> Can't output PGD!\n",
                    SA.eventid, iev, xml_status->SA_status[iev].eventid);
            }

#if defined GFAST_USE_AMQ && defined GFAST_USE_DMLIB
            // Send message via ActiveMQ if appropriate
            if (!send_xml_filter(&props, &SA, pgd, pgd_data, &core, age_of_event)) {
                if (pgdXML != NULL) {
                    sendEventXML(pgdXML);
                }
                LOG_MSG("== Sending xml, [GFAST t0:%f] evid:%s pgdXML=[%s]\n",
                    currentTime, SA.eventid, pgdXML);
            }
#endif /* GFAST_USE_AMQ && GFAST_USE_DMLIB */
            
            if (props.output_interval_mins[0] == 0) { // Output at every iteration
                int index = core.version;
                ierr = eewUtils_writeXML(
                    props.SAoutputDir, SA.eventid, "pgd", pgdXML, index, false);
                LOG_MSG("writeXML for PGD returned ierr=%d", ierr);
            } else if (secs < 3.) {
                LOG_MSG("eventid:%s age:%f mins:%d secs:%f --> check PGD writeXML",
                    SA.eventid, age_of_event, mins, secs);
                check_mins_against_intervals(props, mins, SA.eventid, "pgd", pgdXML,
                    xml_status->SA_status[iev].interval_complete[0], age_of_event);
            }
            LOG_MSG("Leaving PGD writeXML ierr=%d", ierr);
        } //if props.pgd_props.do_pgd && lpgdSuccess

        // Make the CMT quakeML
        if (props.cmt_props.do_cmt && lcmtSuccess) {
            if (props.verbose > 2) {
                LOG_DEBUGMSG("%s", "Generating CMT QuakeML");
            }
            cmtQML = eewUtils_makeXML__quakeML(
                props.anssNetwork,
                props.anssDomain,
                SA.eventid,
                SA.lat,
                SA.lon,
                cmt->srcDepths[cmt->opt_indx],
                SA.time,
                &cmt->mts[6*cmt->opt_indx],
                &ierr);
            if (ierr != 0) {
                LOG_ERRMSG("%s", "Error generating CMT quakeML");
                if (cmtQML != NULL) {
                    free(cmtQML);
                    cmtQML = NULL;
                }
            }
            xmlMessages->cmtQML[xmlMessages->nmessages] = cmtQML;
            if (props.output_interval_mins[0] == 0) { // Output at every iteration
                int index = core.version;
                LOG_MSG("Age_of_event=%f --> Output CMT solution at iter:%d",
                    age_of_event, index);
                ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "cmt",
                    cmtQML, index, false);
            } else if (secs < 3.) {
                LOG_MSG("eventid:%s age:%f mins:%d secs:%f --> check CMT writeXML",
                    SA.eventid, age_of_event, mins, secs);
                check_mins_against_intervals(props, mins, SA.eventid, "cmt", cmtQML,
                    xml_status->SA_status[iev].interval_complete[1], age_of_event);
            }
        } // if props.cmt_props.do_cmt && lcmtSuccess
        // Make the finite fault XML
        if (props.ff_props.do_ff && lffSuccess) {
            if (props.verbose > 2) {
                LOG_DEBUGMSG("Generating FF XML; preferred plane=%d",
                    ff->preferred_fault_plane+1);
            }
            ipf = ff->preferred_fault_plane;
            nstrdip = ff->fp[ipf].nstr*ff->fp[ipf].ndip;
            // Reset depth and mag to be same as SA message.
            core.depth = SA.dep;
            core.mag = SA.mag;
            core.magUncer = 0.5;
            core.numStations = nsites_ff;
            ffXML = eewUtils_makeXML__ff(
                props.opmode,
                "GFAST\0",
                GFAST_VERSION,
                program_instance,
                message_type,
                sversion,
                &core,
                nstrdip,
                ff->fp[ipf].fault_ptr,
                ff->fp[ipf].lat_vtx,
                ff->fp[ipf].lon_vtx,
                ff->fp[ipf].dep_vtx,
                ff->fp[ipf].strike,
                ff->fp[ipf].dip,
                ff->fp[ipf].sslip,
                ff->fp[ipf].dslip,
                ff->fp[ipf].sslip_unc,
                ff->fp[ipf].dslip_unc,
                &ierr); 
            if (ierr != 0) {
                LOG_ERRMSG("%s", "Error generating finite fault XML");
                if (ffXML != NULL) {
                    free(ffXML);
                    ffXML = NULL;
                }
            }
            xmlMessages->ffXML[xmlMessages->nmessages] = ffXML;
            if (props.output_interval_mins[0] == 0) { // Output at every iteration
                int index = core.version;
                LOG_MSG("Age_of_event=%f --> Output FF solution at iter:%d",
                    age_of_event, index);
                ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "ff",
                    ffXML, index, false);
            } else if (secs < 3.) {
                LOG_MSG("eventid:%s age:%f mins:%d secs:%f --> check FF writeXML",
                    SA.eventid, age_of_event, mins, secs);
                check_mins_against_intervals(props, mins, SA.eventid, "ff", ffXML,
                    xml_status->SA_status[iev].interval_complete[2], age_of_event);
            }
        } // if props.ff_props.do_ff && lffSuccess
        xmlMessages->evids[xmlMessages->nmessages] = (char *)
            calloc(strlen(SA.eventid)+1, sizeof(char));
        strcpy(xmlMessages->evids[xmlMessages->nmessages], SA.eventid);
        xmlMessages->nmessages = xmlMessages->nmessages + 1;

        LOG_MSG("Leaving writeXML ierr=%d lfinalize=%d [Timing: %.3fs]",
            ierr, lfinalize, time_timeStamp() - t_time0);
        // End make xmls

        ///////////////////////////////////////////////////////////////////////////
        // Update the archive
        ///////////////////////////////////////////////////////////////////////////
        if (lfinalize || !props.lh5SummaryOnly) {
            // Get the iteration number in the H5 file
            h5k = 0;
            h5k = GFAST_hdf5_updateGetIteration(
                props.h5ArchiveDir,
                SA.eventid,
                currentTime);
            LOG_MSG("time:%lf evid:%s h5k iteration=%d dir=%s Update h5 archive",
                t2, SA.eventid, h5k, props.h5ArchiveDir);
            if (props.verbose > 2) {
                LOG_DEBUGMSG("Writing GPS data for iteration %d", h5k);
            }
            ierr = GFAST_hdf5_update_gpsData(
                props.h5ArchiveDir,
                SA.eventid,
                h5k,
                *gps_data);
            LOG_MSG("update gpsData for iteration:%d returned ierr=%d", h5k, ierr);
            if (props.verbose > 2) {
                LOG_DEBUGMSG("Writing hypocenter for iteration %d", h5k);
            }
            ierr = GFAST_hdf5_updateHypocenter(
                props.h5ArchiveDir,
                SA.eventid,
                h5k,
                SA);
            LOG_MSG("updateHypocenter for iteration:%d returned ierr=%d", h5k, ierr);
            if (props.pgd_props.do_pgd && lpgdSuccess) {
                if (props.verbose > 2) {
                    LOG_DEBUGMSG("Writing PGD for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updatePGD(
                    props.h5ArchiveDir,
                    SA.eventid,
                    h5k,
                    *pgd_data,
                    *pgd);
                if (pgdXML) {
                    ierr = hdf5_updateXMLMessage(
                        props.h5ArchiveDir,
                        SA.eventid,
                        h5k, "pgdXML\0",
                        pgdXML);
                }
            }
            if (props.cmt_props.do_cmt && lcmtSuccess) {
                if (props.verbose > 2) {
                    LOG_DEBUGMSG("Writing CMT for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updateCMT(
                    props.h5ArchiveDir,
                    SA.eventid,
                    h5k,
                    *cmt_data,
                    *cmt);
                if (cmtQML) {
                    ierr = hdf5_updateXMLMessage(
                        props.h5ArchiveDir,
                        SA.eventid,
                        h5k, "cmtQuakeML\0",
                        cmtQML);
                }
            }
            if (props.ff_props.do_ff && lffSuccess) {
                if (props.verbose > 2)  {
                    LOG_DEBUGMSG("Writing FF for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updateFF(
                    props.h5ArchiveDir,
                    SA.eventid,
                    h5k,
                    *ff);
                if (ffXML) {
                    ierr = hdf5_updateXMLMessage(
                        props.h5ArchiveDir,
                        SA.eventid,
                        h5k, "ffXML\0",
                        ffXML);
                }
            }
        } // End check on updating archive or finalizing event
#ifndef ENABLE_PLOG
        // Close the logs
        core_log_closeLogs();
#endif
    } // Loop on the events
    LOG_MSG("MTH: end loop on events ierr=%d [Timing: %.3fs]\n",
        ierr, time_timeStamp() - t_loop);
    // Need to down-date the events should any have expired
    if (nPop > 0) {
        LOG_MSG("time:%lf RemoveExpiredEvents", currentTime);
        nRemoved = core_events_removeExpiredEvents(
            props.processingTime,
            currentTime,
            props.verbose,
            events);
        LOG_MSG("time:%lf syncXMLStatusWithEvents", currentTime);
        core_events_syncXMLStatusWithEvents(events, xml_status);
        LOG_MSG("time:%lf RemoveExpiredEvents nRemoved=%d", currentTime, nRemoved);
        if (nRemoved != nPop) {
            LOG_WARNMSG("%s", "Strange - check removeExpiredEvents");
        }
    }
    LOG_MSG("End driveGFAST time:%lf return ierr=%d", currentTime, ierr);
    return ierr;
}

#include <unistd.h>
int eewUtils_writeXML(const char *dirname,
                      const char *eventid,
                      const char *msg_type,
                      const char *message,
                      int interval,
                      bool interval_in_mins)
{
    char fullpath[128];
    FILE * fp;

    if (interval_in_mins) {
        sprintf(fullpath, "%s/%s.%s.%d_min", dirname, eventid, msg_type, interval);
        LOG_MSG("driveGFAST: evid=%s SA xml mins=%d --> output XML to file=[%s]",
            eventid, interval, fullpath);
    } else {
        sprintf(fullpath, "%s/%s.%s.%d", dirname, eventid, msg_type, interval);
        LOG_MSG("driveGFAST: evid=%s interval=%d --> output XML to file=[%s]",
            eventid, interval, fullpath);
    }

    if (access( fullpath, F_OK ) != -1 ) {
        LOG_MSG("File:%s already exists!\n", fullpath);
    }

    fp = fopen(fullpath, "w");
    fprintf(fp, "%s\n", message);
    fclose(fp);

    return 0;
}

bool check_mins_against_intervals(struct GFAST_props_struct props,
                                  int mins,
                                  char * eventid,
                                  char * suffix,
                                  char * xml,
                                  bool * interval_complete,
                                  double age)
{
    int i, ierr;

    i = props.n_intervals - 1;
    if (mins == props.output_interval_mins[i] && interval_complete[i] == false) {
        LOG_MSG("Eventid:%s age_of_event:%f --> Output minute %d solution for suff:%s [FINAL MIN INTERVAL]",
            eventid, age, props.output_interval_mins[i], suffix);
        ierr = eewUtils_writeXML(props.SAoutputDir, eventid, suffix, xml,
            props.output_interval_mins[i], true);
        if (ierr!=0) LOG_MSG("%s: ierr=%d from eewUtils_writeXML()",__func__,ierr);
        interval_complete[i] = true;
        return true;
    }

    for (i=0; i<props.n_intervals-1; i++) {
        if (mins >= props.output_interval_mins[i] && mins < props.output_interval_mins[i+1]) {
            if (interval_complete[i] == false) {
                ierr = eewUtils_writeXML(props.SAoutputDir, eventid, suffix, xml,
                    props.output_interval_mins[i], true);
                interval_complete[i] = true;
                return true;
            }
        }
    }

    return false;
}

int fill_core_event_info(const char *evid,
                         const int version,
                         const double SA_lat,
                         const double SA_lon,
                         const double SA_depth,
                         const double SA_mag,
                         const double SA_time,
                         const int num_stations,
                         struct coreInfo_struct *core)
{
    strcpy(core->id, evid);
    core->version = version;
    core->mag = SA_mag;
    core->lhaveMag = true;
    core->magUnits = MOMENT_MAGNITUDE;
    core->lhaveMagUnits = true;
    core->magUncer = 0.5;
    core->lhaveMagUncer = true;
    core->magUncerUnits = MOMENT_MAGNITUDE;
    core->lhaveMagUncerUnits = true;
    core->lat = SA_lat; 
    core->lhaveLat = true;
    core->latUnits = DEGREES;
    core->lhaveLatUnits = true;
    core->latUncer = (double) NAN;
    core->lhaveLatUncer = true;
    core->latUncerUnits = DEGREES;
    core->lhaveLatUncerUnits = true;
    // GFAST would call lon -120 as 240 by default. Change this to be
    // consistent with ShakeAlert seismic algorithms
    core->lon = (SA_lon > 180) ? SA_lon - 360: SA_lon;
    core->lhaveLon = true;
    core->lonUnits = DEGREES;
    core->lhaveLonUnits = true;
    core->lonUncer = (double) NAN;
    core->lhaveLonUncer = true;
    core->lonUncerUnits = DEGREES;
    core->lhaveLonUncerUnits = true;
    core->depth = SA_depth;
    core->lhaveDepth = true;
    core->depthUnits = KILOMETERS;
    core->lhaveDepthUnits = true;
    core->depthUncer = (double) NAN;
    core->lhaveDepthUncer = true;
    core->depthUncerUnits = KILOMETERS;
    core->lhaveDepthUncerUnits = true;
    core->origTime = SA_time;
    core->lhaveOrigTime = true;
    core->origTimeUnits = UTC;
    core->lhaveOrigTimeUnits = true;
    core->origTimeUncer = (double) NAN;
    core->lhaveOrigTimeUncer = true;
    core->origTimeUncerUnits = SECONDS;
    core->lhaveOrigTimeUncerUnits = true;
    core->likelihood = 0.8;
    core->lhaveLikelihood = true;
    core->numStations = num_stations;
    return 0;
}

/*
 * Return true if this message should not be sent (false if it should be sent)
 */
bool send_xml_filter(const struct GFAST_props_struct *props,
                     const struct GFAST_shakeAlert_struct *SA,
                     const struct GFAST_pgdResults_struct *pgd,
                     const struct GFAST_peakDisplacementData_struct *pgd_data,
                     const struct coreInfo_struct *core,
                     const double age_of_event) 
{

    // Conditions can be met or exceeded.
    // Exceeded is used if a value being more than a threshold means no throttling
    // Met is used if a value being less than a threshold means no throttling 
    bool pgd_exceeded = false;
    bool mag_exceeded = false;
    bool mag_sigma_met = false;
    // Determine if pgd threshold is exceeded n times
    int num_pgd_exceeded = 0, i, i_throttle;

    // Find the correct throttle criteria based on the time after origin
    i_throttle = -1;
    for (i = 0; i < props->pgd_props.n_throttle; i++) {
        if (props->pgd_props.throttle_time_threshold[i] > age_of_event) break;
        i_throttle++;
    }
    i_throttle = (i_throttle < 0) ? 0: i_throttle;
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: For age_of_event %.2f, using i_throttle=%d, thresholds for time=%.1f, pgd=%.1f, nsta=%d",
            __func__,
            age_of_event,
            i_throttle,
            props->pgd_props.throttle_time_threshold[i_throttle],
            props->pgd_props.throttle_pgd_threshold[i_throttle],
            props->pgd_props.throttle_num_stations[i_throttle])
    }

    // Assumes pgd->nsites = pgd_data->nsites and indices correspond
    if (pgd->nsites != pgd_data->nsites) {
        LOG_ERRMSG("%s: nsites don't match for pgd, pgd_data! %d, %d\n", 
            __func__, pgd->nsites, pgd_data->nsites);
        return false;
    }
    for (i = 0; i < pgd->nsites; i++) {
        // skip site if it wasn't used
        if (!pgd->lsiteUsed[i]) { continue; }
        // pd is in meters, so convert to cm before comparing to threshold
        if (pgd_data->pd[i] * 100. > props->pgd_props.throttle_pgd_threshold[i_throttle]) {
            num_pgd_exceeded++;
        }
    }

    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: PGD threshold of %.1f cm exceeded at %d stations (threshold num: %d)",
            __func__, props->pgd_props.throttle_pgd_threshold[i_throttle], num_pgd_exceeded,
            props->pgd_props.throttle_num_stations[i_throttle]);
    }
    if (num_pgd_exceeded >= props->pgd_props.throttle_num_stations[i_throttle]) {
        LOG_MSG("%s: PGD threshold of %.1f cm exceeded at %d stations (threshold num: %d)",
            __func__, props->pgd_props.throttle_pgd_threshold[i_throttle], num_pgd_exceeded,
            props->pgd_props.throttle_num_stations[i_throttle]);
        pgd_exceeded = true;
    }

    // Determine if SA mag threshold is exceeded
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: SA mag: %f, threshold mag: %f",
            __func__, SA->mag, props->pgd_props.SA_mag_threshold);
    }
    if (SA->mag >= props->pgd_props.SA_mag_threshold) {
        mag_exceeded = true;
        LOG_MSG("%s: SA magnitude exceeded! SA mag: %f, threshold mag: %f",
            __func__, SA->mag, props->pgd_props.SA_mag_threshold);
    }

    // Determine if pgd magnitude sigma threshold is met
    if (props->verbose > 2) {
        LOG_DEBUGMSG("%s: PGD mag sigma: %f, threshold mag sigma: %f, PGD mag: %f",
            __func__, core->magUncer, props->pgd_props.pgd_sigma_throttle, core->mag);
    }
    if (core->magUncer < props->pgd_props.pgd_sigma_throttle) {
        mag_sigma_met = true;
        LOG_MSG("%s: PGD mag sigma met! PGD mag sigma: %f, threshold mag sigma: %f, PGD mag: %f",
            __func__, core->magUncer, props->pgd_props.pgd_sigma_throttle, core->mag);
    }

    if (pgd_exceeded && mag_exceeded && mag_sigma_met) {
        LOG_MSG("%s: Message not throttled (%s v%d)! pgd_exceeded: %d, mag_exceeded: %d, mag_sigma_met: %d",
            __func__, core->id, core->version, pgd_exceeded, mag_exceeded, mag_sigma_met);
        return false;
    }

    LOG_MSG("%s: Message throttled (%s v%d)! pgd_exceeded: %d, mag_exceeded: %d, mag_sigma_met: %d",
        __func__, core->id, core->version, pgd_exceeded, mag_exceeded, mag_sigma_met);
    return true;
}
