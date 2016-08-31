# - Try to find activeMQ 
# Once done this will define
#  LIBAMQ_FOUND - System has activeMQ library
#  LIBAMQ_INCLUDE_DIR - The activeMQ include directories
#  LIBAMQ_LIBRARIES - The libraries needed to use activeMQ
#  LIBAMQ_DEFINITIONS - Compiler switches required for using activeMQ
SET(AMQ_FOUND "NO")

find_path(APR_INCLUDE_DIR
          NAMES apr.h
          HINTS /usr/include /usr/include/apr/ /usr/include/apr-1 /usr/include/apr-1.0
          DOC "Apache runtime headers"
          )

find_path(LIBAMQ_INCLUDE_DIR
          NAMES activemq
          HINTS  $ENV{HOME}/cpp/activemq-cpp-library-3.9.3/ /usr/include/apr-1.0
          PATH_SUFFIXES include/activemq-cpp-3.9.3
          DOC "ActiveMQ include files" )

find_library(LIBAMQ_LIBRARY NAMES activemq-cpp
             HINTS  $ENV{HOME}/cpp/activemq-cpp-library-3.9.3/lib /usr/lib /usr/lib64
             DOC "ActiveMQ library" )

find_library(LSSL_LIBRARY NAMES ssl
             HINTS /usr/lib /usr/lib64
             DOC "Secure socket layer library")

find_library(LCRYPTO_LIBRARY NAME crypto
             HINTS /usr/lib /usr/lib64
             DOC "Encryption library")

SET(FOUND_CRYPTO "NO")
IF (LCRYPTO_LIBRARY)
  MESSAGE("Found crypto")
  SET(FOUND_CRYPTO "YES")
ENDIF (LCRYPTO_LIBRARY)
SET(FOUND_SSL "NO")
IF (LSSL_LIBRARY)
  MESSAGE("Found ssl")
  SET(FOUND_SSL "YES")
ENDIF(LSSL_LIBRARY)
SET(FOUND_APR "NO")
IF (APR_INCLUDE_DIR)
  MESSAGE("Found apr")
  SET(FOUND_APR "YES")
ENDIF (APR_INCLUDE_DIR)

IF (FOUND_APR AND FOUND_SSL AND FOUND_CRYPTO)
  MESSAGE("Found activeMQ prereqs")
  IF (LIBAMQ_LIBRARY AND LIBAMQ_INCLUDE_DIR)
     MESSAGE("FOund activeMQ library")
     SET(LIBAMQ_LIBRARY ${LIBAMQ_LIBRARY})
     SET(AMQ_FOUND "YES")
  ENDIF (LIBAMQ_LIBRARY AND LIBAMQ_INCLUDE_DIR)
ENDIF (FOUND_APR AND FOUND_SSL AND FOUND_CRYPTO)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBAMQ_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libactiveMQ  DEFAULT_MSG
                                  LIBAMQ_LIBRARY LCRYPTO_LIBRARY LSSL_LIBRARY LIBAMQ_INCLUDE_DIR)

mark_as_advanced(LIBAMQ_INCLUDE_DIR LIBAMQ_LIBRARY LCRYPTO_LIBRARY LSSL_LIBRARY)

set(LIBAMQ_LIBRARIES ${LIBAMQ_LIBRARY} )
set(LIBAMQ_INCLUDE_DIR ${LIBAMQ_INCLUDE_DIR} )
