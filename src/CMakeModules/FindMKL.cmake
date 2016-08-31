# - Find the native MKL includes and library
#

# This module defines
#  MKL_LIBRARIES, the libraries to link against to use MKL.
#  MKL_DEFINITIONS - You should ADD_DEFINITONS(${MKL_DEFINITIONS}) before compiling code that includes MKL library files.
#  MKL_FOUND, If false, do not try to use MKL.

SET(MKL_FOUND "NO")

SET(MKL_NAMES ${MKL_NAMES} libmkl_intel_lp64.a libmkl_core.a libmkl_sequential.a)
FOREACH (MKL_NAME ${MKL_NAMES})
   FIND_LIBRARY(${MKL_NAME}_LIBRARY
       NAMES ${MKL_NAME} 
       PATHS
       /opt/intel/mkl/lib/intel64
   )
   SET(TMP_LIBRARY ${${MKL_NAME}_LIBRARY})

   IF(TMP_LIBRARY)
      SET(MKL_LIBRARIES ${MKL_LIBRARIES} ${TMP_LIBRARY})
   ENDIF()
endforeach()

IF (MKL_LIBRARIES)
    SET(MKL_LIBRARIES ${MKL_LIBRARIES})
    SET(MKL_FOUND "YES")
ENDIF (MKL_LIBRARIES)

IF (MKL_FOUND)
    IF (NOT MKL_FIND_QUIETLY)
	MESSAGE(STATUS "Found MKL: ${MKL_LIBRARIES}")
    ENDIF (NOT MKL_FIND_QUIETLY)
    MARK_AS_ADVANCED(MKL_LIBRARIES MKL_LIBRARIES)
ELSE (MKL_FOUND)
    IF (MKL_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find MKL library")
    ENDIF (MKL_FIND_REQUIRED)
ENDIF (MKL_FOUND)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (MKL DEFAULT_MSG MKL_LIBRARIES)
mark_as_advanced(MKL_LIBRARIES)
