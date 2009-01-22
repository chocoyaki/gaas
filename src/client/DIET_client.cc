/****************************************************************************/
/* DIET client interface                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*    - Christophe PERA (pera.christophe@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.131  2009/01/22 09:01:07  bisnard
 * added client method to retrieve workflow container output
 *
 * Revision 1.130  2008/12/09 12:09:38  bisnard
 * new API method for inter-dependent dag submit
 *
 * Revision 1.129  2008/12/02 10:19:48  bisnard
 * functional workflow submission API update
 *
 * Revision 1.128  2008/10/22 14:16:37  gcharrie
 * Adding MultiCall. It is used to devide a profile and make several calls with just one SeD. Some documentation will be added soon.
 *
 * Revision 1.127  2008/09/19 13:05:19  bisnard
 * changed library dependencies for UtilsWf
 * added exception details in diet_call
 *
 * Revision 1.126  2008/07/12 00:24:50  rbolze
 * add stat info when the persistent data are released
 * add stat info about the workflow execution.
 *
 * Revision 1.125  2008/07/11 09:52:04  bisnard
 * Missing estimation vector copy in diet_call_common when sed not provided
 *
 * Revision 1.124  2008/07/08 22:12:45  rbolze
 * change char* initialisation to avoid warning message
 *
 * Revision 1.123  2008/06/25 09:52:46  bisnard
 * - Estimation vector sent with solve request to avoid storing it
 * for each submit request as it depends on the parameters value. The
 * estimation vector is used by SeD to updates internal Gantt chart and
 * provide earliest finish time to submitted requests.
 * ==> added parameter to diet_call_common & diet_call_async_common
 *
 * Revision 1.122  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.121  2008/05/16 12:32:10  bisnard
 * API function to retrieve all workflow results
 *
 * Revision 1.120  2008/05/06 10:52:19  bisnard
 * corrected error madag not defined for non-wf calls
 *
 * Revision 1.119  2008/04/28 07:08:31  glemahec
 * The DAGDA API.
 *
 * Revision 1.118  2008/04/18 13:47:24  glemahec
 * Everything about DAGDA is now in utils/DAGDA directory.
 *
 * Revision 1.117  2008/04/14 09:07:22  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - diet_wf_call* functions no longer used with MaDag v2
 *
 * Revision 1.116  2008/04/10 09:13:31  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.115  2008/04/09 12:57:17  gcharrie
 * Total integration of burst mode
 *
 * Revision 1.114  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.113  2008/04/07 12:57:21  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.112  2008/04/06 15:53:10  glemahec
 * DIET_PERSISTENT_RETURN & DIET_STICKY_RETURN modes are now working.
 * Warning: The clients have to take into account that an out data declared as
 * DIET_PERSISTENT or DIET_STICKY is  only stored on the SeDs and not returned
 * to  the  client. DTM doesn't manage the  DIET_*_RETURN types it and  always
 * returns the out data to the client: A client which uses this bug should not
 * work when activating DAGDA.
 *
 * Revision 1.111  2008/04/03 21:18:46  glemahec
 * Source cleaning, bug correction and headers.
 *
 * Revision 1.110  2008/01/14 13:55:28  glemahec
 * Correction of a Warning in DIET_client.cc
 *
 * Revision 1.109  2008/01/14 10:25:48  glemahec
 * The client can now use DAGDA instead of DTM to manage the data.
 *
 * Revision 1.108  2007/10/29 11:09:13  aamar
 * Workflow support: setting reqID of the profile and adding the
 * updateTimeSinceLastSolve call.
 *
 * Revision 1.107  2007/09/25 09:37:21  aamar
 * Nodes can notify the wf log service with the chosen hostname.
 *
 * Revision 1.106  2007/07/20 13:04:56  ycaniou
 * Remove the use of a temporary variable (reqID is now in the profile) to
 *   avoid later potential bugs
 *
 * Revision 1.105  2007/07/13 10:00:26  ecaron
 * Remove deprecated code (ALTPREDICT part)
 *
 * Revision 1.104  2007/07/11 08:42:09  aamar
 * Adding "custom client scheduling" mode (known as Burst mode). Need to be
 * activated in cmake.
 *
 * Revision 1.103  2007/07/09 18:54:49  aamar
 * Adding Endianness support (CMake option).
 *
 * Revision 1.102  2007/06/28 18:23:19  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.101  2007/05/30 11:16:36  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.100  2007/05/28 12:30:55  aamar
 * Moving endParsing call to the end of diet_initialize method.
 *
 * Revision 1.99  2007/04/16 22:43:43  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.98  2007/03/01 15:55:08  ycaniou
 * Added the updateTimeSinceLastSolve() feature
 *
 * Revision 1.97  2007/01/24 20:35:20  ycaniou
 * Commentary only
 *
 * Revision 1.96  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.95  2006/11/07 11:56:16  rbolze
 * Workflow support: make useMaDagSched in the right place.
 *
 * Revision 1.94  2006/11/07 06:27:15  aamar
 * Workflow support: Move useMaDagSched.
 *
 * Revision 1.93  2006/11/06 15:15:20  aamar
 * Workflow support: some correction
 *
 * Revision 1.92  2006/11/06 12:04:09  aamar
 * Workflow:
 *   - Add _diet_wf_file_get and _diet_wf_matrix_get functions.
 *   - Correct a problem with WfExtReader.setup call.
 *
 * Revision 1.91  2006/11/02 17:11:43  rbolze
 * add some debug info
 *
 * Revision 1.90  2006/10/26 14:00:07  aamar
 * Workflow support: check the returned result of xml reader in all cases
 *
 * Revision 1.89  2006/10/20 09:39:35  aamar
 * Some changes for workflow call methods.
 *
 * Revision 1.88  2006/10/19 21:27:32  mjan
 * JuxMem support in async mode. Reorganized data management (DTM and JuxMem) into functions in the spirit of last modifs by Yves.
 *
 * Revision 1.87  2006/09/21 09:11:31  ycaniou
 * Preliminary change in cmake to handle a BATCH variable
 *
 * Revision 1.86  2006/08/27 18:40:10  ycaniou
 * Modified parallel submission API
 * - client: diet_call_batch() -> diet_parallel_call()
 * - SeD: diet_profile_desc_set_batch() -> [...]_parallel()
 * - from now, internal fields are related to parallel not batch
 * and corrected a bug:
 * - 3 types of submission: request among only seq, only parallel, or all
 *   available services (second wasn't implemented, third bug)
 *
 * Revision 1.85  2006/08/09 21:36:29  aamar
 * Transform status code to GRPC code before returning the result of diet_wait_any
 *
 * Revision 1.84  2006/07/25 14:35:36  ycaniou
 * dietJobID changed to dietReqID
 *
 * Revision 1.83  2006/07/10 13:39:46  aamar
 * Correct some warnings
 *
 * Revision 1.82  2006/07/10 11:25:57  aamar
 * Adding the following functions to the API:
 *   - enable_reordering, set_reordering_delta, nodeIsDone,
 *   nodeIsRunning, nodeIsWaiting
 * Adding the reordering support
 * Adding the workflow monitoring support
 *
 * Revision 1.81  2006/07/07 09:19:00  aamar
 * Some changes to be GRPC compliant : cheking session ID, correction of
 *  error codes.
 * Add the function:
 *  - unmrsh_profile_desc (to unmarshall a profile descritpion), maybe this
 *    function needs to be placed in marshalling.cc file.
 *  - getProfileDesc to search a profile by its name.
 *
 * Revision 1.80  2006/06/30 15:41:47  ycaniou
 * DIET is now capable to submit batch Jobs in synchronous mode. Still some
 *   tuning to do (hard coded NFS path for OAR, tests for synchro between
 *   SeD and the batch job in regard to delete files.., more examples).
 *
 * Put the Data transfer section (JuxMem and DTM) before and after the call to
 * the SeD solve, in inline functions
 *   - downloadSyncSeDData()
 *   - uploadSyncSeDData()
 *
 * Revision 1.79  2006/06/29 15:02:41  aamar
 * Make change to handle the new type definition of grpc_function_handle_t (from a grpc_function_handle_s to grpc_function_handle_s*
 *
 * Revision 1.78  2006/06/29 13:05:02  aamar
 * Little change of get_handle function prototype (handle* to handle** for parameter 1)
 *
 * Revision 1.77  2006/06/29 12:32:21  aamar
 * Adding the following functions to be GridRPC compliant :
 *    - diet_get_handle, diet_get_error, diet_error_string, diet_get_failed_session, diet_probe_or
 *    - diet_save_handle, set_req_error (These two functions can be removed from the header file
 *      but the DIET_grpc.cc needs their declaration)
 *
 * Revision 1.76  2006/06/21 23:23:48  ecaron
 * Taking into account the error code for GRPC_ALREADY_INITIALIZED
 *
 * Revision 1.75  2006/06/16 10:37:32  abouteil
 * Chandra&Toueg&Aguilera fault detector support added
 *
 * Revision 1.74  2006/04/14 14:18:50  aamar
 * Two major modifications for workflow support:
 * a. API extension for workflow support:
 *        diet_error_t diet_wf_call(diet_wf_desc_t* profile);
 *        void set_sched (struct AbstractWfSched * sched);
 *        void diet_wf_free(diet_wf_desc_t * profile);
 *        void set_madag_sched(int b);
 *        int _diet_wf_scalar_get(const char * id, void** value);
 *        int _diet_wf_string_get(const char * id, char** value);
 * b. Modify the diet_initialize function to get the MA_DAG reference
 *    is the entry if set in the client config file.
 *
 * Revision 1.73  2006/01/31 16:43:03  mjan
 * Update on deployment of DIET/JuxMem via GoDIET
 *
 * Revision 1.72  2006/01/31 10:07:40  mjan
 * Update on the API of JuxMem
 *
 * Revision 1.71  2006/01/24 06:33:53  hdail
 * Fix made to code to fix bug whereby on certain systems (like Orsay G5K)
 * client successfully schedules job 3 times in a row but fails at submission
 * each time.  I have commented out contract checking as it was causing the
 * problem, adds overhead to job submission, and has not been really used in
 * DIET for several years.  Can easily be uncommented when someone decides to
 * do research on contract checking.  Fix tested at Orsay.
 *
 * Revision 1.69  2006/01/13 10:40:53  mjan
 * Updating DIET for next JuxMem (0.2)
 *
 * Revision 1.67  2005/09/05 16:07:33  hdail
 * Addition of hostname and location information to client and in submit call.
 *
 * Revision 1.66  2005/08/31 14:49:17  alsu
 * New plugin scheduling interface: new estimation vector interface in
 * the client contract-checking code
 *
 * Revision 1.65  2005/08/30 12:41:37  ycaniou
 * Some changes. Management of dietJobID.
 *
 * Revision 1.64  2005/06/14 16:17:12  mjan
 * Added support of DIET_FILE inside JuxMem-DIET for TLSE code
 *
 * Revision 1.57  2005/05/15 15:44:46  alsu
 * minor changes from estimation vector reorganization
 *
 * Revision 1.56  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.55  2005/04/27 01:14:26  ycaniou
 * Added the diet_call_batch() and diet_call_async_batch() in the API and
 * modified the respective solve functions if the client wants to perform
 * a batch submission
 ****************************************************************************/


