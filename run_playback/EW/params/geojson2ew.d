#
# geojson2ew configuration file
#
# This code receives geoJSON records from an AMQP server, converts them into
# Earthworm trace buf messages, and stuffs them into a wave ring.
#

# EW Config:
# module id for this import
ModuleId    MOD_GEOJSON2EW
# transport ring to use for input/output
RingName    WAVE_RING
#RingName    GPS_RING
#   if 0, don't write logfile
#   if 1, do ???? stderr/stdout ????
#   if 2, log to module log but not stderr/stdout 
LogFile    2
# heartbeat interval in seconds - this should match the geojson2ew.desc heartbeat!
HeartbeatInt    10

# RabbitMQ server connection parameters
HOST         72.233.250.17      # IP address of RabbitMQ server
PORT         5672               # RabbitMQ port (default is 5672)
VHOST        /CWU-ppp           # RabbitMQ vhost the exchange is on
EXCHANGENAME eew                # Exchange name
EXCHANGETYPE fanout             # Exchange type
USERNAME     panga_ro           # Username for access to the exchange
PASSWORD     ro                 # Password for access to the exchange

#
# Each line below maps an Earthworm data element to part of a
# GeoJSON message from this RabbitMQ server exchange
#

# Record Parameter Mapping
MAP_SNCL       properties.SNCL
MAP_TIME       properties.time
MAP_SAMPLERATE properties.sampleRate

# Record Channel Mapping
#
# The geojson2ew plugin will convert data values to integers for Earthworm.
# Thus the original floating point values should be scaled up before they 
# are truncated. In this configuration, NEU positions and sigmas in meters
# are converted to microns by multiplying by 1e6 before being read into 
# Earthworm. The same multiplier is used for off-diagonal correlation 
# factors (unitless).
#
#   channelCode        jsonPath             multiplier      condition

# components (multiplier converts to microns)
MAP_CHAN E features[*].geometry.coordinates[1] 1e6 properties.coordinateType=NEU
MAP_CHAN N features[*].geometry.coordinates[0] 1e6 properties.coordinateType=NEU
MAP_CHAN Z features[*].geometry.coordinates[2] 1e6 properties.coordinateType=NEU
# component sigmas (microns)
MAP_CHAN 1 features[*].properties.EError 1e6
MAP_CHAN 2 features[*].properties.NError 1e6
MAP_CHAN 3 features[*].properties.UError 1e6
# off-diagonal correlations (unitless)
#MAP_CHAN 4 features[*].properties.ENCor 1e6
#MAP_CHAN 5 features[*].properties.EUCor 1e6
#MAP_CHAN 6 features[*].properties.UNCor 1e6
# quality -- already an integer, so multiplier left at 1
#MAP_CHAN Q features[*].properties.quality 1 
