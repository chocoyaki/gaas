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
  add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
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
  string(RANDOM LENGTH 5 ALPHABET "12345" PORTTMP)
  math(EXPR PORTTMP2 "${PORTTMP} + 1")
  set( LOCAL_TEST_PORT "${PORTTMP}" CACHE STRING "Test port for the omniNames" )
  set( LOCAL_TEST_PORT2 "${PORTTMP2}" CACHE STRING "Test port for the second omniNames" )
  file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/log )

  set( SSH_LOCALHOST_USER "$ENV{USER}" CACHE STRING "User allowed to do a \"ssh 127.0.0.1\"" )


  # test setup 
  # for the moment, we only support in-place testing
  set( BIN_DIR ${PROJECT_BINARY_DIR}/bin )
  set( CONFIG_DIR ${PROJECT_BINARY_DIR}/etc )
  set( EXECUTABLE_OUTPUT_PATH ${BIN_DIR} )
  set( WF_XML_DIR ${PROJECT_BINARY_DIR}/test_workflows )
  set( GENERATED_LOG_TESTS_DIR ${PROJECT_BINARY_DIR}/generated_test_logs )
  set( GENERATED_FWD_TESTS_DIR ${PROJECT_BINARY_DIR}/generated_test_forwarders )
  set( GENERATED_LOGFWD_TESTS_DIR ${PROJECT_BINARY_DIR}/generated_test_logs_forwarders )

  set( DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda)
  set( DIET_SERVER_LIBRARIES DIET_SeD DIET_Dagda)
  set( DIET_ADMIN_LIBRARIES DIET_admin)

  diet_test_setup()

  # temporary directories for Dagda
  string(RANDOM LENGTH 32 TMPPATHUUID)
  file( TO_NATIVE_PATH "/tmp/MA_test-${TMPPATHUUID}"     MA_DAGDA_DIR_NATIVE_PATH )
  file( TO_NATIVE_PATH "/tmp/LA_test-${TMPPATHUUID}"     LA_DAGDA_DIR_NATIVE_PATH )
  file( TO_NATIVE_PATH "/tmp/SeD_test-${TMPPATHUUID}"    SED_DAGDA_DIR_NATIVE_PATH )
  file( TO_NATIVE_PATH "/tmp/Client_test-${TMPPATHUUID}" CLIENT_DAGDA_DIR_NATIVE_PATH )

  set( TRANSFER_FILE_TEST1 "" CACHE PATH "File used for transfer tests")
  set( TRANSFER_FILE_TEST2 "" CACHE PATH "File used for transfer tests")

  string(COMPARE EQUAL "${TRANSFER_FILE_TEST1}" "" TRANSFER_FILE_TEST1_EMPTY )
  string(COMPARE EQUAL "${TRANSFER_FILE_TEST2}" "" TRANSFER_FILE_TEST2_EMPTY )
  if ( TRANSFER_FILE_TEST1_EMPTY )
    message( SEND_ERROR "TRANSFER_FILE_TEST1 is not set, it is required when building tests" )
  else  ( TRANSFER_FILE_TEST1_EMPTY )
    file( TO_NATIVE_PATH ${TRANSFER_FILE_TEST1} TRANSFER_FILE_TEST1_NATIVE_PATH )
  endif ( TRANSFER_FILE_TEST1_EMPTY )
  if ( TRANSFER_FILE_TEST2_EMPTY )
    message( SEND_ERROR "TRANSFER_FILE_TEST2 is not set, it is required when building tests" )
  else  ( TRANSFER_FILE_TEST2_EMPTY )
    file( TO_NATIVE_PATH ${TRANSFER_FILE_TEST2} TRANSFER_FILE_TEST2_NATIVE_PATH )
  endif ( TRANSFER_FILE_TEST2_EMPTY )
 

  add_subdirectory(cfgs)
  add_subdirectory(workflows)
  add_subdirectory(src)

endif( BUILD_TESTING )
