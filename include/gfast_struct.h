#include <stdbool.h>
#include <limits.h>
#include "gfast_enum.h"

#ifndef __GFAST_STRUCT_H__
#define __GFAST_STRUCT_H__

struct GFAST_pgd_props_struct
{
    double window_vel;    /*!< Velocity (km/s) used in determining if enough
                               data has arrived at a station in PGD
                               inversion */
    double dist_tol;      /*!< Source-receiver distance tolerance (km) */
    double disp_def;      /*!< If the source receiver epicentral distance
                               is less than dist_tol this is the value assigned
                               to the PGD observation (cm) */
    int min_sites;        /*!< Minimum number of sites required to
                               proceed with PGD inversion */
    int verbose;          /*!< Controls verbosity - errors will always
                                be output.
                               = 1 -> Output generic information.
                               = 2 -> Output generic information and
                                      warnings.
                               = 3 -> Output generic information,
                                      warnings, and debug information
                                      and debug information. */
    int utm_zone;         /*!< UTM zone.  If this is -12345 then will 
                               extract the UTM zone from the event
                               origin. */
    int ngridSearch_deps; /*!< Number of depths in PGD grid-search */
};

struct GFAST_cmt_props_struct
{
    double window_vel;    /*!< Velocity (km/s) used in determining if S wave
                               has passed through the station in CMT
                               inversion */
    double window_avg;    /*!< Amount of time (s) required after S wave
                               has passed through for averaging
                               the offset */
    int min_sites;        /*!< Minimum number of sites required to
                               proceed with CMT inversion */
    int verbose;          /*!< Controls verbosity - errors will always
                                be output.
                               = 1 -> Output generic information.
                               = 2 -> Output generic information and
                                      warnings.
                               = 3 -> Output generic information,
                                      warnings, and debug information
                                      and debug information. */
    int utm_zone;         /*!< UTM zone.  If this is -12345 then will 
                               extract the UTM zone from the event
                               origin. */
    int ngridSearch_deps; /*!< Number of depths in CMT grid-search */
    bool ldeviatoric;     /*!< If true then the CMT inversion is 
                               constrained to purely deviatoric sources.
                               Otherwise, all 6 moment tensor terms
                               are inverted for. */
};

struct GFAST_ff_props_struct
{
    double window_vel;   /*!< Velocity (km/s) used in determining if S wave
                              has passed through the station in FF 
                              inversion */
    double window_avg;   /*!< Amount of time (s) required after S wave
                              has passed through for averaging
                              the offset */
    double flen_pct;     /*!< Fault length safety factor */
    double fwid_pct;     /*!< Fault width safety factor */
    int verbose;         /*!< Controls verbosity - errors will always
                              be output.
                              = 1 -> Output generic information.
                              = 2 -> Output generic information and
                                     warnings.
                              = 3 -> Output generic information,
                                     warnings, and debug information
                                     and debug information. */
    int utm_zone;        /*!< UTM zone.  If this is -12345 then will 
                              extract the UTM zone from the event
                              origin. */
    int min_sites;       /*!< Minimum number of sites to proceed with
                              FF estimation */
    int nstr;            /*!< Number of fault patches along strike */
    int ndip;            /*!< Number of fault patches down dip */
    int nfp;             /*!< Number of fault planes considered in
                              inversion (should be 2) */
};

struct GFAST_props_struct
{
    struct GFAST_pgd_props_struct
           pgd_props;           /*!< PGD properties structure */
    struct GFAST_cmt_props_struct
           cmt_props;           /*!< CMT properties structure */
    struct GFAST_ff_props_struct
           ff_props;            /*!< FF properties structure */
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
    char syndata_dir[PATH_MAX]; /*!< Synthetic data directory */
    char syndata_pre[PATH_MAX]; /*!< Synthetic data prefix (LX) */
    char h5ArchiveDir[PATH_MAX];/*!< HDF5 archive directory */ 
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
    double processingTime;      /*!< Max processing time (s) after origin time
                                     which GFAST module will declare an event 
                                     done. This must be less than bufflen
                                     less than or equal to the synthetic
                                     runtime */
    double eqDefaultDepth;      /*!< Default earthquake depth (km) to be applied
                                     to shakeAlert structure */
    double synthetic_runtime;   /*!< Simulation runtime (s) for offline mode */
    int AMQport;                /*!< ActiveMQ port to access ElarmS messages 
                                    (61620). */
    int RMQport;                /*!< RabbitMQ port to access processed GPS
                                     positions (5672) */
    int utm_zone;               /*!< UTM zone.  If this is -12345 then will 
                                     extract the UTM zone from the event
                                     origin. */
    int verbose;                /*!< Controls verbosity - errors will always
                                     be output.
                                      = 1 -> Output generic information.
                                      = 2 -> Output generic information and
                                             warnings.
                                      = 3 -> Output generic information,
                                             warnings, and debug information
                                             and debug information. */
    enum opmode_type opmode;    /*!< GFAST operation mode (realtime, 
                                     playback, offline) */
    enum dtinit_type dt_init;   /*!< Defines how to initialize GPS sampling
                                     period */
    enum locinit_type loc_init; /*!< Defines how to initialize GPS locations */
};


