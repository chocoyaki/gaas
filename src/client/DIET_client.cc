/**
 * @file  DIET_client.cc
 *
 * @brief   DIET client interface
 *
 * @author   Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *           Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *           Christophe PERA (christophe.pera@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIET_client.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>
#include <omniORB4/CORBA.h>

#include "OSIndependance.hh"
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

#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"

#include "DIETCall.hh"
#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"

#ifdef HAVE_CCS
/** Custom client scheduling */
#include "SpecificClientScheduler.hh"
#endif  // HAVE_CCS

#ifdef HAVE_WORKFLOW
#include "WfLogService.hh"
#include "workflow/CltWfMgr.hh"
#include "workflow/DagWfParser.hh"
#endif

#define BEGIN_API extern "C" {
#define END_API }   // extern "C"

/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The Master Agent reference */
MasterAgent_var MA = MasterAgent::_nil();
omni_mutex *MA_MUTEX = NULL;

/** Error rate for contract checking */
#define ERROR_RATE 0.1

/** Maximum servers to be answered */
unsigned long MAX_SERVERS = 10;

/** IOR reference sent to the SeD to let him contact the callback server */
/* ===> Change !!! No more the IOR !!!! */
char *REF_CALLBACK_SERVER;

#ifdef HAVE_WORKFLOW
/** The MA DAG reference */
static MaDag_var MA_DAG = MaDag::_nil();
#endif

#ifdef USE_LOG_SERVICE
/** The DietLogComponent */
DietLogComponent *dietLogComponent;
#endif

/****************************************************************************/
/* Manage MA name and Session Number for data persistency issue             */
/****************************************************************************/

char *MA_Name;

int num_Session;

char file_Name[256];

/*
 * String representation of error code
 */
const char *const ErrorCodeStr[] = {
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
// BOOST_STATIC_ASSERT(sizeof(ErrorCodeStr)/sizeof(char*) == DIET_LAST_ERROR_CODE + 1);

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
  } // switch

  return err;
} // status_to_grpc_code

/****************************************************************************/
/* Initialize and Finalize session                                          */
/****************************************************************************/
/* Transformation function for the host name. */
int
chgName(int c) {
  if (c == '.') {
    return '-';
  }
  return c;
}

BEGIN_API

diet_error_t
diet_initialize(const char *config_file_name, int argc, char *argv[]) {
  /*int myargc(0);
  char **myargv(NULL);*/
  int    myargc = 0;
  char ** myargv = NULL;

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
    myargv = (char **) malloc(argc * sizeof(char *));
    for (int i = 0; i < argc; i++)
      myargv[i] = argv[i];
  }

  /* Get configuration file parameters */
  FileParser fileParser;
  try {
    fileParser.parseFile(config_file_name);
  } catch (...) {
    ERROR_DEBUG("while parsing " << config_file_name, GRPC_CONFIGFILE_ERROR);
  }
  CONFIGMAP = fileParser.getConfiguration();
  // FIXME: should we also parse command line arguments?


  /* Get the traceLevel */
  unsigned long tmpTraceLevel = TRACE_DEFAULT;
  CONFIG_ULONG(diet::TRACELEVEL, tmpTraceLevel);
  TRACE_LEVEL = tmpTraceLevel;
  if (TRACE_LEVEL >= TRACE_MAX_VALUE) {
    char *level = (char *) calloc(48, sizeof(char *));
    int tmp_argc = myargc + 2;
    myargv = (char **) realloc(myargv, tmp_argc * sizeof(char *));
    myargv[myargc] = strdup("-ORBtraceLevel");
    sprintf(level, "%u", TRACE_LEVEL - TRACE_MAX_VALUE);
    myargv[myargc + 1] = (char *) level;
    myargc = tmp_argc;
  }


  /* Check the parameters */
  std::string tmpString;
  if (!CONFIG_STRING(diet::MANAME, tmpString)) {
    ERROR_DEBUG("No MA name found in the configuration", GRPC_CONFIGFILE_ERROR);
  }

  if (CONFIG_STRING(diet::PARENTNAME, tmpString)) {
    WARNING("No need to specify a parent for a client - ignored");
  }

  if (CONFIG_AGENT(diet::AGENTTYPE, tmpString)) {
    WARNING("agentType is useless for a client - ignored");
  }

  /* Initialize the ORB */
  try {
    ORBMgr::init(myargc, (char **) myargv);
  } catch (...) {
    ERROR_DEBUG("ORB initialization failed", GRPC_NOT_INITIALIZED);
  }

  // Create sole instance of synchronized CallAsyncMgr class
  CallAsyncMgr::Instance();

  // Create servant callback object
  CallbackImpl *cb = new CallbackImpl();

  // activate servant callback
  try {
    ORBMgr::getMgr()->activate(cb);
  } catch (...) {
    return GRPC_NOT_INITIALIZED;
  }

  CORBA::Object_var obj = cb->_this();

  // create corba client callback serveur reference
  std::ostringstream os;
  char host[256];
  gethostname(host, 256);
  host[255] = '\0';
  std::transform(host, host + strlen(host), host, chgName);
  os << "DIET-client-" << host << "-" << getpid();
  REF_CALLBACK_SERVER = CORBA::string_dup(os.str().c_str());
  try {
    ORBMgr::getMgr()->bind(CLIENTCTXT, os.str(), obj);
    ORBMgr::getMgr()->fwdsBind(CLIENTCTXT, os.str(),
                               ORBMgr::getMgr()->getIOR(obj));
  } catch (...) {
    ERROR_DEBUG("Connection to omniNames failed (Callback server bind)",
          GRPC_NOT_INITIALIZED);
  }
  if (REF_CALLBACK_SERVER == NULL) {
    return GRPC_NOT_INITIALIZED;
  }


  /* Find Master Agent */
  CONFIG_STRING(diet::MANAME, tmpString);
  TRACE_TEXT(TRACE_MAIN_STEPS, "MA NAME PARSING = " << tmpString << "\n");
  MA_MUTEX->lock();
  try {
    MA = ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_var>(AGENTCTXT,
                                                                 tmpString);
  } catch (...) {
    MA_MUTEX->unlock();
    return GRPC_NOT_INITIALIZED;
  }
  MA_MUTEX->unlock();

  /* Initialize statistics module */
  stat_init();


