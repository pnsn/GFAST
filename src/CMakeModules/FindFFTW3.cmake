# - Find the native FFTW3 includes and library
#

# This module defines
#  FFTW3_LIBRARIES, the libraries to link against to use fftw3.
#  FFTW3_DEFINITIONS - You should ADD_DEFINITONS(${FFTW3_DEFINITIONS}) before compiling code that includes fftw3 library files.
#  FFTW3_FOUND, If false, do not try to use fftw3.

SET(FFTW3_FOUND "NO")

SET(FFTW3_NAMES ${FFTW3_NAMES} fftw3)
FIND_LIBRARY(FFTW3_LIBRARIES
    NAMES ${FFTW3_NAMES}
)

IF (FFTW3_LIBRARIES)
    SET(FFTW3_LIBRARIES ${FFTW3_LIBRARIES})
    SET(FFTW3_FOUND "YES")
ENDIF (FFTW3_LIBRARIES)

IF (FFTW3_FOUND)
    IF (NOT FFTW3_FIND_QUIETLY)
	MESSAGE(STATUS "Found fftw3: ${FFTW3_LIBRARIES}")
    ENDIF (NOT FFTW3_FIND_QUIETLY)
    MARK_AS_ADVANCED(FFTW3_LIBRARIES FFTW3_LIBRARIES)
ELSE (FFTW3_FOUND)
    IF (FFTW3_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find fftw3 library")
    ENDIF (FFTW3_FIND_REQUIRED)
ENDIF (FFTW3_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW3 DEFAULT_MSG FFTW3_LIBRARIES)
mark_as_advanced(FFTW3_LIBRARIES)