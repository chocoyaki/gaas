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
 * Revision 1.170  2011/03/20 18:57:26  bdepardo
 * Be more robust when logComponent initialization fails
 *
 * Revision 1.169  2011/03/18 16:32:26  bdepardo
 * No need to unbind Dagda elements in DIET_finalize as they automatically
 * unbind themselves during destruction
 *
 * Revision 1.168  2011/03/16 22:20:02  bdepardo
 * Correclty destroy the CltWfMgr
 *
 * Revision 1.167  2011/02/28 15:27:57  bdepardo
 * Add try/catch blocks around MA_MUTEX deletion
 *
 * Revision 1.166  2011/02/24 18:16:12  bdepardo
 * Return GRPC_NOT_INITIALIZED when initialization fails
 *
 * Revision 1.165  2011/02/24 16:57:02  bdepardo
 * Use new parser
 *
 * Revision 1.164  2011/02/23 23:14:27  bdepardo
 * Fixed multiple diet_initialize calls
 *
 * Revision 1.163  2011/02/23 15:05:18  bdepardo
 * Catch exception when opening the configuration file.
 *
 * Revision 1.162  2011/02/10 23:21:54  hguemar
 * fixes compilation
 *
 * Revision 1.161  2011/02/10 23:19:01  hguemar
 * fixes some issues detected by latest cppcheck (1.47)
 *
 * Revision 1.160  2011/02/10 17:43:16  bdepardo
 * Typo
 *
 * Revision 1.159  2011/02/09 11:30:07  bdepardo
 * Quick hack, for the client to work with the new parser
 *
 * Revision 1.158  2011/02/03 19:59:53  bdepardo
 * Reduce variables scope
 *
 * Revision 1.157  2011/01/11 00:24:32  hguemar
 * replace value by appropriate error constant
 *
 * Revision 1.156  2010/12/29 14:56:34  hguemar
 * minor fixes in code error handling
 *
 * Revision 1.155  2010/12/29 14:55:49  hguemar
 * Fix successive diet_finalize() successive call crash
 *
 * Revision 1.154  2010/12/28 10:35:45  hguemar
 * move parsing error codes from src/utils/Parsers.hh to include/DIET_grpc.h
 *
 * Revision 1.153  2010/12/27 16:39:31  bdepardo
 * Removed a few "unused variable" warnings
 *
 * Revision 1.152  2010/12/24 10:33:04  hguemar
 * minor fixes to DIET_client.cc
 *
 * Revision 1.151  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.150  2010/11/05 02:56:12  glemahec
 * Remove omni_thread_fatal error: MA_MUTEX is now a pointer initialized at the beginning of diet_initialize.
 *
 * Revision 1.149  2010/11/04 23:20:38  bdepardo
 * Set signal handlers to default when entering diet_finalize(), as the
 * handler can cause multiple calls to diet_finalize().
 *
 * Revision 1.148  2010/10/18 07:30:00  bisnard
 * - removed USE_ASYNC_API ifdef
 * - fixed bug with MA_MUTEX causing diet_initialize to hang
 * - added exception blocks in diet_initialize for CORBA calls
 *
 * Revision 1.147  2010/09/02 17:46:12  bdepardo
 * Removed strdup in ErrorCodeStr to remove memory leaks.
 *
 * Revision 1.146  2010/07/27 10:24:33  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.145  2010/07/20 09:36:40  glemahec
 * Move "include WfLogservice.hh" inside "ifdef HAVE_WORKFLOW" directive
 *
 * Revision 1.144  2010/07/20 09:05:03  bisnard
 * Activating WfLogService
 *
 * Revision 1.143  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.142  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.141  2010/03/31 19:37:55  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.140  2010/03/08 13:22:23  bisnard
 * initialize DietLogComponent for DAGDA agent and Client Wf Mgr
 *
 * Revision 1.139  2009/09/25 12:42:28  bisnard
 * added dag cancellation method
 *
 * Revision 1.138  2009/09/07 14:34:55  bdepardo
 * Added an option in client configuration file to select, when launching the
 * client, the maximum number of SeD the client can receive.
 * Example in client.cfg:
 * clientMaxNbSeD = 100
 *
 * Revision 1.137  2009/08/26 10:34:28  bisnard
 * added new API methods for workflows (data in/out, transcript)
 *
 * Revision 1.136  2009/07/23 12:26:05  bisnard
 * new API method to get functional wf results as a container
 *
 * Revision 1.135  2009/07/20 12:53:13  bisnard
 * obsoleted file storing persistent data IDs on client when using DTM
 *
 * Revision 1.134  2009/07/07 08:55:50  bisnard
 * Reorganized code in two libraries: new library DIETCall contains all
 * core client functions & classes (including request_submission,
 * diet_call_common, diet_call_async_common).
 * JuxMem functions also moved to DIETCall.
 *
 * Revision 1.133  2009/06/23 09:25:01  bisnard
 * use new classname for WfLogService
 * bug correction for WfLogService option parsing
 *
 * Revision 1.132  2009/02/04 01:01:48  gcharrie
 * Cleaning the code in DIET_client to make a multicall.
 * there is still work to do in MultiCall, but the "public"
 * code is a lot cleaner
 *
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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <csignal>
#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "debug.hh"
#include "est_internal.hh"
#include "DIET_data_internal.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "SeD.hh"
#include "statistics.hh"
#include "configuration.hh"
#include "DIET_grpc.h"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif

