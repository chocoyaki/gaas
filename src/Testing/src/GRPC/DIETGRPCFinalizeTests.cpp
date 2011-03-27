#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( GRPCFinalizeTests, 
			  GRPCSeDFixture )


/*
 * Call grpc_finalize() in right way,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE( finalize_test_1 )
{
  BOOST_TEST_MESSAGE( "-- Test: Finalize Test 1" );

  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("initialize_test_1", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_finalize() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( finalize_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Finalize Test 2" );

  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("initialize_test_1", "client_testing.cfg");

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


BOOST_AUTO_TEST_SUITE_END()

