#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"

BOOST_FIXTURE_TEST_SUITE( GRPCMgmtFuncHandleTests, 
			  GRPCSeDFixture )



/*
 * Call grpc_function_handle_default() with an available function name,
 * checking GRPC_NO_ERROR returned with a pointer of the initialized
 * function handle.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_1 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 1" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("management_function_handle_test_1", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
  
  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_function_handle_default() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 2" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


/*
 * Call grpc_function_handle_init() with an available set of
 * the function name and the server name, checking GRPC_NO_ERROR
 * returned with a pointer of the initialized function handle.
 */ 
BOOST_AUTO_TEST_CASE( management_function_handle_test_3 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 3" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("management_function_handle_test_3", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_init(&handle, server, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_function_handle_init() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_4 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 4" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;

  err = grpc_function_handle_init(&handle, server, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


/*
 * Call grpc_function_handle_destruct() in right way,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_5 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 5" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("management_function_handle_test_5", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_function_handle_destruct() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_6 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 6" );

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


/*
 * Call grpc_get_handle() with a valid session ID, checking
 * GRPC_NO_ERROR returned with a pointer of the function
 * handle specified by ID.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_7 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 7" );

  grpc_function_handle_t handle, *handle_tmp;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  utils::ClientArgs c("management_function_handle_test_7", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_call_async(&handle, &id, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_get_handle(&handle_tmp, id);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_call_async(handle_tmp, &id, x, &y);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_get_handle() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_8 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 8" );

  grpc_function_handle_t handle, *handle_tmp;
  grpc_error_t err;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  utils::ClientArgs c("management_function_handle_test_8", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_function_handle_default(&handle, func_list[0]);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_get_handle(&handle_tmp, id);
  BOOST_CHECK_EQUAL( err, GRPC_INVALID_SESSION_ID );

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_get_handle() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returnd.
 */
BOOST_AUTO_TEST_CASE( management_function_handle_test_9 )
{
  BOOST_TEST_MESSAGE( "-- Test: Management Function Handle Test 9" );

  grpc_function_handle_t *handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;

  err = grpc_get_handle(&handle, id);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


BOOST_AUTO_TEST_SUITE_END()
