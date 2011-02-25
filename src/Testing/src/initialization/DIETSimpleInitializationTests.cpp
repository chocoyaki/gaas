/*
 * SimpleClientConfigFile.cpp
 *
 *  Created on: 27 december 2010
 *  Author: hguemar
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"

BOOST_FIXTURE_TEST_CASE( simple_client_config_file , DietAgentFixture )
{
    BOOST_TEST_MESSAGE( "Test : simple_client_config_file" );

    utils::ClientArgs c("simple_client_config_file", "client.cfg");

    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    
    // check if diet_initialize don't return any error
    BOOST_CHECK_MESSAGE( GRPC_NO_ERROR == error,
			   "diet_initialize() should return "
			   << diet_error_string(GRPC_NO_ERROR)
			   << " instead of "
			   << diet_error_string(error) );

    error = diet_finalize();

    BOOST_CHECK_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_finalize() should return " 
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );
}
