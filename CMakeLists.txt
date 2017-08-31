cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(GFAST)
ENABLE_LANGUAGE(C)
ENABLE_LANGUAGE(CXX)
ENABLE_TESTING()
SET(GFAST_VERSION_MAJOR 1)
SET(GFAST_VERSION_MINOR 2)
SET(GFAST_VERSION_PATCH 0)
SET(GFAST_VERSION ${GFAST_VERSION_MAJOR}.${GFAST_VERSION_MINOR}.${GFAST_VERSION_PATCH})
IF ("${GFAST_INSTANCE}" STREQUAL "")
  SET(GFAST_INSTANCE "DEFAULT")
ENDIF()
MESSAGE("Configuring GFAST version: " ${GFAST_VERSION} " for " ${GFAST_INSTANCE}) 

# Add modules for finding requisites
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
SET(CMAKE_TESTING_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests)
SET(CMAKE_BINARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# For testing
if (GFAST_USE_INTEL)
  MESSAGE("Using Intel Libraries")
  #ADD_DEFINITIONS(-DGFAST_USE_INTEL=1)
  find_package(MKL REQUIRED)
  find_package(IPP REQUIRED)
else()
  find_package(CBLAS REQUIRED)
  find_package(LAPACKE REQUIRED)
  find_package(FFTW3 REQUIRED)
endif (GFAST_USE_INTEL)
if (GFAST_USE_AMQ)
  MESSAGE("Will build GFAST ActiveMQ library")
  find_package(AMQ REQUIRED)
endif (GFAST_USE_AMQ)
if (GFAST_USE_EW)
  MESSAGE("Will use Earthworm")
  find_package(EW REQUIRED)
endif (GFAST_USE_EW)
if (UW_AMAZON)
  MESSAGE("Will use Jansson and libpng for UW/Amazon output")
  find_package(JANSSON REQUIRED)
endif (UW_AMAZON)
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
  ${LIBXML2_INCLUDE_DIR}
  ${H5_C_INCLUDE_DIR}
  ${INIPARSER_INCLUDE_DIR}
  ${ISCL_INCLUDE_DIR}
)

IF (GFAST_USE_INTEL)
INCLUDE_DIRECTORIES(
  ${MKL_INCLUDE_DIR}
)
ELSE ()
INCLUDE_DIRECTORIES(
  ${CBLAS_INCLUDE_DIR}
  ${LAPACKE_INCLUDE_DIR}
)
ENDIF()

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

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(cmopad)
ADD_SUBDIRECTORY(src/core)
ADD_SUBDIRECTORY(src/eewUtils)
ADD_SUBDIRECTORY(src/traceBuffer)
ADD_SUBDIRECTORY(src/xml)
ADD_SUBDIRECTORY(src/hdf5)
ADD_SUBDIRECTORY(unit_tests)

if (GFAST_USE_AMQ)
  include_directories(${LIBAMQ_INCLUDE_DIR} ${APR_INCLUDE_DIR})
  ADD_SUBDIRECTORY(src/activeMQ)
endif (GFAST_USE_AMQ)

if (GFAST_USE_EW)
  add_definitions(-DUSE_EW=1)
  include_directories(${EW_INCLUDE_DIR})
else()
  set(GFAST_USE_EW OFF)
endif (GFAST_USE_EW)

IF (UW_AMAZON)
   ADD_SUBDIRECTORY(src/uw)
   INCLUDE_DIRECTORIES(${JANSSON_INCLUDE_DIR})
ENDIF (UW_AMAZON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EW_BUILD_FLAGS}" CACHE STRING "" FORCE)

ADD_LIBRARY(gfast_core_shared SHARED ${SRCS_CORE})
ADD_LIBRARY(gfast_core_static STATIC ${SRCS_CORE})
ADD_LIBRARY(gfast_xml_shared SHARED ${SRCS_XML})
ADD_LIBRARY(gfast_xml_static STATIC ${SRCS_XML})
ADD_LIBRARY(cmopad_shared SHARED ${SRCS_CMOPAD})
#add_library(cmopad_static STATIC ${SRCS_CMOPAD}) # Need a license agreement from pnsn
ADD_LIBRARY(gfast_traceBuffer_shared SHARED ${SRCS_H5TB})
ADD_LIBRARY(gfast_traceBuffer_static STATIC ${SRCS_H5TB})
IF (GFAST_USE_AMQ)
  ADD_LIBRARY(gfast_activeMQ_shared SHARED ${SRCS_AMQ})
  #add_library(gfast_activeMQ_static STATIC ${SRCS_AMQ})
ENDIF (GFAST_USE_AMQ)
ADD_LIBRARY(gfast_hdf5_shared SHARED ${SRCS_HDF5})
ADD_LIBRARY(gfast_hdf5_static STATIC ${SRCS_HDF5})

ADD_EXECUTABLE(gfast_playback src/gfast_playback.c ${SRCS_EEW})
ADD_EXECUTABLE(gfast_eew src/gfast_eew.c ${SRCS_EEW})

