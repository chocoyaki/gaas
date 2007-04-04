# Set any extra environment variables here:
# - CTEST_ENVIRONMENT uses a shell script to use ssh with the -i flag.
SET( CTEST_ENVIRONMENT "export CVS_RSH=$ENV{HOME}/DashboardDiet/cvs_rsh_dart" )
# SET( CTEST_ENVIRONMENT "export CVS_RSH=$ENV{HOME}/DashboardDiet/cvs_rsh_dart ; export PATH=$PATH:/home/equemene/root/CMake/bin/:/home/equemene/root/OmniORB/bin/:/opt/freeware/bin/" )

# What cmake command to use for configuring this dashboard:
SET( CTEST_CMAKE_COMMAND "$ENV{HOME}/root/CMake/bin/cmake" )

# The name of the module as extracted with cvs:
SET( CVS_MODULE_DIRECTORY "GRAAL/devel/diet/diet" )

####################### Source and binary directory:
SET( CTEST_SOURCE_DIRECTORY "$ENV{HOME}/DashboardDiet/cvs/${CVS_MODULE_DIRECTORY}" )
SET( CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/Bin" )

SET( CTEST_CVS_COMMAND "/usr/bin/cvs" )

SET( CTEST_CVS_CHECKOUT "${CTEST_CVS_COMMAND} -d:ext:dart@graal.ens-lyon.fr:/home/CVS/graal co -N -d$ENV{HOME}/DashboardDiet/cvs ${CVS_MODULE_DIRECTORY}" )

# Using the script argument (and defaulting to Nightly):
SET( MODEL Nightly )
IF( ${CTEST_SCRIPT_ARG} MATCHES Nightly )
  SET( CTEST_NIGHTLY_START_TIME "22:00:00 MEST" )
ENDIF( ${CTEST_SCRIPT_ARG} MATCHES Nightly )

IF( ${CTEST_SCRIPT_ARG} MATCHES Experimental )
  SET( MODEL Experimental )
ENDIF( ${CTEST_SCRIPT_ARG} MATCHES Experimental )

IF( ${CTEST_SCRIPT_ARG} MATCHES Continuous )
  SET( MODEL Continuous )
  SET ( CTEST_CONTINUOUS_DURATION 600 )
  # Pomme is really dogish:
  SET ( CTEST_CONTINUOUS_MINIMUM_INTERVAL 60 )
  # SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE 1 )
ENDIF( ${CTEST_SCRIPT_ARG} MATCHES Continuous )

# Which ctest command to use for running the dashboard
SET( CTEST_COMMAND "$ENV{HOME}/root/CMake/bin/ctest -D ${MODEL}" )

# Ctest should wipe the binary tree before running:
SET( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )

####################################################################
# This is the initial cache to use for the binary tree, be careful to escape
# any quotes inside of this string if you use it:

SET( CTEST_INITIAL_CACHE "

/////////////////////////////////////////////// LOCALISATION
//Name of the build
BUILDNAME:STRING=Powerpc-AIX-XLC_8

//Name of the computer/site where compile is being run
SITE:STRING=decrypthon-1.ens-lyon.fr

//////////////////////////////////////////////// GENERAL SETTING
CMAKE_BUILD_TYPE:STRING=Maintainer

// COMPILER DEFINITIONS AND OPTIONS
CMAKE_CXX_COMPILER:PATH=/usr/vacpp/bin/xlC
CMAKE_C_COMPILER:PATH=/usr/vac/bin/xlc
CMAKE_CXX_FLAGS:STRING='-lpthread'
CMAKE_MAKE_PROGRAM:FILEPATH=/opt/freeware/bin/gmake

OMNIORB4_DIR:PATH=$ENV{HOME}/root/OmniORB

///////////////////////////////////////////////// BUILD_EXAMPLES OPTION
DIET_BUILD_EXAMPLES:BOOL=ON
DIET_BUILD_BLAS_EXAMPLES:BOOL=OFF

///////////////////////////////////////////////// USE_BATCH OPTION
DIET_USE_BATCH:BOOL=ON
APPLESEEDS_DIR:PATH=$ENV{HOME}/root/Appleseeds

///////////////////////////////////////////////// USE_CORI OPTION
//Build DIET with CORI support.
DIET_USE_CORI:BOOL=OFF

///////////////////////////////////////////////// USE_DART OPTION
//Build DIET for dart reporting...
DIET_USE_DART:BOOL=ON

///////////////////////////////////////////////// USE_FD OPTION
//Use Diet Fault Detector.
DIET_USE_FD:BOOL=ON

///////////////////////////////////////////////// USE_WORKFLOW OPTION
//Build DIET with workflow support...
//Set to OFF before xerces compilation under AIX
DIET_USE_WORKFLOW:BOOL=OFF
XERCES_DIR:PATH=$ENV{HOME}/local/xerces-c-2_7_0

")

