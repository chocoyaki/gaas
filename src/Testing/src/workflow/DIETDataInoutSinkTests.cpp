/*
 * DIETDataInoutSinkTests.cpp
 *
 * TODO : description
 *
 * Created on: 01 march 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "genericSimpleWfClientTest.hpp"


BOOST_FIXTURE_TEST_SUITE( workflows_data_inout_sink, AllDataTransferSeDFixtureWF )



BOOST_AUTO_TEST_CASE( data_inout_sink_scalars_character ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout scalars sink character" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_CADD.xml",
                              "Inout Scalars Sink character",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}


BOOST_AUTO_TEST_SUITE_END()
