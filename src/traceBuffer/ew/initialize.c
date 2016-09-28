#ifdef WINNT
#include <window.h>
#include <winuser.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <trace_buf.h>
#include <transport.h>
#include <earthworm.h>
#include <trace_buf.h>
#include "iscl/log/log.h"


typedef struct  
{
   unsigned char GetThisInstId;   /*!< Get messages from this inst id */
   unsigned char GetThisModId;    /*!< Get messages from this module */
   unsigned char MyInstId;        /*!< Local installation */
   unsigned char TypeError;       /*!< Error message id */
   unsigned char TypeHeartBeat;   /*!< Heartbeat message id */
   unsigned char TypeHypoTWC;     /*!< Hypocenter message - TWC format*/
   unsigned char TypeWaveform;    /*!< Earthworm waveform messages */
} EWH;

struct ewRing_struct
{
    char ewRingName[512];  /*!< Earthworm ring name to which we will connect */
    SHM_INFO region;       /*!< Earthworm shared memory region corresponding to
                                the earthworm ring */
    MSG_LOGO *getLogo;     /*!< Logos to scrounge from the ring [nlogo] */
    long ringKey;          /*!< Ring key number */
    short nlogo;           /*!< Number of logos */
    bool linit;            /*!< True if the structure is initialized.
                                False if the structure is not initialized. */
    unsigned msWait;       /*!< microseconds to wait after reading ring */
};

//static SHM_INFO inRegion;
//static long ringKey;

/*!
 * @brief Initializes the HDF5
 *
 * @param[in] ewRing     null terminated earthworm data ring name
 *
 * @param[out] region    shared memory region corresponding to earthworm ring
 *
 *
 */
int traceBuffer_ew_initialize(const char configFile[PATH_MAX],
                              const char tablePath[PATH_MAX],
                              const char *ewRing,
                              const int msWait,
                              SHM_INFO *region,
                              struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ew_initialize\0";
    //long msgLen;
    char fullTablePath[PATH_MAX];
    // Check the inputs
    if (ringInfo == NULL)
    {
        log_errorF("%s: ewRing_struct cannot be NULL\n", fcnm);
        return -1;
    }
    memset(ringInfo, 0, sizeof(struct ewRing_struct));
    strcpy(ringInfo->ewRingName, ewRing);
    if (ewRing == NULL)
    {
        log_errorF("%s: Error ewRing cannot be NULL\n", fcnm);
        return -1;
    }
    if (strlen(ewRing) == 0)
    {
        log_errorF("%s: Error ewRing must be specified\n", fcnm);
        return -1;
    }
    // Environmental parameter to read stations
    memset(fullTablePath, 0, sizeof(fullTablePath));
#ifdef _WINNT
    char *paramdir = getenv("EW_PARAMS"); 
    if (paramdir == (char *) NULL)
    {
        log_errorF("%s: Failed to get the environment\n", fcnm);
        return -1;    
    }
    sprintf(fullTablePath, "%s\\%s", paramdir, configFile);
#else
    sprintf(fullTablePath, "%s", configFile);
#endif
    // Attach to the ring
    ringInfo->ringKey = GetKey(ringInfo->ewRingName);
    if (ringInfo->ringKey ==-1)
    {
        log_errorF("%s: Invalid ring %s\n", fcnm, ewRing);
        return -1;
    }
    tport_attach(&ringInfo->region, ringInfo->ringKey);
/*
    if (GetConfig(fullTablePath, &Gparm) ==-1)
    {
        log_errorF("%s: GetConfig failed with file %s\n", fcnm, configFile);
        return -1;
    }
*/
    
    // Attach to existing transport rings
    //tport_attach(&Gparm.InRegion, Gparm.lInKey);
    // Flush the input waveform ring
/*
    while (tport_copyfrom( &region, getlogo, nLogo, &logo, &gotsize,
           (char *)&msg, MAX_TRACEBUF_SIZ, &sequence_number ) != GET_NONE )
    {
         packet_total++;
         packet_total_size += gotsize;
    } 
*/
/*
    result = tport_getmsg(&Gparm.InRegion, &getlogoW, 1, &logo, &msgLen,
                          waveBuf, MAX_TRACEBUF_SIZ);
    while (result != GET_NONE)
    {
        result = tport_getmsg(&Gparm.InRegion, &getlogoW, 1, &logo, &msgLen,
                              waveBuf, MAX_TRACEBUF_SIZ);
    }
*/
    return 0;
}
//============================================================================//
/*!
 * @brief Classifies the earthworm message
 *
 * @param[in] retval    earthworm return code to classify
 *
 * @retval  1 the requested message was received
 * @retval  0 there are no more messages
 * @retval -1 messages were received but there may be a loss of information
 * @retval -2 an error occurred and no messages were received
 *
 */
