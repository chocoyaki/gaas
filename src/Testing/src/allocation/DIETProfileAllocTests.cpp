/*
 * ExampleTests.cpp
 *
 *  Created on: 5 august 2010
 *  Author: dloureiro
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <DIET_client.h>
#include <DIET_data.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"

BOOST_AUTO_TEST_SUITE(client_config_file_test)
BOOST_GLOBAL_FIXTURE(F);

BOOST_AUTO_TEST_CASE(profile_alloc_test_null_path) {
  std::cout << "Test : profile_alloc_test_null_path" << std::endl;
/*
        diet_profile_t * profile;

        try{
                profile = diet_profile_alloc(NULL,0,0,0);
        }catch(std::runtime_error e)
        {
                std::cout << e.what() << std::endl;
        }
 */
}

BOOST_AUTO_TEST_CASE(profile_alloc_test_empty_path) {
  std::cout << "Test : profile_alloc_test_empty_path" << std::endl;

/**
        diet_profile_t * profile;

                profile = diet_profile_alloc("",0,0,0);
 */
}

BOOST_AUTO_TEST_CASE(profile_alloc_test_zero_zero_zero) {
  std::cout << "Test : profile_alloc_test_zero_zero_zero" << std::endl;
/*
        diet_profile_t * profile;

                profile = diet_profile_alloc("test",0,0,0);
 */
}

BOOST_AUTO_TEST_SUITE_END()

/*
   BOOST_CHECK( add( 2,2 ) == 4 );        // #1 continues on error

   BOOST_REQUIRE( add( 2,2 ) == 4 );      // #2 throws on error

   if( add( 3,2 ) != 4 )
     BOOST_ERROR( "Ouch..." );            // #3 continues on error

   if( add( 2,2 ) != 4 )
    BOOST_FAIL( "Ouch..." );             // #4 throws on error

   if( add( 2,2 ) != 4 ) throw "Ouch..."; // #5 throws on error

   BOOST_CHECK_MESSAGE( add( 2,2 ) == 4,  // #6 continues on error
    "add(..) result: " << add( 2,2 ) );
   BOOST_CHECK_EQUAL( add( 2,2 ), 4 );      // #7 continues on error
 */
