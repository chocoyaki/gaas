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
 * Revision 1.37  2003/09/28 22:11:53  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.36  2003/09/25 10:03:38  cpera
 * Change return function value according to GridRPC API and delete debug messages.
 *
 * Revision 1.35  2003/09/22 21:08:15  pcombes
 * Update to changes of ORBMgr.
 *
 * Revision 1.34  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.33  2003/07/09 17:08:44  pcombes
 * Better management of the variable MA (with mutex) and the handles.
 *
 * Revision 1.32  2003/07/04 09:48:02  pcombes
 * Make diet_initialize thread-safe. Use new ERROR and WARNING macros.
 *
 * Revision 1.26  2003/06/02 08:08:11  cpera
 * Beta version of asynchronize DIET API.
 *
 * Revision 1.25  2003/05/12 14:15:58  ecaron
 * Update call to stat in according to perl script file (extract.pl)
 *
 * Revision 1.24  2003/05/10 08:54:08  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.13  2002/11/22 13:36:12  lbertsch
 * Added alpha linux support
 * Added a package for statistics and some traces
 ****************************************************************************/


#include "DIET_client.h"

#include <CORBA.h>
#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "com_tools.hh"
#include "debug.hh"
#include "dietTypes.hh"
#include "marshalling.hh"
#include "MasterAgent.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "SeD.hh"
#include "statistics.hh"

#include "CallAsyncMgr.hh"
#include "CallbackImpl.hh"


#define BEGIN_API extern "C" {
#define END_API   } // extern "C"



/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/** The trace level. */
extern unsigned int TRACE_LEVEL;

/** The Master Agent reference */
static MasterAgent_var MA = MasterAgent::_nil();
static omni_mutex      MA_MUTEX;

/** Error rate for contract checking */
#define ERROR_RATE 0.1

/** Maximum servers to be answered */
static unsigned long MAX_SERVERS = 10;

/** IOR reference sent to the SeD to let him contact the callback server */
static char* REF_CALLBACK_SERVER;
/** Flag for using the asynchronous API (set at configuration time) */
static size_t USE_ASYNC_API = 1;


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
  char** myargv(NULL);
  void*  value(NULL);
  
  MA_MUTEX.lock();
  
  if (!CORBA::is_nil(MA)) {
    WARNING(__FUNCTION__ << ": diet_finalize has not been called");
    MA_MUTEX.unlock();
    return 0;
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
  if ((res = Parsers::parseCfgFile(false, 1,
				   (Parsers::Results::param_type_t*)compParam))) {
    Parsers::endParsing();
    MA_MUTEX.unlock();
    return res;
  }

  /* Some more checks */
  value = Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (value != NULL)
    WARNING("parsing " << config_file_name
	    << ": it is useless to name a client - ignored");
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
    char   level[48];
    int    tmp_argc = myargc + 2;
    myargv = (char**)realloc(myargv, tmp_argc * sizeof(char*));
    myargv[myargc] = "-ORBtraceLevel";
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
    CallbackImpl* cb = new CallbackImpl;
    CORBA::Object_var obj = cb->_this();
    // activate servant callback
    if (ORBMgr::activate(cb) != 0) return -1;
    // create corba client callback serveur reference ...
    REF_CALLBACK_SERVER = ORBMgr::getIORString(obj);
    // clean
    cb->_remove_ref();
    if (REF_CALLBACK_SERVER == NULL) return -1;
  }
  else {
    if (ORBMgr::init(myargc, (char**)myargv, false)) {
      ERROR("ORB initialization failed", 1);
    }
  }

  /* Find Master Agent */
  MA_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  MA = MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT, MA_name));
  if (CORBA::is_nil(MA)) {
    ERROR("cannot locate Master Agent " << MA_name, 1);
  }
  /* Initialize statistics module */
  stat_init();

  /* We do not need the parsing results any more */
  Parsers::endParsing();

  MA_MUTEX.unlock();
  return 0;
}

diet_error_t
diet_finalize()
{
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


/****************************************************************************/
/* GridRPC call functions                                                   */
/****************************************************************************/


/****************************************
 * Request submission
 ****************************************/

diet_error_t
request_submission(diet_profile_t* profile,
		   SeD_var& chosenServer, diet_reqID_t& reqID)
{
  static int nb_tries(3);
  int server_OK(0), subm_count;
  corba_pb_desc_t corba_pb;
  //corba_pb_desc_t& corba_pb = *(new corba_pb_desc_t());
  corba_response_t* response(NULL);

  chosenServer = SeD::_nil();

  if (mrsh_pb_desc(&corba_pb, profile)) {
    ERROR("profile is wrongly built", 1);
  }

  /* Request submission : try nb_tries times */

  stat_in("Client","request_submission");
  subm_count = 0;
  do {
    response = NULL;

    /* Submit to the agent. */
    try {
      response = MA->submit(corba_pb, MAX_SERVERS);
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      //delete &corba_pb;
      if (response)
	delete response;
      ERROR("caught a CORBA exception (" << tc->name()
	    << ") while submitting problem", 1);
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
	  int idx = response->sortedIndexes[i];
	  TRACE_TEXT(TRACE_MAIN_STEPS,
		     "    " << response->servers[idx].loc.hostName << ":"
		     << response->servers[idx].loc.port << "\n");
	}
      }

      /* Check the contracts of the servers answered. */

      server_OK = 0;
      while ((size_t) server_OK < response->servers.length()) {
	try {
	  int           idx       = response->sortedIndexes[server_OK];
	  SeD_ptr       server    = response->servers[idx].loc.ior;
	  CORBA::Double totalTime = response->servers[idx].estim.totalTime;
	  if (server->checkContract(response->servers[idx].estim, corba_pb)) {
	    server_OK++;
	    continue;
	  }
	  if ((totalTime == response->servers[idx].estim.totalTime) ||
	      ((response->servers[idx].estim.totalTime - totalTime)
	       < (ERROR_RATE *
		  MAX(totalTime,response->servers[idx].estim.totalTime))))
	    break;
	  server_OK++;
	} catch (CORBA::Exception& e) {
	  server_OK++;
	  continue;
	}
      }
      if ((size_t) server_OK == response->servers.length())
	server_OK = -1;
    }

  } while ((response) && (response->servers.length() > 0) &&
	   (server_OK == -1) && (++subm_count < nb_tries));
  stat_out("Client","request_submission");

  if (!response || response->servers.length() == 0) {
    //delete &corba_pb;
    if (response)
      delete response;
    ERROR("unable to find a server", 1);
  }
  if (server_OK == -1) {
    //delete &corba_pb;
    delete response;
    ERROR("unable to find a server after " << nb_tries << " tries."
	  << "The platform might be overloaded, try again later please", 1);
  }