#include "DIET_client.h"

#include <omniORB4/CORBA.h>
#include <iostream>
#include <fstream>
using namespace std;
#include <unistd.h> // For gethostname()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "debug.hh"
#include "est_internal.hh"
#include "DIET_data_internal.hh"
#if HAVE_FD
#include "fd/fd.h"
#endif
#if HAVE_JUXMEM
#include "JuxMem.hh"
#endif // HAVE_JUXMEM
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeD.hh"
#include "statistics.hh"
#if HAVE_DAGDA
#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"

#ifdef HAVE_CCS
/** Custom client scheduling */
#include "SpecificClientScheduler.hh"
#endif // HAVE_CCS

#ifdef HAVE_MULTICALL
//Multiple async call
#include "MultiCall.hh"
#include <vector>
#endif //MULTICALL

// for workflow support
#ifdef HAVE_WORKFLOW
/** for workflow support */
#include "workflow/CltWfMgr.hh"
#endif
//****

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

#ifdef HAVE_FD
#ifndef DIET_DEFAULT_FD_QOS
/* set a default QoS for fault detector
 * 30s detection time, 60s false detection, 1 month between two errors
 */
#define DIET_DEFAULT_FD_QOS 30.0, 2678400.0, 60.0
#endif

/**
 * Performance measurement for bw and latency of JuxMem
 */
#define JUXMEM_LATENCY_THROUGHPUT 0

/* Recovery after failure func */
static void diet_call_failure_recover(fd_handle fd)
{

}
#endif /* HAVE_FD */

#ifdef WITH_ENDIANNESS
extern bool little_endian;
#endif // WITH_ENDIANNESS
/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The Master Agent reference */
static MasterAgent_var MA = MasterAgent::_nil();
static omni_mutex      MA_MUTEX;
#ifdef HAVE_CCS
static omni_mutex      SCHED_MUTEX;
#endif


/** Error rate for contract checking */
#define ERROR_RATE 0.1

/** Maximum servers to be answered */
static unsigned long MAX_SERVERS = 10;

/** IOR reference sent to the SeD to let him contact the callback server */
static char* REF_CALLBACK_SERVER;
/** Flag for using the asynchronous API (set at configuration time) */
static size_t USE_ASYNC_API = 1;

#ifdef HAVE_WORKFLOW
/** The MA DAG reference */
static MaDag_var MA_DAG = MaDag::_nil();
static WfLogSrv_var myWfLogService = WfLogSrv::_nil();
#endif

/****************************************************************************/
/* Manage MA name and Session Number for data persistency issue             */
/****************************************************************************/

  char* MA_Name;

  int num_Session;

  char file_Name[256];

#if HAVE_JUXMEM
  JuxMem::Wrapper * juxmem;
#endif // HAVE_JUXMEM

/*
 * String representation of error code
 */
// const char * const ErrorCodeStr[] = {
char * ErrorCodeStr[] = {	// because of grpc_error_string that returns a char *
  strdup("GRPC_NO_ERROR"),
  strdup("GRPC_NOT_INITIALIZED"),
  strdup("GRPC_CONFIGFILE_NOT_FOUND"),
 strdup("GRPC_CONFIGFILE_ERROR"),
 strdup("GRPC_SERVER_NOT_FOUND"),
 strdup("GRPC_FUNCTION_NOT_FOUND"),
 strdup("GRPC_INVALID_FUNCTION_HANDLE"),
 strdup("GRPC_INVALID_SESSION_ID"),
 strdup("GRPC_RPC_REFUSED"),
 strdup("GRPC_COMMUNICATION_FAILED"),
 strdup("GRPC_SESSION_FAILED"),
 strdup("GRPC_NOT_COMPLETED"),
 strdup("GRPC_NONE_COMPLETED"),
 strdup("GRPC_OTHER_ERROR_CODE"),
 strdup("GRPC_UNKNOWN_ERROR_CODE"),
 strdup("GRPC_ALREADY_INITIALIZED"),
 strdup("GRPC_LAST_ERROR_CODE")};


