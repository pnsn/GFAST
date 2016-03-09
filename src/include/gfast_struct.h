#include <stdbool.h>

#ifndef __GFAST_STRUCT_H__
#define __GFAST_STRUCT_H__

struct GFAST_props_struct
{
    char streamfile[512];  /*!< File of streams to include in the system. */
    char siteposfile[512]; /*!< File of all the site locations.  The
                                format used is currently from the SOPAC
                                SECTOR web service. */
    char eewsfile[512];    /*!< File to output the results of the ActiveMQ
                                listener from the seismic warning file. */
    char eewgfile[512];    /*!< File to output the results of G-FAST */ 
    char syndriver[512];   /*!< The synthetic mode driver file. */
    char synoutput[512];   /*!< The synthetic mode output file. */
    char AMQhost[512];     /*!< ActiveMQ hostname to access ElarmS messages 
                               (siren). */
    char AMQtopic[512];    /*!< ActiveMQ topic to access ElarmS messages
                                (eew.alg.elarms.data). */
    char AMQuser[512];     /*!< ActiveMQ username to access ElarmS messages */
    char AMQpassword[512]; /*!< ActiveMQ password to access ElarmS messages */
    char RMQhost[512];     /*!< RabbitMQ hostname to acceess processed GPS
                                positions.  PANGA separates this into two 
                                distinct processing groups, CWU-ppp and
                                UNAVCO-rtx */
    char RMQtopic[512];    /*!< RabbitMQ topic to access processed GPS
                                positions (www.panga.org) */
    char RMQuser[512];     /*!< RabbitMQ username to acess processed GPS
                                positions */
    char RMQpassword[512]; /*!< RabbitMQ password to access processed GPS
                                positions */
    char RMQexchange[512]; /*!< RabbitMQ exchange to access processed GPS
                                positions (nev-cor) */
    int bufflen;           /*!< The number of epochs to keep in the data
                                buffer. */
    int AMQport;           /*!< ActiveMQ port to access ElarmS messages 
                                (61620). */
    int RMQport;           /*!< RabbitMQ port to access processed GPS
                                positions (5672) */
    bool synmode;          /*!< False -> real-time mode (default).
                                True  -> synthetic mode. */
    int utm_zone;          /*!< UTM zone.  If this is -12345 then will extract
                                the UTM zone from the event origin. */
    int verbose;           /*!< = 0 -> Output nothing.
                                = 1 -> Output errors only.
                                = 2 -> Output errors and generic information.
                                = 3 -> Output errors, generic information, and 
                                       debug information. */
};

struct GFAST_strongMotion_struct
{
    double *z;      // Vertical channel [npts]
    double *n;      // North (radial, 2) channel [npts]
    double *e;      // East (transverse, 3) channel [npts]
    int npts;       // Number of points
    double dt;      // Sampling period (s)
};

struct GFAST_gps_struct
{
    struct GFAST_strongMotion_struct sm;  /*!< Collocated strong motion data
                                               structure */ 
    char site[64];    /*!< Name of site */
    double *ubuff;    /*!< Up position buffer (m?).  If any sample is 
                           not known it should be a NAN. [npts] */
    double *nbuff;    /*!< North position buffer (m?) [npts].  If any sample
                           is not known it should be a NAN. */
    double *ebuff;    /*!< East position buffer (m?) [npts].  If any sample
                           is not known it should be a NAN. */
    double *tbuff;    /*!< Epochal time buffer (s) [npts] */ 
    double epoch;     /*!< Epoch time (seconds) corresponding to first sample 
                           of u, n, and e traces */
    double dt;        /*!< Sampling period (seconds). */
    double sta_lat;   /*!< Station latitude [-90,90] (degrees) */
    double sta_lon;   /*!< Station longitude [0,360] (degrees) */
    int npts;         /*!< Number of points in time series.  This should be 
                           equivalent GFAST_parms_struct's bufflen */
    bool lcollocated; /*!< True -> station is collocated with a strong
                                   motion station.
                           False -> station is not collocated with a strong
                                   motion station. */
};

#endif /* __GFAST_STRUCT_H__ */
