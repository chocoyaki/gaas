/*
 * DIETStringsDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out string
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


const unsigned int SIZE = 3;

static const unsigned int NB_PB_STRING = 1;
static const char *PB_STRING[NB_PB_STRING] =
{"SPRINT"};


BOOST_FIXTURE_TEST_SUITE(strings_data_transfers, AllDataTransferSeDFixture)

BOOST_AUTO_TEST_CASE(strings) {
  BOOST_TEST_MESSAGE("-- Test: strings");
  diet_profile_t *profile = NULL;
  unsigned int i;


  char *s1;
  char *s2;
  char *s3;

  s1 = (char *) malloc(sizeof(char) * (SIZE + 1));
  s2 = (char *) malloc(sizeof(char) * (SIZE + 1));
  for (i = 0; i < SIZE; ++i) {
    s1[i] = 'a';
    s2[i] = 'b';
  }
  s1[SIZE] = '\0';
  s2[SIZE] = '\0';


  utils::ClientArgs c("strings", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* Characters: no choice it has to be DIET_CHAR */
  profile = diet_profile_alloc(PB_STRING[0], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_string_set(diet_parameter(profile, 0), s1, DIET_VOLATILE);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_string_set(diet_parameter(profile, 1), s2, DIET_VOLATILE);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_string_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_string_get(diet_parameter(profile, 2), &s3, NULL);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(s3);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(s1);
  free(s2);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}


BOOST_AUTO_TEST_SUITE_END()
