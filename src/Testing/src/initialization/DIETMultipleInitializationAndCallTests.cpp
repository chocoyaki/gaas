/*
 * DIETMultipleInitializationAndCallTests.cpp
 *
 * call initialize multiple times and call a service
 *
 * Created on: 24 February 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( multiple_initialization_and_call, SimpleAddSeDFixture )

BOOST_AUTO_TEST_CASE( successive_diet_initialize_calls_and_diet_call )
{
    BOOST_TEST_MESSAGE( "-- Test: successive_diet_initialize_calls_and_diet_call " );
    
    utils::ClientArgs c("successive_diet_initialize_calls_and_diet_call", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

    // check if diet_initialize don't return any error
    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
			   "diet_initialize() should return "
			   << diet_error_string(GRPC_NO_ERROR)
			   << " instead of "
			   << diet_error_string(error) );

    // just to be sure
    BOOST_TEST_MESSAGE( "-- Now calling diet_initialize 5 times" );
    for(int i = 0; i < 5; ++i) {
	error = diet_initialize(c.config(), c.argc(), c.argv());
	
	// DIET is already initialized
	BOOST_REQUIRE_MESSAGE( GRPC_ALREADY_INITIALIZED == error,
			       "diet_initialize() should return "
			       << diet_error_string(GRPC_ALREADY_INITIALIZED)
			       << " instead of "
			       << diet_error_string(error) );
    }
    
    BOOST_TEST_MESSAGE( "-- Now calling diet_finalize " );
    error = diet_finalize();

    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_finalize() should return " 
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );

    BOOST_TEST_MESSAGE( "-- Now calling diet_initialize one more time " );
    error = diet_initialize(c.config(), c.argc(), c.argv());
    
    // DIET should gracefully initialize now
    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_initialize() should return "
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );

    // Now calling a real service
    double a = 1;
    double b = 2;
    double *res = NULL;
    diet_profile_t *profile;

    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    // check that profile is not 0

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_call(profile);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
   
    error = diet_scalar_get(diet_parameter(profile, 2), &res, NULL);
    BOOST_CHECK_EQUAL( 0, error );
    BOOST_CHECK_EQUAL( 3, *res );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

    // now we're good
    BOOST_TEST_MESSAGE( "-- Now calling diet_finalize " );
    diet_finalize();

    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_finalize() should return " 
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );
}

BOOST_AUTO_TEST_SUITE_END()