struct GFAST_faultPlane_struct
{
    double *lon_vtx;    /*!< For plotting this defines the longitude of each
                             vertex (degrees) on each fault patch.  The ifp'th
                             fault patch is accessed by 4*(idip*nstr + istr).
                             The size is [4 x nstr x ndip] */
    double *lat_vtx;    /*!< For plotting this defines the latitude of each
                             vertex (degrees) on each fault patch.  The ifp'th
                             fault patch is accessed by 4*(idip*nstr + istr).
                             The size is [4 x nstr x ndip] */
    double *dep_vtx;    /*!< For plotting this defines the depth of each
                             vertex (km) on each fault patch.  The ifp'th
                             fault patch is accessed by 4*(idip*nstr + istr).
                             The size is [4 x nstr x ndip] */
    double *fault_xutm; /*!< Fault easting UTM (m).  The size is
                             [nstr x ndip] with leading dimension nstr */
    double *fault_yutm; /*!< Fault northing UTM (m).  The size is
                             [nstr x ndip] with leading dimension nstr */
    double *fault_alt;  /*!< Depth at fault patch (km) The size is 
                             [nstr x ndip] with leading dimension nstr */
    double *strike;     /*!< Strike angles of fault patches (degrees)
                             [nstr x ndip] with leading dimension nstr */
    double *dip;        /*!< Dip angles of fault patches (degrees).  The size
                             is [nstr x ndip] with leading dimension nstr */
    double *length;     /*!< Length of fault patches (m).  The size is
                             [nstr x ndip] with leading dimension nstr */
    double *width;      /*!< Width of fault patches (m).  The size is 
                             [nstr x ndip] with leading dimension nstr */
    double *sslip;      /*!< Strike-slip along each fault patch.  The size
                             is [nstr x ndip] with leading dimension nstr */
    double *dslip;      /*!< Dip-slip along each fault patch.  The size
                             is [nstr x ndip] with leading dimension nstr */
    double *sslip_unc;  /*!< Uncertainty in strike-slip on each fault patch.
                             The size is [nstr x ndip] with leading dimension
                             nstr. */
    double *dslip_unc;  /*!< Uncertainty in dip slip on each fault patch.
                             The size is [nstr x ndip] with leading dimension
                             nstr. */
    double *EN;         /*!< Modeled east displacements [nsites_used] */
    double *NN;         /*!< Modeled north displacements [nsites_used] */
    double *UN;         /*!< Modeled vertical displacements [nsites_used] */
    double *Einp;       /*!< Observed input east displacements [nsites_used] */
    double *Ninp;       /*!< Observed input north displacements [nsites_used] */
    double *Uinp;       /*!< Observed input vertical displacements
                             [nsites_used] */
    int *fault_ptr;     /*!< Maps from the ifp'th fault patch to the 
                             start index of in (lon_vtx, lat_vtx, dep_vtx)
                             [nstr*ndip + 1] */
    int maxobs;         /*!< Max number of allowable observations */
    int nsites_used;    /*!< Number of sites used in inversion */
    int nstr;           /*!< Number of fault patches along strike */
    int ndip;           /*!< Number of fault patches down dip */
};

struct GFAST_ffResults_struct
{
    struct GFAST_faultPlane_struct *fp;   /*!< Fault planes [nfp] */
    double *vr;                           /*!< Variance reduction on ifp'th
                                               fault plane [nfp] */
    double *Mw;                           /*!< Moment magnitude on ifp'th
                                               fault plane [nfp] */
    double *str;                          /*!< Strike of ifp'th fault
                                               (degrees) [nfp] */
    double *dip;                          /*!< Dip on ifp'th fault plane 
                                               (degrees) [nfp] */
    double SA_lat;                        /*!< Source latitude (degrees) [-90,90] */
    double SA_lon;                        /*!< Source longitude (degrees) [-180,360) */
    double SA_dep;                        /*!< Source depth (km) */
    double SA_mag;                        /*!< Source magnitude (Mw) */
    int preferred_fault_plane;            /*!< Preferred fault plane */
    int nfp;                              /*!< Number of fault planes */
};

