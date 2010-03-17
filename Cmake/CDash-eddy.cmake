MESSAGE( "----- CDASH [BEGIN] ------")

# Set any extra environment variables here:
# - CTEST_ENVIRONMENT uses a shell script to use ssh with the -i flag.
SET( CTEST_ENVIRONMENT "export CVS_RSH=ssh" )
SET ( CVS_RSH "ssh")

# What cmake command to use for configuring this dashboard:
SET( CTEST_CMAKE_COMMAND "/usr/local/bin/cmake" )

# The name of the module as extracted with cvs:
SET( CVS_MODULE_DIRECTORY "GRAAL/devel/diet/diet" )

####################### Source and binary directory:
SET( CTEST_SOURCE_DIRECTORY "$ENV{HOME}/CDashDiet/mybuild" )
SET( CTEST_BINARY_DIRECTORY "$ENV{HOME}/CDashDiet/mybuild_bin" )

MESSAGE ("CTEST_SOURCE_DIRECTORY ${CTEST_SOURCE_DIRECTORY}")

SET( CTEST_CVS_COMMAND "/usr/bin/cvs" )
SET ( DIR_CURRENT "/home/diet/dietCVS" )

MESSAGE ( "CVS_RSH = ${CVS_RSH}" )

MESSAGE ("CVS = ${CTEST_CVS_COMMAND} -d/home/CVS/graal co -d/home/diet/CDashDiet/mybuild/CDashDiet.ici ${CVS_MODULE_DIRECTORY}" )

SET( CTEST_CVS_CHECKOUT "${CTEST_CVS_COMMAND} -d/home/CVS/graal co -d/home/diet/CDashDiet/mybuild ${CVS_MODULE_DIRECTORY} " )

SET (CTEST_BACKUP_AND_RESTORE TRUE)

#  Which ctest command to use for running the dashboard
SET( CTEST_COMMAND "/usr/bin/ctest -D Nightly" )

# Ctest should wipe the binary tree before running:
#SET( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY FALSE )
SET( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )

####################################################################
# This is the initial cache to use for the binary tree, be careful to escape
# any quotes inside of this string if you use it:

MESSAGE( "----- CDASH [END] ------")

SET( CTEST_INITIAL_CACHE "

//
// Name of the build
BUILDNAME:STRING=RHEL3_GCC_3.2.3_OmniORB_4.1.4-Batch-Fd-Workflow

//Name of the computer/site where compile is being run
SITE:STRING=graal.ens-lyon.fr

//////////////////////////////////////////////// GENERAL SETTING
CMAKE_BUILD_TYPE:STRING=Maintainer

BUILD_TESTING:BOOL=ON

LAUNCH_TESTING:BOOL=OFF

// COMPILER DEFINITIONS AND OPTIONS
CMAKE_EXE_LINKER:PATH=/usr/bin/ld.old
CMAKE_CXX_COMPILER:PATH=/usr/bin/g++
CMAKE_C_COMPILER:PATH=/usr/bin/gcc
CMAKE_C_FLAGS_INIT:STRING='-I/usr/include'
CMAKE_CXX_FLAGS:STRING='-I/usr/include/c++/3.2.3'

CMAKE_VERBOSE_MAKEFILE:BOOL=ON

OMNIORB4_DIR:PATH=/home/diet/local/omniORB-4.1.4/

///////////////////////////////////////////////// BUILD_EXAMPLES OPTION
DIET_BUILD_EXAMPLES:BOOL=ON

///////////////////////////////////////////////// USE_BATCH OPTION
DIET_USE_BATCH:BOOL=OFF
APPLESEEDS_DIR:PATH=$ENV{HOME}/local/appleseeds-2.2.1

///////////////////////////////////////////////// USE_CORI OPTION
//Build DIET with CORI support.
DIET_USE_CORI:BOOL=ON

///////////////////////////////////////////////// USE_DART OPTION
//Build DIET for dart reporting...
DIET_USE_DART:BOOL=ON

///////////////////////////////////////////////// USE_FD OPTION
//Use Diet Fault Detector.
DIET_USE_FD:BOOL=OFF

///////////////////////////////////////////////// USE_JXTA OPTION
//Build DIET with JXTA architecture support.
DIET_USE_JXTA:BOOL=OFF

///////////////////////////////////////////////// USE_WORKFLOW OPTION
//Build DIET with workflow support...
DIET_USE_WORKFLOW:BOOL=OFF
XERCES_DIR:PATH=$ENV{HOME}/local/xerces-c-2_7_0

")


