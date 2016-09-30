#ifndef _gfast_xml_h__
#define _gfast_xml_h__ 1

#include "gfast_struct.h"
#include "gfast_enum.h"
#define XML_ENCODING "UTF-8" /*!< UTF encoding consistent with shakeAlert */

enum xml_segmentShape_enum
{
    LINE = 2,      /*!< segment shape is a line with 2 points */
    TRIANGLE = 3,  /*!< segment shape is a triangle with 3 points */
    RECTANGLE = 4  /*!< segment shape is a rectangle with 3 points */
};

struct qmlMT_struct
{
    char id[256];       /*!< Moment tensor ID */
    char agencyID[256]; /*!< AgencyID (e.g. PNSN) */
    double m[6];        /*!< Moment tensor in Up, South, East coordinates with
                             units Dyne-centimeters.  The moment tensor terms 
                             are ordered
                             \f$ \{ m_{rr}, m_{\theta \theta} m_{\phi \phi},
                                    m_{r \theta} m_{r \phi} m_{\theta \phi}
                                 \} \f$ */
    double np1[3];      /*!< Nodal plane 1 described by its strike, dip, and
                             rake respectively (degrees) */
    double np2[3];      /*!< Nodal plane 2 described by its strike, dip, and
                             rake respectively (degrees) */
    double taxis[3];    /*!< Tension axis (degrees), plunge (degrees),
                             and length (Dyne-cm) respectively */
    double naxis[3];    /*!< Null axis azimuth (degrees), plunge (degrees),
                             and length (Dyne-cm) respectively */
    double paxis[3];    /*!< Pressure axis azimuth (degrees), plunge (degrees),
                             and length (Dyne-cm) respectively */ 
    double M0;          /*!< Scalar moment (Dyne-cm) */
    int nstations;      /*!< Number of sites used in inversion */
    int ncomp;          /*!< Number of components used in inversion.  For
                             example if 2 three-component sites are used then
                             this is 6 and not 3. */
};

struct qmlConfidenceEllipse_struct
{
    int bogus; /* TODO fix me */
};

struct qmlOriginUncertainty_struct
{
    int bogus; /* TODO fix me */
};

struct qmlMagnitude_struct
{
    double magnitude;        /*!< Magnitude */
    double magUncer;         /*!< Magnitude uncertainty */
    char type[64];           /*!< Event type TODO: should be an enum */
    bool lhaveMag;           /*!< If true then have the magnitude */
    bool lhaveMagUncer;      /*!< Magnitude uncertainty */
    bool lhaveType;          /*!< If true then have the magnitude type */
};

struct qmlTime_struct
{
    double time;                           /*!< Time */
    double timeUncer;                      /*!< Time uncertainty */
    double confidenceLevel;                /*!< Confidence level */
    enum alert_units_enum time_units;      /*!< Time units */
    enum alert_units_enum timeUncer_units; /*!< Time uncertainty units */
    bool lhaveTime;                        /*!< If true then time is defined */
    bool lhaveTimeUncer;                   /*!< If true timeUncer is defined */
    bool lhaveConfidence;                  /*!< If true confidenceLevel
                                                is defined */
};

struct qmlLatitude_struct
{
    double latitude;                       /*!< Event latitude */
    double latUncer;                       /*!< Latitude uncertainty */
    double confidenceLevel;                /*!< Confidence level */
    enum alert_units_enum latitude_units;  /*!< Latitude units */
    enum alert_units_enum latUncer_units;  /*!< Latitude uncertainty units */
    bool lhaveLat;                         /*!< If true then latitude is
                                                defined */
    bool lhaveLatUncer;                    /*!< If true latUncer is defined */
    bool lhaveConfidence;                  /*!< If true confidenceLevel
                                                is defined */
};

struct qmlLongitude_struct
{
    double longitude;                      /*!< Event longitude */
    double lonUncer;                       /*!< Longitude uncertainty */
    double confidenceLevel;                /*!< Confidence level */
    enum alert_units_enum longitude_units; /*!< Latitude units */
    enum alert_units_enum lonUncer_units;  /*!< Longitude uncertainty units */
    bool lhaveLon;                         /*!< If true then longitude is
                                                defined */
    bool lhaveLonUncer;                    /*!< If true lonUncer is defined */
    bool lhaveConfidence;                   /*!< If true confidenceLevel
                                                 is defined */
};

struct qmlDepth_struct
{
    double depth;                           /*!< Event depth */
    double depthUncer;                      /*!< Depth uncertainty */
    double confidenceLevel;                 /*!< Confidence level */
    enum alert_units_enum depth_units;      /*!< Depth units */
    enum alert_units_enum depthUncer_units; /*!< Depth uncertainty units */
    bool lhaveDepth;                        /*!< If true then depth is
                                                defined */
    bool lhaveDepthUncer;                   /*!< If true depthUncer is
                                                 defined */
    bool lhaveConfidence;                   /*!< If true confidenceLevel
                                                 is defined */
};

