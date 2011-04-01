/*
 * DIETDynamicsDisconnectTests.cpp
 *
 *
 */

#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>
#include <omniORB4/CORBA.h>

#include "SeD.hh"
#include "LocalAgent.hh"
#include "ORBMgr.hh"

#include "fixtures.hpp"
#include "utils.hpp"

using namespace std;


BOOST_FIXTURE_TEST_SUITE( DynamicDisconnect, 
			  DynamicAddRemoveServiceSeDFixture )




#include "dynamicOperations.hpp"


/* Disconnect */
BOOST_AUTO_TEST_CASE( dynamic_hierarchy_disconnect1 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_disconnect1" );
  int res;

  // First SeD, then LA

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disonnect SeD1 from LA1" );
  res = dynamicOperation("SED", "SeD1", DISCONNECT);
  BOOST_CHECK_EQUAL( 0, res );

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disonnect LA1 from MA1" );
  res = dynamicOperation("AGENT", "LA1", DISCONNECT);
  BOOST_CHECK_EQUAL( 0, res );
}

BOOST_AUTO_TEST_CASE( dynamic_hierarchy_disconnect2 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_disconnect2" );
  int res;

  // First LA, then SeD

  // Disconnect LA from MA
  BOOST_TEST_MESSAGE( "-- Disonnect LA1 from MA1" );
  res = dynamicOperation("AGENT", "LA1", DISCONNECT);
  BOOST_CHECK_EQUAL( 0, res );

  // Disconnect SeD from LA
  BOOST_TEST_MESSAGE( "-- Disonnect SeD from LA1" );
  res = dynamicOperation("SED", "SeD1", DISCONNECT);
  BOOST_CHECK_EQUAL( 0, res );
}


BOOST_AUTO_TEST_SUITE_END()