#ifdef USE_LOG_SERVICE
  /* Initialize LogService */
  bool useLS = false;
  bool useLogService = false;

  CONFIG_BOOL(diet::USELOGSERVICE, useLogService);
  if (!useLogService) {
    WARNING("useLogService disabled");
  } else {
    useLS = true;
  }

  if (useLS) {
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
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled\n");

    char *agtTypeName = strdup("CLIENT");
    char *agtParentName = NULL;
    if (CONFIG_STRING(diet::MANAME, tmpString)) {
      agtParentName = strdup(tmpString.c_str());
    }
    if (CONFIG_STRING(diet::NAME, tmpString)) {
      char *userDefName;
      userDefName = strdup(tmpString.c_str());
      dietLogComponent = new DietLogComponent(userDefName, outBufferSize, argc,
                                              argv);
    } else {
      // Use DAGDA agent as component name (same ref as in data transfer logs)
      dietLogComponent = new DietLogComponent(DagdaFactory::getClientName(),
                                              outBufferSize, argc, argv);
    }

    ORBMgr::getMgr()->activate(dietLogComponent);

    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
      WARNING("Could not initialize DietLogComponent");
      dietLogComponent = NULL;
    }
    free(agtTypeName);
  } else {
    // end: if (useLS)
    dietLogComponent = NULL;
  }
#endif  // end: USE_LOG_SERVICE
  // end modif bisnard_logs_1

  // create_file();

  /** get Num session*/
  num_Session = MA->get_session_num();

#ifdef HAVE_CCS
  if (CONFIG_STRING(diet::USE_SPECIFIC_SCHEDULING, tmpString)
      && (tmpString.size() > 1)) {
    SpecificClientScheduler::setSchedulingId(tmpString.c_str());
  }
#endif  // HAVE_CCS

  // Dagda component activation.
  DagdaImpl *tmpDataManager = DagdaFactory::getClientDataManager();
#ifdef USE_LOG_SERVICE
  tmpDataManager->setLogComponent(dietLogComponent);  // modif bisnard_logs_1
#endif
  ORBMgr::getMgr()->activate(tmpDataManager);

