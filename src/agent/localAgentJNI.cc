/****************************************************************************/
/* JNI Local Agent implementation source code                               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*    - Cedric TEDESCHI (Cedric.Tedeschi@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.10  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.9  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.8  2008/01/14 09:51:05  glemahec
 * localAgentJNI.cc modifications to allow the use of DAGDA.
 *
 * Revision 1.7  2006/12/06 18:04:59  ecaron
 * Header bug fix
 *
 * Revision 1.6  2006/11/28 16:13:40  ctedesch
 * add old logs in header
 *
 * Revision 1.5  2006/11/28 15:42:42  ctedesch
 * header
 *
 * Revision 1.4
 * date: 2005/07/18 13:01:48;  author: ctedesch;  state: Exp;  lines: +11 -12
 * Changes inside the DIET code caused the JXTA Multi-MA to be broken. It's 
 * now repaired. Moreover, the JXTA SeD will be automatically updated when 
 * the DIET server API is modified.
 * 
 * Revision 1.3
 * date: 2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *    #if HAVE_JUXMEM
 *       // JuxMem code
 *   #else
 *       // DTM code
 *   #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in AgentImpl.hh - to be
 * replaced by Mathieu's check-in of a configure system for JuxMem.
 * 
 * Revision 1.2
 * date: 2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.1
 * date: 2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 ****************************************************************************/
#include "ExitClass.hh"
#include <cstdlib>
#include <iostream>
using namespace std;

#include "debug.hh"
#include "LocalAgentImpl.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "DietLogComponent.hh"

#include "jni.h"
#include "LA.h"

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
#include "LocMgrImpl.hh"
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

#if HAVE_DAGDA
#include "DagdaImpl.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The DietLogComponent for use with LogService */
DietLogComponent* dietLogComponent;

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
/** The Data Location Manager Object  */
LocMgrImpl *Loc;
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

/** The Agent object. */
AgentImpl* Agt;

JNIEXPORT jint JNICALL 
Java_LA_startDIETLA(JNIEnv *env, 
			    jobject obj, jstring config_file)
{
  char*  config_file_name;
  int res(0);
  int    myargc;
  char** myargv;

  config_file_name = strdup(env->GetStringUTFChars(config_file, 0));
  
  /* Set arguments for ORBMgr::init */

  myargc = 2;
  myargv = (char**)malloc(myargc * sizeof(char*));
  myargv[0] = "dietAgent";
  myargv[1] = config_file_name;

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
	    << ": no parent name specified." << endl, 1);
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

  size_t* port = (size_t*) 
    (Parsers::Results::getParamValue(Parsers::Results::DIETPORT));
  char* host = (char*)
    (Parsers::Results::getParamValue(Parsers::Results::DIETHOSTNAME));
  if ((port != NULL)|| (host !=NULL)) {
    char *  endPoint = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBendPoint";
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
    myargv[myargc] = "-ORBtraceLevel";
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }

  /* Initialize the ORB */

  if (ORBMgr::init(myargc, (char**)myargv)) {
    ERROR("ORB initialization failed",1);
  }

  /* Create the DietLogComponent */
  bool useLS;
  unsigned int* ULSptr;
  int outBufferSize;
  unsigned int* OBSptr;
  int flushTime;
  unsigned int* FTptr;

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
    OBSptr = (unsigned int*)Parsers::Results::getParamValue(
  	       Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

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
    TRACE_TEXT(TRACE_MAIN_STEPS, "LogService enabled" << endl);
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
    delete(agtTypeName);
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS, "LogService disabled" << endl);
    dietLogComponent = NULL;
  }

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
  /* Create the Data Location Manager */
  Loc = new LocMgrImpl();
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA
#if HAVE_DAGDA
  DagdaImpl* dataManager = DagdaFactory::getAgentDataManager();
#endif // HAVE_DAGDA

  /* Create, activate, and launch the agent */

  if (agtType == Parsers::Results::DIET_LOCAL_AGENT) {
    Agt = new LocalAgentImpl();
    ORBMgr::activate((LocalAgentImpl*)Agt);
    Agt->setDietLogComponent(dietLogComponent); /* LogService */
    res = ((LocalAgentImpl*)Agt)->run();
  } else {
    Agt = new MasterAgentImpl();
    ORBMgr::activate((MasterAgentImpl*)Agt);
    Agt->setDietLogComponent(dietLogComponent); /* LogService */
    res = ((MasterAgentImpl*)Agt)->run();
  }


  /* Initialize the ExitClass static object */
  ExitClass::init(Agt);

  /* Launch the agent */
  if (res) {
    ERROR("unable to launch the agent", 1);
  }

#if ! HAVE_JUXMEM
#if ! HAVE_DAGDA
  /* Launch the LocMgr */
  ORBMgr::activate(Loc);
  if (Loc->run()) {
    ERROR("unable to launch the LocMgr", 1);
  }
  Agt->linkToLocMgr(Loc);
#else
  ORBMgr::activate(dataManager);  
  Agt->setDataManager(dataManager);
#endif // ! HAVE_DAGDA
#endif // ! HAVE_JUXMEM

  /* Wait for RPCs (blocking call): */
  if (ORBMgr::wait()) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();

  return 0;

} // start DIET LA

