#include "GFASTMessage.h"

/** Default Constructor.
 */
GFASTMessage::GFASTMessage(void) {
  system_name = GFAST;
  algorithm_version = "-";
  program_instance = "";
  reference_src = "dm";
  originating_system = "";
  event_id="-1";
  magnitude=-9.9;
  magnitude_uncertainty=-9.9;
  latitude=-999.9;
  latitude_uncertainty=-999.9;
  longitude=-999.9;
  longitude_uncertainty=-999.9;
  depth=-9.9;
  depth_uncertainty=-9.9;
  origin_time = -99999.99;
  origin_time_uncertainty=-9.9;
  likelyhood=-9.9;
  message_type = NEW;
  version = 0;
  message_category = LIVE;
  timestamp = "";
  number_stations = 0;
  reference_id = "-1";
  magnitude_units = "Mw";
  magnitude_uncertainty_units = "Mw";
  latitude_units = "deg";
  latitude_uncertainty_units = "deg";
  longitude_units = "deg";
  longitude_uncertainty_units = "deg";
  depth_units = "km";
  depth_uncertainty_units = "km";
  origin_time_units = "UTC";
  origin_time_uncertainty_units = "sec";
}

GFASTMessage::~GFASTMessage() {}

void GFASTMessage::setData(const coreInfo_struct *eventdat) {
  return;
}