#if HAVE_DAGDA
#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

#include "DIETCall.hh"
#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"

#ifdef HAVE_CCS
/** Custom client scheduling */
#include "SpecificClientScheduler.hh"
#endif // HAVE_CCS

#ifdef HAVE_WORKFLOW
#include "WfLogService.hh"
#include "workflow/CltWfMgr.hh"
#include "workflow/DagWfParser.hh"
#endif

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

#ifdef WITH_ENDIANNESS
extern bool little_endian;
#endif // WITH_ENDIANNESS
/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The Master Agent reference */
MasterAgent_var MA = MasterAgent::_nil();
omni_mutex*     MA_MUTEX = NULL;

/** Error rate for contract checking */
#define ERROR_RATE 0.1

/** Maximum servers to be answered */
unsigned long MAX_SERVERS = 10;

/** IOR reference sent to the SeD to let him contact the callback server */
/* ===> Change !!! No more the IOR !!!! */
char* REF_CALLBACK_SERVER;

#ifdef HAVE_WORKFLOW
/** The MA DAG reference */
static MaDag_var MA_DAG = MaDag::_nil();
#endif

#ifdef USE_LOG_SERVICE
/** The DietLogComponent */
DietLogComponent* dietLogComponent;
#endif

/****************************************************************************/
/* Manage MA name and Session Number for data persistency issue             */
/****************************************************************************/

  char* MA_Name;

  int num_Session;

  char file_Name[256];

/*
 * String representation of error code
 */
const char * const ErrorCodeStr[] = {
  "GRPC_NO_ERROR",
  "GRPC_NOT_INITIALIZED",
  "GRPC_CONFIGFILE_NOT_FOUND",
  "GRPC_CONFIGFILE_ERROR",
  "GRPC_SERVER_NOT_FOUND",
  "GRPC_FUNCTION_NOT_FOUND",
  "GRPC_INVALID_FUNCTION_HANDLE",
  "GRPC_INVALID_SESSION_ID",
  "GRPC_RPC_REFUSED",
  "GRPC_COMMUNICATION_FAILED",
  "GRPC_SESSION_FAILED",
  "GRPC_NOT_COMPLETED",
  "GRPC_NONE_COMPLETED",
  "GRPC_OTHER_ERROR_CODE",
  "GRPC_UNKNOWN_ERROR_CODE",
  "GRPC_ALREADY_INITIALIZED",
  "GRPC_LAST_ERROR_CODE",
  "XML_MALFORMED",
  "DAG_BADSTRUCT",
  "WFL_BADSTRUCT",
  "SRV_MISS",
  "DIET_PARSE_ERROR",
  "DIET_FILE_IO_ERROR",
  "DIET_MISSING_PARAMETERS",
  "DIET_LAST_ERROR_CODE"
};