#ifdef HAVE_WORKFLOW
  // Workflow parsing
  /* Find the MA_DAG */
  if (CONFIG_STRING(diet::MADAGNAME, tmpString)) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "MA DAG NAME PARSING = " << tmpString << "\n");
    MA_DAG = ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT,
                                                         tmpString);
    // CORBA::string_dup(tmpString.c_str()));

    if (CORBA::is_nil(MA_DAG)) {
      ERROR_DEBUG("Cannot locate MA DAG " << tmpString, GRPC_NOT_INITIALIZED);
    } else {
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
    TRACE_TEXT(TRACE_MAIN_STEPS, "Connecting to Workflow Log Service\n");
    WfLogService_var wfLogSrv =
      ORBMgr::getMgr()->resolve<WfLogService, WfLogService_var>(WFLOGCTXT,
                                                                "WfLogService");
    if (CORBA::is_nil(wfLogSrv)) {
      ERROR_DEBUG("cannot locate the Workflow Log Service ", GRPC_NOT_INITIALIZED);
    } else {
      CltWfMgr::instance()->setWfLogService(wfLogSrv);
    }
  }

  // Init the Xerces engine
  XMLPlatformUtils::Initialize();

#endif // ifdef HAVE_WORKFLOW

  /* Has the maximum number of SeD been specified? */
  if (CONFIG_ULONG(diet::CLIENT_MAX_NB_SED, MAX_SERVERS)) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Max number of SeD allowed = " << MAX_SERVERS << "\n");
  }

  /* Catch signals to try to exit cleanly. */
  signal(SIGABRT, diet_finalize_sig);
  signal(SIGTERM, diet_finalize_sig);

  return GRPC_NO_ERROR;
} // diet_initialize

/* DIET finalize call through signal catch function. */
void
diet_finalize_sig(int dummy) {
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
    std::cerr << "Exception caught while destroying workflows\n";
  }
  MA_DAG = MaDag::_nil();
#endif  // HAVE_WORKFLOW

  stat_finalize();

  CallAsyncMgr *caMgr = CallAsyncMgr::Instance();
  while (caMgr->areThereWaitRules() > 0) {
    omni_thread::sleep(1);
    // FIXME must be replace by a call to waitall
    // FIXME must be a call to diet_finalize_force ....
    // FIXME Maybe we must split async api from sync api ...
  }
  caMgr->release();

#ifdef USE_LOG_SERVICE
  if (dietLogComponent != NULL) {
    // delete dietLogComponent;  // FIXME: this does not work
    dietLogComponent = NULL;
  }
#endif

  DagdaFactory::reset();

  try {
    ORBMgr *mgr = ORBMgr::getMgr();
    mgr->unbind(CLIENTCTXT, REF_CALLBACK_SERVER);
    mgr->fwdsUnbind(CLIENTCTXT, REF_CALLBACK_SERVER);
    delete mgr;
  } catch (std::runtime_error &e) {
    std::cerr << "Exception caugh: "
              << e.what()
              << "\n";
  } catch (...) {
  }


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
} // diet_finalize

END_API


/****************************************************************************/
/* Data handles                                                             */
/****************************************************************************/

BEGIN_API

diet_data_handle_t *
diet_data_handle_malloc() {
  return new diet_data_handle_t;
}

void *
diet_get(diet_data_handle_t *handle) {
  return handle->value;
}

int
diet_free(diet_data_handle_t *handle) {
  delete handle;
  return GRPC_NO_ERROR;
}

END_API

/****************************************************************
 *    get available Services in the DIET Platform
 ***************************************************************/

/* Only return the name of services available on the platform */
char **
get_diet_services(int *services_number) {
  CORBA::Long length;
  SeqCorbaProfileDesc_t *profileList = MA->getProfiles(length);
  *services_number = (int) length;
  char **services_list = (char **) calloc(length + 1, sizeof(char *));
  fflush(stdout);
  for (int i = 0; i < *services_number; i++) {
    services_list[i] = CORBA::string_dup((*profileList)[i].path);
  }
  return services_list;
} // get_diet_services


/****************************************************************
 *   Add handler id with message msg in the file                *
 ***************************************************************/
void
store_id(char *argID, char *msg) {
}


/******************************************************************
 *  Free persistent data identified by argID                     *
 *****************************************************************/
int
diet_free_persistent_data(char *argID) {
  char statMsg[128];
  sprintf(statMsg, "%s %s", __FUNCTION__, argID);
  stat_in("client", statMsg);
  if (MA->diet_free_pdata(argID) != 0) {
    stat_out("client", statMsg);
    return 1;
  } else {
    std::cerr << "UNKNOWN DATA\n";
    return 0;
  }
} // diet_free_persistent_data

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
diet_call(diet_profile_t *profile) {
  SeD_var chosenServer = SeD::_nil();

#ifdef HAVE_CCS
  if (SpecificClientScheduler::isEnabled()) {
    SpecificClientScheduler::pre_diet_call();
  }
#endif  // HAVE_CCS

  diet_error_t err = diet_call_common(MA, profile, chosenServer, NULL,
                                      MAX_SERVERS);

#ifdef HAVE_CCS
  if (SpecificClientScheduler::isEnabled()) {
    SpecificClientScheduler::post_diet_call();
  }
#endif  // HAVE_CCS

  return err;
} // diet_call