struct qmlOrigin_struct
{
    struct qmlConfidenceEllipse_struct ellipse;  /*!< Error ellipsoid */
    struct qmlOriginUncertainty_struct oriUnc;   /*!< Origin uncertainty */
    struct qmlLatitude_struct latitude;          /*!< Event latitude */
    struct qmlLongitude_struct longitude;        /*!< Event longitude */
    struct qmlDepth_struct depth;                /*!< Event depth */
    struct qmlTime_struct originTime;            /*!< Event origin time */
    bool lhaveEllipse;     /*!< If true then there is a confidence ellipse */
    bool lhaveOriUnc;      /*!< If true then there is an origin uncertainty */
    bool lhaveLatitude;    /*!< If true then there is a latitude */
    bool lhaveLongitude;   /*!< If true then there is a lognitude */
    bool lhaveDepth;       /*!< If true then there is a depth */ 
    bool lhaveOriginTime;  /*!< If true then there is an origin time */
};

#ifdef __cplusplus
extern "C" {
#endif
/* Convert epochal time to string */
int xml_epoch2string(const double epoch, char cepoch[128]);
/* Convert enumerated units to a string */
void __xml_units__enum2string(const enum alert_units_enum enum_units,
                              char char_units[128]);
/* Convert string to enumerated units */
enum alert_units_enum
    __xml_units__string2enum(const char *char_units);
/* Write quakeML depth */
int xml_quakeML_writeDepth(const double depth,
                           const enum alert_units_enum depth_units,
                           const bool lhaveDepth,
                           const double depthUncer,
                           const enum alert_units_enum depthUncer_units,
                           const bool lhaveDepthUncer,
                           const double confidence,
                           const bool lhaveConfidence,
                           void *xml_writer);
/* Write quakeML latitude */
int xml_quakeML_writeLatitude(const double latitude,
                              const enum alert_units_enum lat_units,
                              const bool lhaveLat,
                              const double latUncer,
                              const enum alert_units_enum latUncer_units,
                              const bool lhaveLatUncer,
                              const double confidence,
                              const bool lhaveConfidence,
                              void *xml_writer);
/* Write quakeML longitude */
int xml_quakeML_writeLongitude(const double longitude,
                               const enum alert_units_enum lon_units,
                               const bool lhaveLon,
                               const double lonUncer,
                               const enum alert_units_enum lonUncer_units,
                               const bool lhaveLonUncer,
                               const double confidence,
                               const bool lhaveConfidence,
                               void *xml_writer);
/* Write magnitude */
int xml_quakeML_writeMagnitude(const double magnitude,
                               const bool lhaveMag,
                               const double magUncer,
                               const bool lhaveMagUncer,
                               const char *type,
                               const bool lhaveType,
                               void *xml_writer);
/* Write the focal mechanism */
int xml_quakeML_writeFocalMechanism(const char *publicIDroot,
                                    const char *evid,
                                    const char *method,
                                    const double mt[6],
                                    void *xml_writer);
/* Write the moment tensor to XML message */
int xml_quakeML_writeMomentTensor(const char *publicIDroot,
                                  const char *evid,
                                  const char *method,
                                  const double M_use[6],
                                  const double M0, 
                                  const double dc_pct,
                                  const double clvd_pct,
                                  void *xml_writer);
/* Write the nodal planes */
int xml_quakeML_writeNodalPlanes(const double np1[3],
                                 const double np2[3],
                                 void *xml_writer);
/* Write the origin */
int xml_quakeML_writeOrigin(const char *publicIDroot,
                            const char *evid,
                            const char *method,
                            struct qmlOrigin_struct origin,
                            void *xml_writer);
/* Write the principal axes */
int xml_quakeML_writePrincipalAxes(const double taxis[3],
                                   const double paxis[3],
                                   const double naxis[3],
                                   void *xml_writer);
/* Write a (moment) tensor */
int xml_quakeML_writeTensor(const double Mrr, const double Mtt,
                            const double Mpp, const double Mrt,
                            const double Mrp, const double Mtp,
                            void *xml_writer);
/* Write (origin) time */
int xml_quakeML_writeTime(const double time,
                          const enum alert_units_enum time_units,
                          const bool lhaveTime,
                          const double timeUncer,
                          const enum alert_units_enum timeUncer_units,
                          const bool lhaveTimeUncer,
                          const double confidence,
                          const bool lhaveConfidence,
                          void *xml_writer);
//----------------------------------------------------------------------------//
//                                shakeAlert                                  //
//----------------------------------------------------------------------------//
/* Read shakeAlert coreInfo */
int xml_shakeAlert_readCoreInfo(void *xml_reader,
                                const double SA_NAN,
                                struct coreInfo_struct *core);
/* Write shakeAlert coreInfo */
int xml_shakeAlert_writeCoreInfo(const struct coreInfo_struct core,
                                 void *xml_writer);
/* Write a shakeAlert finite fault segment */
int xml_shakeAlert_writeSegment(const enum xml_segmentShape_enum shape,
                                const double *lats,
                                const enum alert_units_enum lat_units,
                                const double *lons,
                                const enum alert_units_enum lon_units,
                                const double *depths,
                                const enum alert_units_enum depth_units,
                                const double ss, 
                                const enum alert_units_enum ss_units,
                                const double ds, 
                                const enum alert_units_enum ds_units,
                                const double ss_uncer,
                                const enum alert_units_enum ss_uncer_units,
                                const double ds_uncer,
                                const enum alert_units_enum ds_uncer_units,
                                void *xml_writer);
/* Read slip */
int xml_shakeAlert_readSlip(void *xml_reader, const double VTX_NAN,
                            double *ss, double *ss_uncer,
                            double *ds, double *ds_uncer);
/* Write slip */
int xml_shakeAlert_writeSlip(const double ss,
                             const enum alert_units_enum ss_units,
                             const double ds,
                             const enum alert_units_enum ds_units,
                             const double ss_uncer,
                             const enum alert_units_enum ss_uncer_units,
                             const double ds_uncer,
                             const enum alert_units_enum ds_uncer_units,
                             void *xml_writer);
/* Read shakeAlert finite fault vertices */
int xml_shakeAlert_readVertices(void *xml_reader,
                                const enum xml_segmentShape_enum shape,
                                const double VTX_NAN,
                                double *__restrict__ lat,
                                double *__restrict__ lon,
                                double *__restrict__ depth);
/* Write shakeAlert finite fault vertices */
int xml_shakeAlert_writeVertices(const enum xml_segmentShape_enum shape,
                                 const double *lats,
                                 const enum alert_units_enum lat_units,
                                 const double *lons,
                                 const enum alert_units_enum lon_units,
                                 const double *depths,
                                 const enum alert_units_enum depth_units,
                                 void *xml_writer);
/* Read vertex */
int xml_shakeAlert_readVertex(void *xml_reader, const double VTX_NAN,
                              double *lat, double *lon, double *depth);
/* Write a vertex */
int xml_shakeAlert_writeVertex(const double lat,
                               const enum alert_units_enum lat_units,
                               const double lon,
                               const enum alert_units_enum lon_units,
                               const double depth,
                               const enum alert_units_enum depth_units,
                               void *xml_writer);
#define GFAST_xml_quakeML_writeDepth(...)       \
              xml_quakeML_writeDepth(__VA_ARGS__)
#define GFAST_xml_quakeML_writeLatitude(...)       \
              xml_quakeML_writeLatitude(__VA_ARGS__)
#define GFAST_xml_quakeML_writeLongitude(...)       \
              xml_quakeML_writeLongitude(__VA_ARGS__)
#define GFAST_xml_quakeML_writeFocalMechanism(...)       \
              xml_quakeML_writeFocalMechanism(__VA_ARGS__)
#define GFAST_xml_quakeML_writeMagnitude(...)       \
              xml_quakeML_writeMagnitude(__VA_ARGS__)
#define GFAST_xml_quakeML_writeMomentTensor(...)       \
              xml_quakeML_writeMomentTensor(__VA_ARGS__)
#define GFAST_xml_quakeML_writeNodalPlanes(...)       \
              xml_quakeML_writeNodalPlanes(__VA_ARGS__) 
#define GFAST_xml_quakeML_writeOrigin(...)       \
              xml_quakeML_writeOrigin(__VA_ARGS__)
#define GFAST_xml_quakeML_writePrincipalAxes(...)       \
              xml_quakeML_writePrincipalAxes(__VA_ARGS__)
#define GFAST_xml_quakeML_writeTensor(...)       \
              xml_quakeML_writeTensor(__VA_ARGS__)
#define GFAST_xml_quakeML_writeTime(...)       \
              xml_quakeML_writeTime(__VA_ARGS__)


#define GFAST_xml_shakeAlert_readCoreInfo(...)       \
              xml_shakeAlert_readCoreInfo(__VA_ARGS__)
#define GFAST_xml_shakeAlert_writeCoreInfo(...)       \
              xml_shakeAlert_writeCoreInfo(__VA_ARGS__)
#define GFAST_xml_shakeAlert_writeSegment(...)       \
              xml_shakeAlert_writeSegment(__VA_ARGS__)
#define GFAST_xml_shakeAlert_readSlip(...)       \
              xml_shakeAlert_readSlip(__VA_ARGS__)
#define GFAST_xml_shakeAlert_writeSlip(...)       \
              xml_shakeAlert_writeSlip(__VA_ARGS__)
#define GFAST_xml_shakeAlert_readVertex(...)       \
	      xml_shakeAlert_readVertex(__VA_ARGS__)
#define GFAST_xml_shakeAlert_writeVertex(...)       \
              xml_shakeAlert_writeVertex(__VA_ARGS__)
#define GFAST_xml_shakeAlert_readVertices(...)       \
              xml_shakeAlert_readVertices(__VA_ARGS__)
#define GFAST_xml_shakeAlert_writeVertices(...)       \
              xml_shakeAlert_writeVertices(__VA_ARGS__)
#ifdef __cplusplus
}
#endif
#endif /* _gfast_xml_h__ */

