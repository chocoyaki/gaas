/*
 * DIETAllVectorsDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out data of each vectors type
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

static const unsigned int NB_PB_VECTOR = 6;
static const char *PB_VECTOR[NB_PB_VECTOR] =
{"CVADD", "BVADD", "IVADD", "LVADD", "FVADD", "DVADD"};


BOOST_FIXTURE_TEST_SUITE(all_vectors_data_transfers, AllDataTransferSeDFixture)

BOOST_AUTO_TEST_CASE(vectors_characters) {
  BOOST_TEST_MESSAGE("-- Test: vectors_characters");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("vectors_characters", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* Characters */
  profile = diet_profile_alloc(PB_VECTOR[0], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_CHAR, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_CHAR, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_CHAR, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (long **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}


BOOST_AUTO_TEST_CASE(vectors_short) {
  BOOST_TEST_MESSAGE("-- Test: vectors_short");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("vectors_short", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* short */
  profile = diet_profile_alloc(PB_VECTOR[1], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_SHORT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_SHORT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_SHORT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (long **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}


BOOST_AUTO_TEST_CASE(vectors_integer) {
  BOOST_TEST_MESSAGE("-- Test: vectors_integer");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("vectors_integer", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* integer */
  profile = diet_profile_alloc(PB_VECTOR[2], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_INT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_INT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_INT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (long **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}




BOOST_AUTO_TEST_CASE(vectors_long) {
  BOOST_TEST_MESSAGE("-- Test: vectors_long");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("vectors_long", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* long */
  profile = diet_profile_alloc(PB_VECTOR[3], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_LONGINT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_LONGINT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_LONGINT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (long **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}



BOOST_AUTO_TEST_CASE(vectors_float) {
  BOOST_TEST_MESSAGE("-- Test: vectors_float");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  A = (float *) malloc(sizeof(float) * SIZE);
  B = (float *) malloc(sizeof(float) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((float *) A)[i] = i * 2.0;
    ((float *) B)[i] = (i + SIZE) * 2.0;
  }

  utils::ClientArgs c("vectors_float", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* Float */
  profile = diet_profile_alloc(PB_VECTOR[4], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (float *) A, DIET_VOLATILE, DIET_FLOAT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (float *) B, DIET_VOLATILE, DIET_FLOAT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_FLOAT, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (float **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
}


BOOST_AUTO_TEST_CASE(vectors_double) {
  BOOST_TEST_MESSAGE("-- Test: vectors_double");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  A = (double *) malloc(sizeof(double) * SIZE);
  B = (double *) malloc(sizeof(double) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((double *) A)[i] = i * 2.0;
    ((double *) B)[i] = (i + SIZE) * 2.0;
  }

  utils::ClientArgs c("vectors_double", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* double */
  profile = diet_profile_alloc(PB_VECTOR[5], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_vector_set(diet_parameter(profile, 0),
                          (double *) A, DIET_VOLATILE, DIET_DOUBLE, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 1),
                          (double *) B, DIET_VOLATILE, DIET_DOUBLE, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_vector_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_DOUBLE, n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_vector_get(diet_parameter(profile, 2), (double **) &C, NULL, &n);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_free_data(diet_parameter(profile, 2));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  free(C);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

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
