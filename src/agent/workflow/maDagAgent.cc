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

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> [option]\n", s);
  fprintf(stderr, "option = --heft | --fairness\n");
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc != 2) && (argc != 3)) {
    usage(argv[0]);
  }
  if (argc == 3) {
    if (strcmp(argv[2], "--heft") &&
	strcmp(argv[2], "--fairness")) {
      usage(argv[0]);
    }
  }
  return 0;
}


int main(int argc, char * argv[]){
  char * config_file_name = argv[1];
  int    res(0);

  checkUsage(argc,argv);

  /* Parsing */

  // Init the Xerces engine
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

  Parsers::Results::agent_type_t agtType =
    *((Parsers::Results::agent_type_t*)
      Parsers::Results::getParamValue(Parsers::Results::AGENTTYPE));

  char* name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::NAME);

  // choose scheduler type

  MaDag_impl::MaDagSchedType schedType = MaDag_impl::BASIC;
  if (argc >= 3) {
    if (!strcmp(argv[2], "--fairness"))
      schedType = MaDag_impl::FOFT;
    else if (!strcmp(argv[2], "--heft"))
      schedType = MaDag_impl::HEFT;
  }

  // INIT ORB and CREATE MADAG CORBA OBJECT

  if (ORBMgr::init(argc, argv)) {
    ERROR("ORB initialization failed", 1);
  }

  MaDag_impl * maDag_impl = new MaDag_impl(name, schedType);
  ORBMgr::activate((MaDag_impl*)maDag_impl);


  /* Wait for RPCs (blocking call): */
  if (ORBMgr::wait()) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();


  XMLPlatformUtils::Terminate();

  return 0;
}

