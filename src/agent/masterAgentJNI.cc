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

#include "ExitClass.hh"
#include <stdlib.h>
#include <iostream>
using namespace std;

#include "debug.hh"
#include "MasterAgentImpl.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "DietLogComponent.hh"

#include "jni.h"
#include "JXTAMultiMA.h"

#if ! HAVE_JUXMEM
#include "LocMgrImpl.hh"    // DTM header file
#endif // ! HAVE_JUXMEM

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The DietLogComponent for use with LogService */
DietLogComponent* dietLogComponent;

/** The Master Agent object */
MasterAgentImpl* MasterAgt;

#if ! HAVE_JUXMEM
/** The Data Location Manager Object for DTM */
LocMgrImpl *Loc;
#endif // ! HAVE_JUXMEM

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

  int res;

  config_file_name = strdup(env->GetStringUTFChars(config_file, 0));
  
  /* set arguments for ORBMgr::init */

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
	   << ": no parent name specified.\n", 1);
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
    cout << "WARNING: useLogService not configured. Disabled by default\n";
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
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled\n");
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
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled\n");
    dietLogComponent = NULL;
  }

#if ! HAVE_JUXMEM
  /* Create the DTM Data Location Manager */
  Loc = new LocMgrImpl();
#endif // ! HAVE_JUXMEM

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
  /* Launch the LocMgr for DTM */
  ORBMgr::activate(Loc);
  if (Loc->run()) {
    ERROR("unable to launch the LocMgr", 1);
  }
  MasterAgt->linkToLocMgr(Loc);
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
  char *s = new char [20];
  long tmp = i;
  int cpt = 0;
  if (tmp == 0) {
    return ("0");
  }
  else {
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
