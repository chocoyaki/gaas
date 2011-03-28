#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"

BOOST_FIXTURE_TEST_SUITE( GRPCCancelTests, 
			  GRPCSeDFixture )



/*
 * Call grpc_cancel() with a valid session ID, checking
 * GRPC_NO_ERROR returned after the specified session
 * is canceleda.
 */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( cancel_test_1 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Cancel Test 1" );

//   grpc_function_handle_t handle;
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id;
//   int x = 1;
//   utils::ClientArgs c("cancel_test_1", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_function_handle_default(&handle, func_list[3]);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_call_async(&handle, &id, x);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_cancel(id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_wait(id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_function_handle_destruct(&handle);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
// }


/*
 * Call grpc_cancel() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE( cancel_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Cancel Test 2" );

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  utils::ClientArgs c("cancel_test_2", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_cancel(id);
  BOOST_CHECK_EQUAL( err, GRPC_INVALID_SESSION_ID );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_cancel() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( cancel_test_3 )
{
  BOOST_TEST_MESSAGE( "-- Test: Cancel Test 3" );

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;

  err = grpc_cancel(id);
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


/*
 * Call grpc_cancel_all() in right way, checking GRPC_NO_ERROR
 * returned after all of the executing sessions are canceled.
 */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( cancel_test_4 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Cancel Test 4" );

//   grpc_function_handle_t handle[NCALLS];
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id[NCALLS];
//   int i, j, x = 1;

//   for (i=0; i<NCALLS; i++) {
//     id[i] = GRPC_SESSIONID_VOID;
//   }

//   utils::ClientArgs c("cancel_test_4", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_default(&handle[i], func_list[3]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_call_async(&handle[i], &id[i], x);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   err = grpc_cancel_all();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_wait_all();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_destruct(&handle[i]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
// }


/*
 * Call grpc_cancel_all() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE( cancel_test_5 )
{
  BOOST_TEST_MESSAGE( "-- Test: Cancel Test 5" );

  grpc_error_t err = GRPC_NO_ERROR;

  err = grpc_cancel_all();
  BOOST_CHECK_EQUAL( err, GRPC_NOT_INITIALIZED );
}


BOOST_AUTO_TEST_SUITE_END()
