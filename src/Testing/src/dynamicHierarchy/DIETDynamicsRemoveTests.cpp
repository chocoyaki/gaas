/*
 * DIETDynamicsDisconnectTests.cpp
 *
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_admin.h>

#include "fixtures.hpp"
#include "utils.hpp"

using namespace std;


BOOST_FIXTURE_TEST_SUITE(DynamicRemove,
                         DynamicAddRemoveServiceSeDFixture)




/* Remove */
BOOST_AUTO_TEST_CASE(dyn_hier_remove1) {
  BOOST_TEST_MESSAGE("-- Test: dyn_hier_remove1");
  int res;

  // No recursive operation

  // Remove SeD
  BOOST_TEST_MESSAGE("-- Remove SeD");
  res = diet_remove_from_hierarchy(SED, "SeD1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove LA
  BOOST_TEST_MESSAGE("-- Remove LA1 (not recursive)");
  res = diet_remove_from_hierarchy(AGENT, "LA1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove MA
  BOOST_TEST_MESSAGE("-- Remove MA1 (not recursive)");
  res = diet_remove_from_hierarchy(AGENT, "MA1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);
}

BOOST_AUTO_TEST_CASE(dyn_hier_remove2) {
  BOOST_TEST_MESSAGE("-- Test: dyn_hier_remove2");
  int res;

  // No recursive operation

  // Remove MA
  BOOST_TEST_MESSAGE("-- Remove MA1 (not recursive)");
  res = diet_remove_from_hierarchy(MA, "MA1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove LA
  BOOST_TEST_MESSAGE("-- Remove LA1 (not recursive)");
  res = diet_remove_from_hierarchy(LA, "LA1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove SeD
  BOOST_TEST_MESSAGE("-- Remove SeD1");
  res = diet_remove_from_hierarchy(SED, "SeD1", false);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);
}

BOOST_AUTO_TEST_CASE(dyn_hier_remove3) {
  BOOST_TEST_MESSAGE("-- Test: dyn_hier_remove3");
  int res;

  // Recursive operation

  // Remove SeD
  BOOST_TEST_MESSAGE("-- Remove SeD1");
  res = diet_remove_from_hierarchy(SED, "SeD1", true);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove LA
  BOOST_TEST_MESSAGE("-- Remove LA1 (recursive)");
  res = diet_remove_from_hierarchy(AGENT, "LA1", true);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);

  // Remove MA
  BOOST_TEST_MESSAGE("-- Remove MA1 (recursive)");
  res = diet_remove_from_hierarchy(AGENT, "MA1", true);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);
}

BOOST_AUTO_TEST_CASE(dyn_hier_remove4) {
  BOOST_TEST_MESSAGE("-- Test: dyn_hier_remove4");
  int res;

  // Recursive operation

  // Remove MA
  BOOST_TEST_MESSAGE("-- Remove MA1 (recursive)");
  res = diet_remove_from_hierarchy(AGENT, "MA1", true);
  BOOST_CHECK_EQUAL(DIET_NO_ERROR, res);
}

BOOST_AUTO_TEST_CASE(dyn_hier_remove5) {
  BOOST_TEST_MESSAGE("-- Test: dyn_hier_remove5");
  int res;


  // Remove non existing element
  BOOST_TEST_MESSAGE("-- Remove MAx");
  res = diet_remove_from_hierarchy(AGENT, "MAx", true);
  BOOST_CHECK_EQUAL(DIET_COMM_ERROR, res);

  // Remove non existing element
  BOOST_TEST_MESSAGE("-- Remove SeDx");
  res = diet_remove_from_hierarchy(SED, "SeDx", true);
  BOOST_CHECK_EQUAL(DIET_COMM_ERROR, res);
}


BOOST_AUTO_TEST_SUITE_END()