diet_error_t
status_to_grpc_code(int err) {
  switch (err) {
    case STATUS_DONE:
      err = GRPC_NO_ERROR;
      break;
  case STATUS_WAITING:
    err = GRPC_NOT_COMPLETED;
    break;
  case STATUS_RESOLVING:
    err = GRPC_NOT_COMPLETED;
    break;
  case STATUS_CANCEL:
    err = GRPC_NO_ERROR;
    break;
  case STATUS_ERROR:
    err = GRPC_OTHER_ERROR_CODE;
    break;
  }

  return err;
}

/****************************************************************************/
/* Initialize and Finalize session                                          */
/****************************************************************************/

BEGIN_API

diet_error_t
diet_initialize(char* config_file_name, int argc, char* argv[])
{
  char*  MA_name;
  int    res(0);
  int    myargc(0);
  char ** myargv(NULL);
  void*  value(NULL);
  char*  userDefName;


#ifdef HAVE_WORKFLOW
  char*  MA_DAG_name(NULL);
  char*  USE_WF_LOG_SERVICE(NULL);
#endif // HAVE_WORKFLOW

  MA_MUTEX.lock();

  if (!CORBA::is_nil(MA)) {
    WARNING(__FUNCTION__ << ": diet_finalize has not been called");
    MA_MUTEX.unlock();
    return 15;
  }

  /* Set arguments for ORBMgr::init */
  if (argc) {
    myargc = argc;
    myargv = (char**)malloc(argc * sizeof(char*));
    for (int i = 0; i < argc; i++)
      myargv[i] = argv[i];
  }

  /* Parsing */
  Parsers::Results::param_type_t compParam[] = {Parsers::Results::MANAME};

  if ((res = Parsers::beginParsing(config_file_name))) {
    MA_MUTEX.unlock();
    return res;
  }
  res = Parsers::parseCfgFile(false,
                              1,
                              (Parsers::Results::param_type_t*) compParam);
  if (res) {
    Parsers::endParsing();
    MA_MUTEX.unlock();
    return res;
  }

  /* Some more checks */
  userDefName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::NAME);

  value = Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (value != NULL)
    WARNING("parsing " << config_file_name << ": no need "
            << "to specify a parent for a client - ignored");

  value = Parsers::Results::getParamValue(Parsers::Results::AGENTTYPE);
  if (value != NULL)
    WARNING("parsing " << config_file_name
            << ": agentType is useless for a client - ignored");

  /* Get the traceLevel */
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *  level = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBtraceLevel");
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }


  /* Get the USE_ASYNC_API flag */
  value = Parsers::Results::getParamValue(Parsers::Results::USEASYNCAPI);
  if (value != NULL)
    USE_ASYNC_API = *(size_t *)(value);

  if (USE_ASYNC_API == 1) {
    /* Initialize the ORB */
    if (ORBMgr::init(myargc, (char**)myargv)) {
      ERROR("ORB initialization failed", 1);
    }
    // Create sole instance of synchronized CallAsyncMgr class
    CallAsyncMgr::Instance();
    // Create servant callback object
    CallbackImpl* cb = new CallbackImpl();
    // activate servant callback
    if (ORBMgr::activate(cb) != 0) return -1;
    CORBA::Object_var obj = cb->_this();
    // create corba client callback serveur reference ...
    REF_CALLBACK_SERVER = ORBMgr::getIORString(obj);
    //This is done in the activate() of the newest ORBMgr version
    //cb->_remove_ref();
    if (REF_CALLBACK_SERVER == NULL) return -1;
  }
  else {
    if (ORBMgr::init(myargc, (char**)myargv, false)) {
      ERROR("ORB initialization failed", 1);
    }
  }

#if HAVE_JUXMEM
  juxmem = new JuxMem::Wrapper(NULL);
#endif // HAVE_JUXMEM

#if HAVE_CKPT


#endif

  /* Find Master Agent */
  MA_name = (char*)
   Parsers::Results::getParamValue(Parsers::Results::MANAME);
   TRACE_TEXT (TRACE_MAIN_STEPS,"MA NAME PARSING = " << MA_name << endl);
   MA_Name = CORBA::string_dup(MA_name);
  MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
  if (CORBA::is_nil(MA)) {
    ERROR("cannot locate Master Agent " << MA_name, 1);
  }

  /* Initialize statistics module */
  stat_init();


  //create_file();
  MA_MUTEX.unlock();

  /** get Num session*/
  num_Session = MA->get_session_num();

#ifdef HAVE_WORKFLOW
  // Workflow parsing
  /* Find the MA_DAG */
  MA_DAG_name = (char*) Parsers::Results::getParamValue(Parsers::Results::MADAGNAME);
  if (MA_DAG_name != NULL) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "MA DAG NAME PARSING = " << MA_DAG_name << endl);
    MA_DAG_name = CORBA::string_dup(MA_DAG_name);
    MA_DAG =
      MaDag::_narrow(ORBMgr::getObjReference(ORBMgr::MA_DAG, MA_DAG_name));
    if (CORBA::is_nil(MA_DAG)) {
      ERROR("Cannot locate MA DAG " << MA_DAG_name, 1);
    }
    else {
      CltWfMgr::instance()->setMaDag(MA_DAG);
    }
  } // end if (MA_DAG_name != NULL)

  // check if the Workflow Log Service is used
    Parsers::Results::getParamValue(Parsers::Results::USEWFLOGSERVICE);
  if (USE_WF_LOG_SERVICE != NULL) {
    if (!strcmp(USE_WF_LOG_SERVICE, "1")) {
      CORBA::Object_var obj =
        ORBMgr::getObjReference(ORBMgr::WFLOGSERVICE, "WfLogService");
      WfLogSrv_var wfLogSrv = WfLogSrv::_narrow(obj);
      if (CORBA::is_nil(wfLogSrv)) {
	ERROR("cannot locate the Workflow Log Service ", 1);
      }
      else {
        CltWfMgr::instance()->setWfLogSrv(wfLogSrv);
      }
    }
  }

  // Init the Xerces engine
  XMLPlatformUtils::Initialize();

#endif

#ifdef HAVE_CCS
  char * specific_scheduling = (char*)
    Parsers::Results::getParamValue(Parsers::Results::USE_SPECIFIC_SCHEDULING);
  if ((specific_scheduling != NULL) && (strlen(specific_scheduling) > 1)) {
    SpecificClientScheduler::setSchedulingId(specific_scheduling);
  }
#endif // HAVE_CCS

#if HAVE_DAGDA
  // Dagda component activation.
  DagdaImpl* tmpDataManager = DagdaFactory::getClientDataManager();
  ORBMgr::activate(tmpDataManager);
#endif // HAVE_DAGDA

/* DAGDA needs some parameters later... */
#if ! HAVE_DAGDA
  /* We do not need the parsing results any more */
  Parsers::endParsing();
#endif

  return 0;
}

