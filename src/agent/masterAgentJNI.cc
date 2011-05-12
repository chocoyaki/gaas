/****************************************************************************/
/* JNI Master Agent implementation source code                              */
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
 * Revision 1.17  2011/05/12 15:39:00  bdepardo
 * Reduced variables scope
 *
 * Revision 1.16  2011/05/10 07:50:53  bdepardo
 * Use new parser
 *
 * Revision 1.15  2011/02/09 11:27:53  bdepardo
 * Removed endl at the end of the call to the WARNING macro
 *
 * Revision 1.14  2011/01/23 19:25:29  bdepardo
 * Fixed memory leak and variable scope
 *
 * Revision 1.13  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.12  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.11  2008/01/14 10:08:19  glemahec
 * masterAgentJNI.cc modifications to allow the use of DAGDA.
 *
 * Revision 1.10  2006/12/06 18:07:43  ecaron
 * Header bug fix
 *
 * Revision 1.9  2006/11/28 15:41:33  ctedesch
 * add old logs in header
 *
 * Revision 1.8  2006/11/28 15:23:59  ctedesch
 * header
 *
 * Revision 1.7
 * date: 2005/11/18 10:52:10  ctedesch
 * pointer bug fix
 *
 * Revision 1.6
 * date: 2005/07/18 13:01:48  ctedesch
 * Changes inside the DIET code caused the JXTA Multi-MA to be broken. 
 * It's now repaired. Moreover, the JXTA SeD will be automatically updated 
 * when the DIET server API is modified.
 *
 * revision 1.5
 * date: 2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default 
 * and JuxMem will be supported via configure flags.  DIET will always provide 
 * at least one type of persistency.  As a first step, persistency across DTM 
 * and JuxMem is not supported so all persistency handling should be surrounded 
 * by
 *   #if HAVE_JUXMEM
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
 * revision 1.4
 * date: 2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled 
 * by configuration file.
 *
 * revision 1.3
 * date: 2004/08/27 16:28:18  ctedesch
 *�- Use of the asynchronous PIF scheme for propagation of the requests inside
 * the DIET J multi-hierarchy
 * - DIET/JXTA -> DIET J
 * - Change the JXTA examples scripts to build a whole multi-hierarchy
 * 
 * revision 1.2
 * date: 2004/08/23 20:25:00  ctedesch;  state: Exp;  lines: +46 -6
 * Update DIET_J to take into accout the use of the request ID in the log service
 *
 * revision 1.1
 * date: 2004/06/11 15:45:39;  author: ctedesch;  state: Exp;
 * add DIET/JXTA
 ****************************************************************************/

#include "ExitClass.hh"
#include <stdlib.h>
#include <iostream>
using namespace std;

#include "debug.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "configuration.hh"
#include "DietLogComponent.hh"

#include "jni.h"
#include "JXTAMultiMA.h"

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
#include "LocMgrImpl.hh"    // DTM header file
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

#if HAVE_DAGDA
#include "DagdaImpl.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The DietLogComponent for use with LogService */
DietLogComponent* dietLogComponent;

/** The Master Agent object */
MasterAgentImpl* MasterAgt;

#if ! HAVE_JUXMEM && ! HAVE_DAGDA
/** The Data Location Manager Object for DTM */
LocMgrImpl *Loc;
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

void RPCsWait (void* args);

long int atol (char *);
char *ltoa (long);

JNIEXPORT jint JNICALL 
Java_JXTAMultiMA_startDIETAgent(JNIEnv *env, 
                                jobject obj, jstring config_file)
{

  char*  config_file_name;
  jint    myargc;
  char** myargv;

  config_file_name = strdup(env->GetStringUTFChars(config_file, 0));
  
  /* set arguments for ORBMgr::init */

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
    ERROR("ORB initialization failed", 1);
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
  /* Create the DTM Data Location Manager */
  Loc = new LocMgrImpl();
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA
#if HAVE_DAGDA
  DagdaImpl* dataManager = DagdaFactory::getAgentDataManager();
#endif // HAVE_DAGDA

  /* Create and activate the Master Agent */
  MasterAgt = new MasterAgentImpl();
  ORBMgr::activate(MasterAgt);

  MasterAgt->setDietLogComponent(dietLogComponent);

  /* Launch the Master Agent */
  if (MasterAgt->run()) {
    ERROR("Unable to launch the agent.", 1);
  }

  /* Initialize the ExitClass static object */
  ExitClass::init(MasterAgt);

#if ! HAVE_JUXMEM
  // Use Dagda instead of DTM.
#if ! HAVE_DAGDA
  /* Launch the LocMgr for DTM */
  ORBMgr::activate(Loc);
  if (Loc->run()) {
    ERROR("unable to launch the LocMgr", 1);
  }
  MasterAgt->linkToLocMgr(Loc);
#else
  ORBMgr::activate(dataManager);
  MasterAgt->setDataManager(dataManager);
#endif // ! HAVE_DAGDA
#endif // ! HAVE_JUXMEM

  /* Wait for RPCs (blocking call): */
  omni_thread *RPCsWaitThread = omni_thread::create(RPCsWait, (void *)0, omni_thread::PRIORITY_NORMAL);

  return (0);
} // start DIET Agent