struct GFAST_cmtResults_struct
{
    double *objfn;     /*!< Objective function at all depths */
    double *mts;       /*!< Holds the NED moment tensor terms packed
                            \f$ \{m_{xx}, m_{yy}, m_{zz},
                                  m_{xy}, m_{xz}, m_{yz} \} \f$.  [6*ndeps] */
    double *str1;      /*!< Strike (degrees) on first nodal plane for
                            all depths [ndeps]. \f$ \phi \in [0,360] \f$ */
    double *str2;      /*!< Strike (degrees) on second nodal plane for
                            all depths [ndeps]. \f$ \phi \in [0,360] \f$ */
    double *dip1;      /*!< Dip (degrees) on first nodal plane for
                            all depths [ndeps]. \f$ \delta \in [0,90] \f$ */
    double *dip2;      /*!< Dip (degrees) on second nodal plane for
                            all depths [ndeps]. \f$ \delta \in [0,90] \f$ */
    double *rak1;      /*!< Rake (degrees) on first nodal plane for
                            all depths [ndeps].
                            \f$ \lambda \in [-180,180] \f$ */
    double *rak2;      /*!< Rake (degrees) on second nodal plane for 
                            all depths [ndeps].
                            \f$ \lambda \in [-180,180] \f$ */
    double *Mw;        /*!< Moment magnitude for all depths [ndeps] */
    double *srcDepths; /*!< Source depths in moment tensor inversion grid
                            search (km) [ndeps] */
    double *EN;        /*!< Estimates on east component. The estimate
                            for the i'th site at the j'th depth is accessed by
                            j*sites+i [ndeps*nsites] */
    double *NN;        /*!< Estimates on north component.  The estimate
                            for the i'th site at the j'th depth is accessed by 
                            j*nsites+i [ndeps*nsites] */
    double *UN;        /*!< Estimates on vertical component.  The estimate
                            for the i'th site at the j'th depth is accessed by
                            j*nsites+i [ndeps*nsites] */
    double *Einp;      /*!< Observed input east displacements [nsites] */
    double *Ninp;      /*!< Observed input north displacements [nsites] */
    double *Uinp;      /*!< Observed input vertical displacements [nsites] */
    bool *lsiteUsed;   /*!< If true then the isite'th site from the
                            site list was used in the CMT estimation [nsite] */ 
    int opt_indx;      /*!< Optimal index in depth grid search [0, ndeps) */
    int ndeps;         /*!< Number of depths in grid search */
    int nsites;        /*!< Should equal GFAST_data_struct's stream_length */
};

struct GFAST_pgdResults_struct
{
    double *mpgd;      /*!< PGD magnitude at id'th depth [ndeps] */
    double *mpgd_vr;   /*!< PGD variance reduction at id'th depth [ndeps] */
    double *UP;        /*!< PGD estimates for each source depth
                            [nsites*ndeps] */
    double *UPinp;     /*!< PGD observations for each site [nsites] */
    double *srcDepths; /*!< PGD source depths in grid search (km) [ndeps] */
    double *iqr75_25;  /*!< 75-25 interquartile range of the weighted
                            residuals at each depth [ndeps] */
    bool *lsiteUsed;   /*!< If true then the isite'th site from the 
                            site list was used in the PGD estimation [nsites] */
    int ndeps;         /*!< Number of depths in PGD estimation */ 
    int nsites;        /*!< Should equal GFAST_data_struct's stream_length */
};

struct GFAST_peakDisplacementData_struct
{
    char **stnm;     /*!< Name of i'th site */
    double *pd;      /*!< Peak displacement (meters) observed at the i'th
                          site [nsites] */
    double *wt;      /*!< Data weight on the i'th peak displacement observation
                          [nsites] */
    double *sta_lat; /*!< Site latitude [-90,90] (degrees) */
    double *sta_lon; /*!< Site longitude [0,360] (degrees) */
    double *sta_alt; /*!< Site altitude (m) above sea-level */
    bool *lmask;     /*!< If true then mask the i'th site in this inversion
                          [nsites] */
    bool *lactive;   /*!< If true then the i'th site has data from the waveform
                          processor and can be an active participant in the
                          inversion [nsites] */
    int nsites;      /*!< Number of sites (should be consistent with
                          buffered data) */
};