IF (UW_AMAZON)
ADD_EXECUTABLE(gfast2web src/uw/gfast2web.c src/uw/beachball.c src/uw/gfast2json.c)
SET_TARGET_PROPERTIES(gfast2web PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#ADD_SUBDIRECTORY(debug)
SET(DEBUG_SRC src/debug/fakeQueue.c)
ADD_EXECUTABLE(gfast_debug src/debug/debug.c ${DEBUG_SRC})
SET_TARGET_PROPERTIES(gfast_debug PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
ADD_EXECUTABLE(fakeQuake src/debug/fakeQuake.c ${DEBUG_SRC})
SET_TARGET_PROPERTIES(fakeQuake PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
ENDIF (UW_AMAZON)

ADD_EXECUTABLE(xcmopad cmopad/unit_tests.c ${SRCS_CMOPAD}) 
SET_TARGET_PROPERTIES(gfast_playback PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
SET_TARGET_PROPERTIES(gfast_eew PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
SET_TARGET_PROPERTIES(xcmopad PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY})
IF (GFAST_USE_INTEL)
ADD_EXECUTABLE(xcoreTests ${SRCS_UT})
SET_TARGET_PROPERTIES(xcoreTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY})
ENDIF ()


if (GFAST_USE_INTEL)
  message(${ISCL_LIBRARY})
  if (GFAST_USE_EW)
    set (LIB_BASE ${ISCL_LIBARARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                  ${IPP_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY} ${INIPARSER_LIBRARY}
                  ${LIBXML2_LIBRARY} ${EW_LIBRARY} -lm -ldl -lpthread)
  else()
    set (LIB_BASE ${ISCL_LIBARARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                  ${IPP_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY} ${INIPARSER_LIBRARY}
                  ${LIBXML2_LIBRARY})
  endif(GFAST_USE_EW)
  if (GFAST_USE_AMQ)
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
                        gfast_traceBuffer_static
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
                        gfast_activeMQ_shared
                        gfast_traceBuffer_static
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
 target_link_libraries(xcmopad cmopad_shared -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group -lm -ldl -lpthread)
 target_link_libraries(xcoreTests
                       gfast_xml_static
                       gfast_hdf5_static
                       gfast_core_static
                       gfast_traceBuffer_static
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
else()
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        gfast_traceBuffer_static
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
                        gfast_activeMQ_shared
                        gfast_traceBuffer_static
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
                        ${FFTW3_LIBRARY}
                        ${GEOLIB_LIBRARY}
                        ${H5_LIBRARY}
                        ${ZLIB_LIBRARY}
                        ${EW_LIBRARY}
                        ${LIBXML2_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARY}
                        -lm -ldl -lpthread -lgfortran)
  target_link_libraries(xcmopad
                        cmopad_shared
                        ${ISCL_LIBRARY}
                        ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
                        ${FFTW3_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY}
                        -lm -ldl -lpthread -lgfortran)
  #target_link_libraries(xcoreTests
  #                      gfast_xml_shared
  #                      gfast_hdf5_shared
  #                      gfast_core_shared
  #                      cmopad_shared
  #                      ${ISCL_LIBRARY}
  #                      ${LAPACKE_LIBRARY} ${LAPACK_LIBRARY} ${CBLAS_LIBRARY} ${BLAS_LIBRARY}
  #                      ${FFTW3_LIBRARY} ${GEOLIB_LIBRARY} ${H5_LIBRARY}
  #                      ${ISCL_LIBRARY}
  #                      ${LIBXML2_LIBRARY}
  #                      ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
  #                      ${INIPARSER_LIBRARY}
  #                      -lm -ldl -lpthread -lgfortran)
endif(GFAST_USE_INTEL)

IF (UW_AMAZON)
 IF (GFAST_USE_INTEL)
  TARGET_LINK_LIBRARIES(gfast2web
                        gfast_core_shared
                        gfast_traceBuffer_shared
                        gfast_xml_shared
                        gfast_activeMQ_shared
                        gfast_hdf5_shared
                        cmopad_shared
                        ${ISCL_LIBRARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                        ${JANSSON_LIBRARY} ${H5_LIBRARY} ${LIBXML2_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${EW_LIBRARY} ${INIPARSER_LIBRARY} -lcurl -lm -ldl -lpthread -lpng)
  TARGET_LINK_LIBRARIES(fakeQuake
                        gfast_core_shared
                        gfast_traceBuffer_shared
                        gfast_xml_shared
                        gfast_activeMQ_shared
                        gfast_hdf5_shared
                        cmopad_shared
                        ${ISCL_LIBRARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                        ${JANSSON_LIBRARY} ${H5_LIBRARY} ${LIBXML2_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${EW_LIBRARY} ${INIPARSER_LIBRARY} -lm -ldl -lpthread -lpng) 
  TARGET_LINK_LIBRARIES(gfast_debug
                        gfast_core_shared
                        gfast_traceBuffer_shared
                        gfast_xml_shared
                        gfast_activeMQ_shared
                        gfast_hdf5_shared
                        cmopad_shared
                        ${ISCL_LIBRARY} -Wl,--start-group ${MKL_LIBRARY} -Wl,--end-group
                        ${JANSSON_LIBRARY} ${H5_LIBRARY} ${LIBXML2_LIBRARY}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${EW_LIBRARY} ${INIPARSER_LIBRARY} -lm -ldl -lpthread -lpng)
 ELSE ()

 ENDIF (GFAST_USE_INTEL)
ENDIF (UW_AMAZON)

add_test(NAME cmopadTest
         WORKING_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY}
         COMMAND xcmopad)
IF (GFAST_USE_INTEL)
add_test(NAME coreTest 
         WORKING_DIRECTORY ${CMAKE_TESTING_OUTPUT_DIRECTORY}
         COMMAND xcoreTests)
ENDIF ()

set(CPACK_PACKAGE_NAME "GFAST")
set(CPACK_PACKAGE_VENDOR "PNSN")
set(CPACK_PACKAGE_CONTACT "crowellb@uw.edu")
set(CPACK_PACKAGE_LICENSE "GNU Public License 3")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Geodetic first approximation of earthquake size and timing")
