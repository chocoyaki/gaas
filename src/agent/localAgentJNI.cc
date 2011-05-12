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
 * Revision 1.15  2011/05/12 15:39:00  bdepardo
 * Reduced variables scope
 *
 * Revision 1.14  2011/05/10 07:50:53  bdepardo
 * Use new parser
 *
 * Revision 1.13  2011/02/09 11:27:53  bdepardo
 * Removed endl at the end of the call to the WARNING macro
 *
 * Revision 1.12  2011/01/23 19:19:59  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
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
#include "configuration.hh"
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
  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR("while parsing " << config_file_name, DIET_FILE_IO_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();


  /* Check presence of parameters */
  string name;
  if (!CONFIG_STRING(diet::NAME, name)) {
    ERROR("parsing " << config_file_name
          << ": the name of the agent has not been specified", GRPC_CONFIGFILE_ERROR);
  }

  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;


  /* get parameters: agentType and name */
  std::string agentType;
  try {
    if (!CONFIG_AGENT(diet::AGENTTYPE, agentType)) {
      ERROR("parsing " << config_file_name
            << ": the type of the agent has not been specified", GRPC_CONFIGFILE_ERROR);
    }
  } catch (std::runtime_error &e) {
    ERROR(e.what(), GRPC_CONFIGFILE_ERROR);
  }
  std::string parentName = "";
  bool hasParentName = CONFIG_STRING(diet::PARENTNAME, parentName);
  std::string maName;

  /* Some more checks */
  // parentName is mandatory for LA but unneeded for MA
  if (((agentType == "DIET_LOCAL_AGENT") || (agentType == "LA")) &&
      !hasParentName) {
    ERROR("parsing " << configFile
          << ": no parent name specified", GRPC_CONFIGFILE_ERROR);
  } else if(((agentType != "DIET_LOCAL_AGENT") && (agentType != "LA")) &&
            hasParentName) {
    WARNING("parsing " << configFile << ": no need to specify "
            << "a parent name for an MA - ignored");
  }

  if (CONFIG_STRING(diet::MANAME, maName)) {
    WARNING("parsing " << configFile << ": no need to specify "
            << "an MA name for an agent - ignored");
  }
  

  /* Get listening port & hostname */
  int port;
  std::string host;
  bool hasPort = CONFIG_INT(diet::DIETPORT, port);
  bool hasHost = CONFIG_STRING(diet::DIETHOSTNAME, host);
  if (hasPort || hasHost) {
    std::ostringstream endpoint;
    ins("-ORBendPoint") ;
    endpoint << "giop:tcp:" << host << ":";
    if(hasPort) {
      endpoint << port;
    }
    myargv[myargc + 1] = (char*)endPoint.str().c_str();
    myargc = tmp_argc;
  }

  /* Get the traceLevel */
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    std::ostringstream level;
    ins("-ORBtraceLevel");
    level << (TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level.str().c_str();
    myargc = tmp_argc;
  }

  /* Initialize the ORB */

  if (ORBMgr::init(myargc, (char**)myargv)) {
    ERROR("ORB initialization failed",1);
  }

  /* Create the DietLogComponent */
  bool useLS = false;

  CONFIG_BOOL(diet::USELOGSERVICE, useLS);
  if (!useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled" << endl);
    dietLogComponent = NULL;
  } else {
    int outBufferSize;
    int flushTime;

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

    if (dietLogComponent->run(agtTypeName.c_str(), parentName.c_str(), flushTime)) {
      // delete(dietLogComponent); // DLC is activated, do not delete !
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled" << endl);
      dietLogComponent = NULL;
    }
  }

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
  /* Create the Data Location Manager */
  Loc = new LocMgrImpl();
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA
#if HAVE_DAGDA
  DagdaImpl* dataManager = DagdaFactory::getAgentDataManager();
#endif // HAVE_DAGDA

  /* Create, activate, and launch the agent */

  if ((agentType == "DIET_LOCAL_AGENT") || (agentType == "LA")) {
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

