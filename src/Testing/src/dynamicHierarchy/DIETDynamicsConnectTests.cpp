/*
 * DIETDynamicsConnectTests.cpp
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


BOOST_FIXTURE_TEST_SUITE( ServiceAddRemoveTest, 
			  DynamicAddRemoveServiceSeDFixture )




#include "dynamicOperations.hpp"

/* Connect */
BOOST_AUTO_TEST_CASE( dynamic_hierarchy_connect ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_connect" );
  int res;

  // Connect SeD to MA
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to MA1" );
  res = dynamicOperation("SED", "SeD1", CONNECT, "MA1");
  BOOST_CHECK_EQUAL( 0, res );

  // Connect SeD to LA
  BOOST_TEST_MESSAGE( "-- Connect SeD1 to LA1" );
  res = dynamicOperation("SED", "SeD1", CONNECT, "LA1");
  BOOST_CHECK_EQUAL( 0, res );
}


BOOST_AUTO_TEST_SUITE_END()