/* TODO: as soon as we get Boost, enable this piece of code
 * make sure that our error code string array is up2date.
 */
// #include <boost/static_assert.hpp>
// BOOST_STATIC_ASSERT( sizeof(ErrorCodeStr)/sizeof(char*) == DIET_LAST_ERROR_CODE + 1 );

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
/* Transformation function for the host name. */
int chgName(int c) {
	if (c=='.') return '-';
	return c;
}

BEGIN_API

diet_error_t
diet_initialize(const char* config_file_name, int argc, char* argv[])
{
  int    myargc(0);
  char ** myargv(NULL);

  // MA_MUTEX initialization
  if (MA_MUTEX == NULL) {
    MA_MUTEX = new omni_mutex();
  }

  MA_MUTEX->lock();
  if (!CORBA::is_nil(MA)) {
    WARNING(__FUNCTION__ << ": diet_finalize has not been called");
    MA_MUTEX->unlock();
    return GRPC_ALREADY_INITIALIZED;
  }
  MA_MUTEX->unlock();
  
  /* Set arguments for ORBMgr::init */
  if (argc) {
    myargc = argc;
    myargv = (char**)malloc(argc * sizeof(char*));
    for (int i = 0; i < argc; i++)
      myargv[i] = argv[i];
  }

  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR("while parsing " << config_file_name, DIET_FILE_IO_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();
  // FIXME: should we also parse command line arguments?


  /* Check the parameters */
  std::string tmpString;
  if (!CONFIG_STRING(diet::MANAME, tmpString)) {
    ERROR("No MA name found in the configuration", GRPC_CONFIGFILE_ERROR);
  }

  if (CONFIG_STRING(diet::PARENTNAME, tmpString)) {
    WARNING("No need to specify a parent for a client - ignored");
  }
  
  if (CONFIG_AGENT(diet::AGENTTYPE, tmpString)) {
    WARNING("agentType is useless for a client - ignored");
  }

  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *  level = (char *) calloc(48, sizeof(char*)) ;
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = strdup("-ORBtraceLevel");
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char*)level;
    myargc = tmp_argc;
  }

  /* Initialize the ORB */
  try {
    ORBMgr::init(myargc, (char**)myargv);
  }	catch (...) {
    ERROR("ORB initialization failed", GRPC_NOT_INITIALIZED);
  }

  // Create sole instance of synchronized CallAsyncMgr class
  CallAsyncMgr::Instance();

  // Create servant callback object
  CallbackImpl* cb = new CallbackImpl();

  // activate servant callback
  try {
    ORBMgr::getMgr()->activate(cb);
  } catch (...) {
    return -1;
  }

  CORBA::Object_var obj = cb->_this();

  // create corba client callback serveur reference
  ostringstream os;
  char host[256];
  gethostname(host, 256);
  host[255]='\0';
  std::transform(host, host+strlen(host), host, chgName);
  os << "DIET-client-" << host << "-" << getpid();
  REF_CALLBACK_SERVER = CORBA::string_dup(os.str().c_str());
  try {
    ORBMgr::getMgr()->bind(CLIENTCTXT, os.str(), obj);
    ORBMgr::getMgr()->fwdsBind(CLIENTCTXT, os.str(),
                               ORBMgr::getMgr()->getIOR(obj));
  } catch (...) {
    ERROR("Connection to omniNames failed (Callback server bind)", 1);
  }
  if (REF_CALLBACK_SERVER == NULL) {
    return -1;
  }


#if HAVE_JUXMEM
  initJuxMem();
#endif // HAVE_JUXMEM

  /* Find Master Agent */
  CONFIG_STRING(diet::MANAME, tmpString);
  TRACE_TEXT(TRACE_MAIN_STEPS, "MA NAME PARSING = " << tmpString << endl);
  MA_MUTEX->lock();
  try {
    MA = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT, tmpString);
  } catch (...) {
    MA_MUTEX->unlock();
    return -1;
  }
  MA_MUTEX->unlock();

  /* Initialize statistics module */
  stat_init();


