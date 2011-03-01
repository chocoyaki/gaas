/*
 * DIETContainersDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out containers
 *
 * Created on: 01 march 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>
#include <DIET_Dagda.h>

#include "fixtures.hpp"
#include "utils.hpp"


static const unsigned int NB_PB_CONTAINER = 1;
static const char* PB_CONTAINER[NB_PB_CONTAINER] =
  {"LCADD"};


BOOST_FIXTURE_TEST_SUITE( containers_data_transfers, AllDataTransferSeDFixture )

BOOST_AUTO_TEST_CASE( containers ) {
  BOOST_TEST_MESSAGE( "-- Test: containers" );
  diet_profile_t* profile = NULL;
  unsigned int i;
  
  long l1;
  long l2;
  long l3;
  long l4;
  l1 = 1;
  l2 = 2;
  l3 = 3;
  l4 = 4;
  char *p1, *p2, *p3, *p4;
  
  diet_container_t content1;
  long * pl1, *pl2;
  char *s1;
  char *s2;


  utils::ClientArgs c("containers", "client_testing.cfg");
    
  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  
  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE( GRPC_NO_ERROR == error,
                         "diet_initialize() should return "
                         << diet_error_string(GRPC_NO_ERROR)
                         << " instead of "
                         << diet_error_string(error) );
  

  /* Characters: no choice it has to be DIET_CHAR */
  profile = diet_profile_alloc(PB_CONTAINER[0], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  /* IN */
  error = dagda_create_container(&s1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_put_scalar(&l1, DIET_LONGINT, DIET_PERSISTENT, &p1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_add_container_element(s1, p1, 0);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_put_scalar(&l2, DIET_LONGINT, DIET_PERSISTENT, &p2);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_add_container_element(s1, p2, 1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  diet_use_data(diet_parameter(profile,0), s1);

  /* INOUT */
  error = dagda_create_container(&s2);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_put_scalar(&l3, DIET_LONGINT, DIET_PERSISTENT, &p3);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_add_container_element(s2, p3, 0);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_put_scalar(&l4, DIET_LONGINT, DIET_PERSISTENT, &p4);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_add_container_element(s2, p4, 1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  diet_use_data(diet_parameter(profile,1), s2);

  /* OUT */
  error = diet_container_set(diet_parameter(profile,2), DIET_PERSISTENT);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);


  error = diet_call(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  
  s1 = (profile->parameters[1]).desc.id;
  BOOST_CHECK(s1 != NULL);
  error = dagda_get_container(s1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_container_elements(s1, &content1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_scalar(content1.elt_ids[0], &pl1, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_scalar(content1.elt_ids[1], &pl2, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  free(content1.elt_ids);
  
  
  s1 = (profile->parameters[2]).desc.id;
  BOOST_CHECK(s1 != NULL);
  error = dagda_get_container(s1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_container_elements(s1, &content1);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_scalar(content1.elt_ids[0], &pl1, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = dagda_get_scalar(content1.elt_ids[1], &pl2, NULL);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  free(content1.elt_ids);
  
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  
  
  
  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
  free(s1);
  free(s2);
  
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
