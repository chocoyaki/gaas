/*
 * DIETDataInoutSinkVectors.cpp
 *
 * TODO : description
 *
 * Created on: 19 march 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE( workflows_data_inout_vectors_sink, AllDataTransferSeDFixtureWF )

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_character ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink character" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_CVADD.xml",
                              "Inout Vectors Sink character",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}

BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_short ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink short" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_BVADD.xml",
                              "Inout Vectors Sink short",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}

BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_integer ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink integer" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_IVADD.xml",
                              "Inout Vectors Sink integer",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}

BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_long ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink long" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_LVADD.xml",
                              "Inout Vectors Sink long",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}

BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_float ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink float" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_FVADD.xml",
                              "Inout Vectors Sink float",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}

BOOST_AUTO_TEST_CASE( data_inout_sink_vectors_double ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout vectors sink double" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_DVADD.xml",
                              "Inout Vectors Sink double",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_vectors.xml");
}


BOOST_AUTO_TEST_SUITE_END()
