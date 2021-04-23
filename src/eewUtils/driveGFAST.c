#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast.h"
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_hdf5.h"
#include "gfast_traceBuffer.h"
#include "iscl/array/array.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"

int eewUtils_writeXML(const char *dirname, const char *eventid, const char *msg_type, const char *message, int interval, bool interval_in_mins);

bool check_mins_against_intervals(
      struct GFAST_props_struct props,
      int mins,
      char * eventid,
      char * suffix,
      char * xml,
      bool * interval_complete,
      double age
      );

//static void setFileNames(const char *eventid);

/*!
 * @brief Expert earthquake early warning GFAST driver.
 *
 * @param[in] currentTime        Current epochal time (UTC seconds)
 * @param[in] props              Holds the GFAST properties.
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
 * 
 * @result 0 indicates success.
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 * 
 */
int eewUtils_driveGFAST(const double currentTime,
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
    char errorLogFileName[PATH_MAX], infoLogFileName[PATH_MAX], 
         debugLogFileName[PATH_MAX], warnLogFileName[PATH_MAX];
    char *cmtQML, *ffXML, *pgdXML;
    double t1, t2, age_of_event;
    int mins, i;
    float secs;
    int h5k, ierr, iev, ipf, nPop, nRemoved,
        nsites_cmt, nsites_ff, nsites_pgd,
        nstrdip, pgdOpt, shakeAlertMode;
    bool lcmtSuccess, lffSuccess, lfinalize, lpgdSuccess;

    const char *fcnm = "driveGFAST\0";
    //------------------------------------------------------------------------//
    //
    // Nothing to do 
    ierr = 0;
    if (events->nev <= 0){return 0;}
    // Figure out the mode for generating shakeAlert messages
    shakeAlertMode = 1;
    if (props.opmode == PLAYBACK){shakeAlertMode = 2;}
    // Set memory for XML messages
    memset(xmlMessages, 0, sizeof(struct GFAST_xmlMessages_struct));
    xmlMessages->mmessages = events->nev;
    xmlMessages->nmessages = 0;
    xmlMessages->evids  = (char **)
                          calloc((size_t) xmlMessages->mmessages,
                                 sizeof(char *));
    xmlMessages->cmtQML = (char **)
                          calloc((size_t) xmlMessages->mmessages,
                                 sizeof(char *));
    xmlMessages->ffXML  = (char **)
                          calloc((size_t) xmlMessages->mmessages,
                                 sizeof(char *));
    xmlMessages->pgdXML = (char **)
                          calloc((size_t) xmlMessages->mmessages,
                                 sizeof(char *));
    //ldownDate = memory_calloc8l(events->nev);
    nPop = 0;
    // Loop on the events
    for (iev=0; iev<events->nev; iev++)
    {
        // Get the streams for this event
        memcpy(&SA, &events->SA[iev], sizeof(struct GFAST_shakeAlert_struct));
        t1 = SA.time;     // Origin time
        t2 = currentTime;
        if (t1 > t2)
        {
            LOG_WARNMSG("Origin time > currentime? - skipping event %s",
                        SA.eventid);
            continue;
        }

        age_of_event = (t2 - t1);
//printf("driveGFAST: time:%lf evid:%s [age_of_event=%f]\n", t2, SA.eventid, age_of_event);
LOG_MSG("time:%lf evid:%s [age_of_event=%f]", t2, SA.eventid, age_of_event);
        //if ((props.processingTime - age_of_event) < 1)

        mins = (int)floor(age_of_event/60.);
        secs = age_of_event - 60.*mins;

        if (age_of_event >= props.processingTime)
        {
//printf("driveGFAST: time:%lf evid:%s has expired --> finalize\n", t2, SA.eventid);
LOG_MSG("time:%lf evid:%s has expired --> finalize", t2, SA.eventid);
            nPop = nPop + 1;
            lfinalize = true;
LOG_MSG("%s", "Call core_log_closeLogs() before early exit from loop");
            core_log_closeLogs();
            continue;
        }

        // Set the log file names
        eewUtils_setLogFileNames(SA.eventid,
                                 errorLogFileName, infoLogFileName,
                                 debugLogFileName, warnLogFileName);
        core_log_openErrorLog(errorLogFileName);
        core_log_openInfoLog(infoLogFileName);
        core_log_openWarningLog(warnLogFileName);
        core_log_openDebugLog(debugLogFileName);
/*
        log_initErrorLog(&__errorToLog);
        log_initInfoLog(&__infoToLog);
        log_initDebugLog(&__debugToLog);
        log_initWarnLog(&__warnToLog);
*/
        // Get the data for this event
//printf("driveGFAST: get data\n");
LOG_MSG("get data t1:%f t2:%f", t1, t2);
        ierr = GFAST_traceBuffer_h5_getData(t1, t2, h5traceBuffer);
//LOG_MSG("get data t1:%f t2:%f returned ierr=%d", t1, t2, ierr);
        if (ierr != 0)
        {
//printf("driveGFAST: Error getting the data for event --> continue\n");
LOG_MSG("Error getting the data for event:%s --> continue", SA.eventid);
            LOG_ERRMSG("Error getting the data for event %s", SA.eventid);
            continue; 
        }
        // Copy the data onto the buffer
LOG_MSG("%s", "CopyTraceBufferToGFAST");
        ierr = GFAST_traceBuffer_h5_copyTraceBufferToGFAST(h5traceBuffer,
                                                           gps_data);
LOG_MSG("%s returned ierr=%d", "CopyTraceBufferToGFAST", ierr);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error copying trace buffer");
            continue;
        }
LOG_MSG("%s", "Get peakDisp");
//printf("driveGFAST: Get peakDisp\n");
        // Extract the peak displacement from the waveform buffer
        nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
                                    props.pgd_props.utm_zone,
                                    props.pgd_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    *gps_data,
                                    pgd_data,
                                    &ierr);
