#
# Find XQilla library
#
# References: for info on XQilla refer to http://xqilla.sourceforge.net/HomePage
#

# The following variables are set:
# XQILLA_FOUND         - When false, don't try to use XQilla
# XQILLA_INCLUDE_DIR   - Directories to include to use XQilla
# XQILLA_LIBRARIES     - Files to link against to use XQilla
# XQILLA_DIR           - (optional) Suggested installation directory to search
# XQILLA_VERSION_MAJOR - Major version number
# XQILLA_VERSION_MINOR - Minor version number
# XQILLA_VERSION       - Full xerces version (major and minor version numbers)
#
# XQILLA_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when XQilla was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set XQILLA_DIR to point to your specific installation directory.
#

FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindXQilla.cmake: Looking for XQilla C++ library.\n\n" )

FIND_PATH(
  XQILLA_INCLUDE_DIR xqilla/xqilla-simple.hpp
  PATHS
  ${XQILLA_DIR}/include
  /usr/include
  /usr/local/include
)

FIND_LIBRARY(
  XQILLA_LIBRARY xqilla
  PATHS
  ${XQILLA_DIR}/lib
  /usr/lib
  /usr/local/lib
)

SET( XQILLA_FOUND FALSE )

IF( XQILLA_INCLUDE_DIR )
IF( XQILLA_LIBRARY )
  SET( XQILLA_FOUND TRUE )
  MARK_AS_ADVANCED ( XQILLA_DIR )
  MARK_AS_ADVANCED ( XQILLA_INCLUDE_DIR )
  MARK_AS_ADVANCED ( XQILLA_LIBRARY )
ENDIF( XQILLA_LIBRARY )
ENDIF( XQILLA_INCLUDE_DIR )


IF(NOT XQILLA_FOUND )
  MESSAGE("XQilla C++ library was not found. Please provide XQILLA_DIR ")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DXQILLA_DIR:PATH=/usr/local/xerces ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. XQILLA_DIR)")
  MESSAGE("    Providing XQILLA_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    XQilla installation...")
  SET( XQILLA_DIR "" CACHE PATH "Root of XQilla install directory." )
ENDIF( NOT XQILLA_FOUND )

#
# IF( XQILLA_INCLUDE_DIR )
#   # We can't use CMAKE_CURRENT_SOURCE_DIR because this script is invoked
#   # from the top-level cmake:
#   SET( WORKING_DIR ${DIET_BINARY_DIR}/Cmake )
#   SET( XQILLA_VERSION_SOURCE_FILE ${WORKING_DIR}/xercesv.cc )
#   CONFIGURE_FILE(
#     ${DIET_SOURCE_DIR}/Cmake/xercesv.cc.in
#     ${XQILLA_VERSION_SOURCE_FILE}
#     IMMEDIATE
#   )
#   TRY_RUN (COMPILE_RESULT RUN_RESULT
#     ${CMAKE_CURRENT_BINARY_DIR}
#     ${XQILLA_VERSION_SOURCE_FILE}
#     OUTPUT_VARIABLE OUTPUT
#   )
#
#   IF ( RUN_RESULT )
#     EXECUTE_PROCESS ( COMMAND ${CMAKE_CXX_COMPILER}
#       ${XQILLA_VERSION_SOURCE_FILE}
#       -o ${WORKING_DIR}/xercesv
#     )
#     EXECUTE_PROCESS ( COMMAND ${WORKING_DIR}/xercesv
#       OUTPUT_VARIABLE XQILLA_VERSION
#     )
#     STRING (REGEX REPLACE "^([0-9])[0-9]" "\\1"
#       XQILLA_VERSION_MAJOR "${XQILLA_VERSION}")
#     STRING (REGEX REPLACE "^[0-9]([0-9])" "\\1"
#       XQILLA_VERSION_MINOR "${XQILLA_VERSION}")
#     FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
#       "FindXQilla.cmake: major version number = ${XQILLA_VERSION_MAJOR}\n" )
#     FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
#       "FindXQilla.cmake: minor version number = ${XQILLA_VERSION_MINOR}\n" )
#   ELSE ( RUN_RESULT )
#     SET( XQILLA_VERSION "0" )
#     SET( XQILLA_VERSION_MAJOR "NOT-FOUND" )
#     SET( XQILLA_VERSION_MINOR "NOT-FOUND" )
#   ENDIF (RUN_RESULT)
# ENDIF ( XQILLA_INCLUDE_DIR )


