/*
 * DIETLoopContainersTests.cpp
 *
 * TODO : description
 *
 * Created on: 15 march 2011
 * Author: bdepardo
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"


BOOST_FIXTURE_TEST_SUITE(workflows_loop_containers_sink,
                         AllDataTransferSeDFixtureWF)

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE(loop_container_character) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop containers character");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_container_LCADD.xml",
                              "Loop Containers character",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_loop_container.xml");
}


BOOST_AUTO_TEST_SUITE_END()
