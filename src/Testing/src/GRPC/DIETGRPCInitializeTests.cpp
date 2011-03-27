#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( GRPCInitializeTests, 
			  GRPCSeDFixture )

/*
 * Call grpc_initialize() with a correct configuration file,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE( initialize_test_1 )
{
  BOOST_TEST_MESSAGE( "-- Test: Initialize Test 1" );

  grpc_error_t err = GRPC_NO_ERROR;

  utils::ClientArgs c("initialize_test_1", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_initialize() with a correct configuration file twice,
 * checking GRPC_ALREADY_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( initialize_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Initialize Test 2" );

  grpc_error_t err = GRPC_NO_ERROR;

  utils::ClientArgs c("initialize_test_1", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_ALREADY_INITIALIZED );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}

BOOST_AUTO_TEST_SUITE_END()
