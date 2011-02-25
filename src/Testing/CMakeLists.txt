#RE Root CMakeLists.txt file
#
# Project: diet-test
# Author: H. Guémar (haikel.guemar@gmail.com)
# Date: Dec. 2010 21

if( BUILD_TESTING )

  # Tests HAVE TO be built without the gcov compilation flag.
  # Thus, we for the build type to Release, as the gcov compilation
  # flags are set for in Debug mode.
  SET(CMAKE_BUILD_TYPE "Release")

  find_package( Boost 1.44.0 COMPONENTS 
    unit_test_framework
    filesystem
    system 
    thread REQUIRED )
  set( Boost_MULTITHREADED OFF )
  #find_package( GTest REQUIRED )
  include( tests/DietTestingMacros )

  option( ENABLE_REPORTS 
    "add a test-xml target that allow generating xml reports for analysis tools"
    OFF )

  # omniORB setup
  set( OMNINAMES_LOGDIR ${CMAKE_CURRENT_BINARY_DIR}/log
    CACHE PATH "PATH to the omniNames directory" )
  set( OMNIORB_INSTALL_DIR "" CACHE PATH
    "PATH to the omniORB installation directory" )
  set( LOCAL_TEST_PORT "2809" CACHE STRING "Test port for the omniNames" )
  file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/log )

  # test setup 
  # for the moment, we only support in-place testing
  set( BIN_DIR ${PROJECT_BINARY_DIR}/bin )
  set( CONFIG_DIR ${PROJECT_BINARY_DIR}/etc )
  set( EXECUTABLE_OUTPUT_PATH ${BIN_DIR} )

  IF ( DIET_USE_DAGDA )
    SET( DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda)
    SET( DIET_SERVER_LIBRARIES DIET_SeD DIET_Dagda)
  ELSE ( DIET_USE_DAGDA )
    SET( DIET_CLIENT_LIBRARIES DIET_client)
    SET( DIET_SERVER_LIBRARIES DIET_SeD)
  ENDIF ( DIET_USE_DAGDA )

  diet_test_setup()

  add_subdirectory(cfgs)
  add_subdirectory(src)

endif( BUILD_TESTING )
