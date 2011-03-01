/*
 * ExampleTests.cpp
 *
 *  Created on: 5 august 2010
 *  Author: dloureiro
 */

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"

BOOST_FIXTURE_TEST_CASE( no_client_config_file , DietMAFixture )
{
    BOOST_TEST_MESSAGE( "Test : no_client_config_file" );

    utils::ClientArgs c("no_client_config_file");
    
    diet_error_t error = diet_initialize("", c.argc(), c.argv());

    // check if diet_initialize return correct error code
    BOOST_REQUIRE_MESSAGE( DIET_FILE_IO_ERROR == error,
			   "diet_initialize() should return "
			   << diet_error_string(DIET_FILE_IO_ERROR)
			   << " instead of "
			   << diet_error_string(error)
			   << " when not fed an actual config file" );
    
    // at the moment diet_finalize should return GPRC_NO_ERROR in every case
    error = diet_finalize();

    BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error, 
			   "diet_finalize() should return " 
			   << diet_error_string(GRPC_NO_ERROR)
			   << " instead of "
			   << diet_error_string(error) );
}

