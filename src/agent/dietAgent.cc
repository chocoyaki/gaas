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
 * Revision 1.7  2003/09/22 21:07:52  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.5  2003/09/04 10:04:58  pcombes
 * Cast Agt before calling the run method.
 *
 * Revision 1.3  2003/08/28 16:51:32  cpontvie
 * Adding a SIGINT handler to properly clean the agent on interruption (CTRL+C)
 *
 * Revision 1.2  2003/07/04 09:47:58  pcombes
 * Use new ERROR and WARNING macros.
 *
 * Revision 1.1  2003/05/10 08:50:33  pcombes
 * New Parsers let us reunify the two executables into one dietAgent.
 ****************************************************************************/


#include <signal.h>
#include <stdlib.h>

#include "debug.hh"
#include "LocalAgentImpl.hh"
#include "LocMgrImpl.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;


/** The Agent object. */
AgentImpl* Agt;

/** The Data Location Manager Object  */
LocMgrImpl *Loc;


#if 0
/* The SIGINT handler function
   The main function ends here in a fully working Agent !
*/
void handler(int sig)
{
  /* Prevent from raising a new SIGINT handler */
  signal(SIGINT, SIG_IGN);

  TRACE_TEXT(TRACE_MAIN_STEPS, "______________________________\n");
  TRACE_TEXT(TRACE_MAIN_STEPS, "Stopping the Agent...\n");

  /* Deactivate and destroy the agent */
  ORBMgr::deactivate();
  delete Agt;

  TRACE_TEXT(TRACE_MAIN_STEPS, "Agent stopped !\n");
  exit(0);
}

#endif // 0


int
main(int argc, char** argv)
{
  char*  config_file_name;
  int    res(0);
  int    myargc;
  char** myargv;

  if (argc < 2) {
    ERROR(argv[0] << ": missing configuration file", 1);
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
      ERROR("parsing " << config_file_name
            << ": no parent name specified", 1);
    }
  } else {
    if (name != NULL)
      WARNING("parsing " << config_file_name << ": no need to specify "
              << "a parent name for an MA - ignored");
  }

  name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  if (name != NULL)
    WARNING("parsing " << config_file_name << ": no need to specify "
            << "an MA name for an agent - ignored");


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

  if (ORBMgr::init(myargc, (char**)myargv)) {
    ERROR("ORB initialization failed", 1);
  }

  /* Create the Data Location Manager */
  Loc = new LocMgrImpl();

  /* Create, activate, and launch the agent */

  if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
    Agt = new LocalAgentImpl();
    ORBMgr::activate((LocalAgentImpl*)Agt);
    res = ((LocalAgentImpl*)Agt)->run();
  } else {
    Agt = new MasterAgentImpl();
    ORBMgr::activate((MasterAgentImpl*)Agt);
    res = ((MasterAgentImpl*)Agt)->run();
  }

  /* Launch the agent */
  if (res) {
    ERROR("unable to launch the agent", 1);
  }

  /* Launch the LocMgr */
  ORBMgr::activate(Loc);
  if (Loc->run()) {
    ERROR("unable to launch the LocMgr", 1);
  }
  Agt->linkToLocMgr(Loc);

  /* We do not need the parsing results any more */
  Parsers::endParsing();

#if 0
  /* Initialize the SIGINT handler exception */
  signal( SIGINT, handler );
#endif // 0

  /* Wait for RPCs (blocking call): */
  try {
    ORBMgr::wait();
  } catch (...) {}

  /* NEVER REACHED */
  ERROR("'ORBMgr::wait()' failed to run !", 1);
  return 0;
}
