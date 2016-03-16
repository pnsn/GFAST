#include <stdbool.h>
#include <limits.h>
#include "gfast_enum.h"

#ifndef __GFAST_STRUCT_H__
#define __GFAST_STRUCT_H__

struct GFAST_props_struct
{
    char streamfile[PATH_MAX];  /*!< File of streams to include in the
                                     processing. */
    char dtfile[PATH_MAX];      /*!< File with site names and sampling periods
                                     if initializing from file */
    char siteposfile[PATH_MAX]; /*!< File of all the site locations.  The
                                     format used is currently from the SOPAC
                                     SECTOR web service. */
    char eewsfile[PATH_MAX];    /*!< File to output the results of the ActiveMQ
                                     listener from the seismic warning file. */
    char eewgfile[PATH_MAX];    /*!< File to output the results of G-FAST */
    char syndriver[PATH_MAX];   /*!< The synthetic mode driver file. */
    char synoutput[PATH_MAX];   /*!< The synthetic mode output file. */
    char AMQhost[512];          /*!< ActiveMQ hostname to access ElarmS messages
                                     (siren). */
    char AMQtopic[512];         /*!< ActiveMQ topic to access ElarmS messages
                                     (eew.alg.elarms.data). */
    char AMQuser[512];          /*!< ActiveMQ username to access ElarmS
                                     messages */
    char AMQpassword[512];      /*!< ActiveMQ password to access ElarmS
                                     messages */
    char RMQhost[512];          /*!< RabbitMQ hostname to acceess processed GPS
                                     positions.  PANGA separates this into two 
                                     distinct processing groups, CWU-ppp and
                                     UNAVCO-rtx */
    char RMQtopic[512];         /*!< RabbitMQ topic to access processed GPS
                                     positions (www.panga.org) */
    char RMQuser[512];          /*!< RabbitMQ username to acess processed GPS
                                     positions */
    char RMQpassword[512];      /*!< RabbitMQ password to access processed GPS
                                     positions */
    char RMQexchange[512];      /*!< RabbitMQ exchange to access processed GPS
                                     positions (nev-cor) */
    double dt_default;          /*!< Default sampling period (s) for GPS
                                     stations */
    double bufflen;             /*!< The number of seconds to keep in the data
                                     buffers */
    int AMQport;                /*!< ActiveMQ port to access ElarmS messages 
                                    (61620). */
    int RMQport;                /*!< RabbitMQ port to access processed GPS
                                     positions (5672) */
    int utm_zone;               /*!< UTM zone.  If this is -12345 then will 
                                     extract the UTM zone from the event
                                     origin. */
    int verbose;                /*!< Controls verbosity.
                                      = 0 -> Output nothing.
                                      = 1 -> Output errors only.
                                      = 2 -> Output errors, generic
                                             information, and warnings.
                                      = 3 -> Output errors, generic information,
                                             and debug information. */
    enum opmode_type opmode;    /*!< GFAST operation mode (realtime, 
                                     playback, offline) */
    enum dtinit_type dt_init;   /*!< Defines how to initialize GPS sampling
                                     period */
    enum locinit_type loc_init; /*!< Defines how to initialize GPS locations */
};

struct GFAST_strongMotion_struct
{
    double *z;      /*!< Vertical channel [npts] */
    double *n;      /*!< North (radial, 2) channel [npts] */
    double *e;      /*!< East (transverse, 3) channel [npts] */
    int npts;       /*!< Number of points */
    double dt;      /*!< Sampling period (s) */
};

struct GFAST_collocatedData_struct
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
    int nstream;      /*!< Number of streams in GPS structure */
    bool lcollocated; /*!< True -> station is collocated with a strong
                                   motion station.
                           False -> station is not collocated with a strong
                                   motion station. */
    bool lskip_pgd;   /*!< True  -> This site is ignored during the PGD
                                    estimation.
                           False -> Will  attempt to use this site during
                                    the PGD estimation. */
    bool lskip_cmtff; /*!< True  -> this site is ignored during the CMT and
                                    finite fault inversion. 
                           False -> Will attempt to use this site during the
                                    CMT and finite fault inversion */
};

struct GFAST_data_struct
{
    struct GFAST_collocatedData_struct *data;  /*!< Collocated data structure
                                                    [stream_length] */
    int stream_length;                         /*!< Number of streams */
};

struct GFAST_shakeAlert_struct
{
    char eventid[128];  /*!< Event ID */
    double lat;         /*!< Event latitude (degrees) */
    double lon;         /*!< Event longitude (degrees) */
    double dep;         /*!< Event depth (kilometers) */ 
    double mag;         /*!< Event magnitude */
    double time;        /*!< Event epochal time (s) */
};

#endif /* __GFAST_STRUCT_H__ */
