#ifndef FIXTURES_FWD_HPP_
#define FIXTURES_FWD_HPP_

#include "fixtures.hpp"
#include "utils.hpp"

#include <unistd.h>
#include <boost/test/execution_monitor.hpp>
#include <csignal>
#include <exception>

void
handler(int sig) {
  kill(0, SIGTERM);
}

template <int timeout, class parentFixture>
class TimeoutFixture: public parentFixture {
boost::execution_monitor monitor;

public:
TimeoutFixture() {
  BOOST_TEST_MESSAGE("== Test setup [BEGIN]:  Launching timeout ==");
  signal(SIGALRM, handler);
  alarm(timeout);
  BOOST_TEST_MESSAGE("== Test setup [END]:  Launching timeout ==");
}

~TimeoutFixture() {
  BOOST_TEST_MESSAGE("== Test setup [BEGIN]:  Ending timeout ==");
  alarm(0);
  BOOST_TEST_MESSAGE("== Test setup [END]:  Ending timeout ==");
}

static int
wait() {
  for (;;) {
    sleep(1);
  }
}
};

template <const char *config, const char *name,
          const char *omniORBConfig, class parentFixture>
class DietForwarderClientFixture: public parentFixture {
boost::scoped_ptr<bp::child> processFwd;

public:
DietForwarderClientFixture(): processFwd(NULL) {
  BOOST_TEST_MESSAGE(
    "== Test setup [BEGIN]:  Launching DIET client forwarder (config file: "
    << config << ") ==");

  std::string exec;
  try {
    exec = bp::find_executable_in_path("dietForwarder", DIETFWD_DIR);
  } catch (bs::system_error &e) {
    BOOST_TEST_MESSAGE("can't find dietForwarder: " << e.what());
    return;
  }

  BOOST_TEST_MESSAGE("dietForwarder found: " << exec);

  // setup forwarder environment
  bp::context ctx;
  ctx.process_name = "dietForwarder";
  bp::environment::iterator i_c;
  i_c = ctx.env.find(ENV_LIBRARY_PATH_NAME);
  if (i_c != ctx.env.end()) {
    i_c->second = std::string(ENV_LIBRARY_PATH) + i_c->second;
  } else {
    ctx.env[ENV_LIBRARY_PATH_NAME] = ENV_LIBRARY_PATH;
  }
  ctx.env["OMNIORB_CONFIG"] = omniORBConfig;

  // redirect output to /dev/null
  ctx.streams[bp::stdout_id] = bp::behavior::null();
  ctx.streams[bp::stderr_id] = bp::behavior::null();


  // setup dietForwarder arguments
  std::vector<std::string> args = ba::list_of("--name") (name);

  // launch dietForwarder
  const bp::child c = bp::create_child(exec, args, ctx);

  processFwd.reset(utils::copy_child(c));
  boost::this_thread::sleep(boost::posix_time::milliseconds(4 * SLEEP_TIME));
  BOOST_TEST_MESSAGE("== Test setup [END]: Launching client dietForwarder ==");
}

~DietForwarderClientFixture() {
  BOOST_TEST_MESSAGE(
    "== Test teardown [BEGIN]: Stopping client dietForwarder ==");
  if (processFwd) {
    processFwd->terminate();
    processFwd->wait();
  }
  bf::remove_all("/tmp/DIET-forwarder-ior-" + std::string(name) + ".tmp");
  boost::this_thread::sleep(boost::posix_time::milliseconds(4 * SLEEP_TIME));
  BOOST_TEST_MESSAGE("== Test teardown [END]: Stopping client dietForwarder ==");
}
};

template <const char *config, const char *name,
          const char *peerName, const char *remoteHost,
          const char *sshHost,
          const char *omniORBConfig, class parentFixture>
class DietForwarderServerFixture: public parentFixture {
boost::scoped_ptr<bp::child> processFwd;
std::string serverName;

public:
DietForwarderServerFixture(): processFwd(NULL),
  serverName(utils::genID(name)) {
  BOOST_TEST_MESSAGE(
    "== Test setup [BEGIN]:  Launching DIET server forwarder (config file: "
    << config << ") ==");

  std::string exec;
  try {
    exec = bp::find_executable_in_path("dietForwarder", DIETFWD_DIR);
  } catch (bs::system_error &e) {
    BOOST_TEST_MESSAGE("can't find dietForwarder: " << e.what());
    return;
  }

  BOOST_TEST_MESSAGE("dietForwarder found: " << exec);

  // setup forwarder environment
  bp::context ctx;
  ctx.process_name = "dietForwarder";
  bp::environment::iterator i_c;
  i_c = ctx.env.find(ENV_LIBRARY_PATH_NAME);
  if (i_c != ctx.env.end()) {
    i_c->second = std::string(ENV_LIBRARY_PATH) + i_c->second;
  } else {
    ctx.env[ENV_LIBRARY_PATH_NAME] = ENV_LIBRARY_PATH;
  }
  ctx.env["OMNIORB_CONFIG"] = omniORBConfig;

  // redirect output to /dev/null
  ctx.streams[bp::stdout_id] = bp::behavior::null();
  ctx.streams[bp::stderr_id] = bp::behavior::null();


  // setup dietForwarder arguments
  std::vector<std::string> args = ba::list_of("--name") (this->serverName.c_str())
                                    ("--peer-name") (peerName)
                                    ("--remote-host") (remoteHost)
                                    ("--ssh-host") (sshHost)
                                    ("--ssh-login") (FWD_SSH_LOGIN);

  // launch dietForwarder
  const bp::child c = bp::create_child(exec, args, ctx);

  processFwd.reset(utils::copy_child(c));

  // Fowarder sleeps 10s before waking up, so we need to sleep at least 10s
  // Let's say 20 to be sure :-)
  boost::this_thread::sleep(boost::posix_time::milliseconds(20 * 1000));
  BOOST_TEST_MESSAGE("== Test setup [END]: Launching server dietForwarder ==");
}

~DietForwarderServerFixture() {
  BOOST_TEST_MESSAGE(
    "== Test teardown [BEGIN]: Stopping server dietForwarder ==");
  if (processFwd) {
    processFwd->terminate();
    processFwd->wait();
  }
  bf::remove_all("/tmp/DIET-forwarder-ior-" + serverName + ".tmp");
  boost::this_thread::sleep(boost::posix_time::milliseconds(4 * SLEEP_TIME));
  BOOST_TEST_MESSAGE("== Test teardown [END]: Stopping server dietForwarder ==");
}
};