diet_error_t
diet_finalize()
{
#if HAVE_WORKFLOW
  // Terminate the xerces XML engine
  XMLPlatformUtils::Terminate();
#endif // HAVE_WORKFLOW

  stat_finalize();
  if (USE_ASYNC_API == 1) {

    CallAsyncMgr * caMgr = CallAsyncMgr::Instance();
    while (caMgr->areThereWaitRules() > 0) {
      omni_thread::sleep(1);
      // must be replace by a call to waitall
      // must be a call to diet_finalize_force ....
      // Maybe we must split async api from sync api ...
    }
    caMgr->release();
  }
  ORBMgr::destroy();
  MA_MUTEX.lock();
  MA = MasterAgent::_nil();
  MA_MUTEX.unlock();

#if HAVE_JUXMEM
  delete(juxmem);
#endif // HAVE_JUXMEM

  /* end fileName */
  // *fileName='\0';
  return 0;
}

END_API


/****************************************************************************/
/* Data handles                                                             */
/****************************************************************************/

BEGIN_API

diet_data_handle_t*
diet_data_handle_malloc()
{
  return new diet_data_handle_t;
}

void*
diet_get(diet_data_handle_t* handle)
{
  return handle->value;
}

int
diet_free(diet_data_handle_t* handle)
{
  delete handle;
  return 0;
}

END_API

/****************************************************************
 *    get available Services in the DIET Platform
 ***************************************************************/

/* Only return the name of services available on the platform */
char**
get_diet_services(int *services_number){
  CORBA::Long length;
  SeqCorbaProfileDesc_t* profileList = MA->getProfiles(length);
  *services_number= (int)length;
  char** services_list = (char**)calloc(length+1,sizeof(char*));
  fflush(stdout);
  for(int i=0;i<*services_number;i++){
    services_list[i]= CORBA::string_dup((*profileList)[i].path);
  }
  return services_list;
}


/****************************************************************
 *   creation of the file that stores identifiers               *
 ***************************************************************/

void create_file()
{
  sprintf(file_Name,"/tmp/ID_FILE.%s.%d",MA_Name,num_Session);
  create_header();

}

void create_header()
{
  const char * header_id = "Data Handle   ";
  const char * header_msg = "Description \n";

  ofstream f(file_Name,ios_base::app|ios_base::ate);
  int cpt = strlen(header_id);
  f.write(header_id,cpt);
  for(int i = 0; i < 10; i++) {
    f.put(' ');
  }
  cpt = strlen(header_msg);
  f.write(header_msg,cpt);
  f.close();
}

/****************************************************************
 *   Add handler id with message msg in the file                *
 ***************************************************************/

void store_id(char* argID, char* msg)
{

  size_t cpt;

  char* msg1 = new char[strlen(msg)+2];
  cpt=strlen(argID);
  ofstream f(file_Name,ios_base::app|ios_base::ate);
  f.write(argID,cpt);
  for(int i = 0; i < 10; i++) {
    f.put(' ');
  }
  sprintf(msg1,"%s\n",msg);
  cpt = strlen(msg1);
  f.write(msg1,cpt);
  f.close();
}


/******************************************************************
 *  Free persistent data identified by argID                     *
 *****************************************************************/
int
diet_free_persistent_data(char* argID)
{
  char statMsg[128];
  sprintf(statMsg,"%s %s",__FUNCTION__,argID);
  stat_in("client", statMsg);
  if(MA->diet_free_pdata(argID)!=0) {
    stat_out("client", statMsg);
    return 1;
  } else {
    cerr << "UNKNOWN DATA" << endl;
    return 0;
  }
}

/******************************************************************
 *  JuxMem data management functions                              *
 ******************************************************************/
