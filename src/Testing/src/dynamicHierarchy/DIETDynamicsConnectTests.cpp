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




/* Connect */
BOOST_AUTO_TEST_CASE( dynamic_hierarchy_connect ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_connect" );
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


BOOST_AUTO_TEST_SUITE_END()
