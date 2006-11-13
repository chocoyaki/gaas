# Set any extra environment variables here:
# - CTEST_ENVIRONMENT uses a shell script to use ssh with the -i flag.
SET( CTEST_ENVIRONMENT "export CVS_RSH=$ENV{HOME}/DashboardDiet/cvs_rsh_dart" )

# What cmake command to use for configuring this dashboard:
SET( CTEST_CMAKE_COMMAND "$ENV{HOME}/local/bin/cmake" )

# The name of the module as extracted with cvs:
SET( CVS_MODULE_DIRECTORY "GRAAL/devel/diet/diet" )

####################### Source and binary directory:
SET( CTEST_SOURCE_DIRECTORY "$ENV{HOME}/DashboardDiet/ble/cvs/${CVS_MODULE_DIRECTORY}" )
SET( CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/Bin" )

SET( CTEST_CVS_COMMAND "/usr/bin/cvs" )

SET( CTEST_CVS_CHECKOUT "${CTEST_CVS_COMMAND} -d:ext:dart@graal.ens-lyon.fr:/home/CVS/graal co -N -d$ENV{HOME}/DashboardDiet/ble/cvs ${CVS_MODULE_DIRECTORY}" )

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
  SET ( CTEST_CONTINUOUS_MINIMUM_INTERVAL 10 )
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
BUILDNAME:STRING=Debian-GCC_3.3.5-Docs-Batch-Blas-Fd-Juxmem-Mma-Workflow

//Name of the computer/site where compile is being run
SITE:STRING=ble.ens-lyon.fr

//////////////////////////////////////////////// GENERAL SETTING
CMAKE_BUILD_TYPE:STRING=Maintainer
OMNIORB4_DIR:PATH=$ENV{HOME}/local/omniORB-4.0.7

///////////////////////////////////////////////// BUILD_DOCUMENTATION OPTION
DIET_BUILD_DOCUMENTATION:BOOL=ON

///////////////////////////////////////////////// BUILD_EXAMPLES OPTION
DIET_BUILD_EXAMPLES:BOOL=ON

///////////////////////////////////////////////// USE_BATCH OPTION
DIET_USE_BATCH:BOOL=ON
APPLESEEDS_DIR:PATH=$ENV{HOME}/local/appleseeds-2.2.1

///////////////////////////////////////////////// USE_BLAS OPTION
DIET_BUILD_BLAS_EXAMPLES:BOOL=ON
FOUND_CBLAS:FILEPATH=/usr/lib/libcblas.so
FOUND_f77BLAS:FILEPATH=/usr/lib/libf77blas.so

///////////////////////////////////////////////// USE_CORI OPTION
//Build DIET with CORI support.
DIET_USE_CORI:BOOL=ON

///////////////////////////////////////////////// USE_DART OPTION
//Build DIET for dart reporting...
DIET_USE_DART:BOOL=ON

///////////////////////////////////////////////// USE_FD OPTION
//Use Diet Fault Detector.
DIET_USE_FD:BOOL=ON

///////////////////////////////////////////////// USE_JUXMEM OPTION
//Build DIET with JuxMem support.
DIET_USE_JUXMEM:BOOL=ON
JUXMEM_DIR:PATH=$ENV{HOME}/local/juxmem-0.3

///////////////////////////////////////////////// USE_WORKFLOW OPTION
//Build DIET with workflow support...
DIET_USE_WORKFLOW:BOOL=ON
XERCES_DIR:PATH=$ENV{HOME}/local/xerces-c-2_7_0

///////////////////////////////////////////////// WITH_MULTI_MA OPTION
//Build DIET with MULTI-Master-Agent support.
DIET_WITH_MULTI_MA:BOOL=ON

")

