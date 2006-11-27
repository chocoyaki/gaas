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

IF( XERCES_INCLUDE_DIR )
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/xercesv.cc
    "#include \"${XERCES_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp\"\n"
    "#include <stdio.h>\n"
    "int main(int argc, char ** argv) {\n"
    "        printf(\"%d%d\", XERCES_VERSION_MAJOR, XERCES_VERSION_MINOR);\n"
    "        return 0;}\n")
  TRY_RUN (COMPILE_RESULT RUN_RESULT 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/xercesv.cc
    OUTPUT_VARIABLE OUTPUT)

  IF ( RUN_RESULT )
    EXECUTE_PROCESS ( COMMAND ${CMAKE_CXX_COMPILER} 
      ${CMAKE_CURRENT_BINARY_DIR}/xercesv.cc
      -o ${CMAKE_CURRENT_BINARY_DIR}/xercesv)
    EXECUTE_PROCESS ( COMMAND ${CMAKE_CURRENT_BINARY_DIR}/xercesv
      OUTPUT_VARIABLE XERCES_VERSION)

    STRING (REGEX REPLACE "^([0-9])[0-9]" "\\1"
      XERCES_VERSION_MAJOR "${XERCES_VERSION}")
    STRING (REGEX REPLACE "^[0-9]([0-9])" "\\1"
      XERCES_VERSION_MINOR "${XERCES_VERSION}")
  ELSE ( RUN_RESULT )
    SET( XERCES_VERSION "0" )
    SET( XERCES_VERSION_MAJOR "NOT-FOUND" )
    SET( XERCES_VERSION_MINOR "NOT-FOUND" )
  ENDIF (RUN_RESULT)
ENDIF ( XERCES_INCLUDE_DIR )


