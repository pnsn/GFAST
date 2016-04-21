#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum alert_units_enum
{
    UNKNOWN_UNITS = 0,    /*!< No units defined */ 
    DEGREES = 1,          /*!< Distance/location - degrees */
    KILOMETERS = 2,       /*!< Distance - kilometers */
    METERS = 3,           /*!< Distance - meters */
    SECONDS = 4,          /*!< Time - seconds */
    MOMENT_MAGNITUDE = 5, /*!< Moment magnitude Mw */
    DYNE_CENTIMETERS = 6, /*!< Torque - dyne centimeters */
    NEWTON_METERS= 7,     /*!< Toqrue - Newton meters */
};

struct alert_struct
{
    int version;                             /*!< Version number */
    double mag;                              /*!< Magnitude */
    enum alert_units_enum mag_units;         /*!< Magnitude units */
    double mag_uncer;                        /*!< Magnitude uncertainty */
    enum alert_units_enum mag_uncer_units;   /*!< Magnitude uncertainty units */
    double lat;                              /*!< Event latitude */
    enum alert_units_enum lat_units;         /*!< Latitude units */
    double lat_uncer;                        /*!< Latitude uncertainty */
    enum alert_units_enum lat_units_units;   /*!< Latitude uncertainty units */ 
    double lon;                              /*!< Event longitude */
    enum alert_units_enum lon_units;         /*!< Longitude units */
    double lon_uncer;                        /*!< Longitude uncertainty */
    enum alert_units_enum lon_uncer_units;   /*!< Longitude uncertainty units */
    double depth;                            /*!< Event depth */
    enum alert_units_enum depth_units;       /*!< Event depth units */
    double depth_uncer;                      /*!< Depth uncertainty */
    enum alert_units_enum depth_uncer_units; /*!< Depth uncertainty units */
    double otime;                            /*!< UTC origin time */
    enum alert_units_enum otime_units;       /*!< Origin time units */
    double otime_uncer;                      /*!< Origin time uncertainty */
    enum alert_units_enum otime_uncer_units; /*!< Origin time uncertainty
                                                  units */
    double likelihood;  /*!< TODO: I have no idea what likelihood means */
};

//============================================================================//
/*!
 * @brief Converts the enumerated units a string 
 *
 * @param[in] enum_units      enumerated units defined in alert_units_enum
 *
 * @param[out] char_units     corresponding string representation of the
 *                            enum_units
 *
 * @author Ben Baker, ISTI
 *
 */
void __alert_units__units2string(enum alert_units_enum enum_units,
                                 char char_units[256])
{
    const char *fcnm = "__alert_units__units2string\0";
    memset(char_units, 0, sizeof(char_units));
    if (enum_units == DEGREES){
        strcpy(char_units, "deg\0");
    }else if (enum_units == KILOMETERS){
        strcpy(char_units, "km\0");
    }else if (enum_units == METERS){
        strcpy(char_units, "meters\0");
    }else if (enum_units == SECONDS){
        strcpy(char_units, "seconds\0");
    }else if (enum_units == MOMENT_MAGNITUDE){
        strcpy(char_units, "Mw\0");
    }else if (enum_units == DYNE_CENTIMETERS){
        strcpy(char_units, "dyne_cm\0"); 
    }else if (enum_units == NEWTON_METERS){
        strcpy(char_units, "newton_meters\0");
    }else{
        if (enum_units != UNKNOWN_UNITS){
            //log_warnF("%s: Unknown unit %d\n", fcnm, enum_units); 
        }   
    }   
    return;
}
//============================================================================//
/*!
 * @brief Converts string units to enumerated units
 *
 * @param[in] char_units     string represenation of units to convert 
 *                           to enumerated units
 *
 * @result enumerated unit representation fo char_units
 *
 * @author Ben Baker, ISTI
 *
 */
enum alert_units_enum __alert_units__string2enum(const char *char_units)
{
    const char *fcnm = "__alert_units__string2enum\0";
    enum alert_units_enum enum_units = UNKNOWN_UNITS;
    // Not defined
    if (char_units == NULL){return enum_units;}
    if (strlen(char_units) == 0){return enum_units;}
    // Degrees
    if (strcasecmp(char_units, "deg\0")     == 0 ||
        strcasecmp(char_units, "degrees\0") == 0)
    {
        enum_units = DEGREES; 
    }
    // Kilometers
    else if (strcasecmp(char_units, "km\0")          == 0 ||
             strcasecmp(char_units, "kilometers\0") == 0)
    {
        enum_units = KILOMETERS; 
    }
    // Meters
    else if (strcasecmp(char_units, "meters\0") == 0)
    {
        enum_units = METERS;
    }
    // Moment-magnitude
    else if (strcasecmp(char_units, "Mw\0") == 0)
    {
        enum_units = MOMENT_MAGNITUDE;
    }
    // Dyne centimeters
    else if (strcasecmp(char_units, "dyne_cm\0") == 0 ||
             strcasecmp(char_units, "dyne_centimeters\0") == 0)
    {
        enum_units = DYNE_CENTIMETERS;
    } 
    // Newton meters
    else if (strcasecmp(char_units, "newton_meters\0") == 0 ||
             strcasecmp(char_units, "Nm\0") == 0)
    {   
        enum_units = NEWTON_METERS;
    }
    // Seconds
    else if (strcasecmp(char_units, "s\0") == 0 ||
             strcasecmp(char_units, "seconds\0") == 0){
        enum_units = SECONDS;
    }
    // No idea
    else
    {
        //log_warnF("%s: Unknown units: %s\n", fcnm, char_units);
    }
    return enum_units; 
}
//============================================================================//
/*!
 */
void GFAST_alertReader(char *msg)
{
    return;
}
