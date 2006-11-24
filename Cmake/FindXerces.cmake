# 
# Find Xerces library
# 
# References: for info on Xerces refer to http://xml.apache.org/xerces-c
# 

# The following variables are set:
# XERCES_FOUND         - When false, don't try to use Xerces
# XERCES_INCLUDE_DIR   - Directories to include to use Xerces
# XERCES_LIBRARIES     - Files to link against to use Xerces
# XERCES_VERSION_MAJOR - Major version number
# XERCES_VERSION_MINOR - Minor version number
# XERCES_VERSION_REVISION - Revision version number
# XERCES_DIR           - (optional) Suggested installation directory to search
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

FIND_LIBRARY(
  XERCES_LIBRARY xerces-c
  PATHS 
  ${XERCES_DIR}/lib
  /usr/lib
)

SET( XERCES_FOUND FALSE )

IF( XERCES_INCLUDE_DIR )
IF( XERCES_LIBRARY )
  SET( XERCES_FOUND TRUE )
  MARK_AS_ADVANCED ( XERCES_DIR )
  MARK_AS_ADVANCED ( XERCES_INCLUDE_DIR )
  MARK_AS_ADVANCED ( XERCES_LIBRARY )
  # Optionaly, extract the the version number from the acconfig.h file:
  IF( EXISTS ${XERCES_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp )
    FILE( READ ${XERCES_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp 
          XERCES_VERSION_HPP )
    STRING( REGEX MATCH "[^ ]#define[\t ]+XERCES_VERSION_MAJOR[\t ]+[0-9]"
            XERCES_VERSION_MAJOR "${XERCES_VERSION_HPP}" )
    STRING( REGEX REPLACE ".*XERCES_VERSION_MAJOR[\t ]([0-9]).*" "\\1"
            XERCES_VERSION_MAJOR "${XERCES_VERSION_MAJOR}")
    STRING( REGEX MATCH "[^ ]#define[\t ]+XERCES_VERSION_MINOR[\t ]+[0-9]"
            XERCES_VERSION_MINOR "${XERCES_VERSION_HPP}" )
    STRING( REGEX REPLACE ".*XERCES_VERSION_MINOR[\t ]([0-9]).*" "\\1"
            XERCES_VERSION_MINOR "${XERCES_VERSION_MINOR}")
    STRING( REGEX MATCH "[^ ]#define[\t ]+XERCES_VERSION_REVISION[\t ]+[0-9]"
            XERCES_VERSION_REVISION "${XERCES_VERSION_HPP}" )
    STRING( REGEX REPLACE ".*XERCES_VERSION_REVISION[\t ]([0-9]).*" "\\1"
            XERCES_VERSION_REVISION "${XERCES_VERSION_REVISION}" )
  ELSE( EXISTS ${XERCES_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp )
     SET( XERCES_VERSION_MAJOR "NOT-FOUND" )
  ENDIF( EXISTS ${XERCES_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp )
  SET( XERCES_VERSION_MAJOR ${XERCES_VERSION_MAJOR}
       CACHE STRING "Xerces major version number." )
  MARK_AS_ADVANCED( XERCES_VERSION_MAJOR )
  SET( XERCES_VERSION_MINOR ${XERCES_VERSION_MINOR}
       CACHE STRING "Xerces minor version number." )
  MARK_AS_ADVANCED( XERCES_VERSION_MINOR )
  SET( XERCES_VERSION_REVISION ${XERCES_VERSION_REVISION}
       CACHE STRING "Xerces revision version number." )
  MARK_AS_ADVANCED( XERCES_VERSION_REVISION )
ENDIF( XERCES_LIBRARY )
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
