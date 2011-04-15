#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"

BOOST_FIXTURE_TEST_SUITE( GRPCCallTests, 
			  GRPCSeDFixture )



/*
 * Call grpc_call() with an initialized handle and valid arguments,
 * checking GRPC_NO_ERROR returned with correct output arguments.
 */
BOOST_AUTO_TEST_CASE( call_test_1 )
{
  BOOST_TEST_MESSAGE( "-- Test: Call Test 1" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  int x = 3, y = 0;
  utils::ClientArgs c("call_test_1", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_call(&handle, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
  BOOST_CHECK_EQUAL(y, x + 1);

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_call() before calling grpc_initialize(), checking
 * GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( call_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Call Test 2" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  int x = 3, y = 0;

  err = grpc_call(&handle, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}
 

/*
 * Call grpc_call_async() with an initialized handle and valid
 * arguments, checking GRPC_NO_ERROR returned with a valid
 * session ID.
 */
BOOST_AUTO_TEST_CASE( call_test_3 )
{
  BOOST_TEST_MESSAGE( "-- Test: Call Test 3" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;

  utils::ClientArgs c("call_test_3", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_call_async(&handle, &id, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
  BOOST_CHECK_EQUAL(y, x + 1);

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/* 
 * Call grpc_call_async() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( call_test_4 )
{
  BOOST_TEST_MESSAGE( "-- Test: Call Test 4" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;

  err = grpc_call_async(&handle, &id, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}

BOOST_AUTO_TEST_SUITE_END()