#ifdef USE_LOG_SERVICE
  /* Initialize LogService */
  bool useLS = false;
  int outBufferSize;
  int flushTime;
  bool useLogService = false;

  CONFIG_BOOL(diet::USELOGSERVICE, useLogService);
  if (!useLogService) {
    WARNING("useLogService disabled");
  } else {
    useLS = true;
  }

  if (useLS) {
    if (!CONFIG_INT(diet::LSOUTBUFFERSIZE, outBufferSize)) {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }
    
    if (!CONFIG_INT(diet::LSFLUSHINTERVAL, flushTime)) {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled" << endl);

    char* agtTypeName = strdup("CLIENT");
    char* agtParentName = NULL;
    if (CONFIG_STRING(diet::MANAME, tmpString)) {
      agtParentName = strdup(tmpString.c_str());
    }
    if (CONFIG_STRING(diet::NAME, tmpString)) {
      char*  userDefName;
      userDefName = strdup(tmpString.c_str());
      dietLogComponent = new DietLogComponent(userDefName, outBufferSize, argc, argv);
    } else {
#if HAVE_DAGDA
      // Use DAGDA agent as component name (same ref as in data transfer logs)
      dietLogComponent = new DietLogComponent(DagdaFactory::getClientName(), outBufferSize, argc, argv);
#else
      dietLogComponent = new DietLogComponent("", outBufferSize, argc, argv);
#endif // end: HAVE_DAGDA
    }
    //ORBMgr::getMgr()->activate(dietLogComponent);
    
    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled" << endl);
      WARNING("Could not initialize DietLogComponent");
      dietLogComponent = NULL;
    }
    free(agtTypeName);

  } else {
    dietLogComponent = NULL;
  }
#endif // end: USE_LOG_SERVICE
  // end modif bisnard_logs_1

  //create_file();
  MA_MUTEX->unlock();

  /** get Num session*/
  num_Session = MA->get_session_num();

#ifdef HAVE_CCS
  if (CONFIG_STRING(diet::USE_SPECIFIC_SCHEDULING, tmpString)
      && (tmpString.size() > 1)) {
    SpecificClientScheduler::setSchedulingId(tmpString.c_str());
  }
#endif // HAVE_CCS

#if HAVE_DAGDA
  // Dagda component activation.
  DagdaImpl* tmpDataManager = DagdaFactory::getClientDataManager();
#ifdef USE_LOG_SERVICE
  tmpDataManager->setLogComponent( dietLogComponent ); // modif bisnard_logs_1
#endif
  ORBMgr::getMgr()->activate(tmpDataManager);
#endif // HAVE_DAGDA

#ifdef HAVE_WORKFLOW
  // Workflow parsing
  /* Find the MA_DAG */
  if (CONFIG_STRING(diet::MADAGNAME, tmpString)) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "MA DAG NAME PARSING = " << tmpString << endl);
    MA_DAG = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT,
                                                         tmpString);
                                                         // CORBA::string_dup(tmpString.c_str()));
		
    if (CORBA::is_nil(MA_DAG)) {
      ERROR("Cannot locate MA DAG " << tmpString, 1);
    }
    else {
      CltWfMgr::instance()->setMaDag(MA_DAG);
#ifdef USE_LOG_SERVICE
      CltWfMgr::instance()->setLogComponent(dietLogComponent);
#endif
    }
  } // end if (CONFIG_STRING(diet::MADAGNAME, tmpString)
  
  // check if the Workflow Log Service is used
  bool useWfLogService = false;
  CONFIG_BOOL(diet::USEWFLOGSERVICE, useWfLogService);
  if (useWfLogService) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Connecting to Workflow Log Service" << endl);
    WfLogService_var wfLogSrv =
      ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT, "WfLogService");
    if (CORBA::is_nil(wfLogSrv)) {
      ERROR("cannot locate the Workflow Log Service ", 1);
    } else {
      CltWfMgr::instance()->setWfLogService(wfLogSrv);
    }
  }

  // Init the Xerces engine
  XMLPlatformUtils::Initialize();

