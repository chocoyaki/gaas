/*
 * DIETAllScalarsDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out data of each scalar type
 *
 * Created on: 01 march 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


static const unsigned int NB_PB = 6;
static const char* PB[NB_PB] =
  {"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};


BOOST_FIXTURE_TEST_SUITE( all_scalars_data_transfers, AllDataTransferSeDFixture )

BOOST_AUTO_TEST_CASE( scalars_characters ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_characters" );
  diet_profile_t* profile = NULL;
  
  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_characters", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

   /* Characters */
  l1 = (char) 0x1;
  l2 = (char) 0x2;
  profile = diet_profile_alloc(PB[0], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_CHAR);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_CHAR);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_CHAR);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  // BOOST_CHECK_EQUAL( *pl3, 3);
  

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


BOOST_AUTO_TEST_CASE( scalars_short ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_short" );
  diet_profile_t* profile = NULL;
  
  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_short", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* short */
  l1 = (short) 0x11;
  l2 = (short) 0x22;
  profile = diet_profile_alloc(PB[1], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_SHORT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_SHORT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_SHORT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  //  BOOST_CHECK_EQUAL( *pl3, 3);
  

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

BOOST_AUTO_TEST_CASE( scalars_integer ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_integer" );
  diet_profile_t* profile = NULL;
  
  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_integer", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* integer */
  l1 = (int) 0x1111;
  l2 = (int) 0x2222;
  profile = diet_profile_alloc(PB[2], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_INT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_INT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_INT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  //  BOOST_CHECK_EQUAL( *pl3, 3);
  

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

BOOST_AUTO_TEST_CASE( scalars_long ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_long" );
  diet_profile_t* profile = NULL;
  
  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_long", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* long */
  l1 = 0x11111111;
  l2 = 0x22222222;
  profile = diet_profile_alloc(PB[3], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_LONGINT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_LONGINT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_LONGINT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  //  BOOST_CHECK_EQUAL( *pl3, 3);
  

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


BOOST_AUTO_TEST_CASE( scalars_float ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_float" );
  diet_profile_t* profile = NULL;
  
  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_float", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* float */
  f1 = 1.1e38;
  f2 = 2.2e38;
  profile = diet_profile_alloc(PB[4], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &f1,  DIET_VOLATILE, DIET_FLOAT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &f2,  DIET_VOLATILE, DIET_FLOAT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_FLOAT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pf3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  //  BOOST_CHECK_EQUAL( *pf3, 3);
  

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

BOOST_AUTO_TEST_CASE( scalars_double ) {
  BOOST_TEST_MESSAGE( "-- Test: scalars_double" );
  
  diet_profile_t* profile = NULL;

  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float    f1 = 0.0;
  float    f2 = 0.0;
  float*  pf3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;

  utils::ClientArgs c("scalars_double", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* double */
  d1 = 1.1e307;
  d2 = 2.2e307;
  profile = diet_profile_alloc(PB[5], 0, 1, 2);
  BOOST_CHECK( profile != NULL );
  error = diet_scalar_set(diet_parameter(profile,0), &d1,  DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,1), &d2,  DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_scalar_get(diet_parameter(profile,2), &pd3, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  //  BOOST_CHECK_EQUAL( *pd3, 3);
  

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
