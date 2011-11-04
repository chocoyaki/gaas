/*
 * DIETHandleAllocTests.cpp
 *
 *
 *
 * Created on: 29 december 2010
 * Author: hguemar
 */

#include <boost/test/unit_test.hpp>
#include <DIET_client.h>

#include "fixtures.hpp"

BOOST_AUTO_TEST_SUITE(data_handle_alloc_tests)

BOOST_AUTO_TEST_CASE(simple_alloc_free_test1) {
  BOOST_TEST_MESSAGE("Test: simple_alloc_free_test1");

  diet_data_handle_t *handle = diet_data_handle_malloc();

  BOOST_CHECK_NE(handle, static_cast<diet_data_handle_t *>(0));

  diet_error_t error = diet_free(handle);

  BOOST_CHECK_EQUAL(error, GRPC_NO_ERROR);
}

BOOST_AUTO_TEST_CASE(multiple_free_test) {
  BOOST_TEST_MESSAGE("Test: multiple_free_test");

  diet_data_handle_t *handle = diet_data_handle_malloc();

  BOOST_CHECK_NE(handle, static_cast<diet_data_handle_t *>(0));

  /* first free: everything should be ok */
  diet_error_t error = diet_free(handle);
  handle = 0;

  /* BANG: already free'ed */
  try {
    error = diet_free(handle);
  } catch (...) {
  }
}

BOOST_AUTO_TEST_SUITE_END()