#if defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call(diet_profile_t *profile) {
  diet_profile_set_parallel(profile);
  return diet_call(profile);
}

diet_error_t
diet_sequential_call(diet_profile_t *profile) {
  diet_profile_set_sequential(profile);
  return diet_call(profile);
}
#endif // if defined HAVE_ALT_BATCH

END_API


/****************************************
* Asynchronous call
****************************************/

BEGIN_API
/**
 * Request + asynchronous computation submissions.
 */
diet_error_t
diet_call_async(diet_profile_t *profile, diet_reqID_t *reqID) {
  SeD_var chosenServer = SeD::_nil();
  diet_error_t err = diet_call_async_common(MA, profile, chosenServer, NULL,
                                            MAX_SERVERS, REF_CALLBACK_SERVER);
  *reqID = profile->dietReqID;
  set_req_error(*reqID, err);
  return err;
}

#if defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call_async(diet_profile_t *profile, diet_reqID_t *reqID) {
  diet_profile_set_parallel(profile);
  return diet_call_async(profile, reqID);
}
diet_error_t
diet_sequential_call_async(diet_profile_t *profile, diet_reqID_t *reqID) {
  diet_profile_set_sequential(profile);
  return diet_call_async(profile, reqID);
}
#endif // if defined HAVE_ALT_BATCH

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
diet_probe(diet_reqID_t reqID) {
  int err = CallAsyncMgr::Instance()->getStatusReqID(reqID);
  /**
   * transform the request status as defined by the CallAsyncMgr to its
   * equivalent in GridRPC standard
     STATUS_DONE = 0, // Result is available in local memory
     STATUS_WAITING,       // End of solving on Server, result comes
     STATUS_RESOLVING,          // Request is currently solving on Server
     STATUS_CANCEL,        // Cancel is called on a reqID.
     STATUS_ERROR          // Error caught
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
  } // switch

  return err;
} // diet_probe

/****************************************************************************
* diet_cancel GridRPC function.
* This function erases all persistent data that are manipulated by the reqID
* request. Do not forget to calget_data_handle on data you would like
* to save.
****************************************************************************/
int
diet_cancel(diet_reqID_t reqID) {
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
diet_wait(diet_reqID_t reqID) {
  // check if all request ID is valid
  if (!CallAsyncMgr::Instance()->checkSessionID(reqID)) {
    return GRPC_INVALID_SESSION_ID;
  }

  try {
    // Create ruleElements table ...
    ruleElement *simpleWait = new ruleElement[1];
    simpleWait[0].reqID = reqID;
    simpleWait[0].op = WAITOPERATOR(SOLE);
    Rule *rule = new Rule;
    rule->length = 1;
    rule->ruleElts = simpleWait;
    rule->status = STATUS_RESOLVING;

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, be careful using diet_cancel
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char *p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  } catch (const std::exception &e) {
    ERROR_DEBUG(__FUNCTION__ << ": unexpected exception (what="
                       << e.what() << ')', STATUS_ERROR);
  }

  return STATUS_ERROR;
} // diet_wait

/*****************************************************************************
* diet_wait_and GridRPC function
* return error status
* three args :
*      1 - reqID table.
*      2 - size of the table.
*****************************************************************************/
int
diet_wait_and(diet_reqID_t *IDs, size_t length) {
  // check if all the session IDs in the array are valid
  for (unsigned int ix = 0; ix < length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(IDs[ix])) {
      return GRPC_INVALID_SESSION_ID;
    }
  }

  request_status_t rst = STATUS_ERROR;
  try {
    // Create ruleElements table ...
    ruleElement *simpleWait = new ruleElement[length];
    for (size_t k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(AND);
    }
    Rule *rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be careful, there may be others rules using some of this
    // reqID(AsyncCall) So, careful using diet_cancel
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char *p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  } catch (const std::exception &e) {
    ERROR_DEBUG(__FUNCTION__ << ": unexpected exception (what=" << e.what() << ')',
          STATUS_ERROR);
  }
  return rst;
} // diet_wait_and

/*****************************************************************************
* diet_wait_or GridRPC function
* return error status
* three args :
*      1 - reqID table.
*      2 - size of the table.
*      3 - received reqID
*****************************************************************************/
int
diet_wait_or(diet_reqID_t *IDs, size_t length, diet_reqID_t *IDptr) {
  request_status_t rst = STATUS_ERROR;

  // check if all the session IDs in the array are valid
  for (unsigned int ix = 0; ix < length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(IDs[ix])) {
      return GRPC_INVALID_SESSION_ID;
    }
  }

  try {
    // Create ruleElements table ...
    ruleElement *simpleWait = new ruleElement[length];
    for (unsigned int k = 0; k < length; k++) {
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(OR);
    }
    Rule *rule = new Rule; /* freed by addWaitRule */
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
      return -1;  // Unexpected error, no value describing it
      // NOTES: Be careful, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, careful using diet_cancel
    } // switch
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char *p = tc->name();
    if (*p != '\0') {
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    } else {
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
    }
  } catch (const std::exception &e) {
    ERROR_DEBUG(__FUNCTION__ << ": unexpected exception (what="
                       << e.what() << ')', STATUS_ERROR);
  }
  return rst;
} // diet_wait_or

