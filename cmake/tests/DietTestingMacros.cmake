###############################################################################
# diet_test_setup: macro that setup our test environment
# it add a test-xml target
###############################################################################

macro (diet_test_setup )
  enable_testing()
  # add a test-xml targets that allows us to generate Boost.Test xml reports
  add_custom_target( test-xml )
  include(CTest)

  if( ENABLE_REPORTS )
    if ( NOT DEFINED REPORT_OUTPUT_PATH )
      set( REPORT_OUTPUT_PATH ${PROJECT_BINARY_DIR}/reports CACHE PATH "Directory where the reports will be stored" )
    endif()
    file( MAKE_DIRECTORY ${REPORT_OUTPUT_PATH} )
  endif()
  
endmacro()

###############################################################################
# diet_test: macro that setup a test
# @param[in] NAME  test filename stripped from extension
# @param[in] DISABLED (optional) when defined, remove test from master suite
#                     and display a message to the user.
###############################################################################

macro( diet_test NAME )
  if( ${ARGC} MATCHES 2 )
    if( ${ARGV1} MATCHES "DISABLED" )
      message( STATUS "${ARGV0}: explicitly disabled by developer" )
      set( ${ARGV0}-DISABLED ON ) 
    endif()
  endif()

  if( NOT DEFINED ${ARGV0}-DISABLED )
    # create unit tests executable
    add_executable( ${NAME}
      "${NAME}.cpp"
      #entry point
      ../common/TestRunner.cpp )
    
    # link libraries
    target_link_libraries( ${NAME}
      ${Boost_LIBRARIES}
      ${DIET_CLIENT_LIBRARIES} 
      ${DIET_ADMIN_LIBRARIES} 
      utils
      pthread )
    
    # test executable installation has not been tested yet -sic-
    # install( TARGETS ${NAME} DESTINATION bin )
    
    string( REGEX REPLACE "DIET(.*)" "\\1" TEST_NAME ${NAME} )
    
    add_test( ${TEST_NAME} ${BIN_DIR}/${NAME} )
    
    # prevent Boost.Test to catch unrelated exceptions 
    set_property( TEST ${TEST_NAME} 
      PROPERTY ENVIRONMENT "BOOST_TEST_CATCH_SYSTEM_ERRORS=no;" )
    # just make sure that our test are run in a serial fashion
    set_property( TEST ${TEST_NAME} PROPERTY RUN_SERIAL ON )
    
    #
    if( ENABLE_REPORTS )
      add_custom_target( ${TEST_NAME}-xml
	COMMAND ${CMAKE_COMMAND}
	-DTEST_PROG=${NAME}
	-DBIN_PATH=${BIN_DIR}
	-DREPORT_PATH=${REPORT_OUTPUT_PATH}
	-P ${PROJECT_SOURCE_DIR}/cmake/tests/runtest.cmake )
      add_dependencies( test-xml ${TEST_NAME}-xml )
    endif()
  endif()
endmacro()


###############################################################################
# generate_diet_tests: macro that setup a test with potentially a test using
# log central
# @param[in] NAME  test filename stripped from extension
# @param[in] FIXTURENAME fixture name, will be replaced by ${FIXTURENAME}Log
# @param[in] TESTNAME test name, will be replaced by ${TESTNAME}Log
# @param[in] POSTFIX string that is postfixed at the end of the fixture name
# @param[in] INCLUDEFILE fixture include file
# @param[in] GENERATEDDIR directory where the generated files will be stored
###############################################################################
macro( generate_replace_tests NAME FIXTURENAME TESTNAME POSTFIX INCLUDEFILE GENERATEDDIR )
    file( READ "${NAME}.cpp" TEST_CODE )
    string( REPLACE "${FIXTURENAME}" "${FIXTURENAME}${POSTFIX}" TEST_CODE_N "${TEST_CODE}" )
    string( REPLACE "${TESTNAME}" "${TESTNAME}${POSTFIX}" TEST_CODE_N2 "${TEST_CODE_N}" )
    set( NEWNAME "${GENERATEDDIR}/${NAME}${POSTFIX}" )
    file( WRITE "${NEWNAME}.cpp" "#include \"${INCLUDEFILE}\"\n${TEST_CODE_N2}" )


    # create unit tests executable
    add_executable( ${NAME}${POSTFIX}
      "${NEWNAME}.cpp"
      #entry point
      ../common/TestRunner.cpp )
      
    # link libraries
    target_link_libraries( ${NAME}${POSTFIX}
      ${Boost_LIBRARIES}
      ${DIET_CLIENT_LIBRARIES} 
      ${DIET_ADMIN_LIBRARIES} 
      utils
      pthread )
    
    # test executable installation has not been tested yet -sic-
    # install( TARGETS ${NAME} DESTINATION bin )
      
    string( REGEX REPLACE "DIET(.*)" "\\1" TEST_NAME ${NAME}${POSTFIX} )

    add_test( ${TEST_NAME} ${BIN_DIR}/${NAME}${POSTFIX} )

    # prevent Boost.Test to catch unrelated exceptions 
    set_property( TEST ${TEST_NAME} 
      PROPERTY ENVIRONMENT "BOOST_TEST_CATCH_SYSTEM_ERRORS=no;" )
    # just make sure that our test are run in a serial fashion
    set_property( TEST ${TEST_NAME} PROPERTY RUN_SERIAL ON )
      
    #
    if( ENABLE_REPORTS )
      add_custom_target( ${TEST_NAME}-xml
	COMMAND ${CMAKE_COMMAND}
	-DTEST_PROG=${NAME}${POSTFIX}
	-DBIN_PATH=${BIN_DIR}
	-DREPORT_PATH=${REPORT_OUTPUT_PATH}
	-P ${PROJECT_SOURCE_DIR}/cmake/tests/runtest.cmake )
      add_dependencies( test-xml ${TEST_NAME}-xml )
    endif()
endmacro()

###############################################################################
# generate_diet_tests: macro that setup a test with potentially a test using
# log central
# @param[in] NAME  test filename stripped from extension
# @param[in] FIXTURENAME fixture name, will be replaced by ${FIXTURENAME}Log
# @param[in] TESTNAME test name, will be replaced by ${TESTNAME}Log
# @param[in] DISABLED (optional) when defined, remove test from master suite
#                     and display a message to the user.
###############################################################################

macro( generate_diet_tests NAME FIXTURENAME TESTNAME )
  if( ${ARGC} MATCHES 4 )
    if( ${ARGV3} MATCHES "DISABLED" )
      message( STATUS "${ARGV0}: explicitly disabled by developer" )
      set( ${ARGV0}-DISABLED ON ) 
    endif()
  endif()

  if( NOT DEFINED ${ARGV0}-DISABLED )
    #####################
    # create unit tests #
    #####################
    diet_test( ${NAME} )

    ######################
    # Generate log tests #
    ######################
    if( DIET_USE_LOG )
      generate_replace_tests( ${NAME} ${FIXTURENAME} ${TESTNAME} "Log" "fixtures_log.hpp" ${GENERATED_LOG_TESTS_DIR} )
    endif()

    ############################
    # Generate forwarder tests #
    ############################
    # generate_replace_tests( ${NAME} ${FIXTURENAME} ${TESTNAME} "Fwd" "fixtures_fwd.hpp" ${GENERATED_FWD_TESTS_DIR} )

    ##################################
    # Generate forwarder + log tests #
    ##################################
    # generate_replace_tests( ${NAME} ${FIXTURENAME} ${TESTNAME} "LogFwd" "fixtures_logFwd.hpp" ${GENERATED_LOGFWD_TESTS_DIR} )
  endif()
endmacro()
