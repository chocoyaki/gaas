/*
 * DIETAllMatricesDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out data of each Matrix type
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

static const unsigned int NB_PB_MATRIX = 6;
static const char *PB_MATRIX[NB_PB_MATRIX] =
{"CMADD", "BMADD", "IMADD", "LMADD", "FMADD", "DMADD"};


BOOST_FIXTURE_TEST_SUITE(all_matrices_data_transfers, AllDataTransferSeDFixture)

BOOST_AUTO_TEST_CASE(matrices_characters) {
  BOOST_TEST_MESSAGE("-- Test: matrices_characters");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("matrices_characters", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* Characters */
  profile = diet_profile_alloc(PB_MATRIX[0], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_CHAR, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_CHAR, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_CHAR, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (long **) &C, NULL, &m, &n, &oC);
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


BOOST_AUTO_TEST_CASE(matrices_short) {
  BOOST_TEST_MESSAGE("-- Test: matrices_short");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("matrices_short", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* short */
  profile = diet_profile_alloc(PB_MATRIX[1], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_SHORT, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_SHORT, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_SHORT, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (long **) &C, NULL, &m, &n, &oC);
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


BOOST_AUTO_TEST_CASE(matrices_integer) {
  BOOST_TEST_MESSAGE("-- Test: matrices_integer");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("matrices_integer", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* integer */
  profile = diet_profile_alloc(PB_MATRIX[2], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_INT, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_INT, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_INT, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (long **) &C, NULL, &m, &n, &oC);
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



BOOST_AUTO_TEST_CASE(matrices_long) {
  BOOST_TEST_MESSAGE("-- Test: matrices_long");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  /* Use the long type for all "integer" types. */
  A = (long *) malloc(sizeof(long) * SIZE * SIZE);
  B = (long *) malloc(sizeof(long) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((long *) A)[i] = i;
    ((long *) B)[i] = i + SIZE;
  }

  utils::ClientArgs c("matrices_long", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* long */
  profile = diet_profile_alloc(PB_MATRIX[3], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (long *) A, DIET_VOLATILE, DIET_LONGINT, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (long *) B, DIET_VOLATILE, DIET_LONGINT, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_LONGINT, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (long **) &C, NULL, &m, &n, &oC);
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


BOOST_AUTO_TEST_CASE(matrices_float) {
  BOOST_TEST_MESSAGE("-- Test: matrices_float");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  A = (float *) malloc(sizeof(float) * SIZE * SIZE);
  B = (float *) malloc(sizeof(float) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((float *) A)[i] = i * 2.0;
    ((float *) B)[i] = (i + SIZE) * 2.0;
  }

  utils::ClientArgs c("matrices_long", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* float */
  profile = diet_profile_alloc(PB_MATRIX[4], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (float *) A, DIET_VOLATILE, DIET_FLOAT, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (float *) B, DIET_VOLATILE, DIET_FLOAT, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_FLOAT, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (float **) &C, NULL, &m, &n, &oC);
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


BOOST_AUTO_TEST_CASE(matrices_double) {
  BOOST_TEST_MESSAGE("-- Test: matrices_double");
  diet_profile_t *profile = NULL;
  unsigned int i;

  /**
   * Matrix types
   */
  size_t n = SIZE;
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;

  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;


  A = (double *) malloc(sizeof(double) * SIZE * SIZE);
  B = (double *) malloc(sizeof(double) * SIZE * SIZE);
  for (i = 0; i < SIZE * SIZE; ++i) {
    ((double *) A)[i] = i * 2.0;
    ((double *) B)[i] = (i + SIZE) * 2.0;
  }

  utils::ClientArgs c("matrices_long", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* double */
  profile = diet_profile_alloc(PB_MATRIX[5], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_matrix_set(diet_parameter(profile, 0),
                          (double *) A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 1),
                          (double *) B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_matrix_set(diet_parameter(profile, 2),
                          NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_matrix_get(diet_parameter(profile,
                                         2), (double **) &C, NULL, &m, &n, &oC);
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
