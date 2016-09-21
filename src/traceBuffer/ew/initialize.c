#ifdef WINNT
#include <window.h>
#include <winuser.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <trace_buf.h>
#include <earthworm.h>


typedef struct  
{
     unsigned char GetThisInstId;   /* Get messages from this inst id */
     unsigned char GetThisModId;    /* Get messages from this module */
     unsigned char MyInstId;        /* Local installation */
     unsigned char TypeError;       /* Error message id */
     unsigned char TypeHeartBeat;   /* Heartbeat message id */
     unsigned char TypeHypoTWC;     /* Hypocenter message - TWC format*/
     unsigned char TypeWaveform;    /* Earthworm waveform messages */
} EWH;

/*!
 * @brief Initializes the HDF5
 *
 */
int traceBuffer_earthworm_initialize(const char configFile[PATH_MAX],
                                     const char tablePath[PATH_MAX] )
{
    long msgLen;
    // Environmental parameter to read stations
#ifdef _WINNT
 
#endif
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
