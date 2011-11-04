/*
 * DIETDataInoutStringsSinkTests.cpp
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


BOOST_FIXTURE_TEST_SUITE(workflows_data_inout_pstrings_sink,
                         AllDataTransferSeDFixtureWF)

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE(data_inout_sink_pstrings) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout paramstrings sink");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_PSPRINT.xml",
                              "Inout Scalars Sink paramstrings",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_strings.xml");
}

BOOST_AUTO_TEST_SUITE_END()
