/*
 * DIETConfigTests.cpp
 *
 *
 */

#include <boost/scoped_ptr.hpp>

#include "configuration.hh"



BOOST_AUTO_TEST_CASE( config_test)
{
    BOOST_TEST_MESSAGE( "-- Test: config_test" );

    double a = 1;
    double b = 2;
    double *res = NULL;

    diet_profile_t *profile;

    utils::ClientArgs c("simple_client", "client_testing.cfg");

    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error );
    
    profile = diet_profile_alloc("simple_add", 1, 1, 2);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );
    error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    
    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
    
	
}

BOOST_AUTO_TEST_SUITE_END()
