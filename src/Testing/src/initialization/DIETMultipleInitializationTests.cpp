/*
 * DIETMultipleInitializationTests.cpp
 *
 * call initialize/finalize multiple times and 
 * make DIET crash badly
 *
 * Created on: 27 december 2010
 * Author: hguemar
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( multiple_initialization_finalization, DietAgentFixture )


BOOST_AUTO_TEST_CASE( successive_diet_finalize_calls )
{
    BOOST_TEST_MESSAGE( "-- Test: successive_diet_finalize_calls " );

    utils::ClientArgs c("successive_diet_finalize_calls", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

    // check if diet_initialize don't return any error
    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_initialize() should return "
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );

    // we finalize our DIET environment once
    error = diet_finalize();

    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_finalize() should return " 
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );

    // now we're good
    for( int i = 0; i < 5; ++i ) {
    	error = diet_finalize();
	
    	BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
    			       "diet_finalize() should return " 
    			       << diet_error_string(GRPC_NO_ERROR)
    			       << " instead of "
    			       << diet_error_string(error) );
    }
}

BOOST_AUTO_TEST_CASE( successive_diet_initialize_calls )
{
    BOOST_TEST_MESSAGE( "-- Test: successive_diet_initialize_calls " );
    
    utils::ClientArgs c("successive_diet_initialize_calls", "client_testing.cfg");
    
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
