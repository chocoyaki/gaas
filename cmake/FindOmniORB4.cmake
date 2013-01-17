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

set(OMNIORB4_LIBRARY_PREFIX "OMNIORB4_LIBRARY_")

set(omni_libs 
  omniORB4
  omnithread
  omniDynamic4)


# optional libraries

set(opt_libs 
    COS4
    COSDynamic4)

if (DIET_USE_SECURITY)
  set(omni_libs ${omni_libs} omnisslTP4)
else(DIET_USE_SECURITY)
#  set(opt_libs ${opt_libs} omnisslTP4)
endif(DIET_USE_SECURITY)


foreach(lib_name ${omni_libs} ${opt_libs})
    if (WIN32)
      FIND_LIBRARY( ${OMNIORB4_LIBRARY_PREFIX}${lib_name}
        NAMES
        ${CMAKE_STATIC_LIBRARY_PREFIX}${lib_name}${CMAKE_STATIC_LIBRARY_SUFFIX}
        PATHS ${OMNIORB4_DIR}/lib/x86_win32 $ENV{OMNIORB4_DIR}/lib/x86_win32
        NO_DEFAULT_PATH 
      )

      FIND_LIBRARY( ${OMNIORB4_LIBRARY_PREFIX}${lib_name}
        NAMES
        ${CMAKE_STATIC_LIBRARY_PREFIX}${lib_name}${CMAKE_STATIC_LIBRARY_SUFFIX}
      )
    else (WIN32)
      find_library(${OMNIORB4_LIBRARY_PREFIX}${lib_name}
        NAMES ${lib_name}
        PATHS "${OMNIORB4_DIR}/lib${LIB_SUFFIX}" "$ENV{OMNIORB4_DIR}/lib${LIB_SUFFIX}"
        NO_DEFAULT_PATH)

      find_library(${OMNIORB4_LIBRARY_PREFIX}${lib_name}
        NAMES ${lib_name})
    endif (WIN32)
    
  
endforeach(lib_name ${omni_libs} ${opt_libs})




##############################################################################
# find command line tools
##############################################################################
IF (WIN32)
FIND_PROGRAM( OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS ${OMNIORB4_DIR}/bin/x86_win32 $ENV{OMNIORB4_DIR}/bin/x86_win32
  DOC "What is the path where omniidl (the idl compiler) can be found"
  NO_DEFAULT_PATH
)
FIND_PROGRAM( OMNIORB4_IDL_COMPILER
  NAMES omniidl
  DOC "What is the path where omniidl (the idl compiler) can be found"
)

FIND_PROGRAM( OMNIORB4_OMNINAMES_COMMAND
  NAMES omniNames
  PATHS ${OMNIORB4_DIR}/bin/x86_win32 $ENV{OMNIORB4_DIR}/bin/x86_win32
  DOC "What is the path where omniNames (the ORB server) can be found"
  NO_DEFAULT_PATH
)
FIND_PROGRAM( OMNIORB4_OMNINAMES_COMMAND
  NAMES omniNames
  DOC "What is the path where omniNames (the ORB server) can be found"
)
ELSE(WIN32)
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
ENDIF (WIN32)

##############################################################################
# cook our stuff
##############################################################################
set(OMNIORB4_FOUND "FALSE")

if(OMNIORB4_INCLUDE_DIR AND 
    OMNIORB4_IDL_COMPILER)
  set(OMNIORB4_FOUND "TRUE")
  
  foreach(lib_name ${omni_libs})
      if (NOT ${OMNIORB4_LIBRARY_PREFIX}${lib_name})
          set(OMNIORB4_FOUND "FALSE")
      endif()
  endforeach(lib_name ${omni_libs})
  
  if(OMNIORB4_FOUND)
  
  mark_as_advanced(OMNIORB4_DIR)
  mark_as_advanced(OMNIORB4_INCLUDE_DIR)
  mark_as_advanced(OMNIORB4_IDL_COMPILER)
  mark_as_advanced(OMNIORB4_NAMESERVER)
  mark_as_advanced(OMNIORB4_VERSION)

  foreach(lib_name ${omni_libs} ${opt_libs}) 
    mark_as_advanced(${OMNIORB4_LIBRARY_PREFIX}${lib_name})
  endforeach()
  
  set (OMNIORB4_LIBRARIES)
  foreach(lib_name ${omni_libs})
    set (OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${${OMNIORB4_LIBRARY_PREFIX}${lib_name}}) 
  endforeach(lib_name ${omni_libs})
  message(STATUS ${OMNIORB4_LIBRARIES})
  
  foreach(opt_lib ${opt_libs}) 
    if(${OMNIORB4_LIBRARY_PREFIX}${opt_lib})
      set(OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARIES} ${${OMNIORB4_LIBRARY_PREFIX}${opt_lib}})
    endif()
  endforeach(opt_lib ${opt_libs})


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
endif()

unset(omni_libs)
unset(opt_libs)
