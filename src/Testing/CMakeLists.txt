# The following describe the tests that shall be run by ctest (cmake's
# testing utility). For docs on ctest usage refer to:
#      http://www.cmake.org/Wiki/CMake_Testing_With_CTest
# Note: we made the choice of building a test driver i.e. a program that
#       links together many small atomic tests into a single executable.

IF( BUILD_TESTING )

  # The list of the atomic test sources: this is probably the only thing
  # you should edit when adding a new test within this directory:
  SET( DIET_TEST_SOURCES
    TestDummy.cxx
    TestORBMgrInit.cxx
  )

  # TestORBMgrInit needs to be configured with cmake build flags: 
  SET( TESTORBMGRINIT_PROPERTIES
    "-DDIET_CTEST_FQDN_HOSTNAME='\"${DIET_CTEST_FQDN_HOSTNAME}\"'" )
  SET( TESTORBMGRINIT_PROPERTIES 
    "${TESTORBMGRINIT_PROPERTIES} -DDIET_CTEST_OMNIORB_NAMESERVICE_PORT='\"${DIET_CTEST_OMNIORB_NAMESERVICE_PORT}\"'" )
  SET_SOURCE_FILES_PROPERTIES( TestORBMgrInit.cxx PROPERTIES
          COMPILE_FLAGS "${TESTORBMGRINIT_PROPERTIES}")
  
  # Choose a base name for the test driver...
  SET( DIET_TEST_BASENAME dietTestCentral )

  # Require cmake to generate both:
  # - the list of sources (named GENERATED_DIET_MAIN_TEST) of the test suite
  # - the test driver itself (which is a C++ source file mainly calling
  #   the atomic tests (and providing command line syntactic sugar):
  CREATE_TEST_SOURCELIST(
    GENERATED_DIET_MAIN_TEST ${DIET_TEST_BASENAME}.cxx
    ${DIET_TEST_SOURCES}
  )
  
  # In order to drop the binary were ctest will find it:
  SET( DIET_TEST_NAME ${CXX_TEST_PATH}/${DIET_TEST_BASENAME} )
  
  # And build the corresponding binary (indirectly named after the
  # DIET_TEST_BASENAME we chose) out of the above constructed list of tests:
  ADD_EXECUTABLE( ${DIET_TEST_NAME} ${GENERATED_DIET_MAIN_TEST} )
  
  # Some of the test make inclusions:
  INCLUDE_DIRECTORIES(
    ${OMNIORB4_INCLUDE_DIR}
    ${DIET_SOURCE_DIR}/src/CORBA
    ${DIET_SOURCE_DIR}/src/CORBA
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DIET_SOURCE_DIR}/include   # for DIET_data.h
  )

  # Add each atomic test:
  FOREACH( loop_file ${DIET_TEST_SOURCES} )
    GET_FILENAME_COMPONENT( name ${loop_file} NAME_WE )
    ADD_TEST( ${name} ${DIET_TEST_NAME} ${name} )
  ENDFOREACH( loop_file ${DIET_TEST_SOURCES} )

  TARGET_LINK_LIBRARIES( ${DIET_TEST_NAME}
#    UtilsCommon        # FIXME: this is a dependency of the CorbaCommon
                       # target and should NOT figure here !
    CorbaCommon        # Used by TestORBMgrInit.cxx
    DIET_CORBA
    ${OMNIORB4_LIBRARIES}
  )
  
ENDIF( BUILD_TESTING )