/*****************************************************************************
* diet_wait_all GridRPC function
* return error status
*****************************************************************************/
int
diet_wait_all() {
  return CallAsyncMgr::Instance()->addWaitAllRule();
}

/*****************************************************************************
* diet_wait_any GridRPC function
* return the ID of the received request
*****************************************************************************/
int
diet_wait_any(diet_reqID_t *IDptr) {
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
  if (error < 0 || error > DIET_LAST_ERROR_CODE) {
    return strdup(ErrorCodeStr[GRPC_UNKNOWN_ERROR_CODE]);
  }
  return strdup(ErrorCodeStr[error]);
}
/***************************************************************************
* return identifier of the failed session
***************************************************************************/
diet_error_t
diet_get_failed_session(diet_reqID_t *reqIdPtr) {
  return CallAsyncMgr::Instance()->getFailedSession(reqIdPtr);
}
/***************************************************************************
* check if one of the requests contained in the array id reqIdArray has
* completed.
* Return the completed request ID if exist. Otherwise return an error code
***************************************************************************/
diet_error_t
diet_probe_or(diet_reqID_t *reqIdArray,
              size_t length,
              diet_reqID_t *reqIdPtr) {
  unsigned int ix;
  int reqStatus;
  // check if all request IDs are valid
  for (ix = 0; ix < length; ix++) {
    if (!CallAsyncMgr::Instance()->checkSessionID(reqIdArray[ix])) {
      return GRPC_INVALID_SESSION_ID;
    }
  }
  for (ix = 0; ix < length; ix++) {
    reqStatus = CallAsyncMgr::Instance()->getStatusReqID(reqIdArray[ix]);
    if (reqStatus == STATUS_DONE) {
      *reqIdPtr = reqIdArray[ix];
      return GRPC_NO_ERROR;
    }
  } // end for ix

  return GRPC_NONE_COMPLETED;
} // diet_probe_or

/***************************************************************************
* Get the function handle linked to reqID
***************************************************************************/
diet_error_t
diet_get_handle(grpc_function_handle_t **handle,
                diet_reqID_t sessionID) {
  return CallAsyncMgr::Instance()->getHandle(handle, sessionID);
}

