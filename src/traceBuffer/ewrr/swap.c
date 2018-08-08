/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id$
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.14  2010/03/19 16:42:58  paulf
 *     whoops, really fixed the max samples check
 *
 *     Revision 1.13  2010/03/19 16:30:15  paulf
 *     added check for nsamps to be smaller than max samples for a tbuf to WaveMsg2MakeLocal()
 *
 *     Revision 1.12  2007/12/16 19:26:39  paulf
 *     made SwapLong check for not 4 bytes for long....previous check was too limiting
 *
 *     Revision 1.11  2007/12/16 18:42:19  paulf
 *     made swap functions long int tolerant where in some OS, long can be 8 bytes
 *
 *     Revision 1.10  2005/06/13 18:15:41  dietz
 *     Changed logging for bad header values in WaveMsg2MakeLocal() to include
 *     all header values because one never knows which value is bogus.
 *
 *     Revision 1.9  2005/06/10 21:41:06  patton
 *     Modified comment for clarity (at least clear to me).  JMP
 *
 *     Revision 1.8  2005/06/10 21:19:30  patton
 *     Put in more meaningful comment into WaveMsg2MakeLocal to explain to the user
 *     the case that the tracebuf does not end within 5 samples of the given endtime.
 *     JMP
 *
 *     Revision 1.7  2004/04/13 22:59:38  dietz
 *     Added WaveMsg2MakeLocal()
 *
 *     Revision 1.6  2003/10/20 17:08:06  mark
 *     Added compiler flags check
 *
 *     Revision 1.5  2002/06/18 18:33:55  davidk
 *     fixed the sanity check of the header in WaveMsgMakeLocal().
 *     It contained a bug that made it useless.
 *
 *     Revision 1.4  2002/06/11 23:08:17  davidk
 *     Paul Friberg submitted a fix to WaveMsgMakeLocal, that copies the
 *     TRACE_HEADER to local variables to avoid byte alignment problems
 *     before examining the header.  (Bug was introduced by most
 *     recent change by davek that included code which examined the
 *     contents of the header without 8-byte aligning data.
 *
 *     Revision 1.3  2002/03/20 22:13:28  davidk
 *     Modified WaveMsgMakeLocal():
 *      Function now converts the header, performs a checksum on the header,
 *       and then if successful converts the binary data portion of the tracebuf.
 *      Function now returns -2 if the header checksum conversion fails.
 *      Function now returns -1 if _INTEL or _SPARC is not defined.
 *       (Formerly, no conversion was done and success was returned.)
 *
 *     Revision 1.2  2000/06/22 17:49:33  kohler
 *     Modified by WMK to allow in-place swapping of non-byte-alligned data.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */
/*
 * SWAP.C
 *
 *  Byte swapping functions
 */
 
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

/* Make sure one of the platforms is defined properly... */ 
#ifndef _INTEL
 #ifndef _SPARC
  #error _INTEL and _SPARC are both undefined
 #endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swap.h"
static int WaveMsgVersionMakeLocal( TRACE2X_HEADER* wvmsg, char version );
int32_t testInt32Var = 0;
/* SwapUint16: Byte swap 2-byte unsigned integer */
void SwapUint16( void *data )
{
   char temp;
   union {
      char  c[2];
   } dat;
   memcpy( &dat, data, 2 );
   temp     = dat.c[0];
   dat.c[0] = dat.c[1];
   dat.c[1] = temp;
   memcpy( data, &dat, 2 );
}
/* SwapUint32: Byte swap 4-byte unsigned integer */
void SwapUint32( uint32_t *data )
{
   char temp;
   union {
      char c[4];
   } dat;
   memcpy( &dat, data, 4 );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( data, &dat, 4 );
}
/* SwapUint64: Byte swap 8-byte unsigned integer */
void SwapUint64( EW_UNALIGNED void *data )
{
   char temp;
   union {
       char   c[8];
   } dat;
   memcpy( &dat, data, 8 );
   temp     = dat.c[0];
   dat.c[0] = dat.c[7];
   dat.c[7] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[6];
   dat.c[6] = temp;
   temp     = dat.c[2];
   dat.c[2] = dat.c[5];
   dat.c[5] = temp;
   temp     = dat.c[3];
   dat.c[3] = dat.c[4];
   dat.c[4] = temp;
   memcpy( data, &dat, 8 );
}

/* Do byte swapping on the given 4-byte integer. */
/* (Delegates to 'SwapInt32()' function.)        */
void SwapInt( int *data )
{
  SwapInt32( data );
}

void SwapDouble( EW_UNALIGNED double *data )
{
   char temp;

   union {
       char   c[8];
   } dat;

   memcpy( &dat, data, sizeof(double) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[7];
   dat.c[7] = temp;

   temp     = dat.c[1];
   dat.c[1] = dat.c[6];
   dat.c[6] = temp;

   temp     = dat.c[2];
   dat.c[2] = dat.c[5];
   dat.c[5] = temp;

   temp     = dat.c[3];
   dat.c[3] = dat.c[4];
   dat.c[4] = temp;
   memcpy( data, &dat, sizeof(double) );
   return;
}

void SwapShort( short *data )
{
   char temp;

   union {
      char  c[2];
   } dat;

   memcpy( &dat, data, sizeof(short) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[1];
   dat.c[1] = temp;
   memcpy( data, &dat, sizeof(short) );
   return;
}

/* Do byte swapping on the given 4-byte integer. */
void SwapInt32( int32_t *data )
{
   char temp;

   union {
      char c[4];
   } dat;

   memcpy( &dat, data, sizeof(int32_t) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( data, &dat, sizeof(int32_t) );
   return;
}

void SwapFloat( float *data )
{
   char temp;

   union {
      char c[4];
   } dat;

   memcpy( &dat, data, sizeof(float) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( data, &dat, sizeof(float) );
   return;
}


/************************ WaveMsgMakeLocal **************************
*       Byte-swap a universal TYPE_TRACEBUF message in place.       *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsgMakeLocal( TRACE_HEADER* wvmsg )
{
   return WaveMsgVersionMakeLocal((TRACE2X_HEADER *)wvmsg, '1');
}
/************************ WaveMsg2MakeLocal *************************
*       Byte-swap a universal TYPE_TRACEBUF2 message in place.      *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -1 if more than max number of samples for tbuf      *
*        size allowed  2000 or 1000 depending on data type          *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsg2MakeLocal( TRACE2_HEADER* wvmsg )
{
   return WaveMsgVersionMakeLocal((TRACE2X_HEADER *)wvmsg, wvmsg->version[0]);
}
/************************ WaveMsg2XMakeLocal *************************
*       Byte-swap a universal TYPE_TRACEBUF2X message in place.      *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -1 if more than max number of samples for tbuf      *
*        size allowed  2000 or 1000 depending on data type          *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsg2XMakeLocal( TRACE2X_HEADER* wvmsg )
{
   return WaveMsgVersionMakeLocal( wvmsg, wvmsg->version[0]);
}
static int WaveMsgVersionMakeLocal( TRACE2X_HEADER* wvmsg, char version )
{
   int    dataSize;  /* flag telling us how many bytes in the data */
   char   byteOrder;
   int*   intPtr;
   short* shortPtr;
   int    i;
   int    nsamp;
   double samprate,starttime,endtime;
   double tShouldEnd; 
   double dFudgeFactor;
   /* See what sort of data it carries
    **********************************/
   dataSize=0;
   if ( strcmp(wvmsg->datatype, "s4")==0)
   {
        dataSize=4; byteOrder='s';
   }
   else if ( strcmp(wvmsg->datatype, "i4")==0)
   {
        dataSize=4; byteOrder='i';
   }
   else if ( strcmp(wvmsg->datatype, "s2")==0)
   {
        dataSize=2; byteOrder='s';
   }
   else if ( strcmp(wvmsg->datatype, "i2")==0)
   {
        dataSize=2; byteOrder='i';
   }
   else
        return(-1); /* We don't know this message type*/
   /* SWAP the header (if neccessary) */
#if defined( _SPARC )
   if (byteOrder =='i')
   {
        /* swap the header
        *****************/
        SwapInt( &(wvmsg->pinno) );
        SwapInt( &(wvmsg->nsamp) );
        SwapDouble( &(wvmsg->starttime) );
        SwapDouble( &(wvmsg->endtime)   );
        SwapDouble( &(wvmsg->samprate)  );
        if (version == TRACE2_VERSION0) {
           switch (wvmsg->version[1]) {
           case TRACE2_VERSION11:
              SwapFloat( &(wvmsg->x.v21.conversion_factor) );
              break;
           }
        }
   }
#elif defined( _INTEL )
   if (byteOrder =='s')
   {
        /* swap the header
        *****************/
        SwapInt( &(wvmsg->pinno) );
        SwapInt( &(wvmsg->nsamp) );
        SwapDouble( &(wvmsg->starttime) );
        SwapDouble( &(wvmsg->endtime)   );
        SwapDouble( &(wvmsg->samprate)  );
        if (version == TRACE2_VERSION0) {
           switch (wvmsg->version[1]) {
           case TRACE2_VERSION11:
              SwapFloat( &(wvmsg->x.v21.conversion_factor) );
              break;
           }
        }
   }
#else
   printf( "WaveMsg2MakeLocal warning: _INTEL and _SPARC are both undefined." );
   return(-1);
#endif
  if (wvmsg->nsamp > 4032/dataSize) {
     logit("et","WaveMsg2MakeLocal: packet from %s.%s.%s.%s has bad number of samples=%d datatype=%s\n",
                wvmsg->sta, wvmsg->chan, wvmsg->net, wvmsg->loc, wvmsg->nsamp , wvmsg->datatype);
     return(-1);
  }
/* Perform a CheckSumish kind of calculation on the header
 * ensure that the tracebuf ends within 5 samples of the given endtime.
 * DK 2002/03/18
 *********************************************************************/
/* moved nsamp memcpy to here to avoid byte-alignment with next statement */
   memcpy( &nsamp,     &wvmsg->nsamp,     sizeof(int)    );
   memcpy( &samprate,  &wvmsg->samprate,  sizeof(double) );
   memcpy( &starttime, &wvmsg->starttime, sizeof(double) );
   memcpy( &endtime,   &wvmsg->endtime,   sizeof(double) );
   tShouldEnd   = starttime + ((nsamp - 1) / samprate);
   dFudgeFactor = 5.0 / samprate;
/* This is supposed to be a simple sanity check to ensure that the
 * endtime is within 5 samples of where it should be.  We're not
 * trying to be judgemental here, we're just trying to ensure that
 * we protect ourselves from complete garbage, so that we don't segfault
 * when allocating samples based on a bad nsamp
 ***********************************************************************/
   if( endtime < (tShouldEnd-dFudgeFactor) || 
       endtime > (tShouldEnd+dFudgeFactor)    )
   {
      logit("e","WaveMsg2MakeLocal: packet from %s.%s.%s.%s has inconsistent "
                "header values!\n", wvmsg->sta, wvmsg->chan, wvmsg->net, wvmsg->loc );
      logit("e","WaveMsg2MakeLocal: header.starttime  : %.4lf\n", starttime  );
      logit("e","WaveMsg2MakeLocal: header.samplerate : %.1lf\n", samprate   );
      logit("e","WaveMsg2MakeLocal: header.nsample    : %d\n",    nsamp      );
      logit("e","WaveMsg2MakeLocal: header.endtime    : %.4lf\n", endtime    );
      logit("e","WaveMsg2MakeLocal: computed.endtime  : %.4lf\n", tShouldEnd );
      logit("e","WaveMsg2MakeLocal: header.endtime is not within 5 sample intervals "
                "of computed.endtime!\n" );
      return(-2);
   }
 
   /* SWAP the data (if neccessary) */
#if defined( _SPARC )
   if (byteOrder =='i')
   {
   /* Swap the data.  */
        intPtr=(int*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        shortPtr=(short*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        for( i=0; i<nsamp; i++)
        {
                if(dataSize==2) SwapShort( &(shortPtr[i]) );
                if(dataSize==4) SwapInt(  &(intPtr[i])  );
        }
        /* Re-write the data type field in the message
        **********************************************/
        if(dataSize==2) strcpy(wvmsg->datatype,"s2");
        if(dataSize==4) strcpy(wvmsg->datatype,"s4");
   }
#elif defined( _INTEL )
   if (byteOrder =='s')
   {
   /* Swap the data.  */
        intPtr=(int*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        shortPtr=(short*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        for( i=0; i<nsamp; i++)
        {
                if(dataSize==2) SwapShort( &(shortPtr[i]) );
                if(dataSize==4) SwapInt(  &(intPtr[i])  );
        }
        /* Re-write the data type field in the message
        **********************************************/
        if(dataSize==2) strcpy(wvmsg->datatype,"i2");
        if(dataSize==4) strcpy(wvmsg->datatype,"i4");
   }
#else
   printf( "WaveMsg2MakeLocal warning: _INTEL and _SPARC are both undefined." );
#endif
   return(0);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

