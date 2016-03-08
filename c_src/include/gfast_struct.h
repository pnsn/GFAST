#ifndef __GFAST_STRUCT_H__
#define __GFAST_STRUCT_H__
//#ifdef __cplusplus
//extern "C"
//{
//#endif

struct GFAST_parms_struct
{
    char streamfile[512];  /*!< File of streams to include in the system */
    char siteposfile[512]; /*!< File of all the site locations.  The
                                format used is currently from the SOPAC
                                SECTOR web service */
    char eewfile[512];     /*!< File to output the results of the ActiveMQ
                                listener from the seismic warning file */
    char syndriver[512];   /*!< The synthetic mode driver file */
    char synoutput[512];   /*!< The synthetic mode output file */
    char AMQhost[512];     /*!< ActiveMQ hostname to access ElarmsS messages 
                               (siren) */
    int bufflen;           /*!< The number of epochs to keep in the data
                                buffer */
    int AMQport;           /*!< ActiveMQ Port to access ElarmS messages 
                                (61620) */
    bool symmode;          /*!< False -> real-time mode (default) 
                                True  -> synthetic mode */
    int utm_zone;          /*!< UTM zone.  If this is -12345 then will extract
                                the UTM zone from the event origin */
    int verbose;           /*!< = 0 -> output nothing
                                = 1 -> output errors only
                                = 2 -> output errors and generic information
                                = 3 -> output errors, generic information, and 
                                       debug information */
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
    double *u;      // Vertical displacement (m?) [npts]
    double *n;      // North displacement (m?) [npts]
    double *e;      // East displacement (m?) [npts]
    int npts;       // Number of points in time series
    double dt;      // Sampling period (s)
};

//#ifdef __cplusplus
//}
//#endif
#endif /* __GFAST_STRUCT_H__ */
