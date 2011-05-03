/*
 * DIETBadInitializationTests.cpp
 *
 * Feeds DIET with badly formatted config file
 *
 * Created on: 27 december 2010
 * Author: hguemar
 */

#include <string>
#include <sstream>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"

BOOST_FIXTURE_TEST_SUITE( BadInitializationTests, OmniNamesFixture1 )

/* broken comment block */
BOOST_AUTO_TEST_CASE( config_file_parse_error_1 )
{
    BOOST_TEST_MESSAGE( "-- Test: config_file_parse_error_1" );
    utils::ClientArgs c("bad_client_config_file",
		 "client_parse_error_1.cfg");
}

/* option name typo: "TraceLevel" instead of "traceLevel" */
BOOST_AUTO_TEST_CASE( config_file_parse_error_2 )
{
    BOOST_TEST_MESSAGE( "-- Test: config_file_parse_error_2" );
    utils::ClientArgs c("bad_client_config_file",
		 "client_parse_error_2.cfg");
}

BOOST_AUTO_TEST_SUITE_END()
