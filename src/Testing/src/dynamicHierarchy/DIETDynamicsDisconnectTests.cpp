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


BOOST_FIXTURE_TEST_SUITE( DynamicDisconnect, 
			  DynamicAddRemoveServiceSeDFixture )





/* Disconnect */
BOOST_AUTO_TEST_CASE( dynamic_hierarchy_disconnect1 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_disconnect1" );
  int res;

  // First SeD, then LA

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disonnect SeD1 from LA1" );
  res = diet_disconnect_from_hierarchy(SED, "SeD1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disonnect LA1 from MA1" );
  res = diet_disconnect_from_hierarchy(LA, "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}

BOOST_AUTO_TEST_CASE( dynamic_hierarchy_disconnect2 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_disconnect2" );
  int res;

  // First LA, then SeD

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disonnect LA1 from MA1" );
  res = diet_disconnect_from_hierarchy(LA, "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disonnect SeD from LA1" );
  res = diet_disconnect_from_hierarchy(SED, "SeD1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}


BOOST_AUTO_TEST_SUITE_END()
