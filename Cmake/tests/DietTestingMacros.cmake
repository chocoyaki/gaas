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
    file( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/reports )
    set( REPORT_OUTPUT_PATH ${PROJECT_BINARY_DIR}/reports )
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
    add_custom_target( ${TEST_NAME}-xml
	COMMAND ${CMAKE_COMMAND}
	-DTEST_PROG=${NAME}
	-DBIN_PATH=${BIN_DIR}
	-DREPORT_PATH=${REPORT_OUTPUT_PATH}
	-P ${PROJECT_SOURCE_DIR}/cmake/runtest.cmake )
      add_dependencies( test-xml ${TEST_NAME}-xml )
    endif()
endmacro()

###############################################################################
