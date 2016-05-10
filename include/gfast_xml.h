#ifndef __XML_H__
#define __XML_H__

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
    NEWTON_METERS= 8,     /*!< Toqrue - Newton meters */
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
/* Convert enumerated units to a string */
void __xml_units__enum2string(enum alert_units_enum enum_units,
                              char char_units[128]);
/* Convert string to enumerated units */
enum alert_units_enum
    __xml_units__string2enum(const char *char_units);
/* Extract core info from shake Alert XML message */
int GFAST_xml_read__SACoreInfo(const char *message, double SA_NAN,
                               struct coreInfo_struct *core);
#ifdef __cplusplus
}
#endif
#endif /* __GFAST_XML_H__ */