/***************************************************************************
* Save the specified handle and associate it to a sessionID
***************************************************************************/
void
diet_save_handle(diet_reqID_t sessionID,
                 grpc_function_handle_t *handle) {
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
diet_wf_call(diet_wf_desc_t *profile) {
  if (CORBA::is_nil(MA_DAG)) {
    ERROR_DEBUG("No MA DAG defined", 1);
  }
  CltWfMgr::instance()->setMA(MA);
  switch (profile->level) {
  case DIET_WF_DAG:
    return CltWfMgr::instance()->wfDagCall(profile);

  case DIET_WF_FUNCTIONAL:
    return CltWfMgr::instance()->wfFunctionalCall(profile);
  }
  return 1;
} // diet_wf_call

/**
 * Dag cancellation method
 */
diet_error_t
diet_wf_cancel_dag(const char *dagId) {
  if (CORBA::is_nil(MA_DAG)) {
    ERROR_DEBUG("No MA DAG defined", 1);
  }
  return CltWfMgr::instance()->cancelDag(dagId);
}

/**
 * Get a request ID for interdependent dags submit
 */
int
diet_wf_get_reqID() {
  if (CORBA::is_nil(MA_DAG)) {
    ERROR_DEBUG("No MA DAG defined", 1);
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
wf_check_profile_level(diet_wf_desc_t *profile,
                       wf_level_t wf_level) {
  if (!profile) {
    ERROR_DEBUG("Null workflow profile", 1);
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
diet_wf_multi_call(diet_wf_desc_t *profile, int wfReqID) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  profile->wfReqID = wfReqID;
  return CltWfMgr::instance()->wfDagCall(profile);
}


/**
 * Save data file after workflow execution (functional wf)
 */
int
diet_wf_save_data_file(diet_wf_desc_t *profile,
                       const char *data_file_name) {
  if (wf_check_profile_level(profile, DIET_WF_FUNCTIONAL)) {
    return 1;
  }
  if (!data_file_name) {
    return 1;
  }
  return CltWfMgr::instance()->saveWorkflowDataFile(profile, data_file_name);
}

/**
 * Save execution transcript after workflow execution
 * (contains status of dag nodes and data IDs of produced data)
 */
int
diet_wf_save_transcript_file(diet_wf_desc_t *profile,
                             const char *transcript_file_name) {
  if (!transcript_file_name) {
    return 1;
  }
  return CltWfMgr::instance()->
         saveWorkflowExecutionTranscript(profile, transcript_file_name);
}

/**
 * terminate a workflow session *
 * and free the memory *
 */
void
diet_wf_free(diet_wf_desc_t *profile) {
  CltWfMgr::instance()->wf_free(profile);
}


/**
 * Get results of a dag
 */
int
_diet_wf_scalar_get(diet_wf_desc_t *profile,
                    const char *id,
                    void **value) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfOutputScalar(profile, id, value);
} // end _diet_wf_scalar_get

int
_diet_wf_string_get(diet_wf_desc_t *profile,
                    const char *id,
                    char **value) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfOutputString(profile, id, value);
} // end _diet_wf_string_get


int
_diet_wf_file_get(diet_wf_desc_t *profile,
                  const char *id,
                  size_t *size, char **path) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfOutputFile(profile, id, size, path);
}

int
_diet_wf_matrix_get(diet_wf_desc_t *profile,
                    const char *id, void **value,
                    size_t *nb_rows, size_t *nb_cols,
                    diet_matrix_order_t *order) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfOutputMatrix(profile, id,
                                                 value, nb_rows,
                                                 nb_cols, order);
}

int
_diet_wf_container_get(diet_wf_desc_t *profile,
                       const char *id,
                       char **dataID) {
  if (wf_check_profile_level(profile, DIET_WF_DAG)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfOutputContainer(profile, id, dataID);
}

/**
 * Print the value of all exit ports of a dag or functional wf
 */
int
get_all_results(diet_wf_desc_t *profile) {
  return diet_wf_print_results(profile);
}

int
diet_wf_print_results(diet_wf_desc_t *profile) {
  switch (profile->level) {
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
diet_wf_sink_get(diet_wf_desc_t *profile,
                 const char *id,
                 char **dataID) {
  if (wf_check_profile_level(profile, DIET_WF_FUNCTIONAL)) {
    return 1;
  }
  return CltWfMgr::instance()->getWfSinkContainer(profile, id, dataID);
}

#endif  // HAVE_WORKFLOW

END_API


/**
 * return the list of all available profiles
 */
SeqCorbaProfileDesc_t *
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
getProfileDesc(const char *srvName, diet_profile_desc_t &profile) {
  SeqCorbaProfileDesc_t *allProfiles = getProfiles();
  if (allProfiles) {
    for (unsigned int ix = 0; ix < allProfiles->length(); ix++) {
      if (!strcmp((*allProfiles)[ix].path,
                  srvName)) {
        // The service is found
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "The service " << srvName << " is found \n");
        // this function place is marshalling.cc file
        // to fix is necessary
        unmrsh_profile_desc(&profile,
                            &((*allProfiles)[ix]));
        return true;
      }
    }
  }
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "The service " << srvName << " was not found\n");
  return false;
} // getProfileDesc

/*
 * get all the session IDs
 * the array must be deleted by the caller
 */
diet_reqID_t *
get_all_session_ids(int &len) {
  return CallAsyncMgr::Instance()->getAllSessionIDs(len);
}