#endif

  /* Has the maximum number of SeD been specified? */
  if (CONFIG_ULONG(diet::CLIENT_MAX_NB_SED, MAX_SERVERS)) {
    TRACE_TEXT (TRACE_MAIN_STEPS,"Max number of SeD allowed = " << MAX_SERVERS << endl);
  }
  
  /* Catch signals to try to exit cleanly. */
  signal(SIGABRT, diet_finalize_sig);
  signal(SIGTERM, diet_finalize_sig);

  return GRPC_NO_ERROR;
}

/* DIET finalize call through signal catch function. */
void diet_finalize_sig(int dummy) {
	(void) dummy;
	int status = static_cast<int>(diet_finalize());
	exit(status);
}

diet_error_t
diet_finalize() {
  /* Set signal handlers to default */
  signal(SIGINT, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  // ensure that CORBA is active before doing anything
  if (CORBA::is_nil(MA)) {
    WARNING(__FUNCTION__ << ": diet_finalize has already been called");
    return GRPC_NO_ERROR;
  }

#if HAVE_WORKFLOW
  try {
    // Terminate the xerces XML engine
    XMLPlatformUtils::Terminate();
    // Terminate the CltWfMgr
    CltWfMgr::terminate();
  } catch (...) {
    std::cerr << "Exception caught while destroying workflows" << std::endl;
  }
  MA_DAG = MaDag::_nil();
#endif // HAVE_WORKFLOW

  stat_finalize();

  CallAsyncMgr * caMgr = CallAsyncMgr::Instance();
  while (caMgr->areThereWaitRules() > 0) {
    omni_thread::sleep(1);
    // FIXME must be replace by a call to waitall
    // FIXME must be a call to diet_finalize_force ....
    // FIXME Maybe we must split async api from sync api ...
  }
  caMgr->release();

#if HAVE_JUXMEM
  terminateJuxMem();
#endif // HAVE_JUXMEM
  
#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    delete dietLogComponent;
    dietLogComponent = NULL;
  }
#endif

#ifdef HAVE_DAGDA
  DagdaFactory::reset();
#endif

  try {
      ORBMgr *mgr = ORBMgr::getMgr();
      mgr->unbind(CLIENTCTXT, REF_CALLBACK_SERVER);
      mgr->fwdsUnbind(CLIENTCTXT, REF_CALLBACK_SERVER);
      delete mgr;
  } catch( std::runtime_error& e ) {
      std::cerr << "Exception caugh: "
		<< e.what()
		<< "\n";
  } catch( ... ) {}
  
  
  /* end fileName */
  // *fileName='\0';
  
  MA_MUTEX->lock();
  MA = MasterAgent::_nil();
  MA_MUTEX->unlock();
  try {
    delete MA_MUTEX;
  } catch (...) {
  }
  MA_MUTEX = NULL;

  return GRPC_NO_ERROR;
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
  return GRPC_NO_ERROR;
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
/*
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
*/
/****************************************************************
 *   Add handler id with message msg in the file                *
 ***************************************************************/

void store_id(char* argID, char* msg)
{
/*
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
  */
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

/****************************************************************************/
/* GridRPC call functions                                                   */
/****************************************************************************/

/****************************************
 * Synchronous call
 ****************************************/

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

  diet_error_t err = diet_call_common(MA, profile, chosenServer, NULL, MAX_SERVERS);

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

BEGIN_API
/**
 * Request + asynchronous computation submissions.
 */
diet_error_t
diet_call_async(diet_profile_t* profile, diet_reqID_t* reqID)
{
  SeD_var chosenServer = SeD::_nil();
  diet_error_t err = diet_call_async_common(MA, profile, chosenServer, NULL,
                                            MAX_SERVERS, REF_CALLBACK_SERVER);
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
  if (error<0 || error>DIET_LAST_ERROR_CODE)
    return strdup(ErrorCodeStr[GRPC_UNKNOWN_ERROR_CODE]);
  return strdup(ErrorCodeStr[error]);
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
  CltWfMgr::instance()->setMA(MA);
  switch(profile->level) {
    case DIET_WF_DAG:
      return CltWfMgr::instance()->wfDagCall(profile);
    case DIET_WF_FUNCTIONAL:
      return CltWfMgr::instance()->wfFunctionalCall(profile);
  }
  return 1;
}

/**
 * Dag cancellation method
 */
diet_error_t
diet_wf_cancel_dag(const char* dagId) {
  if (CORBA::is_nil(MA_DAG)) {
      ERROR("No MA DAG defined", 1);
  }
  return CltWfMgr::instance()->cancelDag(dagId);
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
 * Check the type of workflow
 * @param profile workflow profile ref
 * @param wf_level  type of workflow
 * @return 1 if workflow profile does not match wf_level
 */
int
wf_check_profile_level(diet_wf_desc_t* profile,
                       wf_level_t wf_level) {
  if (!profile) {
    ERROR("Null workflow profile", 1);
  }
  if (profile->level != wf_level) {
    WARNING("Invalid type of workflow (dag or functional)");
    return 1;
  }
  return 0;
}

/**
 * Interdependent dags execution method
 */
diet_error_t
diet_wf_multi_call(diet_wf_desc_t* profile, int wfReqID) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  profile->wfReqID = wfReqID;
  return CltWfMgr::instance()->wfDagCall(profile);
}


/**
 * Save data file after workflow execution (functional wf)
 */
int
diet_wf_save_data_file(diet_wf_desc_t * profile,
                       const char * data_file_name) {
  if (wf_check_profile_level(profile, DIET_WF_FUNCTIONAL))
    return 1;
  if (!data_file_name)
    return 1;
  return CltWfMgr::instance()->saveWorkflowDataFile(profile, data_file_name);
}

/**
 * Save execution transcript after workflow execution
 * (contains status of dag nodes and data IDs of produced data)
 */
int
diet_wf_save_transcript_file(diet_wf_desc_t * profile,
                             const char * transcript_file_name) {
  if (!transcript_file_name)
    return 1;
  return CltWfMgr::instance()->saveWorkflowExecutionTranscript(profile,
                                                transcript_file_name);
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
 * Get results of a dag
 */
int
_diet_wf_scalar_get(diet_wf_desc_t * profile,
                    const char * id,
		    void** value) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  return CltWfMgr::instance()->getWfOutputScalar(profile, id, value);
} // end _diet_wf_scalar_get

int
_diet_wf_string_get(diet_wf_desc_t * profile,
                    const char * id,
		    char** value) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  return CltWfMgr::instance()->getWfOutputString(profile, id, value);
} // end _diet_wf_string_get


int
_diet_wf_file_get(diet_wf_desc_t * profile,
                  const char * id,
		  size_t* size, char** path) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  return CltWfMgr::instance()->getWfOutputFile(profile, id, size, path);
}