template <class parentFixture1, class parentFixture2>
class JoinFixture: public parentFixture1, parentFixture2 {
public:
JoinFixture() {
  BOOST_TEST_MESSAGE("== Test setup [BEGIN]:  Launching join ==");
  BOOST_TEST_MESSAGE("== Test setup [END]: Launching join ==");
}

~JoinFixture() {
  BOOST_TEST_MESSAGE("== Test teardown [BEGIN]: Stopping join ==");
  BOOST_TEST_MESSAGE("== Test teardown [END]: Stopping join ==");
}
};



/* Define a hierarchy across two "networks"
 * We use two omniNames on different ports to simulate two networks.
 * Thus, we need to use two forwarders to communicate.
 * Hierarchy is as follows:
 * - Network 1
 *   + MA
 *   + [Log services]
 *   + Clients
 * - Network 2
 *   + LA
 *   + SeD
 *   + MADAG
 */
char ConfigForwarder[] = FWD_CONFIG;

char FwdClientName[] = FWD_CLIENT_NAME;
char FwdServerName[] = "serverFwd";
char FwdRemoteHost[] = "localhost";
char FwdSSHHost[] = "127.0.0.1";

typedef OmniNamesFixture<ConfigOmniORB2, OmniORBEndPoint2,
                         OmniORBInitRef2> omniNamesFixture2;
typedef JoinFixture<omniNamesFixture, omniNamesFixture2> joinTwoOmninames;
typedef TimeoutFixture<300, joinTwoOmninames> timeoutOmninames; // timeout at 5min, should be enough
typedef DietForwarderClientFixture<ConfigForwarder, FwdClientName,
                                   ConfigOmniORB,
                                   timeoutOmninames> DietFwdClientFixture;
typedef DietForwarderServerFixture<ConfigForwarder, FwdServerName,
                                   FwdClientName,
                                   FwdRemoteHost, FwdSSHHost, ConfigOmniORB2,
                                   DietFwdClientFixture> DietFwdServerFixture;
typedef DietAgentFixture<ConfigMasterAgent, ConfigOmniORB,
                         DietFwdServerFixture> DietMAFixtureFwd;
typedef DietAgentFixture<ConfigLocalAgent, ConfigOmniORB2,
                         DietMAFixtureFwd> DietLAFixtureFwd;
typedef DietMADAGFixture<ConfigMADAG, ConfigOmniORB2,
                         DietLAFixtureFwd> DietMADAGFixtureLAFwd;
typedef DietSeDFixture<SimpleAddSeD, SimpleAddSeDBinDir, ConfigSimpleAddSeDLA,
                       ConfigOmniORB2, DietLAFixtureFwd> SimpleAddSeDFixtureFwd;
typedef DietSeDFixture<AllDataTransferAddSeD, AllDataTransferBinDir,
                       ConfigSimpleAddSeDLA, ConfigOmniORB2,
                       DietLAFixtureFwd> AllDataTransferSeDFixtureFwd;
typedef DietSeDFixture<AllDataTransferAddSeD, AllDataTransferBinDir,
                       ConfigSimpleAddSeDLA, ConfigOmniORB2,
                       DietMADAGFixtureLAFwd> AllDataTransferSeDFixtureWFFwd;
typedef DietSeDFixture<DynamicAddRemoveServiceAddSeD,
                       DynamicAddRemoveServiceBinDir, ConfigSimpleAddSeDLA,
                       ConfigOmniORB2,
                       DietLAFixtureFwd> DynamicAddRemoveServiceSeDFixtureFwd;
typedef DietSeDFixture<GRPCAddSeD, GRPCBinDir, ConfigSimpleAddSeDLA,
                       ConfigOmniORB2, DietLAFixtureFwd> GRPCSeDFixtureFwd;


#endif // FIXTURES_FWD_HPP_
