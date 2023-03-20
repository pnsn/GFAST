# include(FindLibraryWithDebug)
if (H5_C_INCLUDE_DIR AND H5_LIBRARY)
  set(H5_FIND_QUIETLY TRUE)
endif (H5_C_INCLUDE_DIR AND H5_LIBRARY)
find_path(H5_C_INCLUDE_DIR
  NAMES hdf5.h
  HINTS /usr/include /usr/local/include $ENV{H5DIR}/include
)
find_library(H5_LIBRARY
  NAMES hdf5
  HINTS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 $ENV{H5DIR}/lib
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(H5 DEFAULT_MSG
                                  H5_C_INCLUDE_DIR H5_LIBRARY)
mark_as_advanced(H5_C_INCLUDE_DIR H5_LIBRARY)