#if HAVE_JUXMEM
inline void
uploadClientDataJuxMem(diet_profile_t* profile)
{
  int i = 0;
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif

  for (i = 0; i <= profile->last_inout; i++) {
    /**
     * If there is no data id (for both IN or INOUT case), this a new
     * data.  Therefore, attach the user input and do a msync on
     * JuxMem of the data.  Of course do that only if data are
     * persistent! Else let CORBA move them
     */
    if (profile->parameters[i].desc.id == NULL &&
	profile->parameters[i].desc.mode == DIET_PERSISTENT) {
      profile->parameters[i].desc.id =
	juxmem->attach(profile->parameters[i].value,
		       data_sizeof(&(profile->parameters[i].desc)),
		       1, 1, EC_PROTOCOL, BASIC_SOG);
      TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = "
		 << profile->parameters[i].desc.id
		 << " for IN data has been attached inside JuxMem!\n");
      /* The local memory is flush inside JuxMem */
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_begin, NULL);
#endif
      juxmem->msync(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_end, NULL);
	timersub(&t_end, &t_begin, &t_result);
	latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
	throughput = (data_sizeof(&(profile.parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
	sprintf(statMsg, "IN/INOUT %s msync. Latency: %f, Throughput: %f\n", profile->parameters[i].desc.id, latency, throughput);
	stat_out("JuxMem", statMsg);
#endif
	if (i <= profile->last_in) {
	  juxmem->detach(profile->parameters[i].value);
	}
    }
  }
}
#endif

#if HAVE_JUXMEM
inline void
downloadClientDataJuxMem(diet_profile_t* profile)
{
  int i = 0;
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif

  for (i = profile->last_in + 1; i <= profile->last_out; i++) {
    /**
     * Retrieve INOUT or OUT data only if DIET_PERSISTENT_RETURN.
     * Note that for INOUT data, the value can be already initialized.
     * In this case, JuxMem will use this address to store the
     * data. If value is NULL, a memory area will allocated.
     */
    if (profile->parameters[i].desc.id != NULL &&
	profile->parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
      if (i <= profile->last_inout) {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Reading IN_OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
      } else {
	TRACE_TEXT(TRACE_MAIN_STEPS, "Retrieving OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ...\n");
	profile->parameters[i].value = juxmem->mmap(profile->parameters[i].value, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
      }
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_begin, NULL);
#endif
	juxmem->acquireRead(profile->parameters[i].value);
	juxmem->release(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
	gettimeofday(&t_end, NULL);
	timersub(&t_end, &t_begin, &t_result);
	latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
	timersub(&t_begin, &(this->t_begin), &t_result);
	time = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
	throughput = (data_sizeof(&(profile.parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
	fprintf(stderr, "%f INOUT %s read. Latency: %f, Throughput: %f\n", time, profile.parameters[i].desc.id, latency, throughput);
#endif
      juxmem->detach(profile->parameters[i].value);
    }
  }
}
#endif

/****************************************************************************/
/* GridRPC call functions                                                   */
/****************************************************************************/


/****************************************
 * Request submission
 ****************************************/

diet_error_t
request_submission(diet_profile_t* profile,
                   SeD_var& chosenServer,
                   estVector_t& estim)
{
  static int nb_tries(3);
  int server_OK(0), subm_count, data_OK(0);
  corba_pb_desc_t corba_pb;
  corba_response_t* response(NULL);
  char* bad_id(NULL);
  diet_reqID_t reqID;
  char statMsg[128];
  chosenServer = SeD::_nil();

  if (mrsh_pb_desc(&corba_pb, profile)) {
    ERROR("profile is wrongly built", 1);
  }

  /* Request submission : try nb_tries times */

  stat_in("client","request_submission");
  subm_count = 0;
  do {
    response = NULL;
#if ! HAVE_JUXMEM && ! HAVE_DAGDA
    /* data property base_type and type retrieval : used for scheduler */
    int i = 0;

    for (i = 0, data_OK = 0 ;
         (i <= corba_pb.last_out && data_OK == 0) ;
         i++) {
      char* new_id = strdup(corba_pb.param_desc[i].id.idNumber);

      if(strlen(new_id) != 0) {
	/* then data is known. Check that it's still in plaform */
        corba_data_desc_t *arg_desc = new corba_data_desc_t;
        arg_desc = MA->get_data_arg(new_id);
        const char * tmp="-1";
        if( strcmp(CORBA::string_dup(arg_desc->id.idNumber),tmp) == 0 ) {
          bad_id = new_id;
          data_OK = 1;
        } else {
          const_cast<corba_data_desc_t&>(corba_pb.param_desc[i]) = *arg_desc;
        }
      }
    }
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA

#if HAVE_DAGDA && ! HAVE_JUXMEM
    {
      diet_error_t ret;
      if ((ret = dagda_get_data_desc(corba_pb, MA)))
        return ret;
    }
#endif // HAVE_DAGDA && ! HAVE_JUXMEM

    if(data_OK == 0) {
      /* Submit to the agent. */
      try {
        response = MA->submit(corba_pb, MAX_SERVERS);
      } catch (CORBA::Exception& e) {
        CORBA::Any tmp;
        tmp <<= e;
        CORBA::TypeCode_var tc = tmp.type();
        if (response)
          delete response;
        ERROR("caught a CORBA exception (" << tc->name()
              << ") while submitting problem", 1);
      }

      /* set the req ID here before checking the errors */
      if (response != NULL) {
	reqID = response->reqID;
      }

      /* Check response */
      if (!response || response->servers.length() == 0) {
        WARNING("no server found for problem " << corba_pb.path);
        server_OK = -1;

      } else {

        if (TRACE_LEVEL >= TRACE_MAIN_STEPS) {
          TRACE_TEXT(TRACE_MAIN_STEPS,
                     "The Master Agent found the following server(s):\n");
          for (size_t i = 0; i < response->servers.length(); i++) {
            TRACE_TEXT(TRACE_MAIN_STEPS,
                       "    " << response->servers[i].loc.hostName << ":"
                       << response->servers[i].loc.port << "\n");
          }
        }
#if 0
        /** Contract checking has been commented out in Jan 2006 by Holly
         * because noone has used it for years.  Leaving code here as
         * background info for someone pursuing research in contract
         * checking. */

        /* Check the contracts of the servers answered. */
        server_OK = 0;
        while ((size_t) server_OK < response->servers.length()) {
          try {
            int           idx       = server_OK;
            SeD_ptr       server    = response->servers[idx].loc.ior;

            estVector_t ev = &(response->servers[idx].estim);
            CORBA::Double totalTime =
              diet_est_get_internal(ev, EST_TOTALTIME, HUGE_VAL);

            if (server->checkContract(response->servers[idx].estim,
                                      corba_pb)) {
              server_OK++;
              continue;
            }

            ev = &(response->servers[idx].estim);
            CORBA::Double newTotalTime =
              diet_est_get_internal(ev, EST_TOTALTIME, HUGE_VAL);
            if ((totalTime == newTotalTime) ||
                ((newTotalTime - totalTime) <
                 (ERROR_RATE * MAX(totalTime, newTotalTime)))) {
              break;
            }
            server_OK++;
          } catch (CORBA::Exception& e) {
            server_OK++;
            continue;
          }
        }
        if ((size_t) server_OK == response->servers.length()) {
          server_OK = -1;
        }
#endif    // Contract checking commented out
        server_OK = 0;    // Use this when no contract checking
      } // end else  [if (!response || response->servers.length() == 0)]
    } // end if data ok
  } while ((response) && (response->servers.length() > 0) &&
           (server_OK == -1) && (++subm_count < nb_tries) && (data_OK == 0));

  if(data_OK == 1) {
    ERROR (" data with ID " <<  bad_id << " not inside the platform.", 1);
    delete (bad_id);
  } else {

    if (!response || response->servers.length() == 0) {
      if (response) {
        delete response;
      }
      ERROR("unable to find a server", GRPC_SERVER_NOT_FOUND);
    }
    if (server_OK == -1) {
      delete response;
      ERROR("unable to find a server after " << nb_tries << " tries."
          << "The platform might be overloaded, try again later please", GRPC_SERVER_NOT_FOUND);
    }

    if (server_OK >= 0) {
      chosenServer = response->servers[server_OK].loc.ior;
      /* The estimation vector of the chosen SeD is copied into the profile.
       * This is done because:
       * 1/ the SeD cannot store the estimations for all requests as many
       * won't be followed by a solve and it is not possible to know which ones
       * 2/ the estimation vector depends on the value of the parameters of the
       * profile (and not only on the description) so each request has a
       * different estimation vector.
       */
      estim   = new corba_estimation_t(response->servers[server_OK].estim);

      reqID = response->reqID;
#ifdef HAVE_FD
      fd_handle fd = fd_make_handle();
      fd_set_qos(fd, DIET_DEFAULT_FD_QOS);
      fd_set_service(fd, response->servers[server_OK].loc.hostName, 1);
      fd_observe(fd);
#endif

#ifdef HAVE_CCS
      if (SpecificClientScheduler::isEnabled()) {
	SCHED_MUTEX.lock();
	SpecificClientScheduler::start(chosenServer, response);
	SCHED_MUTEX.unlock();
      }
#endif // HAVE CCS

#ifdef HAVE_MULTICALL
      MultiCall::set_response(response);
#endif //HAVE_MULTICALL

    }
    sprintf(statMsg, "request_submission %ld", (unsigned long) reqID);
    stat_out("Client",statMsg);
    profile->dietReqID = reqID ;
  }

  return 0;
}

/****************************************
 * Synchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call, grpc_call and grpc_call_argstack.
 */
diet_error_t
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer, estVector_t estimVect)
{
  diet_error_t res(0);
  int solve_res(0);
  corba_profile_t corba_profile;
  char statMsg[128];
  corba_estimation_t emptyEstimVect;
  stat_in("client","diet_call");

  if (CORBA::is_nil(chosenServer)) {
    if (!(res = request_submission(profile, chosenServer, estimVect))) {
      corba_profile.estim = *estimVect; // copy estimation vector
      delete estimVect; // vector was allocated in request_submission
    } else { // error in request_submission
      return res;
    }
    if (CORBA::is_nil(chosenServer)) {
      return 1;
    }
  }
  /* Add estimation vector to the corba_profile */
  /* (use an empty vector in case it is not provided, eg for grpc calls) */
  else if (estimVect != NULL) {
    corba_profile.estim = *estimVect;
  } else {
    corba_profile.estim = emptyEstimVect;
  }

  // Server is chosen, update its timeSinceLastSolve
  chosenServer->updateTimeSinceLastSolve() ;
#if HAVE_JUXMEM
  uploadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM

#if HAVE_DAGDA
  dagda_mrsh_profile(&corba_profile, profile, MA);
#else
  /* Convert profile (with data) in corba_profile (with data) */
  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    ERROR("profile is wrongly built", 1);
  }

  int j = 0;
  bool found = false;
  while ((j <= corba_profile.last_out) && (found == false)) {
    if (diet_is_persistent(corba_profile.parameters[j])) {
// && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
       found = true;
    }
    j++;
  }
  if(found == true){
     create_file();
  }
#if ! HAVE_JUXMEM
  /* data property base_type and type retrieval: used for scheduler */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    if(strlen(new_id) != 0) {
      corba_data_desc_t* arg_desc = new corba_data_desc_t;
      arg_desc = MA->get_data_arg(new_id);
      const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
    }
  }

  /* generate new ID for data if not already existant */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    if((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
       (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) &&
       (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
      {
	char* new_id = MA->get_data_id();
	corba_profile.parameters[i].desc.id.idNumber = new_id;
      }
  }
#endif // ! HAVE_JUXMEM
#endif // HAVE_DAGDA
  /* Computation */
  sprintf(statMsg, "computation %ld", (unsigned long) profile->dietReqID);
  try {
    stat_in("client",statMsg);

    TRACE_TEXT(TRACE_MAIN_STEPS, "Calling the ref Corba of the SeD\n");
#if HAVE_FD
    fd_set_transition_handler(diet_call_failure_recover);
#endif
    solve_res = chosenServer->solve(profile->pb_name, corba_profile);
    stat_out("Client",statMsg);
   } catch(CORBA::SystemException& e) {
    ERROR("Got a CORBA " << e._name() << " exception ("
        << e.NP_minorString() << ")\n",1) ;
   } catch(CORBA::UserException& e) {
    ERROR("Got an exception "  << e._name() << endl, 1);
   }

#if ! HAVE_DAGDA
  /* reaffect identifier */
  for(int i = 0;i <= profile->last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
        (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)) {
      profile->parameters[i].desc.id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    }
  }

  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }
#else
  dagda_download_SeD_data(profile, &corba_profile);
#endif // ! HAVE_DAGDA

#if HAVE_JUXMEM
  downloadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM

  sprintf(statMsg, "diet_call %ld", (unsigned long) profile->dietReqID);
  stat_out("Client",statMsg);
  return solve_res;
}

BEGIN_API

/**
 * Request + computation submissions.
 */
diet_error_t
diet_call(diet_profile_t* profile)
{
  SeD_var chosenServer = SeD::_nil();

#ifdef HAVE_CCS
  if (SpecificClientScheduler::isEnabled()) {
    SpecificClientScheduler::pre_diet_call();
  }
#endif // HAVE_CCS

  diet_error_t err = diet_call_common(profile, chosenServer, NULL);

#ifdef HAVE_CCS
 if (SpecificClientScheduler::isEnabled()) {
    SpecificClientScheduler::post_diet_call();
  }
#endif // HAVE_CCS

#ifdef WITH_ENDIANNESS
  // reswap  in and inout parameter
  if (!little_endian) {
    post_call(profile);
  }
#endif // WITH_ENDIANNESS
  return err;
}

#if defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call(diet_profile_t* profile)
{
  diet_profile_set_parallel(profile) ;
  return diet_call(profile) ;
}
diet_error_t
diet_sequential_call(diet_profile_t* profile)
{
  diet_profile_set_sequential(profile) ;
  return diet_call(profile) ;
}
#endif

END_API


/****************************************
 * Asynchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call_async, grpc_call-async
 * and grpc_call_argstack_async.
 */
diet_error_t
diet_call_async_common(diet_profile_t* profile,
                       SeD_var& chosenServer,
                       estVector_t estimVect)
{
  corba_profile_t corba_profile;
  corba_estimation_t emptyEstimVect;
  CallAsyncMgr* caMgr;
  diet_error_t res(0);
  // get sole CallAsyncMgr singleton
  caMgr = CallAsyncMgr::Instance();

  stat_in("client","diet_call_async");

  try {

    if (CORBA::is_nil(chosenServer)) {
      if (!(res = request_submission(profile, chosenServer, estimVect))) {
        corba_profile.estim = *estimVect; // copy estimation vector
        delete estimVect; // vector was allocated in request_submission
      } else { // error in request_submission
        caMgr->setReqErrorCode(profile->dietReqID, res);
        return res;
      }
      if (CORBA::is_nil(chosenServer)) {
	caMgr->setReqErrorCode(profile->dietReqID, GRPC_SERVER_NOT_FOUND);
        return GRPC_SERVER_NOT_FOUND;
      }
    }
    /* Add estimation vector to the corba_profile */
    /* (use an empty vector in case it is not provided, eg for grpc calls) */
    else if (estimVect != NULL) {
      corba_profile.estim = *estimVect;
    } else {
      corba_profile.estim = emptyEstimVect;
    }

#ifdef HAVE_MULTICALL
    corba_response_t * corba_response = MultiCall::get_response();
    vector<int> nb_scenarios = MultiCall::cerfacsSchedule();
    char *s, *stemp, c = '#';
    diet_paramstring_get(diet_parameter(profile, 2), &s, NULL);
    int sSize = strlen(s);
    for (int counter = 0; counter < corba_response->servers.length(); counter++) {
      stemp = (char*)malloc((sSize + 1) * sizeof(char));;
      stemp[0] = '\0';
      //if there is at least a scenario on this SeD
      if (nb_scenarios[counter] != 0) {
	chosenServer = corba_response->servers[counter].loc.ior;
	diet_scalar_set(diet_parameter(profile, 0), &(nb_scenarios[counter]), DIET_VOLATILE, DIET_INT);
	//splits the mnemonics
	for (int counter2 = 0; counter2 < nb_scenarios[counter]; counter2++) {
	  if (counter2 != 0) { stemp = strcat(stemp, &c);}
	  s = strtok(s, &c);
	  stemp = strcat(stemp, s);
	  s = NULL;
	}
	diet_paramstring_set(diet_parameter(profile, 2), stemp, DIET_VOLATILE);
#endif //HAVE_MULTICALL

#if HAVE_JUXMEM
    uploadClientDataJuxMem(profile);
#endif

#if HAVE_DAGDA
  dagda_mrsh_profile(&corba_profile, profile, MA);
#else
    if (mrsh_profile_to_in_args(&corba_profile, profile)) {
      caMgr->setReqErrorCode(profile->dietReqID, GRPC_INVALID_FUNCTION_HANDLE);
      ERROR("profile is wrongly built", 1);
    }

    int j = 0;
    bool found = false;
    while ((j <= corba_profile.last_out) && (found == false)) {
      if (diet_is_persistent(corba_profile.parameters[j])) {
  // && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
        found = true;
      }
      j++;
    }
    if(found == true){
      create_file();
    }
#endif

#if ! HAVE_JUXMEM
#if ! HAVE_DAGDA
    int i = 0;
    /* data property base_type and type retrieval : used for scheduler */
    for(i = 0;i <= corba_profile.last_out;i++) {
      char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
      if(strlen(new_id) != 0) {
        corba_data_desc_t *arg_desc = new corba_data_desc_t;
        arg_desc = MA->get_data_arg(new_id);
        const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
      }
    }
    /* generate new ID for data if not already existant */
    for(i = 0;i <= corba_profile.last_out;i++) {
      if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
          (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)
     && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
      {
        char* new_id = MA->get_data_id();
        corba_profile.parameters[i].desc.id.idNumber = new_id;
      }
    }
#endif // ! HAVE_DAGDA
#endif // ! HAVE_JUXMEM

    // create corba client callback server...
    // TODO : modify addAsyncCall function because profile has the reqID
    if (caMgr->addAsyncCall(profile->dietReqID, profile) != 0) {
      return 1;
    }

    stat_in("client","computation_async");
    chosenServer->solveAsync(profile->pb_name, corba_profile,
                             REF_CALLBACK_SERVER);

    stat_out("client","computation_async");

    if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
      INTERNAL_ERROR("returned profile is wrongly built", 1);
    }

#if HAVE_JUXMEM
    downloadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM

#ifdef HAVE_MULTICALL
      } //endif (nbDags != 0)
      free(stemp);
    } //end for (for each SeD)
#endif

  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING("exception caught in " << __FUNCTION__ << '(' << p << ')');
    } else {
      WARNING("exception caught in " << __FUNCTION__ << '(' << tc->id() << ')');
    }
    profile->dietReqID = -1;
    return 1;
  }
  catch (...) {
    WARNING("exception caught in " << __FUNCTION__);
    profile->dietReqID = -1;
    return 1;
  }

  stat_out("Client","diet_call_async");
  return 0;
}