struct GFAST_offsetData_struct
{
    char **stnm;      /*!< Name of i'th site [nsites x 64] */
    double *ubuff;    /*!< Offset (meters) in up component observed at the
                           i'th site [nsites] */
    double *nbuff;    /*!< Offset (meters) in north component observed at the
                           i'th site [nsites] */
    double *ebuff;    /*!< Offset (meters) in east component observed at the
                           i'th site [nsites] */
    double *wtu;      /*!< Data weight on the i'th up offset 
                           observation [nsites] */
    double *wtn;      /*!< Data weight on the i'th north offset 
                           observation [nsites] */
    double *wte;      /*!< Data weight on the i'th east offset 
                           observation [nsites] */
    double *sta_lat; /*!< Site latitude [-90,90] (degrees) */
    double *sta_lon; /*!< Site longitude [0,360] (degrees) */
    double *sta_alt; /*!< Site altitude (m) above sea-level */
    bool *lmask;     /*!< If true then mask the i'th site in this inversion
                          [nsites] */
    bool *lactive;   /*!< If true then the i'th site has data from the waveform
                          processor and can be an active participant in the
                          inversion [nsites] */
    int nsites;       /*!< Number of sites */
};


struct GFAST_waveformData_struct
{
    char site[64];    /*!< Name of site */
    double *ubuff;    /*!< Up precise-point position buffer (meters).  If any
                           sample is not known it should be a NAN. [maxpts] */
    double *nbuff;    /*!< North precise-point position buffer (meters).  If any
                           sample is not known it should be a NAN. [maxpts] */
    double *ebuff;    /*!< East precise-point position buffer (meters).  If any
                           sample is not known it should be a NAN. */
    double *tbuff;    /*!< Epochal time buffer (s) [maxpts] */ 
    double epoch;     /*!< Epoch time (seconds) corresponding to first sample 
                           of u, n, and e traces */
    double dt;        /*!< Sampling period (seconds). */
    double sta_lat;   /*!< Site latitude [-90,90] (degrees) */
    double sta_lon;   /*!< Site longitude [0,360] (degrees) */
    double sta_alt;   /*!< Site altitude (m) */
    int maxpts;       /*!< Max number of poitns in buffer.  This is 
                           computed from the site sampling period and
                           the GFAST_parm_struct's bufflen */
    int npts;         /*!< Number of points in time series.  This cannot
                           exceed maxpts */
    bool lskip_pgd;   /*!< True  -> This site is ignored during the PGD
                                    estimation.
                           False -> Will  attempt to use this site during
                                    the PGD estimation. */
    bool lskip_cmt;   /*!< True  -> this site is ignored during the CMT 
                                    inversion. 
                           False -> Will attempt to use this site during the
                                    CMT inversion */
    bool lskip_ff;    /*!< True  -> this site is ignored during the finite
                                    fault inversion
                           False -> Will attempt to use this during the 
                                    finite fault inversion */
};

struct GFAST_data_struct
{
    struct GFAST_waveformData_struct *data;  /*!< Collocated data structure
                                                  [stream_length] */
    int stream_length;                       /*!< Number of streams */
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

struct GFAST_activeEvents_struct
{
    struct GFAST_shakeAlert_struct *SA; /*<! Shake alert structure with 
                                             requisite event info [nev] */
    int nev;                            /*!< Number of events */ 
};

struct coreInfo_struct
{
    int version;                             /*!< Version number */
    char id[128];                            /*!< Event ID */
    double mag;                              /*!< Magnitude */
    enum alert_units_enum mag_units;         /*!< Magnitude units */
    double mag_uncer;                        /*!< Magnitude uncertainty */
    enum alert_units_enum mag_uncer_units;   /*!< Magnitude uncertainty units */    double lat;                              /*!< Event latitude */
    enum alert_units_enum lat_units;         /*!< Latitude units */
    double lat_uncer;                        /*!< Latitude uncertainty */
    enum alert_units_enum lat_uncer_units;   /*!< Latitude uncertainty units */
    double lon;                              /*!< Event longitude */
    enum alert_units_enum lon_units;         /*!< Longitude units */
    double lon_uncer;                        /*!< Longitude uncertainty */
    enum alert_units_enum lon_uncer_units;   /*!< Longitude uncertainty units */
    double depth;                            /*!< Event depth */
    enum alert_units_enum depth_units;       /*!< Event depth units */
    double depth_uncer;                      /*!< Depth uncertainty */
    enum alert_units_enum depth_uncer_units; /*!< Depth uncertainty units */
    double orig_time;                        /*!< UTC origin time */
    enum alert_units_enum orig_time_units;   /*!< Origin time units */
    double orig_time_uncer;                  /*!< Origin time uncertainty */
    enum alert_units_enum
         orig_time_uncer_units;              /*!< Origin time uncertainty
                                                  units */
    double likelihood;  /*!< TODO: I have no idea what likelihood means */
};

#endif /* __GFAST_STRUCT_H__ */
