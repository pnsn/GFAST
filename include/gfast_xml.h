#ifndef __XML_H__
#define __XML_H__

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
    
};

struct qmlOriginUncertainty_struct
{

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
    bool lhaveLatUncer;                    /*!< If true lonUncer is defined */
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
    struct qmlConfidenceEllipse_struct *ellipse; /*!< Error ellipsoid */
    struct qmlOriginUncertainty_struct *oriUnc;  /*!< Origin uncertainty */
    struct qmlLatitude_struct latitude;          /*!< Event latitude */
    struct qmlLongitude_struct longitude;        /*!< Event longitude */
    struct qmlDepth_struct depth;                /*!< Event depth */
    struct qmlTime_struct originTime;            /*!< Event origin time */
};

#ifdef __cplusplus
extern "C" {
#endif
/* Convert epochal time to string */
int xml_epoch2string(double epoch, char cepoch[128]);
/* Convert enumerated units to a string */
void __xml_units__enum2string(enum alert_units_enum enum_units,
                              char char_units[128]);
/* Convert string to enumerated units */
enum alert_units_enum
    __xml_units__string2enum(const char *char_units);
/* Extract core info from shake Alert XML message */
int GFAST_xml_coreInfo__read(void *xml_reader, double SA_NAN,
                             struct coreInfo_struct *core);
/* Write core info to shake Alert XML message */
int GFAST_xml_coreInfo__write(struct coreInfo_struct core,
                              void *xml_writer);
/* Write the focal mechanism */
int GFAST_xml_focalMechanism__write(const char *publicIDroot,
                                    const char *evid,
                                    const char *method,
                                    const double mt[6],
                                    void *xml_writer);
/* Write the moment tensor to XML message */
int GFAST_xml_momentTensor__write(const char *publicIDroot,
                                  const char *evid,
                                  const char *method,
                                  double M_use[6],
                                  double M0, 
                                  double dc_pct,
                                  double clvd_pct,
                                  void *xml_writer);
/* Write the nodal planes */
int GFAST_xml_nodalPlanes__write(const double np1[3],
                                 const double np2[3],
                                 void *xml_writer);
/* Write the principal axes */
int GFAST_xml_principalAxes__write(const double taxis[3],
                                   const double paxis[3],
                                   const double naxis[3],
                                   void *xml_writer);
/* Write a segment */
int GFAST_xml_segment__write(enum xml_segmentShape_enum shape,
                             double *lats, enum alert_units_enum lat_units,
                             double *lons, enum alert_units_enum lon_units,
                             double *depths, enum alert_units_enum depth_units,
                             double ss, enum alert_units_enum ss_units,
                             double ds, enum alert_units_enum ds_units,
                             double ss_unc, enum alert_units_enum ss_unc_units,
                             double ds_unc, enum alert_units_enum ds_unc_units,
                             void *xml_writer);
/* Read slip */
int GFAST_xml_slip__read(void *xml_reader, double VTX_NAN,
                         double *ss, double *ss_uncer,
                         double *ds, double *ds_uncer);
/* Write slip */
int GFAST_xml_slip__write(double ss, enum alert_units_enum ss_units,
                          double ds, enum alert_units_enum ds_units,
                          double ss_uncer, enum alert_units_enum ss_uncer_units,
                          double ds_uncer, enum alert_units_enum ds_uncer_units,
                          void *xml_writer);
/* Write a (moment) tensor */
int GFAST_xml_tensor__write(double Mrr, double Mtt, double Mpp,
                            double Mrt, double Mrp, double Mtp,
                            void *xml_writer);
/* Read vertices */
int GFAST_xml_vertices__read(void *xml_reader,
                             enum xml_segmentShape_enum shape,
                             double VTX_NAN,
                             double *__restrict__ lat,
                             double *__restrict__ lon,
                             double *__restrict__ depth);
/* Write vertices */
int GFAST_xml_vertices__write(enum xml_segmentShape_enum shape,
                              double *lats, enum alert_units_enum lat_units,
                              double *lons, enum alert_units_enum lon_units,
                              double *depths, enum alert_units_enum depth_units,
                              void *xml_writer);
/* Read vertex */
int GFAST_xml_vertex__read(void *xml_reader, double VTX_NAN,
                           double *lat, double *lon, double *depth);
/* Write a vertex */
int GFAST_xml_vertex__write(double lat, enum alert_units_enum lat_units,
                            double lon, enum alert_units_enum lon_units,
                            double depth, enum alert_units_enum depth_units,
                            void *xml_writer);
#ifdef __cplusplus
}
#endif
#endif /* __GFAST_XML_H__ */

