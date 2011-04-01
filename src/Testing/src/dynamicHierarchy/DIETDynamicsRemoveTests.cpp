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


BOOST_FIXTURE_TEST_SUITE( DynamicRemove, 
			  DynamicAddRemoveServiceSeDFixture )




#include "dynamicOperations.hpp"


/* Remove */
BOOST_AUTO_TEST_CASE( dynamic_hierarchy_remove1 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_remove1" );
  int res;

  // No recursive operation

  // Remove SeD
  BOOST_TEST_MESSAGE( "-- Remove SeD" );
  res = dynamicOperation("SED", "SeD1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove LA
  BOOST_TEST_MESSAGE( "-- Remove LA1 (not recursive)" );
  res = dynamicOperation("AGENT", "LA1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove MA
  BOOST_TEST_MESSAGE( "-- Remove MA1 (not recursive)" );
  res = dynamicOperation("AGENT", "MA1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );
}

BOOST_AUTO_TEST_CASE( dynamic_hierarchy_remove2 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_remove2" );
  int res;

  // No recursive operation

  // Remove MA
  BOOST_TEST_MESSAGE( "-- Remove MA1 (not recursive)" );
  res = dynamicOperation("AGENT", "MA1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove LA
  BOOST_TEST_MESSAGE( "-- Remove LA1 (not recursive)" );
  res = dynamicOperation("AGENT", "LA1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove SeD
  BOOST_TEST_MESSAGE( "-- Remove SeD1" );
  res = dynamicOperation("SED", "SeD1", REMOVE);
  BOOST_CHECK_EQUAL( 0, res );
}

BOOST_AUTO_TEST_CASE( dynamic_hierarchy_remove3 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_remove3" );
  int res;

  // Recursive operation

  // Remove SeD
  BOOST_TEST_MESSAGE( "-- Remove SeD1" );
  res = dynamicOperation("SED", "SeD1", REMOVE, "", true);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove LA
  BOOST_TEST_MESSAGE( "-- Remove LA1 (recursive)" );
  res = dynamicOperation("AGENT", "LA1", REMOVE, "", true);
  BOOST_CHECK_EQUAL( 0, res );

  // Remove MA
  BOOST_TEST_MESSAGE( "-- Remove MA1 (recursive)" );
  res = dynamicOperation("AGENT", "MA1", REMOVE, "", true);
  BOOST_CHECK_EQUAL( 0, res );
}

BOOST_AUTO_TEST_CASE( dynamic_hierarchy_remove4 ) {
  BOOST_TEST_MESSAGE( "-- Test: dynamic_hierarchy_remove4" );
  int res;

  // Recursive operation

  // Remove MA
  BOOST_TEST_MESSAGE( "-- Remove MA1 (recursive)" );
  res = dynamicOperation("AGENT", "MA1", REMOVE, "", true);
  BOOST_CHECK_EQUAL( 0, res );

}

BOOST_AUTO_TEST_SUITE_END()
