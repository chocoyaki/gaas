/**
 * @file dietAgent.cc
 *
 * @brief  dietAgent main
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


// must be included first
#include "ExitClass.hh"

#include <cstdlib>
#include <csignal>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;
#include "configuration.hh"
#include "debug.hh"
#include "LocalAgentImpl.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "DIET_grpc.h"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif /* USE_LOG_SERVICE */

#include "DagdaImpl.hh"
#include "DagdaFactory.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#ifdef USE_LOG_SERVICE
/** The DietLogComponent */
DietLogComponent *dietLogComponent;
#endif

/** The Agent object. */
AgentImpl *Agt;

class CStringDeleter {
public:
void
operator()(char *it_) const {
  if (it_ != NULL) {
    free(it_);
    it_ = NULL;
  }
}
};

template <typename C>
class CStringInserter {
public:
explicit
CStringInserter(C &c): c_(c) {
}

void
operator()(const char *cstr) {
  c_.push_back(strdup(cstr));
}

void
operator()(std::ostringstream &oss) {
  char *cstr = strdup(oss.str().c_str());
  c_.push_back(cstr);
}

private:
C &c_;
};


int
main(int argc, char *argv[], char *envp[]) {
  // use std::vector instead of C array
  // C++ standard guarantees that its storage is contiguous (C++ Faq 34.3)
  std::vector<char *> args, argsTmp;
  CStringInserter<std::vector<char *> > ins(args);

  // Configuration map
  int res(0);

  std::string copyright = "";  // TODO : add a copyright
  std::string version = DIET_VERSION;

  /* Parsing */
  CmdParser cmdParser(argc, argv);

  CmdEntry configFileEntry = {CmdParser::Param,
                              CmdParser::Mandatory,
                              "configFile",
                              "config-file",
                              "c",
                              "configuration file"};

  CmdEntry agentTypeEntry = {CmdParser::Option,
                             CmdParser::Optional,
                             "agentType",
                             "agent-type",
                             "T",
                             "agent type (either DIET_MASTER_AGENT or MA, "
                             "or DIET_LOCAL_AGENT or LA)"};

  CmdEntry agentNameEntry = {CmdParser::Option,
                             CmdParser::Optional,
                             "name",
                             "agent-name",
                             "n",
                             "agent name"};

  CmdEntry agentParentEntry = {CmdParser::Option,
                               CmdParser::Optional,
                               "parentName",
                               "parent-name",
                               "p",
                               "parent name"};

  CmdEntry agentTraceLevelEntry = {CmdParser::Option,
                                   CmdParser::Optional,
                                   "traceLevel",
                                   "trace-level",
                                   "t",
                                   "trace level (integer)"};


  CmdConfig cmdConfig;
  cmdConfig.push_back(configFileEntry);
  cmdConfig.push_back(agentTypeEntry);
  cmdConfig.push_back(agentNameEntry);
  cmdConfig.push_back(agentParentEntry);
  cmdConfig.push_back(agentTraceLevelEntry);

  cmdParser.setConfig(cmdConfig);
  cmdParser.enableHelp(true);
  cmdParser.enableVersion(version, copyright);
  cmdParser.parse();

  // get configuration file
  std::string &configFile = cmdParser["configFile"];

  FileParser fileParser;
  try {
    fileParser.parseFile(configFile);
  } catch (...) {
    ERROR("while parsing " << configFile, GRPC_CONFIGFILE_ERROR);
  }

  /* now merge our maps */
  CONFIGMAP = cmdParser.getConfiguration();
  const ConfigMap &fileMap = fileParser.getConfiguration();
  CONFIGMAP.insert(fileMap.begin(), fileMap.end());

  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;


  /* get parameters: agentType and name */
  std::string agentType = "MA";
  try {
    CONFIG_AGENT(diet::AGENTTYPE, agentType);
  } catch (std::runtime_error &e) {
    ERROR(e.what(), GRPC_CONFIGFILE_ERROR);
  }
  // std::string& agentName = CONFIG_STRING("name"];  // UNUSED ?
  std::string parentName = "";
  bool hasParentName = CONFIG_STRING(diet::PARENTNAME, parentName);
  std::string maName;


  // parentName is mandatory for LA but unneeded for MA
  if (((agentType == "DIET_LOCAL_AGENT") || (agentType == "LA")) &&
      !hasParentName) {
    ERROR("parsing " << configFile
                     << ": no parent name specified", GRPC_CONFIGFILE_ERROR);
  } else if (((agentType != "DIET_LOCAL_AGENT") && (agentType != "LA")) &&
             hasParentName) {
    WARNING("parsing " << configFile << ": no need to specify "
                       << "a parent name for an MA - ignored");
  }

  if (CONFIG_STRING(diet::MANAME, maName)) {
    WARNING("parsing " << configFile << ": no need to specify "
                       << "an MA name for an agent - ignored");
  }

  /* Copy input parameters into internal structure */
  for (int i = 0; i < argc; i++) {
    ins(argv[i]);
  }

  /* Get listening port & hostname */
  int port;
  std::string host;
  bool hasPort = CONFIG_INT(diet::DIETPORT, port);
  bool hasHost = CONFIG_STRING(diet::DIETHOSTNAME, host);
  if (hasPort || hasHost) {
    std::ostringstream endpoint;
    ins("-ORBendPoint");
    endpoint << "giop:tcp:" << host << ":";
    if (hasPort) {
      endpoint << port;
    }

    ins(endpoint);
  }

  /* Get the traceLevel */
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    std::ostringstream level;
    ins("-ORBtraceLevel");
    level << (TRACE_LEVEL - TRACE_MAX_VALUE);
    ins(level);
  }

  /* Copy the arguments in a temporary vector, as it
   * is modified by ORBMgr::init
   */
  argsTmp = args;

  /* Initialize the ORB */
  try {
    // import use &args[0] and not &args.begin()
    // the latter is not guaranteed to be a T*
    ORBMgr::init(argsTmp.size(), &argsTmp[0]);
  } catch (...) {
    std::for_each(args.begin(), args.end(), CStringDeleter());
    ERROR("ORB initialization failed", 1);
  }

