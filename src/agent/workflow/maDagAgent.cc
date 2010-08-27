/****************************************************************************/
/* Main function of the MA DAG agent                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.16  2010/08/27 07:47:31  bisnard
 * 'fixed warning'
 *
 * Revision 1.15  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.14  2010/03/31 19:37:54  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.13  2008/09/04 15:22:25  bisnard
 * Changed name of multiwf heuristic HEFT to GHEFT
 *
 * Revision 1.12  2008/09/04 14:33:55  bisnard
 * - New option for MaDag to select platform type (servers
 * with same service list or not)
 * - Optimization of the multiwfscheduler to avoid requests to
 * MA for server availability
 *
 * Revision 1.11  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 * Revision 1.10  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 * Revision 1.9  2008/07/08 15:52:03  bisnard
 * Set interRoundDelay as parameter of workflow scheduler
 *
 * Revision 1.8  2008/07/07 11:44:09  bisnard
 * changed options syntax
 *
 * Revision 1.7  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.6  2008/06/03 08:52:05  bisnard
 * handle config parameter for ORB debugging
 *
 * Revision 1.5  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.4  2008/05/31 08:45:55  rbolze
 * add DietLogComponent to the maDagAgent
 *
 * Revision 1.3  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.2  2008/04/28 11:51:43  bisnard
 * choose wf scheduler type when creating madag
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.4  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/

#include <iostream>
using namespace std;
#include "Parsers.hh"
#include "ORBMgr.hh"
#include "MaDag_impl.hh"
#include "MasterAgent.hh"
#include "debug.hh"

/*
#include "DIET_client.h"
#include "HEFT_Sched.hh"
*/

extern unsigned int TRACE_LEVEL;

void usage(char * s) {
  cerr << "Usage: " << s << " <file.cfg> [sched] [pfm] [IRD]" << endl;
  cerr << "sched = -basic (default) | -g_heft | -g_aging_heft | -fairness | -srpt | -fcfs" << endl;
  cerr << "pfm   = -pfm_any (default) | -pfm_sameservices" << endl;
  cerr << "IRD   = -IRD <value>" << endl;
  exit(1);
}

int checkUsage(int argc, char ** argv) {
  if (argc < 2) {
    usage(argv[0]);
  }
  if (argc >= 3) {
    if (strcmp(argv[2], "-basic") &&
        strcmp(argv[2], "-g_heft") &&
	strcmp(argv[2], "-fairness") &&
        strcmp(argv[2], "-g_aging_heft") &&
        strcmp(argv[2], "-srpt") &&
        strcmp(argv[2], "-fcfs"  )) {
      usage(argv[0]);
    }
  }
  if (argc >=4) {
    if (strcmp(argv[3], "-pfm_any") &&
        strcmp(argv[3], "-pfm_sameservices" )) {
      usage(argv[0]);
    }
  }
  if (argc >=5) {
    if (strcmp(argv[4], "-IRD"))
      usage(argv[0]);
  }
  return 0;
}


int main(int argc, char * argv[]){
  char * config_file_name = argv[1];
  int    res(0);
  int    myargc;
  char** myargv;
  bool   IRD;
  int    IRD_value;

  checkUsage(argc,argv);

  /* Set arguments for ORBMgr::init */

  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];

  /* Parsing */

  /* Init the Xerces engine */
  XMLPlatformUtils::Initialize();


  Parsers::Results::param_type_t compParam[] =
    {Parsers::Results::AGENTTYPE, Parsers::Results::NAME};

  if ((res = Parsers::beginParsing(config_file_name)))
    return res;
  if ((res =
       Parsers::parseCfgFile(true, 2,
                             (Parsers::Results::param_type_t*)compParam))) {
    Parsers::endParsing();
    return res;
  }

  char* name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::NAME);

  /* Choose scheduler type */
  MaDag_impl::MaDagSchedType schedType = MaDag_impl::BASIC;
  if (argc >= 3) {
    if (!strcmp(argv[2], "-fairness"))
      schedType = MaDag_impl::FOFT;
    else if (!strcmp(argv[2], "-g_heft"))
      schedType = MaDag_impl::GHEFT;
    else if (!strcmp(argv[2], "-g_aging_heft"))
      schedType = MaDag_impl::GAHEFT;
    else if (!strcmp(argv[2], "-srpt"))
      schedType = MaDag_impl::SRPT;
    else if (!strcmp(argv[2], "-fcfs"))
      schedType = MaDag_impl::FCFS;
    else schedType = MaDag_impl::BASIC;
  }

  /* Choose interRoundDelay */
  IRD = false;
  if (argc >= 5) {
    if (!strcmp(argv[4], "-IRD")) {
      IRD = true;
      if (!sscanf(argv[5],"%d",&IRD_value)) {
        ERROR("Wrong IRD parameter value", 1);
      }
    }
  }

  /* Get the traceLevel */

  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *  level = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBtraceLevel") ;
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }

  /* INIT ORB and CREATE MADAG CORBA OBJECT */

  try {
		ORBMgr::init(myargc, myargv);
	} catch (...) {
    ERROR("ORB initialization failed", 1);
  }

  MaDag_impl * maDag_impl = IRD ? new MaDag_impl(name,schedType,IRD_value) :
      new MaDag_impl(name, schedType);
  ORBMgr::getMgr()->activate((MaDag_impl*)maDag_impl);

  /* Change platform type */
  if (argc >=4) {
    if (!strcmp(argv[3], "-pfm_sameservices"))
      maDag_impl->setPlatformType(MaDag::SAME_SERVICES);
  }

  /* Wait for RPCs (blocking call): */
  try {
		ORBMgr::getMgr()->wait();
	} catch (...) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  delete ORBMgr::getMgr();


  XMLPlatformUtils::Terminate();

  return 0;
}