LOG_MSG("%s returned ierr=%d nsites_pgd=%d", "Get peakDisp", ierr, nsites_pgd);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error processing peak displacement");
            continue;
        }
//printf("driveGFAST: Get Offset for CMT\n");
LOG_MSG("%s", "Get Offset for CMT");
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
LOG_MSG("%s returned ierr=%d nsites_cmt=%d", "Get Offset for CMT", ierr, nsites_cmt);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error processing CMT offset");
            continue;
        }
//printf("driveGFAST: Get Offset for FF\n");
LOG_MSG("%s", "Get Offset for FF");
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
LOG_MSG("%s returned ierr=%d nsites_ff=%d", "Get Offset for FF", ierr, nsites_ff);
        if (ierr != 0)
        {
            LOG_ERRMSG("%s", "Error processing FF offset");
            continue;
        }
        // Run the PGD scaling
        lpgdSuccess = false;
        if (nsites_pgd >= props.pgd_props.min_sites)
        {
            if (props.verbose > 2)
            {
                LOG_INFOMSG("Estimating PGD scaling for %s...", SA.eventid);
            }
            lpgdSuccess = true;
//printf("driveGFAST: drivePGD\n");
LOG_MSG("Call drivePGD eventid=%s", SA.eventid);
            ierr = eewUtils_drivePGD(props.pgd_props,
                                     SA.lat, SA.lon, SA.dep,
                                     *pgd_data,
                                     pgd);
LOG_MSG("drivePGD returned ierr=%d", ierr);
            if (ierr != PGD_SUCCESS)
            {
                LOG_ERRMSG("%s", "Error computing PGD");
                lpgdSuccess = false;
            }
        }
        // Run the CMT inversion
        lcmtSuccess = false;
        if (nsites_cmt >= props.cmt_props.min_sites)
        {
            if (props.verbose > 2)
            {
                LOG_INFOMSG("Estimating CMT for %s...", SA.eventid);
            }
            lcmtSuccess = true;
//printf("driveGFAST: driveCMT\n");
LOG_MSG("%s", "driveCMT");
            ierr = eewUtils_driveCMT(props.cmt_props,
                                     SA.lat, SA.lon, SA.dep,
                                     *cmt_data,
                                     cmt);
LOG_MSG("%s returned ierr=%d", "driveCMT", ierr);
            if (ierr != CMT_SUCCESS || cmt->opt_indx < 0)
            {
                LOG_ERRMSG("%s", "Error computing CMT");
                lcmtSuccess = false;
            }
        }
        // If we got a CMT see if we can run a finite fault inversion  
        lffSuccess = false;
        if (lcmtSuccess && nsites_ff >= props.ff_props.min_sites)
        {
            if (props.verbose > 2)
            {
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
//printf("driveGFAST: driveFF\n");
            ierr = eewUtils_driveFF(props.ff_props,
                                    SA.lat, SA.lon, //SA.dep,
                                    *ff_data,
                                    ff);
            if (ierr != FF_SUCCESS)
            {
                LOG_ERRMSG("%s", "Error computing finite fault");
                lffSuccess = false;
            }
        }
        // Finalize?
        pgdXML = NULL;
        cmtQML = NULL;
        ffXML = NULL;
        lfinalize = false;
        /*
        printf("t1=%lf t2=%lf (t2-t1)=%lf props.processingTime=%lf\n", t1, t2, t2 - t1, props.processingTime);
        if (t2 - t1 >= props.processingTime)
        {
            nPop = nPop + 1;
            lfinalize = true;
        printf("MTH: Set lfinalize true=%d nPop=%d\n", lfinalize, nPop);
        }
        */
        if (true) //if (t2 - t1 >= props.processingTime)
        {
LOG_MSG("driveGFAST: make XML msgs: lpgdSuccess=%d lcmtSuccess=%d lffSuccess=%d\n",
       lpgdSuccess, lcmtSuccess, lffSuccess);
            lfinalize = true;
            // Make the PGD xml
            if (lpgdSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("%s", "Generating pgd XML");
                }
                pgdOpt = array_argmax64f(pgd->ndeps, pgd->dep_vr_pgd, &ierr);
                pgdXML = eewUtils_makeXML__pgd(props.opmode, //shakeAlertMode,
                                               "GFAST\0",
                                               GFAST_VERSION,
                                               GFAST_INSTANCE,
                                               "new\0",
                                               GFAST_VERSION,
                                               SA.eventid,
                                               SA.lat,
                                               SA.lon,
                                               pgd->srcDepths[pgdOpt],
                                               pgd->mpgd[pgdOpt],
                                               SA.time,
                                               &ierr);
                if (ierr != 0)
                {
                    LOG_ERRMSG("%s", "Error generating PGD XML");
                    if (pgdXML != NULL)
                    {
                        free(pgdXML);
                        pgdXML = NULL;
                    }
                }
                xmlMessages->pgdXML[xmlMessages->nmessages] = pgdXML;

                //LOG_MSG("Age_of_event=%f [%d] mins %.3f secs", age_of_event, mins, secs);

                if (props.output_interval_mins[0] == 0) { // Output at every iteration
                  int index = (int)(currentTime - SA.time);
                  ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "pgd", pgdXML, index, false);
                }
                else {
                  LOG_MSG("eventid:%s age:%f --> check_mins_against_intervals\n", SA.eventid, age_of_event);
                  if (strcmp(SA.eventid, xml_status->SA_status[iev].eventid) == 0){
                      check_mins_against_intervals(props, mins, SA.eventid, "pgd", pgdXML,
                                                   xml_status->SA_status[iev].interval_complete,
                                                   age_of_event);
                                                   //xml_status->SA_status[iev].interval_complete,

                  /*

                  if (strcmp(SA.eventid, xml_status->SA_status[iev].eventid) == 0){
                    // We have the same one
                    for (i=0; i<props.n_intervals-1; i++){
                      if (mins >= props.output_interval_mins[i] && mins < props.output_interval_mins[i+1]) {
                        if (xml_status->SA_status[iev].interval_complete[i] == false) {
                          LOG_MSG("Age_of_event=%f --> Output minute %d PGD solution",
                                  age_of_event, props.output_interval_mins[i]);
                          ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "pgd",
                                                  pgdXML, props.output_interval_mins[i], true);
                          xml_status->SA_status[iev].interval_complete[i] = true;
                          break;
                        }
                      }
                    }
                  */

                    /*
                    for (i=0; i<props.n_intervals; i++){
                      if (mins == props.output_interval_mins[i] && secs < 1.){
                        LOG_MSG("Age_of_event=%f --> Output minute %d PGD solution",
                                age_of_event, props.output_interval_mins[i]);
                        ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "pgd",
                                                pgdXML, props.output_interval_mins[i], true);
                      }
                    }
                    */
                  }
                  else {
                    LOG_MSG("Mismatch between SA.eventid=%s and xml_status.SA_status[%d].eventid=%s --> Can't output PGD!\n",
                             SA.eventid, iev, xml_status->SA_status[iev].eventid);
                    LOG_ERRMSG("Mismatch between SA.eventid=%s and xml_status.SA_status[%d].eventid=%s --> Can't output PGD!\n",
                             SA.eventid, iev, xml_status->SA_status[iev].eventid);
                  }

		            }
            } //if lpgdSuccess
            // Make the CMT quakeML
            if (lcmtSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("%s", "Generating CMT QuakeML");
                }
                cmtQML = eewUtils_makeXML__quakeML(props.anssNetwork,
                                                  props.anssDomain,
                                                  SA.eventid,
                                                  SA.lat,
                                                  SA.lon,
                                                  cmt->srcDepths[cmt->opt_indx],
                                                  SA.time,
                                                  &cmt->mts[6*cmt->opt_indx],
                                                  &ierr);
                if (ierr != 0)
                {
                    LOG_ERRMSG("%s", "Error generating CMT quakeML");
                    if (cmtQML != NULL)
                    {
                        free(cmtQML);
                        cmtQML = NULL;
                    }
                }
                xmlMessages->cmtQML[xmlMessages->nmessages] = cmtQML;
                if (props.output_interval_mins[0] == 0) { // Output at every iteration
		              int index = (int)(currentTime - SA.time);
                  LOG_MSG("Age_of_event=%f --> Output CMT solution at iter:%d", age_of_event, index);
                  ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "cmt",
                                           cmtQML, index, false);
                }
                else {
                  for (i=0; i<props.n_intervals; i++){
                    if (mins == props.output_interval_mins[i] && secs < 1.){
                      LOG_MSG("Age_of_event=%f --> Output minute %d CMT solution",
                          age_of_event, props.output_interval_mins[i]);
                      ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "cmt",
                                               cmtQML, props.output_interval_mins[i], true);
                    }
                  }
                }
            }
            // Make the finite fault XML
            if (lffSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("Generating FF XML; preferred plane=%d",
                                 ff->preferred_fault_plane+1);
                }
                ipf = ff->preferred_fault_plane;
                nstrdip = ff->fp[ipf].nstr*ff->fp[ipf].ndip;
                ffXML = eewUtils_makeXML__ff(props.opmode,
                                             "GFAST\0",
                                             GFAST_VERSION,
                                             GFAST_INSTANCE,
                                             "new\0",
                                             GFAST_VERSION,
                                             SA.eventid,
                                             SA.lat,
                                             SA.lon,
                                             SA.dep,
                                             SA.mag,
                                             SA.time,
                                             nstrdip,
                                             ff->fp[ipf].fault_ptr,
                                             ff->fp[ipf].lat_vtx,
                                             ff->fp[ipf].lon_vtx,
                                             ff->fp[ipf].dep_vtx,
                                             ff->fp[ipf].sslip,
                                             ff->fp[ipf].dslip,
                                             ff->fp[ipf].sslip_unc,
                                             ff->fp[ipf].dslip_unc,
                                             &ierr); 
                if (ierr != 0)
                {
                    LOG_ERRMSG("%s", "Error generating finite fault XML");
                    if (ffXML != NULL)
                    {
                        free(ffXML);
                        ffXML = NULL;
                    }
                }
                xmlMessages->ffXML[xmlMessages->nmessages] = ffXML;
                if (props.output_interval_mins[0] == 0) { // Output at every iteration
		              int index = (int)(currentTime - SA.time);
                  LOG_MSG("Age_of_event=%f --> Output FF solution at iter:%d", age_of_event, index);
                  ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "ff",
                                           ffXML, index, false);
                }
                else {
                  for (i=0; i<props.n_intervals; i++){
                      if (mins == props.output_interval_mins[i] && secs < 1.){
                        LOG_MSG("Age_of_event=%f --> Output minute %d FF solution",
                                 age_of_event, props.output_interval_mins[i]);
                        ierr = eewUtils_writeXML(props.SAoutputDir, SA.eventid, "ff",
                                                ffXML, props.output_interval_mins[i], true);
                    }
                  }
                }
            }
            xmlMessages->evids[xmlMessages->nmessages]
                = (char *)calloc(strlen(SA.eventid)+1, sizeof(char));
            strcpy(xmlMessages->evids[xmlMessages->nmessages], SA.eventid);
            xmlMessages->nmessages = xmlMessages->nmessages + 1;
        } // End check on finalizing
        // Update the archive
        if (lfinalize || !props.lh5SummaryOnly)
        {
            // Get the iteration number in the H5 file
            h5k = 0;
            h5k = GFAST_hdf5_updateGetIteration(props.h5ArchiveDir,
                                                SA.eventid,
                                                currentTime);
//printf("driveGFAST: time:%lf evid:%s iteration=%d Update h5 archive\n", t2, SA.eventid, h5k);
LOG_MSG("time:%lf evid:%s iteration=%d Update h5 archive", t2, SA.eventid, h5k);
            if (props.verbose > 2)
            {
                LOG_DEBUGMSG("Writing GPS data for iteration %d", h5k);
            }
            ierr = GFAST_hdf5_update_gpsData(props.h5ArchiveDir,
                                             SA.eventid,
                                             h5k,
                                             *gps_data);
            if (props.verbose > 2)
            {
                LOG_DEBUGMSG("Writing hypocenter for iteration %d", h5k);
            }
            ierr = GFAST_hdf5_updateHypocenter(props.h5ArchiveDir,
                                               SA.eventid,
                                               h5k,
                                               SA);
            if (lpgdSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("Writing PGD for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updatePGD(props.h5ArchiveDir,
                                            SA.eventid,
                                            h5k,
                                            *pgd_data,
                                            *pgd);
                if (pgdXML)
                {
                    ierr = hdf5_updateXMLMessage(props.h5ArchiveDir,
                                                 SA.eventid,
                                                 h5k, "pgdXML\0",
                                                 pgdXML);
                }
            }
            if (lcmtSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("Writing CMT for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updateCMT(props.h5ArchiveDir,
                                            SA.eventid,
                                            h5k,
                                            *cmt_data,
                                            *cmt);
                if (cmtQML)
                {
                    ierr = hdf5_updateXMLMessage(props.h5ArchiveDir,
                                                 SA.eventid,
                                                 h5k, "cmtQuakeML\0",
                                                 cmtQML);
                }
            }
            if (lffSuccess)
            {
                if (props.verbose > 2)
                {
                    LOG_DEBUGMSG("Writing FF for iteration %d", h5k);
                }
                ierr = GFAST_hdf5_updateFF(props.h5ArchiveDir,
                                           SA.eventid,
                                           h5k,
                                           *ff);
                if (ffXML)
                {
                    ierr = hdf5_updateXMLMessage(props.h5ArchiveDir,
                                                 SA.eventid,
                                                 h5k, "ffXML\0",
                                                 ffXML);
                }
            }
/*
            // TODO: Write the XML to the HDF5 file
            if (lfinalize && cmtQML)
            {
 
            }
            if (lfinalize && ffXML)
            {

            }
            if (lfinalize && pgdXML)
            {

            }
            if (lfinalize)
            {
                //ldownDate[iev] = true;
                nPop = nPop + 1;
            }
*/
        } // End check on updating archive or finalizing event
        // Close the logs
        //log_closeLogs();
        core_log_closeLogs();
