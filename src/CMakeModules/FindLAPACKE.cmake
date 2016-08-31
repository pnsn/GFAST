# include(FindLibraryWithDebug)
if (LAPACKE_INCLUDES AND LAPACKE_LIBRARIES)
  set(LAPACKE_FIND_QUIETLY TRUE)
endif (LAPACKE_INCLUDES AND LAPACKE_LIBRARIES)
find_path(LAPACKE_INCLUDES
  NAMES
  lapacke.h
  PATHS
  $ENV{LAPACKEDIR}/include
  ${INCLUDE_INSTALL_DIR}
)
find_library(LAPACKE_LIBRARIES
  lapacke 
  lapack
  PATHS
  $ENV{LAPACKEDIR}/lib
  ${LIB_INSTALL_DIR}
)
find_file(LAPACKE_LIBRARIES
  liblapacke.so.3
  liblapack.so
  liblapack.a
  PATHS
  /usr/lib
  $ENV{LAPACKEDIR}/lib
  ${LIB_INSTALL_DIR}
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LAPACKE DEFAULT_MSG
                                  LAPACKE_INCLUDES LAPACKE_LIBRARIES)
mark_as_advanced(LAPACKE_INCLUDES LAPACKE_LIBRARIES)
