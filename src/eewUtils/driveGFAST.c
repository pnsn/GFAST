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
#include "iscl/log/log.h"
#include "iscl/log/logfiles.h"
#include "iscl/memory/memory.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"

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
                        struct GFAST_xmlMessages_struct *xmlMessages)
{
    const char *fcnm = "eewUtils_driveGFAST\0";
    struct GFAST_shakeAlert_struct *SAall, SA;
    //char errorLogFileName[PATH_MAX], infoLogFileName[PATH_MAX], 
    //     debugLogFileName[PATH_MAX], warnLogFileName[PATH_MAX];
    char *cmtQML, *ffXML, *pgdXML;
    double t1, t2;
    int h5k, ierr, iev, ipf, nev0, nPop, nsites_cmt, nsites_ff, nsites_pgd,
        nstrdip, pgdOpt, shakeAlertMode;
    bool *ldownDate, lcmtSuccess, lffSuccess, lfinalize, lgone, lpgdSuccess;
    //------------------------------------------------------------------------//
    //
    // Nothing to do 
    ierr = 0;
    if (events->nev <= 0){return 0;}
    // Figure out the mode for generating shakeAlert messages
    shakeAlertMode = 1;
    if (props.opmode == PLAYBACK){shakeAlertMode = 2;}
    // Set memory for XML messages
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
    ldownDate = memory_calloc8l(events->nev);
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
            log_warnF("%s: Origin time > currentime? - skipping event %s\n",
                      fcnm, SA.eventid);
            continue;
        }
        // Set the log file names
        eewUtils_setLogFileNames(SA.eventid);
        log_initErrorLog(&__errorToLog);
        log_initInfoLog(&__infoToLog);
        log_initDebugLog(&__debugToLog);
        log_initWarnLog(&__warnToLog);
        // Get the data for this event
printf("getting data\n");
        ierr = GFAST_traceBuffer_h5_getData(t1, t2, h5traceBuffer);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting the data for event %s\n",
                       fcnm, SA.eventid);
            continue; 
        }
printf("copying data\n");
        // Copy the data onto the buffer
        ierr = GFAST_traceBuffer_h5_copyTraceBufferToGFAST(h5traceBuffer,
                                                           gps_data);
        if (ierr != 0)
        {
            log_errorF("%s: Error copying trace buffer\n", fcnm);
            continue;
        }
printf("waveform processing\n");
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
        if (ierr != 0)
        {
            log_errorF("%s: Error processing peak displacement\n", fcnm);
            continue;
        }
printf("waveform prcoessing 2\n");
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
        if (ierr != 0)
        {
            log_errorF("%s: Error processing CMT offset\n", fcnm);
            continue;
        }
printf("waveform processing 3\n");
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
        if (ierr != 0)
        {
            log_errorF("%s: Error processing FF offset\n", fcnm);
            continue;
        }
