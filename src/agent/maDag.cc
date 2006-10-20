/****************************************************************************/
/* Main function of the MA DAG                                              */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Logi: $
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

  // enable or not the multi-workflow support
  if ((argc >= 3)  && (!strcmp(argv[2], "--enable-multiwf")))
    maDag_impl->enable_multi_wf(true);

  if ((argc >= 4)  && (!strcmp(argv[2], "--enable-multiwf"))) {
    if (!strcmp(argv[3], "--fairness")) {
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
