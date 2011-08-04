/****************************************************************************/
/* dietAgent main                                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.64  2011/05/13 09:05:10  bdepardo
 * More explicit error message
 *
 * Revision 1.63  2011/04/07 08:51:54  bdepardo
 * Take into account the traceLevel sooner
 *
 * Revision 1.62  2011/04/06 17:17:07  bdepardo
 * Fixed bug #197.
 * Exceptions are caught when the omniNames is not reachable
 *
 * Revision 1.61  2011/04/05 14:02:31  bdepardo
 * IOR is printed only when the tracelevel is at least TRACE_MAIN_STEPS
 *
 * Revision 1.60  2011/03/21 08:27:38  bdepardo
 * Correctly register the logcomponent into the ORB, and correclty detroy it.
 *
 * Revision 1.59  2011/03/20 18:48:18  bdepardo
 * Be more robust when logComponent initialization fails
 *
 * Revision 1.58  2011/03/16 20:36:35  bdepardo
 * Fixed a few memleaks when an error is encountered
 *
 * Revision 1.57  2011/03/11 11:25:48  bdepardo
 * Fixed a problem in configuration parser. The agent type was not correctly
 * parsed.
 *
 * Revision 1.56  2011/03/03 00:23:11  bdepardo
 * Resolved a few fix me
 *
 * Revision 1.55  2011/02/24 16:57:02  bdepardo
 * Use new parser
 *
 * Revision 1.54  2011/02/24 12:50:32  bdepardo
 * Use new version of CONFIG_XXX macros
 *
 * Revision 1.53  2011/02/23 15:05:17  bdepardo
 * Catch exception when opening the configuration file.
 *
 * Revision 1.52  2011/02/15 18:41:41  bdepardo
 * Removed compiler warning
 *
 * Revision 1.51  2011/02/15 16:20:57  bdepardo
 * Correctly handle sigint with new ORB.
 * Fixed a bug on deletion of strings in "args" variable.
 *
 * Revision 1.50  2011/02/09 15:09:55  hguemar
 * configuration backend changed again: more CONFIG_XXX
 *
 * Revision 1.49  2011/02/08 16:53:01  bdepardo
 * Add DIET version
 *
 * Revision 1.48  2011/02/08 09:44:43  bdepardo
 * Typo in warning message
 *
 * Revision 1.47  2011/02/07 19:00:53  bdepardo
 * Correct option parsing for logService
 *
 * Revision 1.46  2011/02/07 18:54:30  hguemar
 * fix issue with LogService configuration
 *
 * Revision 1.45  2011/02/04 16:02:20  hguemar
 * fix build compilation
 *
 * Revision 1.44  2011/02/04 15:29:54  hguemar
 * fix some GCC warnings
 *
 * Revision 1.43  2011/02/04 15:20:48  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.42  2011/02/02 17:48:36  bdepardo
 * The agent type can also be LA or MA
 *
 * Revision 1.41  2011/02/02 13:32:28  hguemar
 * configuration parsers: environment variables, command line arguments, file configuration parser
 * moved Dagda and dietAgent (yay auto-generated help) to new configuration subsystem
 *
 * Revision 1.40  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.39  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.38  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.37  2010/03/08 13:21:51  bisnard
 * initialize DietLogComponent for DAGDA agent
 *
 * Revision 1.36  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.35  2010/01/14 11:06:22  bdepardo
 * Compiles with gcc 4.4
 *
 * Revision 1.34  2009/11/30 17:57:47  bdepardo
 * New methods to remove the agent in a cleaner way when killing it.
 *
 * Revision 1.33  2009/10/26 09:15:54  bdepardo
 * When using dynamic hierarchy management, catches SIGINT for clean termination.
 *
 * Revision 1.32  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.31  2008/06/01 09:12:40  rbolze
 * remove free on logService pointer
 *
 * Revision 1.30  2008/05/31 08:43:52  rbolze
 * add some free on unused pointers (LogService)
 *
 * Revision 1.29  2008/04/28 07:08:30  glemahec
 * The DAGDA API.
 *
 * Revision 1.28  2008/04/07 12:19:13  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.27  2008/01/14 09:44:14  glemahec
 * dietAgent.cc modifications to allow the use of DAGDA.
 *
 * Revision 1.26  2007/12/07 08:44:42  bdepardo
 * Added AckFile support in CMake files.
 * No longer need to add -DADAGE to use it, instead -DHAVE_ACKFILE is automatically added when the option is selected.
 * /!\ Parsing problem on Mac: do not recognize the parameter ackFile within the configuration file.
 *
 * Revision 1.25  2007/07/31 14:26:35  bdepardo
 * Added the support for the option ackFile to touch a file at the end of the initialization.
 * Currently needs to use -D ADAGE when compiling in order to use this feature.
 *
 * Revision 1.24  2006/12/14 11:40:12  aamar
 * Making the agent (MA or LA) display its IOR when starting.
 *
 * Revision 1.23  2006/05/12 12:12:32  sdahan
 * Add some documentation about multi-MA
 *
 * Bug fix:
 *  - segfault when the neighbours configuration line was empty
 *  - deadlock when a MA create a link on itself
 *
 * Revision 1.22  2005/06/28 15:56:56  hdail
 * Changing the debug level of messages to make DIET less verbose (and in
 * agreement with the doc =).
 *
 * Revision 1.21  2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *         // JuxMem code
 *     #else
 *         // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in AgentImpl.hh - to be
 * replaced by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.20  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.19  2004/10/04 11:26:58  hdail
 * Changed delete(agtTypeName) to free to agree with allocation.
 *
 * Revision 1.18  2004/10/04 08:10:49  hdail
 * Removed printf("Hello!\n") debugging message added with multi-MA check-in and
 * shortened the CVS log.
 *
 * Revision 1.17  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.16  2004/07/05 14:56:13  rbolze
 * correct bug on 64 bit plat-form, when parsing cfg file :
 * remplace size_t by unsigned int for config options
 *
 * Revision 1.15  2004/05/28 10:53:21  mcolin
 * change the endpoint option names for agents and servers
 *  endPointPort -> dietPort
 *  endPointHostname -> dietHostname
 *
 * Revision 1.14  2004/04/16 19:04:40  mcolin
 * Fix patch for the vthd demo with the endPoint option in config files.
 * This option is now replaced by two options:
 *   endPointPort: precise the listening port of the agent/server
 *   endPointHostname: precise the listening interface of the agent/server
 *
 * Revision 1.13  2004/03/03 16:10:53  mcolin
 * correct a bug in the construction of the corba option for the endPoint :
 * %u replaced by %s
 ****************************************************************************/

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

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
#include "LocMgrImpl.hh"    // DTM header file
#endif /* ! HAVE_JUXMEM && ! HAVE_DAGDA */

