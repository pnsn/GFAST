# - Find the native IPP includes and library
#

# This module defines
#  IPP_INCLUDE_DIR, where to find ipp.h, etc.
#  IPP_LIBRARY, the libraries to link against to use IPP.
#  IPP_DEFINITIONS - You should ADD_DEFINITONS(${IPP_DEFINITIONS}) before compiling code that includes IPP library files.
#  IPP_FOUND, If false, do not try to use IPP.

SET(IPP_FOUND "NO")

FIND_PATH(IPP_INCLUDE_DIR
    NAMES ipps.h
    HINTS /opt/intel/ipp/include /usr/include /usr/ipp/include /usr/local/include /usr/local/include/ipp
)

SET(IPP_NAMES ${IPP_NAMES} libipps.a libippvm.a libippcore.a)
FOREACH (IPP_NAME ${IPP_NAMES})
   FIND_LIBRARY(${IPP_NAME}_LIBRARY
       NAMES ${IPP_NAME} 
       PATHS /opt/intel/ipp/lib/intel64
   )
   SET(TMP_LIBRARY ${${IPP_NAME}_LIBRARY})

   IF(TMP_LIBRARY)
      SET(IPP_LIBRARY ${IPP_LIBRARY} ${TMP_LIBRARY})
   ENDIF()
endforeach()

#FIND_LIBRARY(IPP_LIBRARY
#    /opt/intel/ipp/lib/intel64/libipps.a
#    /opt/intel/ipp/lib/intel64/libippvm.a
#    /opt/intel/ipp/lib/intel64/libippcore.a
#    /usr/lib/
#    NAMES ${IPP_NAMES}
#)

#IF (IPP_LIBRARY AND IPP_INCLUDE_DIR)
IF (IPP_LIBRARY)
    SET(IPP_LIBRARY ${IPP_LIBRARY})
    SET(IPP_FOUND "YES")
ENDIF(IPP_LIBRARY)
#ENDIF (IPP_LIBRARY AND IPP_INCLUDE_DIR)

IF (IPP_FOUND)
    IF (NOT IPP_FIND_QUIETLY)
	MESSAGE(STATUS "Found IPP: ${IPP_LIBRARY}")
    ENDIF (NOT IPP_FIND_QUIETLY)
    MARK_AS_ADVANCED(IPP_INCLUDE_DIR IPP_LIBRARY IPP_LIBRARY)
ELSE (IPP_FOUND)
    IF (IPP_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find IPP library")
    ENDIF (IPP_FIND_REQUIRED)
ENDIF (IPP_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (IPP DEFAULT_MSG IPP_LIBRARY)
mark_as_advanced(IPP_LIBRARY)
#find_package_handle_standard_args (IPP DEFAULT_MSG IPP_LIBRARY IPP_INCLUDE_DIR)
#mark_as_advanced(IPP_INCLUDE_DIR IPP_LIBRARY)