printf("pgd scaling..\n");
        // Run the PGD scaling
        lpgdSuccess = false;
        if (nsites_pgd >= props.pgd_props.min_sites)
        {
            if (props.verbose > 2)
            {
                log_infoF("%s: Estimating PGD scaling...\n", fcnm);
            }
            lpgdSuccess = true;
            ierr = eewUtils_drivePGD(props.pgd_props,
                                     SA.lat, SA.lon, SA.dep,
                                     *pgd_data,
                                     pgd);
            if (ierr != PGD_SUCCESS)
            {
                log_errorF("%s: Error computing PGD\n", fcnm);
                lpgdSuccess = false;
            }
        }
        // Run the CMT inversion
        lcmtSuccess = false;
        if (nsites_cmt >= props.cmt_props.min_sites)
        {
            if (props.verbose > 2)
            {
                log_infoF("%s: Estimating CMT...\n", fcnm);
            }
            lcmtSuccess = true;
            ierr = eewUtils_driveCMT(props.cmt_props,
                                     SA.lat, SA.lon, SA.dep,
                                     *cmt_data,
                                     cmt);
            if (ierr != CMT_SUCCESS)
            {
                log_errorF("%s: Error computing CMT\n", fcnm);
                lcmtSuccess = false;
            }
        }
        // If we got a CMT see if we can run a finite fault inversion  
        lffSuccess = false;
        if (lcmtSuccess && nsites_ff >= props.ff_props.min_sites)
        {
            if (props.verbose > 2)
            {
                log_infoF("%s: Estimating finite fault...\n", fcnm);
            }
            ff->SA_lat = events->SA[iev].lat;
            ff->SA_lon = events->SA[iev].lon;
            ff->SA_dep = cmt->srcDepths[cmt->opt_indx];
            ff->SA_mag = cmt->Mw[cmt->opt_indx];
            ff->str[0] = cmt->str1[cmt->opt_indx];
            ff->str[1] = cmt->str2[cmt->opt_indx];
            ff->dip[0] = cmt->dip1[cmt->opt_indx];
            ff->dip[1] = cmt->dip2[cmt->opt_indx];
            lffSuccess = true;
            ierr = eewUtils_driveFF(props.ff_props,
                                    SA.lat, SA.lon, //SA.dep,
                                    *ff_data,
                                    ff);
            if (ierr != FF_SUCCESS)
            {
                log_errorF("%s: Error computing finite fault\n", fcnm);
                lffSuccess = false;
            }
        }
        // Finalize?
        pgdXML = NULL;
        cmtQML = NULL;
        ffXML = NULL;
        lfinalize = false;
        //printf("%lf %lf %lf %lf\n", t1, t2, t2 - t1, props.processingTime);
        if (t2 - t1 >= props.processingTime)
        {
            lfinalize = true;
            // Make the PGD xml
            if (lpgdSuccess)
            {
                pgdOpt = ISCL_array_argmax64f(pgd->ndeps, pgd->dep_vr_pgd);
                pgdXML = eewUtils_makeXML__pgd(shakeAlertMode,
                                               "GFAST\0",
                                               GFAST_ALGORITHM_VERSION,
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
                    log_errorF("%s: Error generating PGD XML\n", fcnm);
                    if (pgdXML != NULL)
                    {   
                        free(pgdXML);
                        pgdXML = NULL;
                    }
                }
                xmlMessages->pgdXML[xmlMessages->nmessages] = pgdXML;
            }
            // Make the CMT quakeML
            if (lcmtSuccess)
            {
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
                    log_errorF("%s: Error generating CMT quakeML\n", fcnm);
                    if (cmtQML != NULL)
                    {
                        free(cmtQML);
                        cmtQML = NULL;
                    }
                }
                xmlMessages->cmtQML[xmlMessages->nmessages] = cmtQML;
            }
            // Make the finite fault XML
            if (lffSuccess)
            {
                ipf = ff->preferred_fault_plane;
                nstrdip = ff->fp[ipf].nstr*ff->fp[ipf].ndip;
                ffXML = eewUtils_makeXML__ff(props.opmode,
                                             "GFAST\0",
                                             GFAST_ALGORITHM_VERSION,
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
                    log_errorF("%s: Error generating finite fault XML\n", fcnm);
                    if (ffXML != NULL)
                    {
                        free(ffXML);
                        ffXML = NULL;
                    }
                }
                xmlMessages->ffXML[xmlMessages->nmessages] = ffXML;
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
            ierr = GFAST_hdf5_update_gpsData(props.h5ArchiveDir,
                                             SA.eventid,
                                             h5k,
                                             *gps_data);
            ierr = GFAST_hdf5_updateHypocenter(props.h5ArchiveDir,
                                               SA.eventid,
                                               h5k,
                                               SA);
            if (lpgdSuccess)
            {
                ierr = GFAST_hdf5_updatePGD(props.h5ArchiveDir,
                                            SA.eventid,
                                            h5k,
                                            *pgd_data,
                                            *pgd);
            }
            if (lcmtSuccess)
            {
                ierr = GFAST_hdf5_updateCMT(props.h5ArchiveDir,
                                            SA.eventid,
                                            h5k,
                                            *cmt_data,
                                            *cmt);
            }
            if (lffSuccess)
            {
                ierr = GFAST_hdf5_updateFF(props.h5ArchiveDir,
                                           SA.eventid,
                                           h5k,
                                           *ff);
            }
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
                ldownDate[iev] = true;
                nPop = nPop + 1;
            }
        } // End check on updating archive or finalizing event
        // Close the logs
        log_closeLogs();
    } // Loop on the events
    // Need to down-date the events should any have expired
    if (nPop > 0 && events->nev > 0)
    {
        nev0 = events->nev;
        SAall = (struct GFAST_shakeAlert_struct *)
                calloc((size_t) nev0, sizeof(struct GFAST_shakeAlert_struct));
        for (iev=0; iev<events->nev; iev++)
        {
            memcpy(&SAall[iev], &events->SA[iev],
                   sizeof(struct GFAST_shakeAlert_struct));
        }
        for (iev=0; iev<nev0; iev++)
        {
            if (ldownDate[iev])
            {
                lgone = core_events_removeExpiredEvent(props.processingTime,
                                                       currentTime,
                                                       props.verbose,
                                                       SAall[iev], events);
                if (!lgone)
                {
                    log_warnF("%s: Strange - but keeping %s\n",
                              fcnm, SAall[iev].eventid);
                }
            }
         }
         free(SAall);
     }
        
/*
        t1 = SA.time;     // Origin time
        t2 = currentTime;
        core_events_removeExpiredEvent(props.processingTime, currentTime, 
*/
/*
    while (iev < events->nev)
    {
bool core_events_removeExpiredEvent(const double maxtime,
                                    const double currentTime,
                                    const int verbose,
                                    struct GFAST_shakeAlert_struct SA, 
                                    struct GFAST_activeEvents_struct *events)
        
    }
*/
    memory_free8l(&ldownDate);
    return ierr;
}

