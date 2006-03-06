#############################################################################
# DIET main cmake entry point: comment or perish...                         #
#############################################################################

PROJECT(DIET)

SET(DIET_version 2.0)

# --------------------- OPTIONS OF THE PROJECT -----------------------------
OPTION( DIET_BUILD_EXAMPLES "Build DIET examples." ON )
OPTION( DIET_BUILD_DOCUMENTATION "Build the users manual documentation." OFF )
OPTION( DIET_USE_BATCH "Build DIET with Batch support." OFF )
OPTION( DIET_USE_BLAS "Build DIET with BLAS support." OFF )
OPTION( DIET_USE_CORI "Build DIET with CORI support." OFF )
OPTION( DIET_USE_JXTA "Build DIET with JXTA architecture support." OFF )
OPTION( DIET_USE_FAST "Build DIET with FAST support." OFF )
OPTION( DIET_WITH_STATISTICS "Build DIET with generation of statistics." OFF )
OPTION( DIET_WITH_MULTI_MA   "Build DIET with MULTI-Master-Agent support." OFF )
OPTION( DIET_MAINTAINER_MODE "Build additional documentation..." OFF )

# --------------------- BUILDING DEFAULT OPTIONS  --------------------------
# By default build dynamic libraries:
OPTION( BUILD_SHARED_LIBS "Build DIET with shared libraries." ON )
# Set the default build mode to compile c++ and c code with optimizations
# and debug info (roughly speaking "-O2 -g" on Un*x):
#FIXME SET( CMAKE_BUILD_TYPE
#FIXME   RelWithDebInfo CACHE STRING
#FIXME   "Choose the type of build, options are: None(CMAKE_CXX_FLAGS "
#FIXME   "or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
#FIXME   FORCE)

# --------------------- DEPENDENCIES TOWARDS "CLASSIC" C++ HEADERS ---------
# This is just a convenience to advance the detection of any potential missing
# C++ "classic" header to the cmake stage. If we were to forget the following
# tests (and such a header was missing), things would simply break at
# preprocessing stage of the compilation...
# [Those tests are a translation of the AC_CHECK_HEADER(...) that were expressed
# in the configure.ac of the autotools version].
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)

SET( DIET_INCLUDES_TO_CHECK
  assert.h
  iostream
  limits.h
  math.h
  stdlib.h
  string.h
  unistd.h
  sys/stat.h
  sys/types.h )

FOREACH( include_to_check ${DIET_INCLUDES_TO_CHECK} )
  CHECK_INCLUDE_FILE_CXX( ${include_to_check} dummyfound${include_to_check} )
  IF ( NOT dummyfound${include_to_check} )
    MESSAGE(FATAL_ERROR "Missing ${include_to_check} header file.")
  ENDIF( NOT dummyfound${include_to_check} )
ENDFOREACH( include_to_check )

# --------------------- DEPENDENCIES TOWARDS SYSTEM FUNCTIONS ------------
# This is just a convenience to advance the detection of any potential missing
# external function to the cmake stage. If we were to forget the following
# tests (and such an external function was missing), things would simply break
# at linking stage...
# [Those tests are a translation of the AC_CHECK_FUNCS(...) that were expressed
# in the configure.ac of the autotools version].
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

SET( DIET_FUNCTIONS_TO_CHECK
  gethostname
  gettimeofday
  strdup
  strtok_r )

FOREACH( function_to_check ${DIET_FUNCTIONS_TO_CHECK} )
  CHECK_FUNCTION_EXISTS( ${function_to_check} dummyfound${function_to_check} )
  IF ( NOT dummyfound${function_to_check} )
    MESSAGE(FATAL_ERROR "Missing ${function_to_check} function.")
  ENDIF( NOT dummyfound${function_to_check} )
ENDFOREACH( function_to_check )

### CORI (COllector of Ressource Infomation) is heavily dependant on 
# system calls for probing ressources:
IF( DIET_USE_CORI )
  ADD_DEFINITIONS( -DHAVE_CORI )
  INCLUDE( ${DIET_SOURCE_DIR}/Cmake/ConfigureCORI.cmake )
ENDIF( DIET_USE_CORI )

