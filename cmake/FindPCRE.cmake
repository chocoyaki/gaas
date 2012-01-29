# 
# Find PCRE library
# 
# References: for info on PCRE refer to http://www.pcre.org/
#
# PCRE is needed when compiling natvely on windows 
#
# The following variables are set:
# PCRE_FOUND         - When false, don't try to use PCRE
# PCRE_INCLUDE_DIR   - Directories to include to use PCRE
# PCRE_LIBRARIES     - Files to link against to use PCRE
# PCRE_DIR           - (optional) Suggested installation directory to search
# PCRE_VERSION_MAJOR - Major version number
# PCRE_VERSION_MINOR - Minor version number
# PCRE_VERSION       - Full pcre version (major and minor version numbers)
#
# PCRE_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when PCRE was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set PCRE_DIR to point to your specific installation directory.
#

FILE( APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "FindPCRE.cmake: Looking for PCRE C++ library.\n\n" )

FIND_PATH(
  PCRE_INCLUDE_DIR pcre.h
  PATHS
  ${PCRE_DIR}/include
)

  # Search for static library
  FIND_LIBRARY(
    PCRE_LIBRARY_st 
    NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}pcreposix${CMAKE_STATIC_LIBRARY_SUFFIX}
    PATHS 
    ${PCRE_DIR}/lib
  )
 
   FIND_FILE(
    PCRE_LIBRARY_sh 
    NAMES ${CMAKE_SHARED_LIBRARY_PREFIX}pcreposix3${CMAKE_SHARED_LIBRARY_SUFFIX}
    PATHS 
    ${PCRE_DIR}/bin
  ) 
#  SET( PCRE_LIBRARY_sh ${PCRE_DIR}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}pcreposix3${CMAKE_SHARED_LIBRARY_SUFFIX})  
  

SET( PCRE_FOUND FALSE )
  

IF( PCRE_INCLUDE_DIR )
  IF( PCRE_LIBRARY_st AND PCRE_LIBRARY_sh )
    SET( PCRE_FOUND TRUE )
    MARK_AS_ADVANCED ( PCRE_DIR )
    MARK_AS_ADVANCED ( PCRE_INCLUDE_DIR )
    MARK_AS_ADVANCED ( PCRE_LIBRARY_st )
    MARK_AS_ADVANCED ( PCRE_LIBRARY_sh )
  ENDIF( PCRE_LIBRARY_st AND PCRE_LIBRARY_sh )
ENDIF( PCRE_INCLUDE_DIR )


IF(NOT PCRE_FOUND )
  MESSAGE("PCRE library was not found. Please provide PCRE_DIR ")
  MESSAGE("  - through the GUI when working with ccmake, ")
  MESSAGE("  - as a command line argument when working with cmake e.g. ")
  MESSAGE("    cmake .. -DPCRE_DIR:PATH=C:\\pcre ")
  MESSAGE("Note: the following message is triggered by cmake on the first ")
  MESSAGE("    undefined necessary PATH variable (e.g. PCRE_DIR)")
  MESSAGE("    Providing PCRE_DIR (as above described) is probably the")
  MESSAGE("    simplest solution unless you have a really customized/odd")
  MESSAGE("    PCRE installation...")
  SET( PCRE_DIR "" CACHE PATH "Root of PCRE install directory." )
ENDIF( NOT PCRE_FOUND )