#if HAVE_DAGDA
#include "DagdaImpl.hh"
#include "DagdaFactory.hh"
#endif /* HAVE_DAGDA */


/** The trace level. */
extern unsigned int TRACE_LEVEL;

#ifdef USE_LOG_SERVICE
/** The DietLogComponent */
DietLogComponent* dietLogComponent;
#endif

/** The Agent object. */
AgentImpl* Agt;

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
/** The DTM Data Location Manager Object  */
LocMgrImpl *Loc;
#endif /* ! HAVE_JUXMEM && ! HAVE_DAGDA */


class CStringDeleter {
public:
  void
  operator() (char *it_) const {
    if (it_ != NULL) {
      free(it_);
      it_ = NULL;
    }
  }
};

template <typename C>
class CStringInserter {
private:
  C& c_;
public:
  explicit CStringInserter(C& c) : c_(c) {}

  void
  operator() (const char *cstr) {
    c_.push_back(strdup(cstr));
  }

  void
  operator() (std::ostringstream& oss) {
    char *cstr = strdup(oss.str().c_str());
    c_.push_back(cstr);
  }
};


int main(int argc, char* argv[], char *envp[]) {
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

  CmdEntry configFileEntry = { CmdParser::Param,
                               CmdParser::Mandatory,
                               "configFile",
                               "config-file",
                               "c",
                               "configuration file" };

  CmdEntry agentTypeEntry = { CmdParser::Option,
                              CmdParser::Optional,
                              "agentType",
                              "agent-type",
                              "T",
                              "agent type (either DIET_MASTER_AGENT or MA, "
                              "or DIET_LOCAL_AGENT or LA)" };

  CmdEntry agentNameEntry = { CmdParser::Option,
                              CmdParser::Optional,
                              "name",
                              "agent-name",
                              "n",
                              "agent name" };

  CmdEntry agentParentEntry = { CmdParser::Option,
                                CmdParser::Optional,
                                "parentName",
                                "parent-name",
                                "p",
                                "parent name" };

  CmdEntry agentTraceLevelEntry = { CmdParser::Option,
                                    CmdParser::Optional,
                                    "traceLevel",
                                    "trace-level",
                                    "t",
                                    "trace level (integer)" };


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
  std::string& configFile = cmdParser["configFile"];

  FileParser fileParser;
  try {
    fileParser.parseFile(configFile);
  } catch (...) {
    ERROR("while parsing " << configFile, GRPC_CONFIGFILE_ERROR);
  }

  /* now merge our maps */
  CONFIGMAP = cmdParser.getConfiguration();
  const ConfigMap& fileMap = fileParser.getConfiguration();
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
  // std::string& agentName = CONFIG_STRING("name"]; // UNUSED ?
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
      // delete(dietLogComponent); // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled" << endl);
      dietLogComponent = NULL;
    }
  }
