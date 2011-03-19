/*
 * DIETDataInoutSinkFilesTests.cpp
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


BOOST_FIXTURE_TEST_SUITE( workflows_data_inout_files_sink, AllDataTransferSeDFixtureWF )

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE( data_inout_sink_files ) {
  BOOST_TEST_MESSAGE( "-- Test: workflow data inout files sink" );

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_FTRANSFER.xml",
                              "Inout Scalars Sink files",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_files.xml");
}

BOOST_AUTO_TEST_SUITE_END()
