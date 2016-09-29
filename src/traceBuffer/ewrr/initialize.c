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
#include "gfast_traceBuffer.h"
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

/*!
 * @brief Initializes the HDF5
 *
 * @param[in] ewRing     null terminated earthworm data ring name
 *
 * @param[out] region    shared memory region corresponding to earthworm ring
 *
 *
 */
int traceBuffer_ewrr_initialize(const char *configFile,
                                const char *ewRing,
                                const int msWait,
                                struct ewRing_struct *ringInfo)
{
    const char *fcnm = "traceBuffer_ewrr_initialize\0";
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
    // Look up instrumentID 
    if (GetLocalInst(&ringInfo->instLocalID) != 0)
    {
        log_errorF("%s: Error getting local instrument id!\n", fcnm);
        return -1;
    }
    if (GetInst("INST_WILDCARD", &ringInfo->instWildcardID ) != 0)
    {
        log_errorF("%s: Error getting INST_WILDCARD!\n", fcnm);
        return -1;
    } 
    // Look up the tracebuffer2 type 
    if (GetType("TYPE_TRACEBUF2", &ringInfo->traceBuffer2Type) != 0)
    {
        log_errorF("%s Error getting TYPE_TRACEBUF2!\n", fcnm);
        return -1;
    }
    // Look up the heartbeat type 
    if (GetType("TYPE_HEARTBEAT", &ringInfo->heartBeatType) != 0)
    {   
        log_errorF("%s Error getting TYPE_HEARTBEAT!\n", fcnm);
        return -1; 
    }
    // Look up the error type
    if (GetType("TYPE_ERROR", &ringInfo->errorType) != 0)
    {
        log_errorF("%s Error getting TYPE_ERROR!\n", fcnm);
        return -1;
    }
    if (GetModId( "MOD_WILDCARD", &ringInfo->modWildcardID) != 0 )
    {
        log_errorF("%s: MOD_WILDCARD Missing from earthworm(_global).d\n",
                  fcnm);
        return -1;
    }
    // Hook the getLogo's up for reading tracebuffer2s
    ringInfo->nlogo = 1;
    ringInfo->getLogo = (MSG_LOGO *)calloc(ringInfo->nlogo, sizeof(MSG_LOGO));
    ringInfo->getLogo[0].type = ringInfo->traceBuffer2Type;
    // Attach to the ring
    tport_attach(&ringInfo->region, ringInfo->ringKey);
    ringInfo->linit = true;
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
