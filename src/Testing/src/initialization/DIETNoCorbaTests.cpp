/*
 *
 * Created on: 27 december 2010
 * Author: hguemar
 *
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"

BOOST_AUTO_TEST_CASE( no_corba_service ) {
    BOOST_TEST_MESSAGE( "-- Test: no_corba_service" );
    
    utils::ClientArgs c("no_corba_service", "client_testing.cfg");
    
    diet_error_t error;
    // TODO: add BOOST_CHECK_NO_THROW
    // C API users should NEVER have to face C++ exceptions
    try {
	error = diet_initialize(c.config(), c.argc(), c.argv());
    } catch(...) {}
    
    // check if diet_initialize return GRPC_NOT_INITIALIZED
    BOOST_CHECK_MESSAGE( GRPC_NOT_INITIALIZED == error,
    			   "diet_initialize() should return "
    			   << diet_error_string(GRPC_NOT_INITIALIZED)
    			   << " instead of "
    			   << diet_error_string(error) );
    
    error = diet_finalize();
    
    BOOST_CHECK_MESSAGE( GRPC_NO_ERROR == error,
    			   "diet_finalize() should return " 
    			   << diet_error_string(GRPC_NO_ERROR)
    			   << " instead of "
    			   << diet_error_string(error) );
}
