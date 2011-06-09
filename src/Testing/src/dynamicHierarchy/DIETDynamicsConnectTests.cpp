/*
 * DIETDynamicsConnectTests.cpp
 *
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_admin.h>

#include "fixtures.hpp"
#include "utils.hpp"

using namespace std;


BOOST_FIXTURE_TEST_SUITE( DynamicConnect, 
			  DynamicAddRemoveServiceSeDFixture )




/* Connect to another agent */
BOOST_AUTO_TEST_CASE( dyn_hier_connect ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect" );
  int res;

  // Connect SeD to MA
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to MA1" );
  res = diet_change_parent(SED, "SeD1", "MA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Connect SeD to LA
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to LA1" );
  res = diet_change_parent(SED, "SeD1", "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}

/* Try to connect an element to the same parent
 * shouldn't return an error
 */
BOOST_AUTO_TEST_CASE( dyn_hier_connect_same ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect_same" );
  int res;

  // Connect LA to MA
  BOOST_TEST_MESSAGE( "-- Connect LA1 to MA1" );
  res = diet_change_parent(LA, "LA1", "MA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Connect SeD to MA
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to LA1" );
  res = diet_change_parent(SED, "SeD1", "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}

/* Try to connect an element that doesn't exist to an existing element */
BOOST_AUTO_TEST_CASE( dyn_hier_connect_absent ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect_absent" );
  int res;

  // Connect SeDx to LA1
  BOOST_TEST_MESSAGE( "-- Connect SeDx to LA1" );
  res = diet_change_parent(SED, "SeDx", "LA1");
  BOOST_CHECK_EQUAL( DIET_COMM_ERROR, res );

  // Connect LAx to MA
  BOOST_TEST_MESSAGE( "-- Connect LAx to MA1" );
  res = diet_change_parent(LA, "LAx", "MA1");
  BOOST_CHECK_EQUAL( DIET_COMM_ERROR, res );
}

/* Try to connect an element that exists to a non existing element */
BOOST_AUTO_TEST_CASE( dyn_hier_connect_absent2 ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect_absent2" );
  int res;

  // Connect SeD1 to LAx
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to LAx" );
  res = diet_change_parent(SED, "SeD1", "LAx");
  BOOST_CHECK_EQUAL( DIET_ADMIN_CALL_ERROR, res );

  // Connect LA1 to MAx
  BOOST_TEST_MESSAGE( "-- Connect LA1 to MAx" );
  res = diet_change_parent(LA, "LA1", "MAx");
  BOOST_CHECK_EQUAL( DIET_ADMIN_CALL_ERROR, res );
}

/* Try to connect an LA to itselft */
BOOST_AUTO_TEST_CASE( dyn_hier_connect_LAloop ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect_LAloop" );
  int res;

  // Create a loop with LA1
  BOOST_TEST_MESSAGE( "-- Connect LA1 to LA1" );
  res = diet_change_parent(LA, "LA1", "LA1");
  BOOST_CHECK_EQUAL( DIET_ADMIN_CALL_ERROR, res );
}


/* Try to connect an MA */
BOOST_AUTO_TEST_CASE( dyn_hier_connect_MA ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_connect_MA" );
  int res;

  // Connect MA1
  BOOST_TEST_MESSAGE( "-- Connect MA1 to MA1" );
  res = diet_change_parent(MA, "MA1", "MA1");
  BOOST_CHECK_EQUAL( DIET_UNKNOWN_ERROR, res );
}


BOOST_AUTO_TEST_SUITE_END()
