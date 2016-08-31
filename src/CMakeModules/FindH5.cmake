# include(FindLibraryWithDebug)
if (H5_C_INCLUDE_DIR AND H5_LIBRARIES)
  set(H5_FIND_QUIETLY TRUE)
endif (H5_C_INCLUDE_DIR AND H5_LIBRARIES)
find_path(H5_C_INCLUDE_DIR
  NAMES hdf5.h
  HINTS /usr/include $ENV{H5DIR}/include ${INCLUDE_INSTALL_DIR}
)
find_library(H5_LIBRARIES
  NAMES hdf5
  HINTS /usr/lib /usr/lib64 $ENV{H5DIR}/lib ${LIB_INSTALL_DIR}
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(H5 DEFAULT_MSG
                                  H5_C_INCLUDE_DIR H5_LIBRARIES)
mark_as_advanced(H5_C_INCLUDE_DIR H5_LIBRARIES)
