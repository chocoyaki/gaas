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

#include "ExitClass.hh"
#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

#include "debug.hh"
#include "LocalAgentImpl.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "DietLogComponent.hh"

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
#include "LocMgrImpl.hh"    // DTM header file
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

#if HAVE_DAGDA
#include "DagdaImpl.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The DietLogComponent */
DietLogComponent* dietLogComponent;

/** The Agent object. */
AgentImpl* Agt;

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
/** The DTM Data Location Manager Object  */
LocMgrImpl *Loc;
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

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


  /* Get listening port & hostname */

    // size_t --> unsigned int
  unsigned int* port = (unsigned int*)
    (Parsers::Results::getParamValue(Parsers::Results::DIETPORT));
  char* host = (char*)
    (Parsers::Results::getParamValue(Parsers::Results::DIETHOSTNAME));
  if ((port != NULL)|| (host !=NULL)) {
    char *  endPoint = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBendPoint") ;
    if (port == NULL) {
	    sprintf(endPoint, "giop:tcp:%s:", host);
    } else if (host == NULL)  {
	    sprintf(endPoint, "giop:tcp::%u", *port);
    } else {
	    sprintf(endPoint, "giop:tcp:%s:%u", host,*port);
    }
    myargv[myargc + 1] = (char*)endPoint;
    myargc = tmp_argc;
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


  /* Initialize the ORB */

  if (ORBMgr::init(myargc, (char**)myargv)) {
    ERROR("ORB initialization failed", 1);
  }

  /* Create the DietLogComponent for use with LogService */
  bool useLS;
    // size_t --> unsigned int
  unsigned int* ULSptr;
  int outBufferSize;
    // size_t --> unsigned int
  unsigned int* OBSptr;
  int flushTime;
    // size_t --> unsigned int
  unsigned int* FTptr;

    // size_t --> unsigned int
  ULSptr = (unsigned int*)Parsers::Results::getParamValue(
              Parsers::Results::USELOGSERVICE);
  useLS = false;
  if (ULSptr == NULL) {
    WARNING(" useLogService not configured. Disabled by default" << endl);
  } else {
    if (*ULSptr) {
      useLS = true;
    }
  }
  if (useLS) {
    // size_t --> unsigned int
    OBSptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }
    // size_t --> unsigned int
    FTptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSFLUSHINTERVAL);
    if (FTptr != NULL) {
      flushTime = (int)(*FTptr);
    } else {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }
  }

  if (useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled" << endl);
    char* agtTypeName;
    char* agtParentName;
    char* agtName;
    agtParentName = (char*)Parsers::Results::getParamValue
                          (Parsers::Results::PARENTNAME);
    agtName =       (char*)Parsers::Results::getParamValue
                          (Parsers::Results::NAME);
    // the agent names should be correct if we arrive here

    dietLogComponent = new DietLogComponent(agtName, outBufferSize);
    ORBMgr::activate(dietLogComponent);

    if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
      agtTypeName = strdup("LA");
    } else {
      agtTypeName = strdup("MA");
    }
    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
      // delete(dietLogComponent); // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      dietLogComponent = NULL; // this should not happen;
    }
    free(agtTypeName);

  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled" << endl);
    dietLogComponent = NULL;
  }

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
  /* Create the DTM Data Location Manager */
  Loc = new LocMgrImpl();
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA
#if HAVE_DAGDA
  DagdaImpl* dataManager = DagdaFactory::getAgentDataManager();
  dataManager->setLogComponent( dietLogComponent ); // modif bisnard_logs_1
#endif // HAVE_DAGDA

  /* Create, activate, and launch the agent */

  if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
    Agt = new LocalAgentImpl();
    TRACE_TEXT(NO_TRACE,
	       "## LA_IOR " << ORBMgr::getIORString(Agt->_this()) << endl);
    fsync(1);
    fflush(NULL);

    ORBMgr::activate((LocalAgentImpl*)Agt);
    Agt->setDietLogComponent(dietLogComponent);   /* LogService */
    res = ((LocalAgentImpl*)Agt)->run();
  } else {
    Agt = new MasterAgentImpl();
    TRACE_TEXT(NO_TRACE,
	     "## MA_IOR " << ORBMgr::getIORString(Agt->_this()) << endl);
    fsync(1);
    fflush(NULL);

    ORBMgr::activate((MasterAgentImpl*)Agt);
    Agt->setDietLogComponent(dietLogComponent);   /* LogService */
    res = ((MasterAgentImpl*)Agt)->run();
  }

  /* Initialize the ExitClass static object */
  ExitClass::init(Agt);

  /* Launch the agent */
  if (res) {
    ERROR("unable to launch the agent", 1);
  }

#if ! HAVE_JUXMEM
  // Use Dagda instead of DTM.
#if ! HAVE_DAGDA
  /* Launch the DTM LocMgr */
  ORBMgr::activate(Loc);
  if (Loc->run()) {
    ERROR("unable to launch the LocMgr", 1);
  }
  Agt->linkToLocMgr(Loc);
#else
  ORBMgr::activate(dataManager);
  Agt->setDataManager(dataManager->_this());
#endif // ! HAVE_DAGDA
#endif // ! HAVE_JUXMEM


#ifdef HAVE_ACKFILE
  /* Touch a file to notify the end of the initialization */
  char* ackFile = (char*)
    Parsers::Results::getParamValue(Parsers::Results::ACKFILE);
  if (ackFile == NULL) {
    WARNING("parsing " << config_file_name << ": no ackFile specified");
  }
  else
  {
    cerr << "Open OutFile: "<< ackFile <<endl;
    ofstream out (ackFile);
    out << "ok" << endl << endl;
    out.close();
  }
#endif


  /* Wait for RPCs (blocking call): */
  if (ORBMgr::wait()) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

#ifdef HAVE_DYNAMICS
  signal(SIGINT, SIG_IGN);
  Agt->removeElementClean(false);
  signal(SIGINT, SIG_DFL);
#endif // HAVE_DYNAMICS


  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();

  TRACE_TEXT(TRACE_ALL_STEPS, "Agent has exited" << std::endl);

  return 0;
}
