/*
 * DIETSimpleAsyncClientTests.cpp
 *
 * Stupid client that talks with stupid SeD
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( SimpleAsyncClientTest, 
			  SimpleAddSeDFixture )

BOOST_AUTO_TEST_CASE( simple_async_client)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client" );

    double a = 1;
    double b = 2;
    double *res = NULL;
    diet_reqID_t reqID;
    diet_profile_t *profile;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_call_async(profile, &reqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_probe(reqID);
    BOOST_CHECK( GRPC_OTHER_ERROR_CODE != error );
   
    error = diet_wait(reqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_probe(reqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_get_error(reqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_scalar_get(diet_parameter(profile, 2), &res, NULL);
    BOOST_CHECK_EQUAL( 0, error );
    BOOST_CHECK_EQUAL( 3, *res );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
}


BOOST_AUTO_TEST_CASE( simple_async_client_cancel)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_cancel" );

    double a = 1;
    double b = 2;
    double *res = NULL;
    diet_reqID_t reqID;
    diet_profile_t *profile;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_call_async(profile, &reqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_cancel(reqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait(reqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );

    error = diet_probe(reqID);
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
}

BOOST_AUTO_TEST_CASE( simple_async_client_allcancel)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_allcancel" );

    double a = 1;
    double b = 2;
    double a2 = 1;
    double b2 = 2;
    double *res = NULL;
    diet_reqID_t reqID, reqID2;
    diet_profile_t *profile, *profile2;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );
    profile2 = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 0), &a2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 1), &b2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );


    error = diet_call_async(profile, &reqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_call_async(profile2, &reqID2);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_cancel_all() ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait(reqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );
    error = diet_probe(reqID);
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );

    error = diet_wait(reqID2) ;
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );
    error = diet_probe(reqID2);
    BOOST_REQUIRE_EQUAL( GRPC_INVALID_SESSION_ID, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    error = diet_profile_free(profile2);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
}


BOOST_AUTO_TEST_CASE( simple_async_client_waitall)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_waitall" );

    double a = 1;
    double b = 2;
    double a2 = 1;
    double b2 = 2;
    double *res = NULL;
    diet_reqID_t *reqID = new diet_reqID_t[2];
    diet_profile_t *profile, *profile2;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );
    profile2 = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 0), &a2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 1), &b2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );


    error = diet_call_async(profile, &reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_call_async(profile2, &reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_all() ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_probe(reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    error = diet_profile_free(profile2);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    delete [] reqID;
}


BOOST_AUTO_TEST_CASE( simple_async_client_waitand)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_waitand" );

    double a = 1;
    double b = 2;
    double a2 = 1;
    double b2 = 2;
    double *res = NULL;
    diet_reqID_t *reqID = new diet_reqID_t[2];
    diet_profile_t *profile, *profile2;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );
    profile2 = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 0), &a2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 1), &b2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );


    error = diet_call_async(profile, &reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_call_async(profile2, &reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_and(reqID, 2) ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    error = diet_profile_free(profile2);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    delete [] reqID;
}


BOOST_AUTO_TEST_CASE( simple_async_client_waitor)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_waitor" );

    double a = 1;
    double b = 2;
    double a2 = 1;
    double b2 = 2;
    double *res = NULL;
    diet_reqID_t *reqID = new diet_reqID_t[2];
    diet_reqID_t orReqID;
    diet_profile_t *profile, *profile2;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );
    profile2 = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 0), &a2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 1), &b2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );


    error = diet_call_async(profile, &reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_call_async(profile2, &reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_or(reqID, 2, &orReqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(orReqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_all() ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    error = diet_profile_free(profile2);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    delete [] reqID;
}



BOOST_AUTO_TEST_CASE( simple_async_client_any)
{
    BOOST_TEST_MESSAGE( "-- Test: simple_async_client_any" );

    double a = 1;
    double b = 2;
    double a2 = 1;
    double b2 = 2;
    double *res = NULL;
    diet_reqID_t *reqID = new diet_reqID_t[2];
    diet_reqID_t orReqID;
    diet_profile_t *profile, *profile2;
    
    utils::ClientArgs c("simple_sync_client", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );
    profile2 = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 0), &a2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 1), &b2, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile2, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );


    error = diet_call_async(profile, &reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_call_async(profile2, &reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_any(&orReqID) ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(orReqID);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_wait_all() ;
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[0]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    error = diet_probe(reqID[1]);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    error = diet_profile_free(profile2);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    delete [] reqID;
}


BOOST_AUTO_TEST_SUITE_END()
