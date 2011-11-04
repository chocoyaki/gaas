/*
 * DIETFilesDataTransfers.cpp
 *
 * call a service with 1 in, 1 inout and 1 out files
 *
 * Created on: 01 march 2011
 * Author: bdepardo
 */

#include <cstring>
#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


static const unsigned int NB_PB_FILE = 1;
static const char *PB_FILE[NB_PB_FILE] =
{"FTRANSFER"};


BOOST_FIXTURE_TEST_SUITE(files_data_transfers, AllDataTransferSeDFixture)

BOOST_AUTO_TEST_CASE(files) {
  BOOST_TEST_MESSAGE("-- Test: files");
  diet_profile_t *profile = NULL;
  unsigned int i;

  size_t arg_size = 0;
  struct stat buf;
  int status = 0;
  char *s1;
  char *s2;
  char *s3;

  s1 = strdup(TRANSFER_FILE_TEST1);
  s2 = strdup(TRANSFER_FILE_TEST2);


  utils::ClientArgs c("files", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /* Characters: no choice it has to be DIET_CHAR */
  profile = diet_profile_alloc(PB_FILE[0], 0, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_file_set(diet_parameter(profile, 0), DIET_PERSISTENT, s1);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_file_set(diet_parameter(profile, 1), DIET_VOLATILE, s2);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_file_set(diet_parameter(profile, 2), DIET_VOLATILE, NULL);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_file_get(diet_parameter(profile, 2), NULL, &arg_size, &s3);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  status = stat(s2, &buf);
  BOOST_CHECK_EQUAL(0, status);
  BOOST_CHECK(buf.st_mode & S_IFREG);

  status = stat(s3, &buf);
  BOOST_CHECK_EQUAL(0, status);
  BOOST_CHECK(buf.st_mode & S_IFREG);

  error = diet_free_data(diet_parameter(profile, 1));
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  // error = diet_free_data(diet_parameter(profile, 2));
  // BOOST_CHECK_EQUAL( GRPC_NO_ERROR, error);
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