void RPCsWait (void* args)
{
  /* Wait for RPCs (blocking call): */
  if (ORBMgr::wait()) {
    WARNING("Error while exiting the ORBMgr::wait() function");
  }

  /* shutdown and destroy the ORB
   * Servants will be deactivated and deleted automatically */
  ORBMgr::destroy();  
} // RPCs Wait

JNIEXPORT jobjectArray JNICALL 
Java_JXTAMultiMA_submitJXTA(JNIEnv *env, 
			  jobject obj, 
			  jstring pbName, jstring nbRow, jstring nbCol)
{

  // 3 matrices
  long int nbR, nbC;

  nbR = atol(strdup(env->GetStringUTFChars(nbRow, 0)));
  nbC = atol(strdup(env->GetStringUTFChars(nbCol, 0)));

  corba_matrix_specific_t mat1, mat2, mat3;
  
  mat1.nb_r = nbR;
  mat1.nb_c = nbC;
  mat1.order = 1;

  mat2.nb_r = nbR;
  mat2.nb_c = nbC;
  mat2.order = 1;

  mat3.nb_r = nbR;
  mat3.nb_c = nbC;
  mat3.order = 1;

  // 3 data desc
  corba_data_desc_t data1, data2, data3;

  data1.id.dataCopy = DIET_KLEENEXCOPY;
  data1.id.state = DIET_FREE;
  data1.mode = 0;
  data1.base_type = 5;
  data1.specific.mat(mat1);

  data2.id.dataCopy = DIET_KLEENEXCOPY;
  data2.id.state = DIET_FREE;
  data2.mode = 0;
  data2.base_type = 5;
  data2.specific.mat(mat2);

  data3.id.dataCopy = DIET_KLEENEXCOPY;
  data3.id.state = DIET_FREE;
  data3.mode = 0;
  data3.base_type = 5;
  data3.specific.mat(mat3);

  // Sequence of data desc
  SeqCorbaDataDesc_t seqData;
  seqData.length(3);
  seqData[0] = data1;
  seqData[1] = data2;
  seqData[2] = data3;

  // Corba pb desc
  corba_pb_desc_t pb;
  pb.path = CORBA::string_dup(env->GetStringUTFChars(pbName, 0));
  pb.last_in = 1;
  pb.last_inout = 1;
  pb.last_out = 2;
  pb.param_desc.length (3);
  pb.param_desc = seqData;

  /* DIET submission */
  corba_response_t* resp = MasterAgt->submit(pb, 10);

  /* Create the object to be returned */
  jobjectArray uuid_ret = (jobjectArray)env->NewObjectArray
    (resp->servers.length(), env->FindClass("java/lang/String"),
     env->NewStringUTF(""));
  
  if (resp->servers.length() == 0)
    cout << "MA DIET: No server found." << endl;
  else {
    cout << "MA DIET: " << resp->servers.length() << " server(s) found." << endl;
    jstring uuid_string;
    
    for (int respCt = 0; respCt < resp->servers.length(); respCt++) {
      
      uuid_string = NULL;
      char *uuid_char = (char *)((((resp->servers[respCt]).loc).uuid).in());
      const char* reqIDNbr = ltoa(resp->reqID);
      
      uuid_char = strcat(uuid_char, (const char *)" ");
      uuid_char = strcat(uuid_char, reqIDNbr);

      uuid_string = env->NewStringUTF(uuid_char);
      env->SetObjectArrayElement(uuid_ret, respCt, uuid_string);

      // delete [] reqIDNbr;
//       delete [] uuid_char;
    }
  }
  return (uuid_ret);
  
} // submitJXTA

/* return a string representing the parameter long value */
char *ltoa (long i)
{
  long tmp = i;
  if (tmp == 0) {
    return ("0");
  }
  else {
    int cpt = 0;
    char *s = new char [20];

    while (tmp >= 1) {
	s[cpt++] = tmp % 10 + '0';
	tmp-=tmp % 10;
	tmp/=10;
    }
    s[cpt] = '\0';
    char *srev = new char[strlen(s) + 1];
    int irev = 0;

    for (int i = strlen(s)- 1; i >= 0; i--)
      srev[irev++] = s[i];
    srev[irev] = '\0';
    
    delete [] s;
    return (srev);
  }
} // ltoa

long int atol (char *str)
{
  long int d = 0;
  int diz = 1; 
  for(int i = strlen(str) - 1; i >= 0; i--)
  {
    d = d + (str[i]-'0') * diz;
    diz = diz * 10;
  }
  return (d);
} // atol
