# include(FindLibraryWithDebug)
#if (NOT COMPEARTH_INCLUDE_DIR AND COMPEARTH_LIBRARY)
  if (COMPEARTH_INCLUDE_DIR AND COMPEARTH_LIBRARY)
    set(COMPEARTH_FIND_QUIETLY TRUE)
  endif (COMPEARTH_INCLUDE_DIR AND COMPEARTH_LIBRARY)
  find_path(COMPEARTH_INCLUDE_DIR
    NAMES compearth.h
    HINTS /usr/include /usr/local/include $ENV{COMPEARTH_DIR}/include
  )
  find_library(COMPEARTH_LIBRARY
    NAMES compearth compearth_shared compearth_static
    HINTS /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64 $ENV{COMPEARTH_DIR}/lib
  )
  if (COMPEARTH_INCLUDE_DIR)
    message("Found COMPEARTH include")
  endif (COMPEARTH_INCLUDE_DIR)
  if (COMPEARTH_LIBRARY)
    message("Found COMPEARTH library")
  endif (COMPEARTH_LIBRARY)
  include(FindPackageHandleStandardArgs)
#endif()
find_package_handle_standard_args(COMPEARTH DEFAULT_MSG
                                  COMPEARTH_INCLUDE_DIR COMPEARTH_LIBRARY)

mark_as_advanced(COMPEARTH_INCLUDE_DIR COMPEARTH_LIBRARY)