# --------------------- DEPENDENCIES TOWARDS EXTERNAL PACKAGES -------------
INCLUDE( ${DIET_SOURCE_DIR}/Cmake/FindOmniORB.cmake )
# Path to additional modules (i.e. used by FIND_PACKAGE commands making
# reference to non CMake defined "standard" modules):
SET( CMAKE_MODULE_PATH ${DIET_SOURCE_DIR}/Cmake )

IF( DIET_BUILD_DOCUMENTATION )
  # DIET_MAINTAINER_MODE is used within the doc subdirectory
  FIND_PACKAGE( LATEX )
  FIND_PACKAGE( Transfig )
  FIND_PACKAGE( ImageMagick )
  FIND_PACKAGE( Doxygen )
  SUBDIRS( doc )
ENDIF( DIET_BUILD_DOCUMENTATION )

IF( DIET_USE_BATCH )
  FIND_PACKAGE( AppleSeeds )
  IF( APPLESEEDS_FOUND )
    # FIXME: This is only used within src/SeD: do it cleaner !
    ADD_DEFINITIONS( -DHAVE_BATH )
  ELSE( APPLESEEDS_FOUND )
    SET( DIET_USE_BATCH OFF )
  ENDIF( APPLESEEDS_FOUND )
ENDIF( DIET_USE_BATCH )

IF( DIET_USE_JXTA )
  FIND_PACKAGE( JavaExtended )
  IF( JAVA_FOUND )
    ADD_DEFINITIONS(-DHAVE_JXTA)
  ELSE( JAVA_FOUND )
    SET( DIET_USE_JXTA OFF )
  ENDIF( JAVA_FOUND )
ENDIF( DIET_USE_JXTA )

IF( DIET_USE_FAST )
  FIND_PACKAGE( FAST )
  IF( FAST_FOUND )
    ADD_DEFINITIONS( -DHAVE_FAST )
    ### FIXME: we only ported the FAST 0.8.x API way of doing things.
    ### Nevertheless the code still makes references to the FAST 0.4.x API
    ### (search for defines __FAST_0_4__ versus __FAST_0_8__. Should we trash
    ### the FAST 0.4.x API support ?
    ADD_DEFINITIONS( -D__FAST_0_8__ )
  ENDIF( FAST_FOUND )
ENDIF( DIET_USE_FAST )

IF( DIET_WITH_STATISTICS )
  # Enable generation of the statistics log
  ADD_DEFINITIONS( -DHAVE_STATISTICS )
ENDIF( DIET_WITH_STATISTICS )

IF( DIET_WITH_MULTI_MA )
  # Enable usage of so called MULTI-Master-Agent
  ADD_DEFINITIONS( -DHAVE_MULTI_MA )
ENDIF( DIET_WITH_MULTI_MA )

IF( DIET_BUILD_EXAMPLES AND DIET_USE_BLAS )
  # BLAS (Basic Linear Algebric Subroutines) examples subdir requires...BLAS !
  FIND_PACKAGE( BLAS )
ENDIF( DIET_BUILD_EXAMPLES AND DIET_USE_BLAS )

# --------------------- HANDLE COMPILER SPECIFICITIES  --------------------
INCLUDE(${DIET_SOURCE_DIR}/Cmake/CheckCCompiler.cmake)
  
CHECK_C_COMPILER_SUPPORTS_INLINE( INLINE_VALUE INLINE_SUPPORTED )
MESSAGE( STATUS "Inline C compiler support ${INLINE_SUPPORTED} ${INLINE_VALUE}")

# FIXME: Injay2461 must finalize the following:
# SET( CMAKE_C_FLAGS
#   "${CMAKE_C_FLAGS} -Dinline""""""""""""
#  CACHE STRING
#   "Choose the type of build, options are: None(CMAKE_CXX_FLAGS "
#   "or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
#   FORCE)

# --------------------- BUILD THE CONFIGURATION FILE -----------------------
### FIXME: consider using ADD_DEFINITIONS cmake feature instead of a configure
###        file !!!!
CONFIGURE_FILE( ${DIET_SOURCE_DIR}/Cmake/DIET_config.h.in
                ${DIET_BINARY_DIR}/include/DIET_config.h )

SUBDIRS(src)

# --------------------------------------------------------------------------
# QUESTIONS:
# - what is the general purpose of diet/include dir ? (sub lib or includes?)
#        (see src/agent/CMakeLists.txt INCLUDE_DIRECTORIES)
## eof - CMakeLists.txt