#ifdef USE_LOG_SERVICE
  /* Create the DietLogComponent for use with LogService */
  bool useLS = false;
  int outBufferSize;
  int flushTime;

  CONFIG_BOOL(diet::USELOGSERVICE, useLS);
  if (!useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled" << endl);
    dietLogComponent = NULL;
  } else {
    if (!CONFIG_INT(diet::LSOUTBUFFERSIZE, outBufferSize)) {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

    if (!CONFIG_INT(diet::LSFLUSHINTERVAL, flushTime)) {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }

    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled" << std::endl);
    std::string agtTypeName;
    std::string name = "";
    CONFIG_STRING(diet::NAME, name);

    if ((agentType == "DIET_LOCAL_AGENT") || (agentType == "LA")) {
      agtTypeName = "LA";
    } else {
      agtTypeName = "MA";
    }

    // the agent names should be correct if we arrive here
    dietLogComponent = new DietLogComponent(name.c_str(),
                                            outBufferSize,
                                            argsTmp.size(),
                                            &argsTmp[0]);
    ORBMgr::getMgr()->activate(dietLogComponent);

    if (dietLogComponent->run(agtTypeName.c_str(),
                              parentName.c_str(),
                              flushTime)) {
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled" << endl);
      dietLogComponent = NULL;
    }
  }
#endif /* USE_LOG_SERVICE */

  DagdaImpl *dataManager;
  try {
    dataManager = DagdaFactory::getAgentDataManager();
#ifdef USE_LOG_SERVICE
    dataManager->setLogComponent(dietLogComponent);  // modif bisnard_logs_1
#endif /* USE_LOG_SERVICE */
  } catch (...) {
    ERROR("Problem while instantiating the data manager."
          << "Is omniNames running and"
          << " is OMNIORB_CONFIG variable correctly set?",
          GRPC_COMMUNICATION_FAILED);
  }

  /* Create, activate, and launch the agent */
  if ((agentType == "DIET_LOCAL_AGENT") || (agentType == "LA")) {
    Agt = new LocalAgentImpl();
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "## LA_IOR " << ORBMgr::getMgr()->getIOR(Agt->_this())
                            << endl);
  } else {
    Agt = new MasterAgentImpl();
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "## MA_IOR " << ORBMgr::getMgr()->getIOR(Agt->_this())
                            << endl);
  }
  fsync(1);
  fflush(0);

  ORBMgr::getMgr()->activate(Agt);
#ifdef USE_LOG_SERVICE
  Agt->setDietLogComponent(dietLogComponent);   /* LogService */
#endif /* USE_LOG_SERVICE */
  res = Agt->run();

  /* Initialize the ExitClass static object */
  ExitClass::init(Agt);

  /* Launch the agent */
  if (res) {
    std::for_each(args.begin(), args.end(), CStringDeleter());
    ERROR("unable to launch the agent", 1);
  }

  ORBMgr::getMgr()->activate(dataManager);
  Agt->setDataManager(dataManager->_this());

  /* Wait for RPCs (blocking call): */
  try {
    ORBMgr::getMgr()->wait();
  } catch (...) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

  Agt->removeElementClean(false);

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically
   */
  delete ORBMgr::getMgr();

  std::for_each(args.begin(), args.end(), CStringDeleter());

  TRACE_TEXT(TRACE_ALL_STEPS, "Agent has exited" << std::endl);

  return 0;
} // main
