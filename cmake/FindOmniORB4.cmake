# Find OmniORB4 cmake module
#
# sets the following variables:
# OMNIORB4_FOUND        - TRUE if OmniORB4 installation has been found
# OMNIORB4_INCLUDE_DIR  - OmniORB4 headers path
# OMNIORB4_LIBRARIES    - OmniORB4 libraries 
# OMNIORB4_VERSION      - OmniORB4 version
# OMNIORB4_IDL_COMPILER - OmniORB4 idl compiler command (omniidl)
# OMNIORB4_NAMESERVER   - OmniORB4 CORBA naming service (omniNames)
#
# optional variables:
# OMNIORB4_DIR          - OmniORB4 local installation path
#
# This module could use OMNIORB4_DIR environment variable is set
# WARNING: The precedence order is the following:
#   1. OMNIORB4_DIR cmake variable
#   2. OMNIORB4_DIR environment variable
#   3. default cmake search paths
# NOTE: this goes against cmake default behavior for Find* macros, 
# more on this issue: 
# http://www.mail-archive.com/kde-buildsystem@kde.org/msg00589.html
#

# minimum OmniORB4 supported version
# 4.0.6 has known bug float/double marshalling using CORBA::Any
set(OMNIORB4_MINIMUM_VERSION "4.1.2")

##############################################################################
# find headers
##############################################################################
find_path(OMNIORB4_INCLUDE_DIR omniORB4/CORBA.h
  PATHS "${OMNIORB4_DIR}/include" "$ENV{OMNIORB4_DIR}/include"
  NO_DEFAULT_PATH)

find_path(OMNIORB4_INCLUDE_DIR omniORB4/CORBA.h)


##############################################################################
# find libraries
##############################################################################
find_library(OMNIORB4_LIBRARY_omniORB4
  NAMES omniORB4
  PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
  NO_DEFAULT_PATH)

find_library(OMNIORB4_LIBRARY_omniORB4
  NAMES omniORB4)

find_library(OMNIORB4_LIBRARY_omnithread
  NAMES omnithread
  PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
  NO_DEFAULT_PATH)

find_library(OMNIORB4_LIBRARY_omnithread
  NAMES omnithread)

find_library(OMNIORB4_LIBRARY_omniDynamic4
  NAMES omniDynamic4
  PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
  NO_DEFAULT_PATH)

find_library(OMNIORB4_LIBRARY_omniDynamic4
  NAMES omniDynamic4)

# optional libraries
find_library(OMNIORB4_LIBRARY_COS4
  NAMES COS4
  PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
  NO_DEFAULT_PATH)

find_library(OMNIORB4_LIBRARY_COS4
  NAMES COS4)

find_library(OMNIORB4_LIBRARY_COSDynamic4
  NAMES COSDynamic4
  PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
  NO_DEFAULT_PATH)

find_library(OMNIORB4_LIBRARY_COSDynamic4
  NAMES COSDynamic4)

##############################################################################
# find command line tools
##############################################################################
find_program(OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS "${OMNIORB4_DIR}/bin" "$ENV{OMNIORB4_DIR}/bin"
  NO_DEFAULT_PATH)
  
find_program(OMNIORB4_IDL_COMPILER
  NAMES omniidl)

find_program(OMNIORB4_NAMESERVER
  NAMES omniNames
  PATHS "${OMNIORB4_DIR}/bin" "$ENV{OMNIORB4_DIR}/bin"
  NO_DEFAULT_PATH)
  
find_program(OMNIORB4_NAMESERVER
  NAMES omniNames)


##############################################################################
# cook our stuff
##############################################################################
set(OMNIORB4_FOUND "FALSE")

if(OMNIORB4_INCLUDE_DIR AND
    OMNIORB4_LIBRARY_omniORB4 AND
    OMNIORB4_LIBRARY_omnithread AND
    OMNIORB4_LIBRARY_omniDynamic4 AND
    OMNIORB4_IDL_COMPILER)
  set(OMNIORB4_FOUND "TRUE")
  mark_as_advanced(OMNIORB4_DIR)
  mark_as_advanced(OMNIORB4_INCLUDE_DIR)
  mark_as_advanced(OMNIORB4_LIBRARY_omniORB4)
  mark_as_advanced(OMNIORB4_LIBRARY_omnithread)
  mark_as_advanced(OMNIORB4_LIBRARY_omniDynamic4)
  mark_as_advanced(OMNIORB4_IDL_COMPILER)
  mark_as_advanced(OMNIORB4_NAMESERVER)
  mark_as_advanced(OMNIORB4_VERSION)
  mark_as_advanced(OMNIORB4_LIBRARY_COS4)
  mark_as_advanced(OMNIORB4_LIBRARY_COSDynamic4)

  set(OMNIORB4_LIBRARIES
    ${OMNIORB4_LIBRARY_omniORB4}
    ${OMNIORB4_LIBRARY_omnithread}
    ${OMNIORB4_LIBRARY_omniDynamic4})

  if(OMNIORB4_LIBRARY_COS4)
    set(OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${OMNIORB4_LIBRARY_COS4})
  endif()
  if(OMNIORB4_LIBRARY_COSDynamic4)
    set(OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${OMNIORB4_LIBRARY_COSDynamic4})
  endif()

  # Optionaly, extract the the version number from the acconfig.h file:
  if( EXISTS ${OMNIORB4_INCLUDE_DIR}/omniORB4/acconfig.h )
    file( READ ${OMNIORB4_INCLUDE_DIR}/omniORB4/acconfig.h OMNIORB_ACCONFIG_H )
    string( REGEX MATCH "#define[\t ]+PACKAGE_VERSION[\t ]+\"([0-9]+.[0-9]+.[0-9]+)\"" OMNIORB_ACCONFIG_H "${OMNIORB_ACCONFIG_H}" )
    string( REGEX REPLACE ".*\"([0-9]+.[0-9]+.[0-9]+)\".*" "\\1" OMNIORB4_VERSION "${OMNIORB_ACCONFIG_H}" )

    if( ${OMNIORB4_VERSION} VERSION_LESS ${OMNIORB4_MINIMUM_VERSION} )
      message( "WARNING: your version of omniORB is older than the minimum required one (${OMNIORB4_MINIMUM_VERSION}), using DIET with this version may result in undetermined behaviors." )
    endif()


  else( EXISTS ${OMNIORB4_INCLUDE_DIR}/omniORB4/acconfig.h )
    set( OMNIORB4_VERSION "NOT-FOUND" )
  endif( EXISTS ${OMNIORB4_INCLUDE_DIR}/omniORB4/acconfig.h )
  set( OMNIORB4_VERSION ${OMNIORB4_VERSION}
      CACHE STRING "OmniORB version number." )

endif()