# Set any extra environment variables here:
# - CTEST_ENVIRONMENT uses a shell script to use ssh with the -i flag.
SET( CTEST_ENVIRONMENT "export CVS_RSH=$ENV{HOME}/DashboardDiet/cvs_rsh_dart" )

# What cmake command to use for configuring this dashboard:
SET( CTEST_CMAKE_COMMAND "$ENV{HOME}/local/bin/cmake" )

# The name of the module as extracted with cvs:
SET( CVS_MODULE_DIRECTORY "GRAAL/devel/diet/diet" )

####################### Source and binary directory:
SET( CTEST_SOURCE_DIRECTORY "$ENV{HOME}/DashboardDiet/graal/cvs/${CVS_MODULE_DIRECTORY}" )
SET( CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/Bin-Gcc420-Orb410" )

SET( CTEST_CVS_COMMAND "/usr/bin/cvs" )

SET( CTEST_CVS_CHECKOUT "${CTEST_CVS_COMMAND} -d:ext:dart@graal.ens-lyon.fr:/home/CVS/graal co -N -d$ENV{HOME}/DashboardDiet/graal/cvs ${CVS_MODULE_DIRECTORY}" )

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
  SET ( CTEST_CONTINUOUS_MINIMUM_INTERVAL 60 )
  # SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE 1 )
ENDIF( ${CTEST_SCRIPT_ARG} MATCHES Continuous )

# Which ctest command to use for running the dashboard
SET( CTEST_COMMAND "$ENV{HOME}/local/bin/ctest -D ${MODEL}" )

# Ctest should wipe the binary tree before running:
SET( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )

####################################################################
# This is the initial cache to use for the binary tree, be careful to escape
# any quotes inside of this string if you use it:

SET( CTEST_INITIAL_CACHE "

/////////////////////////////////////////////// LOCALISATION
//Name of the build
BUILDNAME:STRING=RHEL3_GCC_4.2.0_OmniORB_4.1.0-Batch-Fd-Workflow

//Name of the computer/site where compile is being run
SITE:STRING=graal.ens-lyon.fr

//////////////////////////////////////////////// GENERAL SETTING
CMAKE_BUILD_TYPE:STRING=Maintainer

// It's necessary to split Build from Launch tests in order to avoid problem
// under graal development platform !

BUILD_TESTING:BOOL=ON

LAUNCH_TESTING:BOOL=OFF

// COMPILER DEFINITIONS AND OPTIONS
CMAKE_EXE_LINKER:PATH=/usr/bin/ld
CMAKE_CXX_COMPILER:PATH=/usr/local/gcc-4.2.0/bin/g++
CMAKE_C_COMPILER:PATH=/usr/local/gcc-4.2.0/bin/gcc
CMAKE_C_FLAGS_INIT:STRING='-I/usr/local/gcc-4.2.0/include/c++/4.2.0'
CMAKE_CXX_FLAGS:STRING='-I/usr/local/gcc-4.2.0/include/c++/4.2.0'

CMAKE_VERBOSE_MAKEFILE:BOOL=ON

OMNIORB4_DIR:PATH=$ENV{HOME}/local/omniORB-4.1.0-4.2.0

///////////////////////////////////////////////// BUILD_EXAMPLES OPTION
DIET_BUILD_EXAMPLES:BOOL=ON

///////////////////////////////////////////////// USE_BATCH OPTION
DIET_USE_BATCH:BOOL=ON
APPLESEEDS_DIR:PATH=$ENV{HOME}/local/appleseeds-2.2.1

///////////////////////////////////////////////// USE_CORI OPTION
//Build DIET with CORI support.
DIET_USE_CORI:BOOL=ON

///////////////////////////////////////////////// USE_DART OPTION
//Build DIET for dart reporting...
DIET_USE_DART:BOOL=ON

///////////////////////////////////////////////// USE_FD OPTION
//Use Diet Fault Detector.
DIET_USE_FD:BOOL=ON

///////////////////////////////////////////////// USE_JXTA OPTION
//Build DIET with JXTA architecture support.
DIET_USE_JXTA:BOOL=OFF

///////////////////////////////////////////////// USE_WORKFLOW OPTION
//Build DIET with workflow support...
DIET_USE_WORKFLOW:BOOL=OFF
XERCES_DIR:PATH=$ENV{HOME}/local/xerces-c-2_7_0

")