//printf("driveGFAST: next event\n");
    } // Loop on the events
    // Need to down-date the events should any have expired
    if (nPop > 0)
    {
LOG_MSG("time:%lf RemoveExpiredEvents", currentTime);
        nRemoved = core_events_removeExpiredEvents(props.processingTime,
                                                   currentTime,
                                                   props.verbose,
                                                   events);
        core_events_syncXMLStatusWithEvents(events, xml_status);
LOG_MSG("time:%lf RemoveExpiredEvents nRemoved=%d", currentTime, nRemoved);
        if (nRemoved != nPop)
        {
            LOG_WARNMSG("%s", "Strange - check removeExpiredEvents");
        }
// MTH: Temp TEST !!!
/*
if (events->nev == 1){ 
LOG_MSG("%s", "** events.nev=1: Test finished event set ierr=1");
ierr = 1;
}
*/
    }
LOG_MSG("time:%lf return ierr=%d", currentTime, ierr);
    return ierr;

}


#include <unistd.h>
int eewUtils_writeXML(const char *dirname,
                      const char *eventid,
                      const char *msg_type,
                      const char *message,
                      int interval,
                      bool interval_in_mins
                      )
{
   char fullpath[128];

   //*ierr = 1;
   FILE * fp;

   if (interval_in_mins){
       sprintf(fullpath, "%s/%s.%s.%d_min", dirname, eventid, msg_type, interval);
       LOG_MSG("driveGFAST: evid=%s SA xml index=%d --> output XML to file=[%s]\n",
           eventid, interval, fullpath);
   }
   else {
       sprintf(fullpath, "%s/%s.%s.%d", dirname, eventid, msg_type, interval);
       LOG_MSG("driveGFAST: evid=%s min=%d --> output XML to file=[%s]\n",
           eventid, interval, fullpath);
   }
   //puts(fullpath);

   if (access( fullpath, F_OK ) != -1 ) {
     LOG_MSG("File:%s already exists!\n", fullpath);
   }
   /*
   else {
     LOG_MSG("File:%s doesn't exist\n", fullpath);
   }
   */

   fp = fopen(fullpath, "w");
   fprintf(fp, "%s\n", message);
   fclose(fp);

   return 0;

}

