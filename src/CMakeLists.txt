cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(GFAST C CXX)
# Add modules for finding requisites
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
find_package(MKL)
if (MKL_FOUND)

else()
  find_package(CBLAS REQUIRED)
  find_package(LAPACKE REQUIRED)
  find_package(FFTW3 REQUIRED)
endif (MKL_FOUND)
find_package(IniParser REQUIRED)
find_package(ISCL REQUIRED)
find_package(GeographicLib REQUIRED)
find_package(H5 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(XML2 REQUIRED)
find_package(IPP)
find_package(AMQ)
# Bring in headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
  ${includes}
  cmopad
  ${CBLAS_INCLUDE_DIR}
  ${LAPACKE_INCLUDE_DIR}
  ${CBLAS_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${H5_C_INCLUDE_DIR}
  ${INIPARSER_INCLUDE_DIR}
  ${ISCL_INCLUDE_DIR})

#macro for adding sources
macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else (_src ${ARGN})
            list (APPEND SRCS "${_src}")
        endif (_relPath)
    endforeach (_src ${ARGN})
    #propigate up to parent
    if (_relPath)
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif (_relPath)
endmacro (add_sources)

add_subdirectory(include)
add_subdirectory(cmopad)
add_subdirectory(core)
add_subdirectory(eewUtils)
add_subdirectory(events)
add_subdirectory(xml)
add_subdirectory(hdf5)
if (AMQ_FOUND)
MESSAGE("Will build GFAST ActiveMQ library")
include_directories(${LIBAMQ_INCLUDE_DIR} ${APR_INCLUDE_DIR})
add_subdirectory(activeMQ)
endif (AMQ_FOUND)


#set(CORE_SOURCE
#    add_subdirectory(core)
#)

add_library(gfast_core_shared SHARED ${SRCS_CORE})
add_library(gfast_core_static STATIC ${SRCS_CORE})
add_library(gfast_xml_shared SHARED ${SRCS_XML})
add_library(gfast_xml_static STATIC ${SRCS_XML})
add_library(cmopad_shared SHARED ${SRCS_CMOPAD})
#add_library(cmopad_static STATIC ${SRCS_CMOPAD}) # Need a license agreement from pnsn
#add_library(gfast_traceBuffer SHARED ${SRCS_ 
if (AMQ_FOUND)
add_library(gfast_activeMQ_shared SHARED ${SRCS_AMQ})
add_library(gfast_activeMQ_static STATIC ${SRCS_AMQ})
endif (AMQ_FOUND)
add_library(gfast_hdf5_shared SHARED ${SRCS_HDF5})
add_library(gfast_hdf5_static STATIC ${SRCS_HDF5})

add_executable(gfast_eew gfast_eew.c ${SRCS})

if (MKL_FOUND)
 if (IPP_FOUND)
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARIES}
                        -Wl,--start-group
                        ${MKL_LIBRARIES}
                        -Wl,--end-group
                        ${IPP_LIBRARIES}
                        ${GEOLIB_LIBRARIES}
                        ${H5_LIBRARIES}
                        ${HDF5_hdf5_LIBRARY_RELEASE}
                        ${ZLIB_LIBRARY}
                        ${LIBXML2_LIBRARIES}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARIES}
                        -lm -ldl -lpthread)
 else()
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARIES}
                        -Wl,--start-group
                        ${MKL_LIBRARIES}
                        -Wl,--end-group
                        ${GEOLIB_LIBRARIES}
                        ${HDF5_LIBRARIES}
                        ${HDF5_hdf5_LIBRARY_RELEASE}
                        ${ZLIB_LIBRARY}
                        ${LIBXML2_LIBRARIES}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARIES}
                        -lm)
 endif(IPP_FOUND)
else()
 if (IPP_FOUND)
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARIES}
                        ${LAPACKE_LIBRARIES} ${LAPACK_LIBRARIES} ${CBLAS_LIBRARIES} ${BLAS_LIBRARIES}
                        ${IPP_LIBRARIES}
                        ${FFTW3_LIBRARIES}
                        ${GEOLIB_LIBRARIES}
                        ${H5_LIBRARIES}
                        ${HDF5_hdf5_LIBRARY_RELEASE}
                        ${ZLIB_LIBRARY}
                        ${LIBXML2_LIBRARIES}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARIES}
                        -lm -ldl -lpthread -lgfortran)
 else()
  target_link_libraries(gfast_eew
                        gfast_xml_static
                        gfast_hdf5_static
                        gfast_core_static
                        gfast_activeMQ_shared
                        cmopad_shared
                        ${ISCL_LIBRARIES}
                        ${LAPACKE_LIBRARIES} ${LAPACK_LIBRARIES} ${CBLAS_LIBRARIES} ${BLAS_LIBRARIES}
                        ${FFTW3_LIBRARIES}
                        ${GEOLIB_LIBRARIES}
                        ${HDF5_LIBRARIES}
                        ${HDF5_hdf5_LIBRARY_RELEASE}
                        ${ZLIB_LIBRARY}
                        ${LIBXML2_LIBRARIES}
                        ${LIBAMQ_LIBRARY} ${LCRYPTO_LIBRARY} ${LSSL_LIBRARY}
                        ${INIPARSER_LIBRARIES}
                        -lm -ldl -lpthread -lgfortran)
 endif(IPP_FOUND)
endif(MKL_FOUND)

set(CPACK_PACKAGE_NAME "GFAST")
set(CPACK_PACKAGE_VENDOR "PNSN")
set(CPACK_PACKAGE_CONTACT "crowellb@uw.edu")
set(CPACK_PACKAGE_LICENSE "GNU Public License 3")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Geodetic first approximation of earthquake size and timing")