#if HAVE_CICHLID
  static int already_initialized(0);
  char str_tmp[1000];

  if (!already_initialized) {
    init_communications();
    already_initialized = 1;
  }

  strcpy(str_tmp, response->servers[server_OK].loc.hostName);
  strcat(str_tmp, "_SeD");
  add_communication("client", str_tmp, profile_size(&corba_pb));
#endif // HAVE_CICHLID
  
  //delete &corba_pb;

  if (server_OK >= 0) {
    chosenServer = response->servers[server_OK].loc.ior;
    reqID = response->reqID;
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
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer)
{
  diet_error_t res(0);
  int solve_res(0);
  diet_reqID_t reqID;
  corba_profile_t corba_profile;

  stat_in("Client","diet_call");

  if (CORBA::is_nil(chosenServer)) {
    if ((res = request_submission(profile, chosenServer, reqID)))
      return res;
    if (CORBA::is_nil(chosenServer))
      return 1;
  }
  
  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    ERROR("profile is wrongly built", 1);
  }

  stat_in("Client","computation");
  solve_res = chosenServer->solve(profile->pb_name, corba_profile);
  stat_out("Client","computation");

  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }
 
  stat_out("Client","diet_call");
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
  return diet_call_common(profile, chosenServer);
}

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
		       SeD_var& chosenServer, diet_reqID_t* reqID)
{
   corba_profile_t corba_profile;
   CallAsyncMgr* caMgr;
   diet_error_t res(0);
  
  if (!reqID) {
    ERROR(__FUNCTION__ << ": 2nd argument has not been allocated", 1);
  }

  stat_in("Client","diet_call_async");

  try {

    if (CORBA::is_nil(chosenServer)) {
      if ((res = request_submission(profile, chosenServer, *reqID)))
	return res;
      if (CORBA::is_nil(chosenServer))
	return 1;
    }
    
    if (mrsh_profile_to_in_args(&corba_profile, profile)) {
      ERROR("profile is wrongly built", 1);
    }

    // get sole CallAsyncMgr singleton
    caMgr = CallAsyncMgr::Instance();
    // create corba client callback server...
    if (caMgr->addAsyncCall(*reqID, profile) != 0)
      return 1;

    stat_in("Client","computation_async");
    chosenServer->solveAsync(profile->pb_name, corba_profile, 
			     *reqID, REF_CALLBACK_SERVER);
    stat_out("Client","computation_async");

    if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
      INTERNAL_ERROR("returned profile is wrongly built", 1);
    }
   
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0')
      WARNING("exception caught in " << __FUNCTION__ << '(' << p << ')');
    else
      WARNING("exception caught in " << __FUNCTION__ << '(' << tc->id() << ')');
    *reqID = -1;
    return 1;
  }
  catch (...) {
    WARNING("exception caught in " << __FUNCTION__);
    *reqID = -1;
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
  return diet_call_async_common(profile, chosenServer, reqID);
}

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
  return CallAsyncMgr::Instance()->getStatusReqID(reqID);
}
  
/****************************************************************************  
 * diet_cancel GridRPC function.
 * This function erases all persistent data that are manipulated by the reqID
 * request. Do not forget to call diet_get_data_handle on data you would like
 * to save.                   
 ****************************************************************************/
int
diet_cancel(diet_reqID_t reqID)
{
  return CallAsyncMgr::Instance()->deleteAsyncCall(reqID);
}

int
diet_wait(diet_reqID_t reqID)
{
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
    if (*p != '\0')
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    else
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
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
 *  	1 - reqID table.
 *  	2 - size of the table.
 *****************************************************************************/
int
diet_wait_and(diet_reqID_t* IDs, size_t length)
{
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
    // NOTES: Be carefull, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, carefull using diet_cancel
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0')
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    else
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
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
 *  	1 - reqID table.
 *  	2 - size of the table.
 *  	3 - received reqID
 *****************************************************************************/
int
diet_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr)
{
  request_status_t rst = STATUS_ERROR;
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
      return -1; // Unexcpected error, no value describing it
      // NOTES: Be carefull, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, carefull using diet_cancel
    }
  }
  catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0')
      WARNING(__FUNCTION__ << ": exception caught (" << p << ')');
    else
      WARNING(__FUNCTION__ << ": exception caught (" << tc->id() << ')');
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
  return CallAsyncMgr::Instance()->addWaitAnyRule(IDptr);
}

END_API

