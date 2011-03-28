#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"


BOOST_FIXTURE_TEST_SUITE( GRPCErrorReportTests, 
			  GRPCSeDFixture )



/*
 * Call grpc_error_string() with a defined error code,
 * checking the corresponded error string returned.
 */
BOOST_AUTO_TEST_CASE( error_reporting_test_1 )
{
  BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 1" );

  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("error_reporting_test_1", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
	
  BOOST_CHECK_EQUAL("GRPC_NO_ERROR", grpc_error_string(GRPC_NO_ERROR));
  BOOST_CHECK_EQUAL("GRPC_NOT_INITIALIZED", grpc_error_string(GRPC_NOT_INITIALIZED)); 
  BOOST_CHECK_EQUAL("GRPC_ALREADY_INITIALIZED", grpc_error_string(GRPC_ALREADY_INITIALIZED));
  BOOST_CHECK_EQUAL("GRPC_CONFIGFILE_NOT_FOUND", grpc_error_string(GRPC_CONFIGFILE_NOT_FOUND));
  BOOST_CHECK_EQUAL("GRPC_CONFIGFILE_ERROR", grpc_error_string(GRPC_CONFIGFILE_ERROR));
  BOOST_CHECK_EQUAL("GRPC_SERVER_NOT_FOUND", grpc_error_string(GRPC_SERVER_NOT_FOUND));
  BOOST_CHECK_EQUAL("GRPC_FUNCTION_NOT_FOUND", grpc_error_string(GRPC_FUNCTION_NOT_FOUND));
  BOOST_CHECK_EQUAL("GRPC_INVALID_FUNCTION_HANDLE", grpc_error_string(GRPC_INVALID_FUNCTION_HANDLE));
  BOOST_CHECK_EQUAL("GRPC_INVALID_SESSION_ID", grpc_error_string(GRPC_INVALID_SESSION_ID));
  BOOST_CHECK_EQUAL("GRPC_RPC_REFUSED", grpc_error_string(GRPC_RPC_REFUSED));
  BOOST_CHECK_EQUAL("GRPC_COMMUNICATION_FAILED", grpc_error_string(GRPC_COMMUNICATION_FAILED));
  BOOST_CHECK_EQUAL("GRPC_SESSION_FAILED", grpc_error_string(GRPC_SESSION_FAILED));
  BOOST_CHECK_EQUAL("GRPC_NOT_COMPLETED", grpc_error_string(GRPC_NOT_COMPLETED));
  BOOST_CHECK_EQUAL("GRPC_NONE_COMPLETED", grpc_error_string(GRPC_NONE_COMPLETED));
  BOOST_CHECK_EQUAL("GRPC_OTHER_ERROR_CODE", grpc_error_string(GRPC_OTHER_ERROR_CODE));
  BOOST_CHECK_EQUAL("GRPC_UNKNOWN_ERROR_CODE", grpc_error_string(GRPC_UNKNOWN_ERROR_CODE));
  BOOST_CHECK_EQUAL("GRPC_LAST_ERROR_CODE", grpc_error_string(GRPC_LAST_ERROR_CODE));

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


/*
 * Call grpc_error_string with a non-defined error code, checking
 * the string which means "GRPC_UNKNOWN_ERROR_CODE" returned.
 */
BOOST_AUTO_TEST_CASE( error_reporting_test_2 )
{
  BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 2" );

  grpc_error_t err = GRPC_NO_ERROR;
  utils::ClientArgs c("error_reporting_test_2", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = -1;
  BOOST_TEST_MESSAGE("  A non-defined error code (" << err << ") corresponds to '" << grpc_error_string(err) << "'");
  BOOST_CHECK_EQUAL("GRPC_UNKNOWN_ERROR_CODE", grpc_error_string(err));
  err = 1000;
  BOOST_TEST_MESSAGE("  A non-defined error code (" << err << ") corresponds to '" << grpc_error_string(err) << "'");
  BOOST_CHECK_EQUAL("GRPC_UNKNOWN_ERROR_CODE", grpc_error_string(err));

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


// /*
//  * Call grpc_get_error() with a valid session ID, checking
//  * the error code associated with the given session returned.
//  */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( error_reporting_test_3 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 3" );

//   grpc_function_handle_t handle;
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id = GRPC_SESSIONID_VOID;
//   int x = 3, y = 0;
//   utils::ClientArgs c("error_reporting_test_3", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_function_handle_default(&handle, func_list[0]);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   err = grpc_call_async(&handle, &id, x, &y);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_TEST_MESSAGE( "-- After call async" );
  
//   err = grpc_get_error(id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_TEST_MESSAGE( "-- After get error" );

//   err = grpc_wait(id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_TEST_MESSAGE( "-- After wait" );

//   err = grpc_function_handle_destruct(&handle);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_TEST_MESSAGE( "-- After destruct" );

//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_TEST_MESSAGE( "-- After finalize" );
// }


/*
 * Call grpc_get_error() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE( error_reporting_test_4 )
{
  BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 4" );

  grpc_error_t err;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  utils::ClientArgs c("error_reporting_test_4", "client_testing.cfg");
	
  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

  err = grpc_get_error(id);
  BOOST_CHECK_EQUAL( err, GRPC_INVALID_SESSION_ID );

  err = grpc_finalize();
  BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
}


// /*
//  * Call grpc_get_error_failed_sessionid() when there is
//  * no failed session, checking GRPC_NO_ERROR returned.
//  */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( error_reporting_test_5 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 5" );

//   grpc_function_handle_t handle[NCALLS];
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
//   int i, j, x = 3, y[NCALLS];

//   for (i=0; i<NCALLS; i++){
//     y[i] = 0;
//     id[i] = GRPC_SESSIONID_VOID;
//   }

//   utils::ClientArgs c("error_reporting_test_4", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_default(&handle[i], func_list[0]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   err = grpc_wait_all();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     BOOST_CHECK( y[i] != (x + 1) );
//   }

//   err = grpc_get_failed_sessionid(&ret_id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_destruct(&handle[i]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
// }


// /*
//  * Call grpc_get_failed_sessionid() when there is one
//  * failed session, checking a pointer of that session ID
//  * returned as idPtr.
//  */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( error_reporting_test_6 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 6" );

//   grpc_function_handle_t handle[NCALLS];
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
//   int i, j, x = 10;

//   for (i=0; i<NCALLS; i++) {
//     id[i] = GRPC_SESSIONID_VOID;
//   }
  
//   utils::ClientArgs c("error_reporting_test_6", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     if (i == 0) {
//       err = grpc_function_handle_default(&handle[i], func_list[2]);
//     } else {
//       err = grpc_function_handle_default(&handle[i], func_list[1]);
//     }
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_call_async(&handle[i], &id[i], x);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }
//   err = grpc_wait_all();
//   /* and ignore any errors */

//   err = grpc_get_failed_sessionid(&ret_id);
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   BOOST_CHECK( ret_id != GRPC_SESSIONID_VOID );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_destruct(&handle[i]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
// }


// /*
//  * Call grpc_get_failed_sessionid() repeatedly when
//  * there are more than two failed sessions, checking
//  * a pointer of that session ID returned one by one,
//  * until all of them are popped out.
//  */
// FIXME: currently does not work because of grpc_call_async
// BOOST_AUTO_TEST_CASE( error_reporting_test_7 )
// {
//   BOOST_TEST_MESSAGE( "-- Test: Error Reporting Test 7" );

//   grpc_function_handle_t handle[NCALLS];
//   grpc_error_t err = GRPC_NO_ERROR;
//   grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
//   int i, j, counter = 0, x = 10;

//   utils::ClientArgs c("error_reporting_test_7", "client_testing.cfg");
	
//   err = grpc_initialize(c.config());
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_default(&handle[i], func_list[2]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_call_async(&handle[i], &id[i], x);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }
//   err = grpc_wait_all();
//   /* and ignore any errors */

//   for (;;) {
//     err = grpc_get_failed_sessionid(&ret_id);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );

//     if (ret_id == GRPC_SESSIONID_VOID){
//       if (counter == NCALLS) {
//         BOOST_CHECK_EQUAL(counter, NCALLS);
//       } 
//       break;
//     } else {
//       counter++;
//     }
//   }

//   for (i=0; i<NCALLS; i++) {
//     err = grpc_function_handle_destruct(&handle[i]);
//     BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
//   }
//   err = grpc_finalize();
//   BOOST_CHECK_EQUAL( err, GRPC_NO_ERROR );
// }


BOOST_AUTO_TEST_SUITE_END()
