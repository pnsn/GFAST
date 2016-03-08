#ifndef __GFAST_STRUCT_H__
#define __GFAST_STRUCT_H__
//#ifdef __cplusplus
//extern "C"
//{
//#endif

struct GFAST_parms_struct
{
    int utm_zone;   // UTM zone
                    // if this is -12345 then will extract the UTM zone
                    // from the event origin
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
