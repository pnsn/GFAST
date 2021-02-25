#ifndef ISCL_TIME_H__
#define ISCL_TIME_H__ 1

typedef unsigned long long timerTimeStampType;

enum time_UTCGPS_enum
{
    UTC_TIME = 0,    /*! Time is given in UTC.  If expressed as epochal time
                         then this is the time since January 1 1970. */
    GPS_TIME = 1     /*! Time is given in GPS time If expressed in seconds
                         this is the time since January 6, 1980. */
};

/*!
 * @defgroup time Time
 * @brief Time converters and program timing utilities.
 * @copyright ISTI distributed under the Apache 2 license.
 */

#ifdef __cplusplus
extern "C" 
{
#endif
  
/* Convert calendar to epochal time */
double time_calendar2epoch(const int nzyear, const int nzjday, const int nzhour,
                           const int nzmin, const int nzsec, const int nzmusec);
double time_calendar2epoch2(const int nzyear, const int month, const int dom,
                            const int nzhour, const int nzmin, const int nzsec,
                            const int nzmusec);
/* Convert epochal to calendar time */
enum isclError_enum
    time_epoch2calendar(const double epoch,
                        int *nzyear, int *nzjday, int *month, int *mday,
                        int *nzhour, int *nzmin, int *nzsec, int *nzmusec);
/* Epochal time to GPS time (Jan 6 1980) */
double time_epoch2gps(double epoch, enum isclError_enum *ierr);
/* GPS time (Jan 6 1980) to epochal time */
double time_gps2epoch(double gps, enum isclError_enum *ierr);
/* Get the number of leapseconds */
int time_leapseconds(double time, enum time_UTCGPS_enum type);
/* Returns the current time - useful for timing functions */
double time_timeStamp(void);
/* Matlab'eqsue timing utility */
void time_tic(void);
double time_toc(void);
  /*
#define ISCL_time_calendar2epoch(...)  time_calendar2epoch(__VA_ARGS__)
#define ISCL_time_calendar2epoch2(...) time_calendar2epoch2(__VA_ARGS__)
#define ISCL_time_epoch2calendar(...)  time_epoch2calendar(__VA_ARGS__)
#define ISCL_time_epoch2gps(...)       time_epoch2gps(__VA_ARGS__)
#define ISCL_time_gps2epoch(...)       time_gps2epoch(__VA_ARGS__)
#define ISCL_time_leapseconds(...)     time_leapseconds(__VA_ARGS__)
#define ISCL_time_timeStamp(...)       time_timeStamp(__VA_ARGS__)
#define ISCL_time_tic(...)             time_tic(__VA_ARGS__)
#define ISCL_time_toc(...)             time_toc(__VA_ARGS__)
  */
#ifdef __cplusplus
}
#endif
#endif /* _iscl_time_h__ */
