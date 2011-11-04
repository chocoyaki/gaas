/*
 * DIETErrorStringTests.cpp
 *
 * Checks that diet_error_string works fine
 *
 * Created on: 29 december 2010
 * Author: hguemar
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"

BOOST_AUTO_TEST_SUITE(error_string_tests)
using boost::test_tools::output_test_stream;

/* we're nice and start with positive integer */
BOOST_AUTO_TEST_CASE(error_string_test1) {
  BOOST_TEST_MESSAGE("Test: error_string_test1");

  output_test_stream out;

  /* every error code in DIET should be associated to
     a proper error string */
  for (int i = 0; i < GRPC_UNKNOWN_ERROR_CODE; ++i) {
    out << diet_error_string(i);
    /* set flush_stream to false so i can do further tests
       on stream content */
    BOOST_CHECK(!out.is_equal("GRPC_UNKNOWN_ERROR_CODE", false)
                && !out.is_empty());
  }

  /* GridRPC standard */
  out << diet_error_string(GRPC_UNKNOWN_ERROR_CODE);
  BOOST_CHECK(out.is_equal("GRPC_UNKNOWN_ERROR_CODE"));

  /* other valid error codes */
  for (int i = GRPC_UNKNOWN_ERROR_CODE + 1; i <= DIET_LAST_ERROR_CODE; ++i) {
    out << diet_error_string(i);
    BOOST_CHECK(!out.is_equal("GRPC_UNKNOWN_ERROR_CODE", false)
                && !out.is_empty());
  }

  /* invalid error codes */
  for (int i = DIET_LAST_ERROR_CODE + 1; i < 100; ++i) {
    out << diet_error_string(i);
    BOOST_CHECK(out.is_equal("GRPC_UNKNOWN_ERROR_CODE"));
  }

  for (int i = -100; i < 0; ++i) {
    out << diet_error_string(i);
    BOOST_CHECK(out.is_equal("GRPC_UNKNOWN_ERROR_CODE"));
  }
}

/* let's get naughty */
BOOST_AUTO_TEST_SUITE_END()
