/*
 * DIETServiceAddRemove.cpp
 *
 * Stupid client that calls two services which add/remove services in a SeD
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( ServiceAddRemoveTest, 
			  DynamicAddRemoveServiceSeDFixture )

BOOST_AUTO_TEST_CASE( service_add_remove) {
    BOOST_TEST_MESSAGE( "-- Test: service_add_remove" );

    diet_profile_t *profile;
    std::string service;
    int a;
    
    utils::ClientArgs c("service_add_remove", "client_testing.cfg");
    
    diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
    

    profile = diet_profile_alloc("dyn_add_rem_0", 0, 0, 0);
    BOOST_CHECK( profile != NULL );

    a = 2;
    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_INT);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_call(profile);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
   
    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);



    profile = diet_profile_alloc("dyn_add_rem_1", 0, 0, 0);
    BOOST_CHECK( profile != NULL );

    error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_INT);
    BOOST_CHECK_EQUAL( 0, error );

    error = diet_call(profile);
    BOOST_REQUIRE_EQUAL( GRPC_NO_ERROR, error );
   
    error = diet_profile_free(profile);
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);


    error = diet_finalize();
    BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
}

BOOST_AUTO_TEST_SUITE_END()
