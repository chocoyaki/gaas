/*
 * DIETDataInoutScalarsSinkTests.cpp
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


BOOST_FIXTURE_TEST_SUITE(workflows_data_inout_scalars_sink,
                         AllDataTransferSeDFixtureWF)

#include "genericSimpleWfClientTest.hpp"


BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_character) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink character");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_CADD.xml",
                              "Inout Scalars Sink character",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_short) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink short");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_BADD.xml",
                              "Inout Scalars Sink short",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_integer) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink integer");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_IADD.xml",
                              "Inout Scalars Sink integer",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_long) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink long");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_LADD.xml",
                              "Inout Scalars Sink long",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_float) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink float");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_FADD.xml",
                              "Inout Scalars Sink float",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}

BOOST_AUTO_TEST_CASE(data_inout_sink_scalars_double) {
  BOOST_TEST_MESSAGE("-- Test: workflow data inout scalars sink double");

  genericSimpleWorkflowClient(WF_DIRECTORY "inout_sink_DADD.xml",
                              "Inout Scalars Sink double",
                              DIET_WF_FUNCTIONAL,
                              WF_DIRECTORY "data_inout_sink.xml");
}


BOOST_AUTO_TEST_SUITE_END()
