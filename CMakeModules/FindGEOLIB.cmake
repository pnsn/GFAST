# include(FindLibraryWithDebug)
if (NOT GEOLIB_LIBRARY)
  if (GEOLIB_LIBRARY)
    set(GEOLIB_FIND_QUIETLY TRUE)
  endif (GEOLIB_LIBRARY)
  find_library(GEOLIB_LIBRARY
    NAMES Geographic libGeographic.a
    HINTS /usr/lib/ /usr/lib64 $ENV{GEOLIBDIR}/lib ${LIB_INSTALL_DIR}
    DOC "Library for geodetic computations"
  )
  find_file(GEOLIB_LIBRARY
    HINTS Geographic
    PATHS /usr/lib $ENV{GEOLIBDIR}/lib
  )
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GEOLIB DEFAULT_MSG GEOLIB_LIBRARY)
mark_as_advanced(GEOLIB_LIBRARY)
