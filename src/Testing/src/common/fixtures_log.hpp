#ifndef FIXTURES_LOG_HPP_
#define FIXTURES_LOG_HPP_

#include "fixtures.hpp"

#ifdef USE_LOG_SERVICE
template <const char *config, const char *omniORBConfig, class parentFixture>
class LogServiceFixture : public parentFixture {
  boost::scoped_ptr<bp::child> process;

public:
  LogServiceFixture() : process(NULL) {
    BOOST_TEST_MESSAGE( "== Test setup [BEGIN]: Launching LogService ==" );
	
    std::string exec;
    try {
      exec = bp::find_executable_in_path(LOGSERVICE_COMMAND, LOGSERVICE_PATH);
    } catch (bs::system_error& e) {
      BOOST_TEST_MESSAGE( "can't find " << LOGSERVICE_COMMAND << ": " << e.what() );
      return;
    }

    BOOST_TEST_MESSAGE( LOGSERVICE_COMMAND << " found: " << exec );

    // setup LogService environment
    bp::context ctx;
    ctx.process_name = LOGSERVICE_COMMAND;
    ctx.env["OMNIORB_CONFIG"] = omniORBConfig;
	
    // redirect output to /dev/null
    ctx.streams[bp::stdout_id] = bp::behavior::null();
    ctx.streams[bp::stderr_id] = bp::behavior::null();

    // setup LogService arguments
    std::vector<std::string> args = ba::list_of(std::string("-config"))
      (std::string(config));

    // launch LogService
    bp::child c = bp::create_child(exec, args, ctx);
    process.reset(utils::copy_child(c));
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_TIME));
    BOOST_TEST_MESSAGE( "== Test setup [END]:  Launching LogService ==" );
  }

  ~LogServiceFixture() {
    BOOST_TEST_MESSAGE( "== Test teardown [BEGIN]: Stopping LogService ==" );
    if (process) {
      process->terminate();
      process->wait();
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_TIME));
    BOOST_TEST_MESSAGE( "== Test teardown [END]: Stopping LogService ==" );
  }
};


template <const char *config, const char *omniORBConfig, class parentFixture>
class DIETLogToolFixture : public parentFixture {
  boost::scoped_ptr<bp::child> process;

public:
  DIETLogToolFixture() : process(NULL) {
    BOOST_TEST_MESSAGE( "== Test setup [BEGIN]: Launching DIETLogTool ==" );
	
    std::string exec;
    try {
      exec = bp::find_executable_in_path(DIETLOGTOOL_COMMAND, DIETLOGTOOL_PATH);
    } catch (bs::system_error& e) {
      BOOST_TEST_MESSAGE( "can't find " << DIETLOGTOOL_COMMAND << ": " << e.what() );
      return;
    }

    BOOST_TEST_MESSAGE( DIETLOGTOOL_COMMAND << " found: " << exec );

    // setup LogService environment
    bp::context ctx;
    ctx.process_name = DIETLOGTOOL_COMMAND;
    ctx.env["OMNIORB_CONFIG"] = omniORBConfig;
	
    // redirect output to /dev/null
    ctx.streams[bp::stdout_id] = bp::behavior::null();
    ctx.streams[bp::stderr_id] = bp::behavior::null();

    // setup LogService arguments
    std::vector<std::string> args= ba::list_of("");

    // launch DIETLogTool
    bp::child c = bp::create_child(exec, args, ctx);
    process.reset(utils::copy_child(c));
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_TIME));
    BOOST_TEST_MESSAGE( "== Test setup [END]:  Launching DIETLogTool ==" );
  }

  ~DIETLogToolFixture() {
    BOOST_TEST_MESSAGE( "== Test teardown [BEGIN]: Stopping DIETLogTool ==" );
    if (process) {
      process->terminate();
      process->wait();
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_TIME));
    BOOST_TEST_MESSAGE( "== Test teardown [END]: Stopping DIETLogTool ==" );
  }
};


char LogServiceConfig[] = LOGSERVICE_CONFIG;
typedef DIETLogToolFixture<LogServiceConfig, ConfigOmniORB, 
                           LogServiceFixture<LogServiceConfig, ConfigOmniORB,
                                             omniNamesFixture>
                           > LogServiceFixtureConf;
typedef DietAgentFixture<ConfigMasterAgent, ConfigOmniORB, LogServiceFixtureConf> DietMAFixtureLog;
typedef DietAgentFixture<ConfigLocalAgent, ConfigOmniORB, DietMAFixtureLog> DietLAFixtureLog;
typedef DietMADAGFixture<ConfigMADAG, ConfigOmniORB, DietLAFixtureLog> DietMADAGFixtureLALog;
typedef DietSeDFixture <SimpleAddSeD, SimpleAddSeDBinDir, ConfigSimpleAddSeDLA, ConfigOmniORB, DietLAFixtureLog>SimpleAddSeDFixtureLog;
typedef DietSeDFixture <AllDataTransferAddSeD, AllDataTransferBinDir, ConfigSimpleAddSeDLA, ConfigOmniORB, DietLAFixtureLog>AllDataTransferSeDFixtureLog;
typedef DietSeDFixture <AllDataTransferAddSeD, AllDataTransferBinDir, ConfigSimpleAddSeDLA, ConfigOmniORB, DietMADAGFixtureLALog>AllDataTransferSeDFixtureWFLog;
typedef DietSeDFixture <DynamicAddRemoveServiceAddSeD, DynamicAddRemoveServiceBinDir, ConfigSimpleAddSeDLA, ConfigOmniORB, DietLAFixtureLog>DynamicAddRemoveServiceSeDFixtureLog;
typedef DietSeDFixture <GRPCAddSeD, GRPCBinDir, ConfigSimpleAddSeDLA, ConfigOmniORB, DietLAFixtureLog>GRPCSeDFixtureLog;

#endif // USE_LOG_SERVICE


#endif // FIXTURES_LOG_HPP_
