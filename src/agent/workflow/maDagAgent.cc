/**
 * @file maDagAgent.cc
 *
 * @brief  Main function of the MA DAG agent
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
#include "DIET_grpc.h"
#include "ORBMgr.hh"
#include "MaDag_impl.hh"
#include "MasterAgent.hh"
#include "debug.hh"
#include "configuration.hh"

/*
   #include "DIET_client.h"
   #include "HEFT_Sched.hh"
 */

extern unsigned int TRACE_LEVEL;

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
private:
C &c_;
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
};


void
usage(char *s) {
  std::cerr << "Usage: " << s << " <file.cfg> [sched] [pfm] [IRD]"
            << std::endl;
  std::cerr <<
  "sched = -basic (default) | -g_heft | -g_aging_heft | -fairness | -srpt | -fcfs"
            << std::endl;
  std::cerr << "pfm   = -pfm_any (default) | -pfm_sameservices" << std::endl;
  std::cerr << "IRD   = -IRD <value>" << std::endl;
  exit(1);
}

int
checkUsage(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0]);
  }
  if (argc >= 3) {
    if (strcmp(argv[2], "-basic") &&
        strcmp(argv[2], "-g_heft") &&
        strcmp(argv[2], "-fairness") &&
        strcmp(argv[2], "-g_aging_heft") &&
        strcmp(argv[2], "-srpt") &&
        strcmp(argv[2], "-fcfs")) {
      usage(argv[0]);
    }
  }
  if (argc >= 4) {
    if (strcmp(argv[3], "-pfm_any") &&
        strcmp(argv[3], "-pfm_sameservices")) {
      usage(argv[0]);
    }
  }
  if (argc >= 5) {
    if (strcmp(argv[4], "-IRD")) {
      usage(argv[0]);
    }
  }
  return 0;
} // checkUsage


int
main(int argc, char *argv[]) {
  char *config_file_name = argv[1];
  bool IRD;
  int IRD_value;

  // use std::vector instead of C array
  // C++ standard guarantees that its storage is contiguous (C++ Faq 34.3)
  std::vector<char *> args;
  CStringInserter<std::vector<char *> > ins(args);

  checkUsage(argc, argv);


  /* Parsing */

  /* Init the Xerces engine */
  XMLPlatformUtils::Initialize();


  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR_DEBUG("while parsing " << config_file_name, DIET_FILE_IO_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();
  // FIXME: should we also parse command line arguments?


  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    std::ostringstream level;
    ins("-ORBtraceLevel");
    level << (TRACE_LEVEL - TRACE_MAX_VALUE);
    ins(level);
  }

  /* Check the parameters */
  std::string name;
  std::string agentType;
  if (!CONFIG_STRING(diet::NAME, name)) {
    ERROR_DEBUG("No name found in the configuration", GRPC_CONFIGFILE_ERROR);
  }
  if (!CONFIG_STRING(diet::AGENTTYPE, agentType)) {
    ERROR_DEBUG("No agentType found in the configuration", GRPC_CONFIGFILE_ERROR);
  }

  /* Choose interRoundDelay */
  IRD = false;
  if (argc >= 5) {
    if (!strcmp(argv[4], "-IRD")) {
      IRD = true;
      if (!sscanf(argv[5], "%d", &IRD_value)) {
        ERROR_DEBUG("Wrong IRD parameter value", 1);
      }
    }
  }

  /* Copy input parameters into internal structure */
  for (int i = 0; i < argc; i++) {
    ins(argv[i]);
  }


  /* Choose scheduler type */
  MaDag_impl::MaDagSchedType schedType = MaDag_impl::BASIC;
  if (argc >= 3) {
    if (!strcmp(argv[2], "-fairness")) {
      schedType = MaDag_impl::FOFT;
    } else if (!strcmp(argv[2], "-g_heft")) {
      schedType = MaDag_impl::GHEFT;
    } else if (!strcmp(argv[2], "-g_aging_heft")) {
      schedType = MaDag_impl::GAHEFT;
    } else if (!strcmp(argv[2], "-srpt")) {
      schedType = MaDag_impl::SRPT;
    } else if (!strcmp(argv[2], "-fcfs")) {
      schedType = MaDag_impl::FCFS;
    } else {
      schedType = MaDag_impl::BASIC;
    }
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
  } else {
    ins("-ORBendPointPublish");
    ins("all(addr)");
  }

  /* INIT ORB and CREATE MADAG CORBA OBJECT */
  try {
    // import use &args[0] and not &args.begin()
    // the latter is not guaranteed to be a T*
    ORBMgr::init(args.size(), &args[0]);
  } catch (...) {
    std::for_each(args.begin(), args.end(), CStringDeleter());
    ERROR_DEBUG("ORB initialization failed", 1);
  }

  MaDag_impl *maDag_impl = IRD ? new MaDag_impl(
    name.c_str(), schedType, IRD_value) :
                           new MaDag_impl(name.c_str(), schedType);
  ORBMgr::getMgr()->activate(maDag_impl);

  /* Change platform type */
  if (argc >= 4) {
    if (!strcmp(argv[3], "-pfm_sameservices")) {
      maDag_impl->setPlatformType(MaDag::SAME_SERVICES);
    }
  }

  /* Wait for RPCs (blocking call): */
  try {
    ORBMgr::getMgr()->wait();
  } catch (...) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }


  // Terminate the xerces XML engine
  XMLPlatformUtils::Terminate();

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::kill();

  // FIXME: currently do not delete args as it produces a double free error
  // This is due to ORBMgr::init which adds elements in args.
  // std::for_each(args.begin(), args.end(), CStringDeleter());
  TRACE_TEXT(TRACE_ALL_STEPS, "maDagAgent has exited" << endl);

  return 0;
} // main
