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
 * Revision 1.1  2003/05/10 08:50:33  pcombes
 * New Parsers let us reunify the two executables into one dietAgent.
 *
 ****************************************************************************/


#include <iostream>
#include <stdlib.h>

#include "LocalAgentImpl.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"

extern unsigned int TRACE_LEVEL;

int
main(int argc, char** argv)
{
  AgentImpl* Agt;
  char*  config_file_name;
  int    res(0);
  int    myargc;
  char** myargv;
  
  if (argc < 2) {
    std::cerr << argv[0] << ": Missing configuration file.\n";
    return 1;
  }
  config_file_name = argv[1];

  /* Set arguments for ORBMgr::init */

  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];


  /* Parsing */

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

  /* Some more checks */
  
  // agtType should be ! NULL, as it is a compulsory param
  Parsers::Results::agent_type_t agtType =
    *((Parsers::Results::agent_type_t*)
      Parsers::Results::getParamValue(Parsers::Results::AGENTTYPE));
  char* name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);

  if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
    // For a local agent, PARENTNAME is compulsory.
    if (name == NULL) {
      cerr << "Error while parsing " << config_file_name
	   << ": no parent name specified.\n";
      return 1;
    }
  } else {
    if (name != NULL)
      cerr << "Warning while parsing " << config_file_name
	   << ": no need to specify a parent name for an MA - ignored.\n";
  }
  
  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  if (name != NULL)
    cerr << "Warning while parsing " << config_file_name
	 << ": no need to specify an MA name for an agent - ignored.\n";

  
  /* Get listening port */
  
  size_t* port = (size_t*)
    (Parsers::Results::getParamValue(Parsers::Results::ENDPOINT));
  if (port != NULL) {
    char   endPoint[48];
    int    tmp_argc = myargc + 2;
    realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBendPoint";
    sprintf(endPoint, "giop:tcp::%u", *port);
    myargv[myargc + 1] = (char*)endPoint;
    myargc = tmp_argc;
  }

  /* Get the traceLevel */

  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char   level[48];
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBtraceLevel";
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }
  

  /* Initialize the ORB */

  if (ORBMgr::init(myargc, (char**)myargv, true)) {
    cerr << "ORB initialization failed.\n";
    return 1;
  }


  /* Create and activate the agent */

  if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
    Agt = new LocalAgentImpl();
    ORBMgr::activate((LocalAgentImpl*)Agt);
  } else {
    Agt = new MasterAgentImpl();
    ORBMgr::activate((MasterAgentImpl*)Agt);
  }
  
  /* Launch the agent */
  if (Agt->run()) {
    std::cerr << "Unable to launch the agent.\n";
    return 1;
  }  

  /* We do not need the parsing results any more */
  Parsers::endParsing();  

  /* Wait for RPCs (blocking call): */
  ORBMgr::wait();
  
  return 0;
}
