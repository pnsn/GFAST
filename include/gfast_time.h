#include <stdarg.h>
#include <time.h>

#ifndef __TIME_H__
#define __TIME_H__

typedef unsigned long long timerTimeStampType;

#ifdef __cplusplus
extern "C"
{
#endif
/* Convert calendar to epochal time */
double time_calendar2epoch(int nzyear, int nzjday, int nzhour,
                           int nzmin, int nzsec, int nzmusec);
double time_calendar2epoch2(int nzyear, int month, int dom, int nzhour,
                            int nzmin, int nzsec, int nzmusec);
/* Convert epochal to calendar time */
int time_epoch2calendar(double epoch,
                        int *nzyear, int *nzjday, int *month, int *mday,
                        int *nzhour, int *nzmin, int *nzsec, int *nzmusec);
/* Get the UTC epochal time right now */
double time_currentTime(void);
/* Get time stampe - useful for timing a function */
double time_timeStamp(void);
/* Begins and ends clock for matlab esque tictoc */
void time_tic(void);
double time_toc(void);
#ifdef __cplusplus
}
#endif
#endif /* __TIME_H__ */
