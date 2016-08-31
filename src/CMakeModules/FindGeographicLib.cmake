# include(FindLibraryWithDebug)
if (GEOLIB_LIBRARIES)
  set(GEOLIB_FIND_QUIETLY TRUE)
endif (GEOLIB_LIBRARIES)
find_library(GEOLIB_LIBRARIES
  Geographic 
  HINTS /usr/lib/ /usr/lib64
  DOC "Library for geodetic computations"
)
find_file(GEOLIB_LIBRARIES
  libGegoraphic.a
  PATHS
  /usr/lib
  $ENV{GEOLIBDIR}/lib
  ${LIB_INSTALL_DIR}
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GEOLIB DEFAULT_MSG
                                  GEOLIB_LIBRARIES)
mark_as_advanced(GEOLIB_LIBRARIES)
