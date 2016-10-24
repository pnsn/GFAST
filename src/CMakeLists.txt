cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(GFAST C CXX)
enable_testing()

# Add modules for finding requisites
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
# For testing
if (USE_INTEL)
  MESSAGE("Using Intel Libraries")
  find_package(MKL REQUIRED)
  find_package(IPP REQUIRED)
  find_path(CBLAS_INCLUDE_DIR
            NAMES cblas.h
            HINTS /usr/include /usr/local/include $ENV{CBLASDIR}/include
  )
  find_path(LAPACKE_INCLUDE_DIR
            NAMES lapacke.h
            HINTS /usr/include /usr/local/include $ENV{LAPACKEDIR}/include 
  )
else()
  find_package(CBLAS REQUIRED)
  find_package(LAPACKE REQUIRED)
  find_package(FFTW3 REQUIRED)
endif (USE_INTEL)
if (USE_AMQ)
  MESSAGE("Will build GFAST ActiveMQ library")
  find_package(AMQ REQUIRED)
endif (USE_AMQ)
if (USE_EW)
  MESSAGE("Will use Earthworm")
  find_package(EW REQUIRED)
endif (USE_EW)
find_package(IniParser REQUIRED)
find_package(ISCL REQUIRED)
find_package(GEOLIB REQUIRED)
find_package(H5 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(XML2 REQUIRED)

# Bring in headers
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  cmopad
  ${CBLAS_INCLUDE_DIR}
  ${LAPACKE_INCLUDE_DIR}
  ${CBLAS_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${H5_C_INCLUDE_DIR}
  ${INIPARSER_INCLUDE_DIR}
  ${ISCL_INCLUDE_DIR}
)

#macro for adding sources
set(SRCS "")
macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else (_src ${ARGN})
            list (APPEND SRCS "${_src}")
        endif (_relPath)
    endforeach (_src ${ARGN})
    #propagate up to parent
    if (_relPath)
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif (_relPath)
endmacro (add_sources)

add_subdirectory(include)
add_subdirectory(cmopad)
add_subdirectory(core)
add_subdirectory(eewUtils)
add_subdirectory(events)
add_subdirectory(traceBuffer)
add_subdirectory(xml)
add_subdirectory(hdf5)

if (USE_AMQ)
  include_directories(${LIBAMQ_INCLUDE_DIR} ${APR_INCLUDE_DIR})
  add_subdirectory(activeMQ)
endif (USE_AMQ)

if (USE_EW)
  set(GFAST_USE_EW ON)
  add_definitions(-DUSE_EW=1)
  include_directories(${EW_INCLUDE_DIR})
else()
  set(GFAST_USE_EW OFF)
endif (USE_EW)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EW_BUILD_FLAGS}" CACHE STRING "" FORCE)

add_library(gfast_core_shared SHARED ${SRCS_CORE})
add_library(gfast_core_static STATIC ${SRCS_CORE})
add_library(gfast_xml_shared SHARED ${SRCS_XML})
add_library(gfast_xml_static STATIC ${SRCS_XML})
add_library(cmopad_shared SHARED ${SRCS_CMOPAD})
#add_library(cmopad_static STATIC ${SRCS_CMOPAD}) # Need a license agreement from pnsn
#add_library(gfast_traceBuffer SHARED ${SRCS_ 
if (USE_AMQ)
  add_library(gfast_activeMQ_shared SHARED ${SRCS_AMQ})
  #add_library(gfast_activeMQ_static STATIC ${SRCS_AMQ})
endif (USE_AMQ)
add_library(gfast_hdf5_shared SHARED ${SRCS_HDF5})
add_library(gfast_hdf5_static STATIC ${SRCS_HDF5})

add_executable(gfast_playback gfast_playback.c ${SRCS_EEW} ${SRCS_H5TB})
add_executable(gfast_eew gfast_eew.c ${SRCS_EEW} ${SRCS_H5TB})

add_executable(unit_tests/xcmopad cmopad/unit_tests.c ${SRCS_CMOPAD}) 

if (USE_INTEL)
  message(${ISCL_LIBRARY})
  if (USE_EW)
    set (LIB_BASE ${ISCL_LIBARARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                  ${IPP_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY} ${INIPARSER_LIBRARY}
                  ${LIBXML2_LIBRARY} ${EW_LIBRARY} -lm -ldl -lpthread)
  else()
    set (LIB_BASE ${ISCL_LIBARARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                  ${IPP_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY} ${INIPARSER_LIBRARY}
                  ${LIBXML2_LIBRARY})
  endif(USE_EW)
  if (USE_AMQ)
    set (LINK_LIB_ALL ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY} ${LIB_BASE})
  else ()
    set (LINK_LIB_ALL ${LIB_BASE})
  endif ()
  message(${LINK_LIB_ALL})
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        -Wl,--start-group
                        ${MKL_LIBRARY}
                        -Wl,--end-group
                        ${IPP_LIBRARY}
                        ${GEOLIB_LIBRARY}
                        ${H5_LIBRARY}
                        ${LIBXML2_LIBRARY}
                        ${EW_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARY}
                        -lm -ldl -lpthread)
  target_link_libraries(gfast_playback
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        -Wl,--start-group
                        ${MKL_LIBRARY}
                        -Wl,--end-group
                        ${IPP_LIBRARY}
                        ${GEOLIB_LIBRARY}
                        ${H5_LIBRARY}
                        ${LIBXML2_LIBRARY}
                        ${EW_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARY}
                        -lm -ldl -lpthread)
 target_link_libraries(unit_tests/xcmopad cmopad_shared -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group -lm -ldl -lpthread)
else()
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
                        ${FFTW3_LIBRARY}
                        ${GEOLIB_LIBRARY}
                        ${H5_LIBRARY}
                        ${ZLIB_LIBRARY}
                        ${LIBXML2_LIBRARY}
                        ${EW_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARY}
                        -lm -ldl -lpthread -lgfortran)
  target_link_libraries(gfast_playback
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
                        ${FFTW3_LIBRARY}
                        ${GEOLIB_LIBRARY}
                        ${H5_LIBRARY}
                        ${ZLIB_LIBRARY}
                        ${EW_LIBRARY}
                        ${LIBXML2_LIBRARY}
                        ${INIPARSER_LIBRARY}
                        -lm -ldl -lpthread -lgfortran)
   target_link_libraries(unit_tests/xcmopad cmopad_shared ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY} -lm -ldl -lpthread -lgfortran)
endif(USE_INTEL)

add_test(cmopadTest unit_tests/xcmopad)

set(CPACK_PACKAGE_NAME "GFAST")
set(CPACK_PACKAGE_VENDOR "PNSN")
set(CPACK_PACKAGE_CONTACT "crowellb@uw.edu")
set(CPACK_PACKAGE_LICENSE "GNU Public License 3")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Geodetic first approximation of earthquake size and timing")