#endif /* USE_LOG_SERVICE */

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
  /* Create the DTM Data Location Manager */
  Loc = new LocMgrImpl();
#endif /* ! HAVE_JUXMEM && ! HAVE_DAGDA */
#if HAVE_DAGDA
  DagdaImpl* dataManager;
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
#endif /* HAVE_DAGDA */

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

#if ! HAVE_JUXMEM
  // Use Dagda instead of DTM.
#if ! HAVE_DAGDA
  /* Launch the DTM LocMgr */
  ORBMgr::getMgr()->activate(Loc);
  if (Loc->run()) {
    std::for_each(args.begin(), args.end(), CStringDeleter());
    ERROR("unable to launch the LocMgr", 1);
  }
  Agt->linkToLocMgr(Loc);
#else
  ORBMgr::getMgr()->activate(dataManager);
  Agt->setDataManager(dataManager->_this());
#endif /* ! HAVE_DAGDA */
#endif /* ! HAVE_JUXMEM */


#ifdef HAVE_ACKFILE
  /* Touch a file to notify the end of the initialization */
  std::string ackFile;
  if (!CONFIG_STRING(diet::ACKFILE, ackFile)) {
    WARNING("parsing " << configFile << ": no ackFile specified");
  } else {
    cerr << "Open OutFile: "<< ackFile <<endl;
    ofstream out(ackFile.c_str());
    out << "ok" << endl << endl;
    out.close();
  }
#endif /* HAVE_ACKFILE */


  /* Wait for RPCs (blocking call): */
  try {
    ORBMgr::getMgr()->wait();
  } catch (...) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

#ifdef HAVE_DYNAMICS
  Agt->removeElementClean(false);
#endif /* HAVE_DYNAMICS */

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically
   */
  delete ORBMgr::getMgr();

  std::for_each(args.begin(), args.end(), CStringDeleter());

  TRACE_TEXT(TRACE_ALL_STEPS, "Agent has exited" << std::endl);

  return 0;
}
