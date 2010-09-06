# 
# Find Xerces library
# 
# References: for info on Xerces refer to http://xml.apache.org/xerces-c
# 

# The following variables are set:
# XERCES_FOUND         - When false, don't try to use Xerces
# XERCES_INCLUDE_DIR   - Directories to include to use Xerces
# XERCES_LIBRARIES     - Files to link against to use Xerces
# XERCES_DIR           - (optional) Suggested installation directory to search
# XERCES_VERSION_MAJOR - Major version number
# XERCES_VERSION_MINOR - Minor version number
# XERCES_VERSION       - Full xerces version (major and minor version numbers)
#
# XERCES_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when Xerces was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set XERCES_DIR to point to your specific installation directory.
#

FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindXerces.cmake: Looking for Xerces C++ library.\n\n" )

FIND_PATH(
  XERCES_INCLUDE_DIR xercesc/dom/DOMDocument.hpp
  PATHS
  ${XERCES_DIR}/include
  /usr/include
  /usr/local/include
)

IF ( CYGWIN )

  # Search for shared library
  FIND_LIBRARY(
    XERCES_LIBRARY_sh 
    NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}xerces-c${CMAKE_SHARED_LIBRARY_SUFFIX}
    PATHS 
    ${XERCES_DIR}/bin
    /bin
    /usr/bin
    /usr/local/bin
    ${XERCES_DIR}/lib
    /lib
    /usr/lib
    /usr/local/lib
  )

  # Search for static library
  FIND_LIBRARY(
    XERCES_LIBRARY_st 
    NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c${CMAKE_STATIC_LIBRARY_SUFFIX}
    PATHS 
    ${XERCES_DIR}/bin
    /bin
    /usr/bin
    /usr/local/bin
    ${XERCES_DIR}/lib
    /lib
    /usr/lib
    /usr/local/lib
  )
  
ELSE ( CYGWIN )
  
  # Search for shared library
  FIND_LIBRARY(
    XERCES_LIBRARY_sh NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}xerces-c${CMAKE_SHARED_LIBRARY_SUFFIX}
    PATHS
    ${XERCES_DIR}/lib
    /lib
    /usr/lib
    /usr/local/lib
  )

  # Search for static library
  FIND_LIBRARY(
    XERCES_LIBRARY_st NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}xerces-c${CMAKE_STATIC_LIBRARY_SUFFIX}
    PATHS
    ${XERCES_DIR}/lib
    /lib
    /usr/lib
    /usr/local/lib
  )
  
ENDIF( CYGWIN )

SET( XERCES_FOUND FALSE )
  

IF( XERCES_INCLUDE_DIR )
  IF( XERCES_LIBRARY_sh )
    SET( XERCES_FOUND TRUE )
    MARK_AS_ADVANCED ( XERCES_DIR )
    MARK_AS_ADVANCED ( XERCES_INCLUDE_DIR )
    MARK_AS_ADVANCED ( XERCES_LIBRARY_sh )
    MARK_AS_ADVANCED ( XERCES_LIBRARY_st )
  ENDIF( XERCES_LIBRARY_sh )
ENDIF( XERCES_INCLUDE_DIR )


IF(NOT XERCES_FOUND )
  MESSAGE("Xerces C++ library was not found. Please provide XERCES_DIR ")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DXERCES_DIR:PATH=/usr/local/xerces ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. XERCES_DIR)")
  MESSAGE("    Providing XERCES_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    Xerces installation...")
  SET( XERCES_DIR "" CACHE PATH "Root of Xerces install directory." )
ENDIF( NOT XERCES_FOUND )

IF( XERCES_INCLUDE_DIR )
  # We can't use CMAKE_CURRENT_SOURCE_DIR because this script is invoked
  # from the top-level cmake:
  SET( WORKING_DIR ${DIET_BINARY_DIR}/Cmake )
  SET( XERCES_VERSION_SOURCE_FILE ${WORKING_DIR}/xercesv.cc )
  CONFIGURE_FILE(
    ${DIET_SOURCE_DIR}/Cmake/xercesv.cc.in
    ${XERCES_VERSION_SOURCE_FILE}
    IMMEDIATE
  )

  TRY_RUN (RUN_RESULT COMPILE_RESULT
    ${CMAKE_CURRENT_BINARY_DIR}
    ${XERCES_VERSION_SOURCE_FILE}
    OUTPUT_VARIABLE OUTPUT
  )

  IF ( COMPILE_RESULT AND NOT RUN_RESULT )
    EXECUTE_PROCESS ( COMMAND ${CMAKE_CXX_COMPILER} 
      ${XERCES_VERSION_SOURCE_FILE}
      -o ${WORKING_DIR}/xercesv
    )
    EXECUTE_PROCESS ( COMMAND ${WORKING_DIR}/xercesv
      OUTPUT_VARIABLE XERCES_VERSION
    )
    STRING (REGEX REPLACE "^([0-9])[0-9]" "\\1"
      XERCES_VERSION_MAJOR "${XERCES_VERSION}")
    STRING (REGEX REPLACE "^[0-9]([0-9])" "\\1"
      XERCES_VERSION_MINOR "${XERCES_VERSION}")
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindXerces.cmake: major version number = ${XERCES_VERSION_MAJOR}\n" )
    FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindXerces.cmake: minor version number = ${XERCES_VERSION_MINOR}\n" )
  ELSE ( COMPILE_RESULT AND NOT RUN_RESULT )
    SET( XERCES_VERSION "0" )
    SET( XERCES_VERSION_MAJOR "NOT-FOUND" )
    SET( XERCES_VERSION_MINOR "NOT-FOUND" )
  ENDIF (COMPILE_RESULT AND NOT RUN_RESULT)
ENDIF ( XERCES_INCLUDE_DIR )


