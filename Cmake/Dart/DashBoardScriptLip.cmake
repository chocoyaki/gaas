# Set any extra environment variables here:
SET( CTEST_ENVIRONMENT "export CVS_RSH=/usr/bin/ssh -i $ENV{HOME}/.ssh/dart_id_dsa" )
# MESSAGE( SEND_ERROR "AAAAAA ${CTEST_ENVIRONMENT}" )

# The name of the module as extracted with cvs:
SET( CVS_MODULE_DIRECTORY "GRAAL/devel/diet/diet" )

# Source and binary directory:
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
  SET ( CTEST_CONTINUOUS_MINIMUM_INTERVAL 10 )
  # SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE 1 )
ENDIF( ${CTEST_SCRIPT_ARG} MATCHES Continuous )


# Which ctest command to use for running the dashboard
SET( CTEST_COMMAND "$ENV{HOME}//local/bin/ctest -D ${MODEL}" )
# The above could be breaked down to three subtasks:
# SET( CTEST_COMMAND 
#   "/usr/local/bin/ctest -D NightlyStart -D NightlyUpdate -D NightlyConfigure"
#   "/usr/local/bin/ctest -D NightlyBuild -D NightlyTest -D NightlySubmit"
#  "/usr/local/bin/ctest -D NightlyMemCheck -D NightlySubmit"
#  )

# What cmake command to use for configuring this dashboard:
SET( CTEST_CMAKE_COMMAND "$ENV{HOME}/local/bin/cmake" )


####################################################################
# The values in this section are optional you can either
# have them or leave them commented out
####################################################################

# Ctest should wipe the binary tree before running:
SET( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )

SET( OMNIORB4 $ENV{HOME}/local/omniORB-4.0.7 )

# This is the initial cache to use for the binary tree, be careful to escape
# any quotes inside of this string if you use it
SET( CTEST_INITIAL_CACHE "

/////////////////////////////////////////////// LOCALISATION
//Name of the build
BUILDNAME:STRING=Debian-GCC_3.2.5-Batch-MMA-Docs

//Name of the computer/site where compile is being run
SITE:STRING=ble.ens-lyon.fr

//////////////////////////////////////////////// GENERAL SETTING
//Build DIET for dart reporting...
DIET_USE_DART:BOOL=ON

//Build DIET with CORI support.
DIET_USE_CORI:BOOL=ON

//Build DIET with MULTI-Master-Agent support.
DIET_WITH_MULTI_MA:BOOL=ON

//Build DIET examples.
DIET_BUILD_EXAMPLES:BOOL=ON

DIET_USE_BATCH:BOOL=ON

//Build type defaulted to Maintainer...
CMAKE_BUILD_TYPE:STRING=Maintainer

///////////////////////////////////////////////// BLAS SECTION
DIET_USE_BLAS:BOOL=ON

FOUND_CBLAS:FILEPATH=/usr/lib/libcblas.so

FOUND_f77BLAS:FILEPATH=/usr/lib/libf77blas.so

///////////////////////////////////////////////// OMNIORB SECTION
OMNIORB4_DIR:PATH=${OMNIORB4}

//What is the path where omniidl (the idl compiler) can be found
OMNIORB4_IDL_COMPILER:FILEPATH=${OMNIORB4}/bin/omniidl

//What is the path where the file omniORB4/CORBA.h can be found
OMNIORB4_INCLUDE_DIR:PATH=${OMNIORB4}/include

//Where can the COS4 library be found
OMNIORB4_LIBRARY_COS4:FILEPATH=${OMNIORB4}/lib/libCOS4.so

//Where can the COSDynamic4 library be found
OMNIORB4_LIBRARY_COSDynamic4:FILEPATH=${OMNIORB4}/lib/libCOSDynamic4.so

//Where can the omniDynamic4 library be found
OMNIORB4_LIBRARY_omniDynamic4:FILEPATH=${OMNIORB4}/lib/libomniDynamic4.so

//Where can the omniORB4 library be found
OMNIORB4_LIBRARY_omniORB4:FILEPATH=${OMNIORB4}/lib/libomniORB4.so

//Where can the omnithread library be found
OMNIORB4_LIBRARY_omnithread:FILEPATH=${OMNIORB4}/lib/libomnithread.so

////////////////////////////////////////////////// Appleseed SECTION
APPLESEEDS_DIR:PATH=/home/eboix/local/appleseeds-2.2.1

APPLESEEDS_INCLUDE_DIR:PATH=/home/eboix/local/appleseeds-2.2.1/include

APPLESEEDS_LIBRARY:FILEPATH=/home/eboix/local/appleseeds-2.2.1/lib/libappleseeds.a

")

