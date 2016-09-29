# - Find the native Earthworm includes and library
#

# This module defines
#  EW_INCLUDE_DIR, where to find ipp.h, etc.
#  EW_LIBRARY, the libraries to link against to use EW.
#  EW_DEFINITIONS - You should ADD_DEFINITONS(${EW_DEFINITIONS}) before compiling code that includes EW library files.
#  EW_FOUND, If false, do not try to use EW.

SET(EW_FOUND "NO")

FIND_PATH(EW_INCLUDE_DIR
    NAMES earthworm.h
    HINTS /usr/include/earthworm
)

SET(EW_NAMES ${EW_NAMES} libew.a swap.o)
FOREACH (EW_NAME ${EW_NAMES})
   FIND_LIBRARY(${EW_NAME}_LIBRARY
       NAMES ${EW_NAME} 
       PATHS /usr/lib64/ /usr/lib
   )
   SET(TMP_LIBRARY ${${EW_NAME}_LIBRARY})

   IF(TMP_LIBRARY)
      SET(EW_LIBRARY ${EW_LIBRARY} ${TMP_LIBRARY})
   ENDIF()
endforeach()

IF (EW_LIBRARY AND EW_INCLUDE_DIR)
    SET(EW_LIBRARY ${EW_LIBRARY})
    SET(EW_FOUND "YES")
ENDIF (EW_LIBRARY AND EW_INCLUDE_DIR)

IF (EW_FOUND)
    IF (NOT EW_FIND_QUIETLY)
        MESSAGE(STATUS "Found Earthworm: ${EW_LIBRARY}")
    ENDIF (NOT EW_FIND_QUIETLY)
    MARK_AS_ADVANCED(EW_INCLUDE_DIR EW_LIBRARY)
ELSE (EW_FOUND)
    IF (EW_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Earthworm library")
    ENDIF (EW_FIND_REQUIRED)
ENDIF (EW_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (EW DEFAULT_MSG EW_LIBRARY EW_INCLUDE_DIR)
mark_as_advanced(EW_INCLUDE_DIR EW_LIBRARY)
