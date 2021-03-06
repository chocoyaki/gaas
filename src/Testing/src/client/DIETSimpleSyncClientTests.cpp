/*
 * DIETSimpleSyncClientTests.cpp
 *
 * Stupid client that talks with stupid SeD
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE(SimpleSyncClientTest,
                         SimpleAddSeDFixture)

BOOST_AUTO_TEST_CASE(simple_sync_client) {
  BOOST_TEST_MESSAGE("-- Test: simple_sync_client");

  double a = 1;
  double b = 2;
  double *res = NULL;

  diet_profile_t *profile;

  utils::ClientArgs c("simple_sync_client", "client_testing.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());
  BOOST_REQUIRE_EQUAL(GRPC_NO_ERROR, error);

  profile = diet_profile_alloc("simple_add", 1, 1, 2);
  BOOST_CHECK(profile != NULL);

  error = diet_scalar_set(diet_parameter(profile,
                                         0), &a, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL(0, error);
  error = diet_scalar_set(diet_parameter(profile,
                                         1), &b, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL(0, error);
  error = diet_scalar_set(diet_parameter(profile,
                                         2), NULL, DIET_VOLATILE, DIET_DOUBLE);
  BOOST_CHECK_EQUAL(0, error);

  error = diet_call(profile);
  BOOST_REQUIRE_EQUAL(GRPC_NO_ERROR, error);

  error = diet_scalar_get(diet_parameter(profile, 2), &res, NULL);
  BOOST_CHECK_EQUAL(0, error);
  BOOST_CHECK_EQUAL(3, *res);

  error = diet_profile_free(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  error = diet_finalize();
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
}

BOOST_AUTO_TEST_SUITE_END()
