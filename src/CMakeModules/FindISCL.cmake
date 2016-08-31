# include(FindLibraryWithDebug)
if (ISCL_INCLUDE_DIR AND ISCL_LIBRARIES)
  set(ISCL_FIND_QUIETLY TRUE)
endif (ISCL_INCLUDE_DIR AND ISCL_LIBRARIES)
find_path(ISCL_INCLUDE_DIR
  PATHS
  $ENV{ISCLDIR}/include
  ${INCLUDE_INSTALL_DIR}
)
find_library(ISCL_LIBRARIES
  iscl
  PATHS
  $ENV{ISCLDIR}/lib
  ${LIB_INSTALL_DIR}
)
find_file(ISCL_LIBRARIES
  libiscl_static.a
  PATHS
  /usr/lib
  /usr/lib/lib64
  $ENV{ISCLDIR}/lib
  ${LIB_INSTALL_DIR}
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ISCL DEFAULT_MSG
                                  ISCL_INCLUDE_DIR ISCL_LIBRARIES)
mark_as_advanced(ISCL_INCLUDE_DIR ISCL_LIBRARIES)
