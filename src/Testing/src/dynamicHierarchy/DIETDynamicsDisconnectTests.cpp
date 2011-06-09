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
BOOST_AUTO_TEST_CASE( dyn_hier_disconnect1 ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_disconnect1" );
  int res;

  // First SeD, then LA

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disconnect SeD1 from LA1" );
  res = diet_disconnect_from_hierarchy(SED, "SeD1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disconnect LA1 from MA1" );
  res = diet_disconnect_from_hierarchy(LA, "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}

BOOST_AUTO_TEST_CASE( dyn_hier_disconnect2 ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_disconnect2" );
  int res;

  // First LA, then SeD

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disconnect LA1 from MA1" );
  res = diet_disconnect_from_hierarchy(LA, "LA1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disconnect SeD from LA1" );
  res = diet_disconnect_from_hierarchy(SED, "SeD1");
  BOOST_CHECK_EQUAL( DIET_NO_ERROR, res );
}

BOOST_AUTO_TEST_CASE( dyn_hier_disconnect3 ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_disconnect3" );
  int res;

  // Disconnect non existent LA
  BOOST_TEST_MESSAGE( "-- Disconnect LAx" );
  res = diet_disconnect_from_hierarchy(LA, "LAx");
  BOOST_CHECK_EQUAL( DIET_COMM_ERROR, res );

  // Disconnect non existent SeD
  BOOST_TEST_MESSAGE( "-- Disconnect SeDx" );
  res = diet_disconnect_from_hierarchy(SED, "SeDx");
  BOOST_CHECK_EQUAL( DIET_COMM_ERROR, res );
}


BOOST_AUTO_TEST_CASE( dyn_hier_disconnect4 ) {
  BOOST_TEST_MESSAGE( "-- Test: dyn_hier_disconnect4" );
  int res;

  // Try to disconnect an element other than LA and SeD
  BOOST_TEST_MESSAGE( "-- Disconnect MA" );
  res = diet_disconnect_from_hierarchy(MA, "LA1");
  BOOST_CHECK_EQUAL( DIET_UNKNOWN_ERROR, res );

}


BOOST_AUTO_TEST_SUITE_END()