BEGIN_API
/**
 * Request + asynchronous computation submissions.
 */
diet_error_t
diet_call_async(diet_profile_t* profile, diet_reqID_t* reqID)
{
  SeD_var chosenServer = SeD::_nil();
  diet_error_t err = diet_call_async_common(profile, chosenServer, NULL);
  *reqID = profile->dietReqID;
  set_req_error(*reqID, err);
  return err;
}

#if defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call_async(diet_profile_t* profile, diet_reqID_t* reqID)
{
  diet_profile_set_parallel(profile) ;
  return diet_call_async(profile, reqID) ;
}
diet_error_t
diet_sequential_call_async(diet_profile_t* profile, diet_reqID_t* reqID)
{
  diet_profile_set_sequential(profile) ;
  return diet_call_async(profile, reqID) ;
}
#endif

END_API


/****************************************************************************/
/* Asynchronous control and wait functions                                  */
/****************************************************************************/

BEGIN_API

/****************************************************************************
 * diet_probe GridRPC function.
 * This function probes status of an asynchronous request.
 * return reqID status.
 ****************************************************************************/
int
diet_probe(diet_reqID_t reqID)
{
  int err = CallAsyncMgr::Instance()->getStatusReqID(reqID);
  /**
   * transform the request status as defined by the CallAsyncMgr to its
   * equivalent in GridRPC standard
  STATUS_DONE = 0, // Result is available in local memory
  STATUS_WAITING,       // End of solving on Server, result comes
  STATUS_RESOLVING,          // Request is currently solving on Server
  STATUS_CANCEL,	// Cancel is called on a reqID.
  STATUS_ERROR		// Error caught
   */
  switch (err) {
    case STATUS_DONE:
      err = GRPC_NO_ERROR;
      break;
  case STATUS_WAITING:
    err = GRPC_NOT_COMPLETED;
    break;
  case STATUS_RESOLVING:
    err = GRPC_NOT_COMPLETED;
    break;
  case STATUS_CANCEL:
    err = GRPC_NO_ERROR;
    break;
  case STATUS_ERROR:
    err = GRPC_OTHER_ERROR_CODE;
    break;
  }

  return err;
}