int
_diet_wf_matrix_get(diet_wf_desc_t * profile,
                    const char * id, void** value,
		    size_t* nb_rows, size_t *nb_cols,
		    diet_matrix_order_t* order) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  return CltWfMgr::instance()->getWfOutputMatrix(profile, id, value, nb_rows, nb_cols, order);
}

int
_diet_wf_container_get(diet_wf_desc_t * profile,
                       const char * id,
                       char** dataID) {
  if (wf_check_profile_level(profile, DIET_WF_DAG))
    return 1;
  return CltWfMgr::instance()->getWfOutputContainer(profile, id, dataID);
}

/**
 * Print the value of all exit ports of a dag or functional wf
 */
int
get_all_results(diet_wf_desc_t * profile) {
  return diet_wf_print_results(profile);
}

int
diet_wf_print_results(diet_wf_desc_t * profile) {
  switch(profile->level) {
    case DIET_WF_DAG:
      return CltWfMgr::instance()->printAllDagResults(profile);
    case DIET_WF_FUNCTIONAL:
      return CltWfMgr::instance()->printAllFunctionalWfResults(profile);
  }
  return 1;
}

/**
 * Get a container containing the data received by a sink
 * in a functional workflow
 */
int
diet_wf_sink_get(diet_wf_desc_t * profile,
                 const char * id,
                 char** dataID) {
  if (wf_check_profile_level(profile, DIET_WF_FUNCTIONAL))
    return 1;
  return CltWfMgr::instance()->getWfSinkContainer(profile, id, dataID);
}

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
