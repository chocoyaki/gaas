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
 * Revision 1.5  2007/10/25 11:02:35  aamar
 * Restoring scheduler setting from command line arguments
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

int main(int argc, char * argv[]){
  char * config_file_name = argv[1];
  int    res(0);
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

  if (agtType == Parsers::Results::DIET_MA_DAG) {
    cout << "MA_DAG type" << endl;
  }


  if (ORBMgr::init(argc, argv)) {
    ERROR("ORB initialization failed", 1);
  }

  MaDag_impl * maDag_impl = new MaDag_impl(name);
  ORBMgr::activate((MaDag_impl*)maDag_impl);

  // Set up the scheduler, by default use heft
  if ( (argc >= 3) && 
       (!strcmp(argv[2], "--rr")) ) {
    maDag_impl->set_sched(round_robbin_sched);
  }
  // last else
  else {
    maDag_impl->set_sched(heft_sched);
  }

  // enable or not the multi-workflow support
  if ((argc >= 4)  && (!strcmp(argv[3], "--enable-multiwf")))
    maDag_impl->enable_multi_wf(true);

  if ((argc >= 5)  && (!strcmp(argv[3], "--enable-multiwf"))) {
    if (!strcmp(argv[4], "--fairness")) {
      maDag_impl->enable_multi_wf(true, MWF_FAIRNESS);
    }
  }

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
