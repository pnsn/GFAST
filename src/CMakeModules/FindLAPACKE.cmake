# include(FindLibraryWithDebug)
if (LAPACKE_INCLUDE_DIR AND LAPACKE_LIBRARIES AND LAPACK_LIBRARIES)
  set(LAPACKE_FIND_QUIETLY TRUE)
endif (LAPACKE_INCLUDE_DIR AND LAPACKE_LIBRARIES AND LAPACK_LIBRARIES
find_path(LAPACKE_INCLUDE_DIR
  NAMES
  lapacke.h
  PATHS
  $ENV{LAPACKEDIR}/include
  ${INCLUDE_INSTALL_DIR}
)
find_library(LAPACKE_LIBRARIES
  lapacke 
  PATHS /usr/lib64/ /usr/lib $ENV{LAPACKEDIR}/lib ${LIB_INSTALL_DIR}
)
find_file(LAPACKE_LIBRARIES
  liblapacke.so.3
  liblapacke.so
  liblapacke.a
  PATHS /usr/lib64 /usr/lib $ENV{LAPACKEDIR}/lib ${LIB_INSTALL_DIR}
)
find_library(LAPACK_LIBRARIES)
(
  lapack
  PATHS /usr/lib64/ /usr/lib $ENV{LAPACKEDIR}/lib ${LIB_INSTALL_DIR}
)
find_file(LAPACK_LIBRARIES
  liblapack.so.3
  liblapack.so
  liblapack.a
  PATHS /usr/lib64 /usr/lib $ENV{LAPACKEDIR}/lib ${LIB_INSTALL_DIR}
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LAPACKE DEFAULT_MSG
                                  LAPACKE_INCLUDE_DIR LAPACKE_LIBRARIES LAPACK_LIBRARIES)
mark_as_advanced(LAPACKE_INCLUDE_DIR LAPACKE_LIBRARIES LAPACK_LIBRARIES)
