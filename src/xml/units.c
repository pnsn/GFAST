#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_xml.h"

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
void __xml_units__enum2string(enum alert_units_enum enum_units,
                              char char_units[128])
{
    const char *fcnm = "__alert_units__units2string\0";
    memset(char_units, 0, 128);
    if (enum_units == DEGREES)
    {
        strcpy(char_units, "deg\0");
    }
    else if (enum_units == KILOMETERS)
    {
        strcpy(char_units, "km\0");
    }
    else if (enum_units == METERS)
    {
        strcpy(char_units, "meters\0");
    }
    else if (enum_units == SECONDS)
    {
        strcpy(char_units, "sec\0");
    }
    else if (enum_units == UTC)
    {
        strcpy(char_units, "UTC\0");
    }
    else if (enum_units == MOMENT_MAGNITUDE)
    {
        strcpy(char_units, "Mw\0");
    }
    else if (enum_units == DYNE_CENTIMETERS)
    {
        strcpy(char_units, "dyne_cm\0");
    }
    else if (enum_units == NEWTON_METERS)
    {
        strcpy(char_units, "newton_meters\0");
    }
    else
    {
        if (enum_units != UNKNOWN_UNITS)
        {
            printf("%s: Warning unknown unit %d\n", fcnm, enum_units);
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
enum alert_units_enum
    __xml_units__string2enum(const char *char_units)
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
    // Time - Seconds
    else if (strcasecmp(char_units, "sec\0") == 0 ||
             strcasecmp(char_units, "seconds\0") == 0){
        enum_units = SECONDS;
    }
    // Time - UTC
    else if (strcasecmp(char_units, "UTC\0") == 0){
        enum_units = UTC;
    }
    // No idea
    else
    {
        printf("%s: Unknown units: %s\n", fcnm, char_units);
    }
    return enum_units;
}
