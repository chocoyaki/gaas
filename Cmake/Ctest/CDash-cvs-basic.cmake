####### Include the file CDash-common-env.txt  with the Ctest environment 
####### You can fill this file with information of your local environment
############################
INCLUDE ( "CDash-common-cvs.txt" )
INCLUDE ( "CDash-common-env.txt" )

SET(CTEST_BUILD_NAME "myos_gcc_x.x.x_OmniORB_x.x.x")

####### Option for the compilation 
############################

SET( CTEST_INITIAL_CACHE "

/////// GENERAL SETTING
CMAKE_BUILD_TYPE:STRING=Maintainer

BUILD_TESTING:BOOL=ON

LAUNCH_TESTING:BOOL=OFF

CMAKE_VERBOSE_MAKEFILE:BOOL=ON

OMNIORB4_DIR:PATH=${OMNIORB4_DIR}

/////// BUILD_EXAMPLES OPTION
DIET_BUILD_EXAMPLES:BOOL=OFF

/////// USE_BATCH OPTION
DIET_USE_BATCH:BOOL=OFF

/////// Build DIET with CORI
DIET_USE_CORI:BOOL=OFF

/////// Build DIET with Fault Tolerance 
DIET_USE_FD:BOOL=OFF

/////// Build DIET with JXTA architecture support.
DIET_USE_JXTA:BOOL=OFF 

/////// Build DIET with workflow support...
DIET_USE_WORKFLOW:BOOL=OFF

")

INCLUDE ( "CDash-common-cmd.txt" )