bool check_mins_against_intervals(
      struct GFAST_props_struct props,
      int mins,
      char * eventid,
      char * suffix,
      char * xml,
      bool * interval_complete,
      double age
      )
{
   int i, ierr;
   LOG_MSG("MTH: check_mins_agains_intervals: eventid:%s suffix:%s age:%f\n",
       eventid, suffix, age);
   for (i=0;i<16;i++){
     printf("interval_complete[%d]=%d\n", i, interval_complete[i]);
   }
   LOG_MSG("MTH: check_mins_agains_intervals: eventid:%s suffix:%s age:%f\n",
       eventid, suffix, age);
   printf("MTH: check_mins_agains_intervals: eventid:%s suffix:%s age:%f mins:%d\n",
       eventid, suffix, age, mins);

   for (i=0; i<props.n_intervals-1; i++){
      printf("i=%d output_interval[%d]=%d\n",
          i, i, props.output_interval_mins[i]);
      if (mins >= props.output_interval_mins[i] && mins < props.output_interval_mins[i+1]) {
      printf("mins > output_interval[%d]=%d and mins < output_interval[%d]=%d\n",
          i, props.output_interval_mins[i], i+1, props.output_interval_mins[i+1]);
        if (interval_complete[i] == false) {
          LOG_MSG("Eventid:%s age_of_event:%f --> Output minute %d solution for suff:%s",
                  age, props.output_interval_mins[i], suffix);

          ierr = eewUtils_writeXML(props.SAoutputDir, eventid, suffix, xml,
                                   props.output_interval_mins[i], true);

          interval_complete[i] = true;
          return true;
      }
    }
   }

   return false;
}
