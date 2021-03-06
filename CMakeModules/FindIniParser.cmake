# - Find the native INIPARSER includes and library
#

# This module defines
#  INIPARSER_INCLUDE_DIR, where to find iniparser.h, etc.
#  INIPARSER_LIBRARY, the libraries to link against to use INIPARSER.
#  INIPARSER_DEFINITIONS - You should ADD_DEFINITONS(${INIPARSER_DEFINITIONS}) before compiling code that includes INIPARSER library files.
#  INIPARSER_FOUND, If false, do not try to use INIPARSER.

SET(INIPARSER_FOUND "NO")

FIND_PATH(INIPARSER_INCLUDE_DIR
    NAMES iniparser.h
    HINTS /usr/include /usr/local/include
)

SET(INIPARSER_NAMES ${INIPARSER_NAMES} libiniparser.a)
FOREACH (INIPARSER_NAME ${INIPARSER_NAMES})
   FIND_LIBRARY(${INIPARSER_NAME}_LIBRARY
       NAMES ${INIPARSER_NAME} 
       PATHS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64
   )
   SET(TMP_LIBRARY ${${INIPARSER_NAME}_LIBRARY})

   IF(TMP_LIBRARY)
      SET(INIPARSER_LIBRARY ${INIPARSER_LIBRARY} ${TMP_LIBRARY})
   ENDIF()
endforeach()

#FIND_LIBRARY(INIPARSER_LIBRARY
#    /opt/intel/ipp/lib/intel64/libipps.a
#    /opt/intel/ipp/lib/intel64/libippvm.a
#    /opt/intel/ipp/lib/intel64/libippcore.a
#    /usr/lib/
#    NAMES ${INIPARSER_NAMES}
#)

IF (INIPARSER_LIBRARY AND INIPARSER_INCLUDE_DIR)
    SET(INIPARSER_LIBRARY ${INIPARSER_LIBRARY})
    SET(INIPARSER_FOUND "YES")
ENDIF (INIPARSER_LIBRARY AND INIPARSER_INCLUDE_DIR)

IF (INIPARSER_FOUND)
    IF (NOT INIPARSER_FIND_QUIETLY)
	MESSAGE(STATUS "Found IniParser: ${INIPARSER_LIBRARY}")
    ENDIF (NOT INIPARSER_FIND_QUIETLY)
    MARK_AS_ADVANCED(INIPARSER_INCLUDE_DIR INIPARSER_LIBRARY INIPARSER_LIBRARY)
ELSE (INIPARSER_FOUND)
    IF (INIPARSER_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find INIPARSER library")
    ENDIF (INIPARSER_FIND_REQUIRED)
ENDIF (INIPARSER_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (INIPARSER DEFAULT_MSG INIPARSER_LIBRARY INIPARSER_INCLUDE_DIR)
mark_as_advanced(INIPARSER_INCLUDE_DIR INIPARSER_LIBRARY)
