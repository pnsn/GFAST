#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_core.h"
#include "gfast_eewUtils.h"
#include "gfast_events.h"
#include "gfast_hdf5.h"
#include "gfast_traceBuffer.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"
#include "iscl/time/time.h"

static void setFileNames(const char *eventid);

/*!
 * @brief Expert earthquake early warning GFAST driver
 *
 * @param[in] currentTime     current epochal time
 * @param[in] xmlMessage      shakeAlert XML message
 */
int eewUtils_driveGFAST(const double currentTime,
                        const char *xmlMessage,
                        struct GFAST_data_struct *gps_data,
                        struct h5traceBuffer_struct *h5traceBuffer,
                        struct GFAST_props_struct props,
                        struct GFAST_peakDisplacementData_struct *pgd_data,
                        struct GFAST_offsetData_struct *cmt_data,
                        struct GFAST_offsetData_struct *ff_data,
                        struct GFAST_activeEvents_struct *events,
                        struct GFAST_pgdResults_struct *pgd,
                        struct GFAST_cmtResults_struct *cmt,
                        struct GFAST_ffResults_struct *ff,
                        struct GFAST_xmlMessages_struct *xmlMessages)
{
    const char *fcnm = "eewUtils_driveGFAST\0";
    struct GFAST_shakeAlert_struct SA;
    //char errorLogFileName[PATH_MAX], infoLogFileName[PATH_MAX], 
    //     debugLogFileName[PATH_MAX], warnLogFileName[PATH_MAX];
    char *cmtQML, *ffXML, *pgdXML;
    double t1, t2;
    int h5k, ierr, iev, nsites_cmt, nsites_ff, nsites_pgd;
    bool lcmtSuccess, lffSuccess, lfinalize, lnewEvent, lpgdSuccess;
    const double SA_NAN =-12345.0; 
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    // Parse the core XML for the hypocenter information 
    if (xmlMessage != NULL)
    { 
        ierr = GFAST_eewUtils_parseCoreXML(xmlMessage, SA_NAN, &SA);
        if (ierr != 0)
        {
            log_errorF("%s: Error parsing XML message\n", fcnm);
            return ierr;
        }
        // If this is a new event we have some file handling to do
        lnewEvent = GFAST_events_newEvent(SA, events);
        if (lnewEvent)
        {
            // And the logs
            if (props.verbose > 0)
            {   
                log_infoF("%s: New event %s added\n", fcnm, SA.eventid);
                if (props.verbose > 2){GFAST_events_printEvents(SA);}
            }
            // Set the log file names
            setFileNames(SA.eventid);
            if (os_path_isfile(errorLogFileName)){remove(errorLogFileName);}
            if (os_path_isfile(infoLogFileName)){remove(infoLogFileName);}
            if (os_path_isfile(debugLogFileName)){remove(debugLogFileName);}
            if (os_path_isfile(warnLogFileName)){remove(warnLogFileName);}
            // Initialize the HDF5 file
            ierr = GFAST_hdf5_initialize(props.h5ArchiveDir,
                                         SA.eventid,
                                         props.propfilename);
            if (ierr != 0)
            {
                log_errorF("%s: Error initializing the archive file\n", fcnm);
                return -1;
            }
        }
    }
    // Nothing to do
    if (events->nev <= 0){return 0;}
    // Set memory for XML messages
    xmlMessages->mmessages = events->nev;
    xmlMessages->nmessages = 0;
    xmlMessages->evids  = (char **)calloc(xmlMessages->mmessages,
                                          sizeof(char *));
    xmlMessages->cmtQML = (char **)calloc(xmlMessages->mmessages,
                                          sizeof(char *));
    xmlMessages->ffXML  = (char **)calloc(xmlMessages->mmessages,
                                          sizeof(char *));
    xmlMessages->pgdXML = (char **)calloc(xmlMessages->mmessages,
                                          sizeof(char *));
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
        setFileNames(SA.eventid);
        log_initErrorLog(&__errorToLog);
        log_initInfoLog(&__infoToLog);
        log_initDebugLog(&__debugToLog);
        log_initWarnLog(&__warnToLog);
        // Get the data for this event
        ierr = GFAST_traceBuffer_h5_getData(t1, t2, h5traceBuffer);
        if (ierr != 0)
        {
            log_errorF("%s: Error getting the data for event %s\n",
                       fcnm, SA.eventid);
            continue; 
        }
        // Copy the data onto the buffer
        ierr = GFAST_traceBuffer_h5_copyTraceBufferToGFAST(h5traceBuffer,
                                                           gps_data);
        if (ierr != 0)
        {
            log_errorF("%s: Error copying trace buffer\n", fcnm);
            continue;
        }
        // Extract the peak displacement from the waveform buffer
        nsites_pgd = GFAST_core_waveformProcessor_peakDisplacement(
                                    props.pgd_props.utm_zone,
                                    props.pgd_props.window_vel,
                                    SA.lat,
                                    SA.lon,
                                    SA.dep,
                                    SA.time,
                                    *gps_data, //tempData,
                                    pgd_data,
                                    &ierr);
        if (ierr != 0)
        {
            log_errorF("%s: Error processing peak displacement\n", fcnm);
            continue;
        }
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
                                    SA.lat, SA.lon, SA.dep,
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
        if (currentTime - SA.time >= props.processingTime)
        {
            lfinalize = true;

        }
        // Update the archive
        if (lfinalize || !props.lh5SummaryOnly)
        {
            // Get the iteration number in the H5 file
            h5k = 0;
            h5k = GFAST_hdf5_update__getIteration(props.h5ArchiveDir,
                                                  SA.eventid,
                                                  currentTime);
            ierr = GFAST_hdf5_update__gpsData(props.h5ArchiveDir,
                                              SA.eventid,
                                              h5k,
                                              *gps_data);
            ierr = GFAST_hdf5_update__hypocenter(props.h5ArchiveDir,
                                                 SA.eventid,
                                                 h5k,
                                                 SA);
            if (lpgdSuccess)
            {
                ierr = GFAST_hdf5_update__pgd(props.h5ArchiveDir,
                                              SA.eventid,
                                              h5k,
                                              *pgd_data,
                                              *pgd);
            }
            if (lcmtSuccess)
            {
                ierr = GFAST_hdf5_update__cmt(props.h5ArchiveDir,
                                              SA.eventid,
                                              h5k,
                                              *cmt_data,
                                              *cmt);
            }
            if (lffSuccess)
            {
                ierr = GFAST_hdf5_update__ff(props.h5ArchiveDir,
                                             SA.eventid,
                                             h5k,
                                             *ff_data,
                                             *ff);
            }
            // Write the XML to the HDF5 file
            if (lfinalize && cmtQML)
            {

            }
            if (lfinalize && ffXML)
            {

            }
            if (lfinalize && pgdXML)
            {

            }
        }
        // Close the logs
        log_closeLogs();
    } // Loop on the events
    return ierr;
}

static void setFileNames(const char *eventid)
{
    // Set the log file names
    memset(errorLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(errorLogFileName, eventid);
    strcat(errorLogFileName, "_error.log\0");
    memset(infoLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(infoLogFileName, eventid);
    strcat(infoLogFileName, "_info.log\0");
    memset(debugLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(debugLogFileName, eventid);
    strcat(debugLogFileName, "_debug.log\0");
    memset(warnLogFileName, 0, PATH_MAX*sizeof(char));
    strcpy(warnLogFileName, eventid);
    strcat(warnLogFileName, "_debug.log\0");
    return;
}
