#ifndef __XML_H__
#define __XML_H__

#define XML_ENCODING "UTF-8" /*!< UTF encoding consistent with shakeAlert */

enum alert_units_enum
{
    UNKNOWN_UNITS = 0,    /*!< No units defined */
    DEGREES = 1,          /*!< Distance/location - degrees */
    KILOMETERS = 2,       /*!< Distance - kilometers */
    METERS = 3,           /*!< Distance - meters */
    SECONDS = 4,          /*!< Time - seconds */
    UTC = 5,              /*!< Time - UTC */
    MOMENT_MAGNITUDE = 6, /*!< Moment magnitude Mw */
    DYNE_CENTIMETERS = 7, /*!< Torque - dyne centimeters */
    NEWTON_METERS= 8,     /*!< Torque - Newton meters */
};

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

struct coreInfo_struct
{
    int version;                             /*!< Version number */
    char id[128];                            /*!< Event ID */
    double mag;                              /*!< Magnitude */
    enum alert_units_enum mag_units;         /*!< Magnitude units */
    double mag_uncer;                        /*!< Magnitude uncertainty */
    enum alert_units_enum mag_uncer_units;   /*!< Magnitude uncertainty units */
    double lat;                              /*!< Event latitude */
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
int GFAST_xml_tensor__write(double Mrr, enum alert_units_enum Mrr_units,
                            double Mtt, enum alert_units_enum Mtt_units,
                            double Mpp, enum alert_units_enum Mpp_units,
                            double Mrt, enum alert_units_enum Mrt_units,
                            double Mrp, enum alert_units_enum Mrp_units,
                            double Mtp, enum alert_units_enum Mtp_units,
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

