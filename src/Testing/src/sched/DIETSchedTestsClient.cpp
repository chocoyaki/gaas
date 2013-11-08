#include <DIET_client.h>
#include <iostream>
#include "fixtures.hpp"
#include "utils.hpp"

BOOST_FIXTURE_TEST_SUITE(SchedClient, SchedSedFixture)

BOOST_AUTO_TEST_CASE(sched_client) {

  BOOST_TEST_MESSAGE("sched_client test: start client");

  double a = 1;
  int b = 2;
  double *res = NULL;

  diet_profile_t *profile;

  utils::ClientArgs c("sched_client", "SchedTestClient.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  profile = diet_profile_alloc("testsched", 1, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_scalar_set(diet_parameter(profile, 0), &a, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL(0, error);
  error = diet_scalar_set(diet_parameter(profile, 1), &b, DIET_VOLATILE, DIET_INT);
  BOOST_CHECK_EQUAL(0, error);
  error = diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL(0, error);
  error = diet_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  error = diet_finalize();
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
}

BOOST_AUTO_TEST_SUITE_END()