/****************************************************************************
 * diet_cancel GridRPC function.
 * This function erases all persistent data that are manipulated by the reqID
 * request. Do not forget to calget_data_handle on data you would like
 * to save.
 ****************************************************************************/
int
diet_cancel(diet_reqID_t reqID)
{
  return CallAsyncMgr::Instance()->deleteAsyncCall(reqID);
}

/****************************************************************************
 * diet_cancel_all GridRPC function.
 * This function executes a diet_cancel for every client request.
 ****************************************************************************/
int
diet_cancel_all() {
  return CallAsyncMgr::Instance()->deleteAllAsyncCall();
}


int
diet_wait(diet_reqID_t reqID)
{
  // check if all request ID is valid
  if (!CallAsyncMgr::Instance()->checkSessionID(reqID))
    return GRPC_INVALID_SESSION_ID;

  try {
      // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[1];
    simpleWait[0].reqID = reqID;
    simpleWait[0].op = WAITOPERATOR(SOLE);
    Rule * rule = new Rule;
    rule->length = 1;
    rule->ruleElts = simpleWait;
    rule->status = STATUS_RESOLVING;

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, be careful using diet_cancel
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  }
  catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what="
          << e.what() << ')', STATUS_ERROR);
  }

  return STATUS_ERROR;
}

/*****************************************************************************
 * diet_wait_and GridRPC function
 * return error status
 * three args :
 *      1 - reqID table.
 *      2 - size of the table.
 *****************************************************************************/
int
diet_wait_and(diet_reqID_t* IDs, size_t length)
{
  // check if all the session IDs in the array are valid
  for (unsigned int ix=0; ix<length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(IDs[ix]))
      return GRPC_INVALID_SESSION_ID;
  }

  request_status_t rst = STATUS_ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (size_t k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(AND);
    }
    Rule * rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules using some of this
    // reqID(AsyncCall) So, careful using diet_cancel
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  }
  catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what=" << e.what() << ')',
          STATUS_ERROR);
  }
  return rst;
}

/*****************************************************************************
 * diet_wait_or GridRPC function
 * return error status
 * three args :
 *      1 - reqID table.
 *      2 - size of the table.
 *      3 - received reqID
 *****************************************************************************/
int
diet_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr)
{
  request_status_t rst = STATUS_ERROR;

  // check if all the session IDs in the array are valid
  for (unsigned int ix=0; ix<length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(IDs[ix]))
      return GRPC_INVALID_SESSION_ID;
  }

  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (unsigned int k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(OR);
    }
    Rule * rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;

    // get lock on condition/waitRule
    // and manage return rule status
    switch (CallAsyncMgr::Instance()->addWaitRule(rule)) {
    case STATUS_DONE:
      for (unsigned int k = 0; k < length; k++) {
        if (CallAsyncMgr::Instance()->getStatusReqID(IDs[k]) == STATUS_DONE) {
          *IDptr = IDs[k];
          return STATUS_DONE;
        }
      }
      return STATUS_ERROR;
    case STATUS_CANCEL:
      return STATUS_CANCEL;
    case STATUS_ERROR:
      return STATUS_ERROR;
    default:
      return -1; // Unexpected error, no value describing it
      // NOTES: Be careful, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, careful using diet_cancel
    }
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  } catch (const exception& e) {
    ERROR(__FUNCTION__ << ": unexpected exception (what="
          << e.what() << ')', STATUS_ERROR);
  }
  return rst;
}

/*****************************************************************************
 * diet_wait_all GridRPC function
 * return error status
 *****************************************************************************/
int
diet_wait_all()
{
  return CallAsyncMgr::Instance()->addWaitAllRule();
}

/*****************************************************************************
 * diet_wait_any GridRPC function
 * return the ID of the received request
 *****************************************************************************/
int
diet_wait_any(diet_reqID_t* IDptr)
{
  grpc_error_t err = CallAsyncMgr::Instance()->addWaitAnyRule(IDptr);
  err = status_to_grpc_code(err);
  return err;
}


/***************************************************************************
 * return the error code of the asynchronous call identified by reqID
 ***************************************************************************/
diet_error_t
diet_get_error(diet_reqID_t reqID) {
  return CallAsyncMgr::Instance()->getReqErrorCode(reqID);
}
/***************************************************************************
 * return the corresponding error string
 ***************************************************************************/
char *
diet_error_string(diet_error_t error) {
  if (error<0 || error>16)
    return strdup("GRPC_UNKNOWN_ERROR CODE");
  return ErrorCodeStr[error];
}
/***************************************************************************
 * return identifier of the failed session
 ***************************************************************************/