int traceBuffer_ew_classifyRetval(const int retval)
{
    const char *fcnm = "traceBuffer_ew_classifyRetval\0";
    char msg[128];
    // Got a requested message (modid, type, class)
    if (retval == GET_OK){return 1;}
    // No logos of requested messages in in memory 
    if (retval == GET_NONE){return 0;}
    memset(msg, 0, 128*sizeof(char)); 
    // Got a message but missed some
    if (retval == GET_MISS)
    {
        sprintf(msg, "%s: Some messages were missed\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // Got a message but ntrack_get was exceeded
    if (retval == GET_NOTRACK)
    {
        sprintf(msg, "%s: Message exceeded NTRACK_GET\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // Message requested exceeded my buffer
    if (retval == GET_TOOBIG)
    {
        sprintf(msg,
               "%s: Next message of requested logo(s) is too big\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -2;
    } 
    // Didn't check ring fast enough and missed a message
    if (retval == GET_MISS_LAPPED)
    {
        sprintf(msg, "%s: Some messages were overwritten\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // Message contains a gap
    if (retval == GET_MISS_SEQGAP)
    {
        sprintf(msg, "%s: A gap in messages was detected\n", fcnm);
        logit("et", msg);
        log_warnF("%s", msg);
        return -1;
    }
    // I don't know
    sprintf(msg, "%s: Could not classify return value\n", fcnm);
    logit("et", msg);
    log_warnF("%s", msg);
    return -2;
}
//============================================================================//
int traceBuffer_ew_flushRing(const int msWait, struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ew_flushRing\0";
    MSG_LOGO logo;
    char msg[MAX_TRACEBUF_SIZ];
    unsigned char sequenceNumber;
    long gotSize;
    int retval;
    //------------------------------------------------------------------------//
    //
    // Make sure user knows the ring was flushed
    if (ringInfo->linit == false)
    {
        log_errorF("%s: Error the ring was never initialized\n", fcnm);
        return -1;
    }
    //int   tport_copyfrom( SHM_INFO *, MSG_LOGO *, short, MSG_LOGO *,
    //                      long *, char *, long, unsigned char * );
    // Scroung the ring until we are done reading
    retval = tport_copyfrom(&ringInfo->region,
                            ringInfo->getLogo, ringInfo->nlogo,
                            &logo, &gotSize, msg, MAX_TRACEBUF_SIZ,
                            &sequenceNumber);
    while (retval != GET_NONE)
    {
        retval = tport_copyfrom(&ringInfo->region,
                                ringInfo->getLogo, ringInfo->nlogo,
                                &logo, &gotSize, msg, MAX_TRACEBUF_SIZ,
                                &sequenceNumber);
    }
    if (msWait < 0)
    {
        return 0;
    }
    sleep_ew(ringInfo->msWait);
    return 0;
}
//============================================================================//
/*!
 * @brief Disconnects from the ring and clears the earthworm ring structure
 *
 * @param[inout] ringInfo   on input contains the initialized earthworm ring
 *                          information.
 *                          on output contains the cleared earthworm ring
 *                          information.
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int traceBuffer_ew_finalize(struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ew_finalize\0";
    if (!ringInfo->linit)
    {
        log_errorF("%s: Error ewRing_struct was never initialized\n", fcnm);
        return -1;
    }
    tport_detach(&ringInfo->region);
    memset(ringInfo, 0, sizeof(struct ewRing_struct));
    return 0;
}

int GetEwh( EWH *Ewh )
{
   if ( GetLocalInst( &Ewh->MyInstId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting MyInstId.\n" );
      return -1;
   }
   if ( GetInst( "INST_WILDCARD", &Ewh->GetThisInstId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting GetThisInstId.\n" );
      return -2;
   }
   if ( GetModId( "MOD_WILDCARD", &Ewh->GetThisModId ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting GetThisModId.\n" );
      return -3;
   }
   if ( GetType( "TYPE_HEARTBEAT", &Ewh->TypeHeartBeat ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TypeHeartbeat.\n" );
      return -4;
   }
   if ( GetType( "TYPE_ERROR", &Ewh->TypeError ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TypeError.\n" );
      return -5;
   }
   if ( GetType( "TYPE_TRACEBUF2", &Ewh->TypeWaveform ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TYPE_TRACEBUF2.\n" );
      return -6;
   }
   if ( GetType( "TYPE_HYPOTWC", &Ewh->TypeHypoTWC ) != 0 )
   {
      fprintf( stderr, "Theta: Error getting TYPE_HYPOTWC.\n" );
      return -7;
   }
   return 0;
}
