/*
 * DIETDataInoutSinkContainersTests.cpp
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


BOOST_FIXTURE_TEST_SUITE(workflows_data_inout_containers_sink,
                         AllDataTransferSeDFixtureWF)

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE(data_inout_sink_containers) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout containers sink");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_LCADD.xml",
                              "Inout Scalars Sink containers",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink_containers.xml");
}

BOOST_AUTO_TEST_SUITE_END()
