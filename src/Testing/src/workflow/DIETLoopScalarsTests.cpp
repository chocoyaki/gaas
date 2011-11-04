/*
 * DIETLoopScalarsTests.cpp
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


BOOST_FIXTURE_TEST_SUITE(workflows_loop_scalars_sink,
                         AllDataTransferSeDFixtureWF)

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE(loop_scalars_character) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars character");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_CADD.xml",
                              "Loop Scalars character",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(loop_scalars_short) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars short");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_BADD.xml",
                              "Loop Scalars short",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(loop_scalars_integer) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars integer");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_IADD.xml",
                              "Loop Scalars integer",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(loop_scalars_long) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars long");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_LADD.xml",
                              "Loop Scalars long",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(loop_scalars_float) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars float");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_FADD.xml",
                              "Loop Scalars float",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(loop_scalars_double) {
  BOOST_TEST_MESSAGE("-- Test: workflow loop scalars double");

  genericSimpleWorkflowClient(WF_DIRECTORY "loop_FADD.xml",
                              "Loop Scalars double",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}


BOOST_AUTO_TEST_SUITE_END()