diet_error_t
diet_get_failed_session(diet_reqID_t* reqIdPtr) {
  return CallAsyncMgr::Instance()->getFailedSession(reqIdPtr);
}
/***************************************************************************
 * check if one of the requests contained in the array id reqIdArray has
 * completed.
 * Return the completed request ID if exist. Otherwise return an error code
 ***************************************************************************/
diet_error_t
diet_probe_or(diet_reqID_t* reqIdArray,
	      size_t length,
	      diet_reqID_t* reqIdPtr) {
  unsigned int ix;
  int reqStatus;
  // check if all request IDs are valid
  for (ix=0; ix< length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(reqIdArray[ix]))
      return GRPC_INVALID_SESSION_ID;
  }
  for (ix=0; ix< length; ix++) {
    reqStatus = CallAsyncMgr::Instance()->getStatusReqID(reqIdArray[ix]);
    if  (reqStatus == STATUS_DONE) {
      *reqIdPtr = reqIdArray[ix];
      return GRPC_NO_ERROR;
    }
  } // end for ix

  return GRPC_NONE_COMPLETED;
}

/***************************************************************************
 * Get the function handle linked to reqID
 ***************************************************************************/
diet_error_t
diet_get_handle(grpc_function_handle_t** handle,
		diet_reqID_t sessionID) {

  return CallAsyncMgr::Instance()->getHandle(handle, sessionID);
}
/***************************************************************************
 * Save the specified handle and associate it to a sessionID
 ***************************************************************************/
void
diet_save_handle(diet_reqID_t sessionID,
		 grpc_function_handle_t* handle) {
  CallAsyncMgr::Instance()->saveHandle(sessionID, handle);
}

/***************************************************************************
 * Set the error code of the defined session (reqID)
 ***************************************************************************/
void
set_req_error(diet_reqID_t sessionID,
	      diet_error_t error) {
  CallAsyncMgr::Instance()->setReqErrorCode(sessionID, error);
}

// for workflow support
#ifdef HAVE_WORKFLOW
/**
 * Workflow execution method
 */
diet_error_t
diet_wf_call(diet_wf_desc_t* profile) {
  if (CORBA::is_nil(MA_DAG)) {
      ERROR("No MA DAG defined", 1);
  }
  switch(profile->level) {
    case DIET_WF_DAG:
      return CltWfMgr::instance()->wfDagCall(profile);
    case DIET_WF_FUNCTIONAL:
      return CltWfMgr::instance()->wfFunctionalCall(profile);
  }
}

/**
 * Get a request ID for interdependent dags submit
 */
int
diet_wf_get_reqID() {
  if (CORBA::is_nil(MA_DAG)) {
      ERROR("No MA DAG defined", 1);
  }
  return (int) CltWfMgr::instance()->getNewWfReqID();
}

/**
 * Interdependent dags execution method
 */
diet_error_t
diet_wf_multi_call(diet_wf_desc_t* profile, int wfReqID) {
  if (profile->level != DIET_WF_DAG) {
    return 1;
  }
  profile->wfReqID = wfReqID;
  return CltWfMgr::instance()->wfDagCall(profile);
}

/**
 * terminate a workflow session *
 * and free the memory *
 */

void
diet_wf_free(diet_wf_desc_t * profile) {
  CltWfMgr::instance()->wf_free(profile);
}


/**
 * Get a scalar result of the workflow
 */
int
_diet_wf_scalar_get(diet_wf_desc_t * profile,
                    const char * id,
		    void** value) {
  return CltWfMgr::instance()->getWfOutputScalar(profile, id, value);
} // end _diet_wf_scalar_get


/**
 * Get a string result of the workflow
 */
int
_diet_wf_string_get(diet_wf_desc_t * profile,
                    const char * id,
		    char** value) {
  return CltWfMgr::instance()->getWfOutputString(profile, id, value);
} // end _diet_wf_string_get


int
_diet_wf_file_get(diet_wf_desc_t * profile,
                  const char * id,
		  size_t* size, char** path) {
  return CltWfMgr::instance()->getWfOutputFile(profile, id, size, path);
}

int
_diet_wf_matrix_get(diet_wf_desc_t * profile,
                    const char * id, void** value,
		    size_t* nb_rows, size_t *nb_cols,
		    diet_matrix_order_t* order) {
  return CltWfMgr::instance()->getWfOutputMatrix(profile, id, value, nb_rows, nb_cols, order);
}

int
_diet_wf_container_get(diet_wf_desc_t * profile,
                       const char * id,
                       char** dataID) {
  return CltWfMgr::instance()->getWfOutputContainer(profile, id, dataID);
}

/**
 * Print the value of all exit ports of a dag or functional wf
 */
int
get_all_results(diet_wf_desc_t * profile) {
  switch(profile->level) {
    case DIET_WF_DAG:
      return CltWfMgr::instance()->printAllDagResults(profile);
    case DIET_WF_FUNCTIONAL:
      return CltWfMgr::instance()->printAllFunctionalWfResults(profile);
    default:
      ;
  }
} // end get_all_results

#endif // HAVE_WORKFLOW

END_API

// this function place is marshalling.cc file
// to fix if necessary
int unmrsh_profile_desc( diet_profile_desc_t* dest,
			 const corba_profile_desc_t* src) {
  dest->path       = strdup(src->path);
  dest->last_in    = src->last_in;
  dest->last_inout = src->last_inout;
  dest->last_out   = src->last_out;
  dest->param_desc = new diet_arg_desc_t[src->last_out + 1];
  for (int i = 0; i <= src->last_out; i++) {
    (dest->param_desc[i]).base_type = (diet_base_type_t)((src->param_desc[i]).base_type);
    (dest->param_desc[i]).type      = (diet_data_type_t)(src->param_desc[i]).type;
  }
#if defined HAVE_ALT_BATCH
  dest->parallel_flag = src->parallel_flag ;
#endif

  // unmarshall the aggregator field
  // TO FIX
  // Since this function is used only by GRPC client lib side, this is not
  // necessary

  return 0;
}

/**
 * return the list of all available profiles
 */
SeqCorbaProfileDesc_t*
getProfiles() {
  CORBA::Long len;
  if (MA) {
    return MA->getProfiles(len);
  }
  return NULL;
}
/**
 * search for a particular service
 * if the service is found, it is stored in the parameter profiles and the
 * function return true
 * otherwise return false, the profile parameter is unchanged
 */
bool
getProfileDesc(const char * srvName, diet_profile_desc_t& profile) {
  SeqCorbaProfileDesc_t * allProfiles = getProfiles();
  if (allProfiles) {
    for (unsigned int ix=0; ix < allProfiles->length(); ix++) {
      if (!strcmp ( (*allProfiles)[ix].path,
		    srvName)) {
	// The service is found
        TRACE_TEXT (TRACE_MAIN_STEPS,"The service " << srvName << " is found " << endl);
	// this function place is marshalling.cc file
	// to fix is necessary
	unmrsh_profile_desc( &profile,
			     &((*allProfiles)[ix]));
	return true;
      }
    }
  }
  TRACE_TEXT (TRACE_MAIN_STEPS,"The service " << srvName << " was not found" << endl);
  return false;
}

/*
 * get all the session IDs
 * the array must be deleted by the caller
 */
diet_reqID_t*
get_all_session_ids(int& len) {
  return CallAsyncMgr::Instance()->getAllSessionIDs(len);
}


MasterAgent_var
getMA() {
  return MA;
}

