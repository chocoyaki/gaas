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
 * Revision 1.29  2003/06/25 09:20:55  cpera
 * Change internal reqId from long int to int according to GridRPC and diet_reqID
 * type.
 *
 * Revision 1.28  2003/06/04 14:40:05  cpera
 * Resolve bugs, change type of reqID (long int) and modify
 * diet_wait_all/diet_wait_any.
 *
 * Revision 1.27  2003/06/02 08:48:35  cpera
 * Delete debug infos.
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
 * Revision 1.19  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.13  2002/11/22 13:36:12  lbertsch
 * Added alpha linux support
 * Added a package for statistics and some traces
 *
 * Revision 1.5  2002/10/03 17:58:16  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.4  2002/10/02 17:06:15  pcombes
 * Complete ArgStack API
 *
 * Revision 1.2  2002/08/30 16:50:15  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
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

extern "C" {


/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/* The Master Agent */
static MasterAgent_var MA;

/* Trace Level */
extern unsigned int TRACE_LEVEL;

/* Error rate for contract checking */
#define ERROR_RATE 0.1

/* Maximum servers to be answered */
static unsigned long MAX_SERVERS = 10;

static char * refCallbackServer;

static int DIET_ct = 1;
/****************************************************************************/
/* GridRPC API                                                              */
/****************************************************************************/



/****************************************************************************/
/* Initialize and Finalize session                                          */
int
diet_initialize(char* config_file_name, int argc, char* argv[])
{
  char*  MA_name;
  int    res(0);
  int    myargc;
  char** myargv;
  
  /* Set arguments for ORBMgr::init */

  myargc = argc;
  myargv = (char**)malloc(argc * sizeof(char*));
  for (int i = 0; i < argc; i++)
    myargv[i] = argv[i];

  /* Parsing */

  Parsers::Results::param_type_t compParam[] = {Parsers::Results::MANAME};
  
  if ((res = Parsers::beginParsing(config_file_name)))
    return res;
  if ((res = Parsers::parseCfgFile(false, 1,
			     (Parsers::Results::param_type_t*)compParam))) {
    Parsers::endParsing();
    return res;
  }

  /* Some more checks */
  void* value =
    Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (value != NULL)
    cerr << "Warning while parsing " << config_file_name
	 << ": it is useless to name a client - ignored.\n";
  value =
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (value != NULL)
    cerr << "Warning while parsing " << config_file_name << ": no need "
	 << "to specify a parent for a client - ignored.\n";
  value =
    Parsers::Results::getParamValue(Parsers::Results::AGENTTYPE);
  if (value != NULL)
    cerr << "Warning while parsing " << config_file_name
	  << ": agentType is useless for a client - ignored.\n";
  value = Parsers::Results::getParamValue(Parsers::Results::USEASYNCAPI);
  DIET_ct = *(size_t *)(value);
    
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
  
  if (DIET_ct == 1){
    /* Initialize the ORB */
    if (ORBMgr::init(myargc, (char**)myargv, true, 1)) {
      cerr << "ORB initialization failed.\n";
      return 1;
    }
    // Create sole instance of synchronized CallAsyncMgr class
    CallAsyncMgr::Instance();
    // Create servant callback object
    CallbackImpl* cb = new CallbackImpl;
    CORBA::Object_var obj = cb->_this();
    // activate servant callback
    if (ORBMgr::activate(cb) != 0) return -1;
    // create corba client callback serveur reference ...
    refCallbackServer = ORBMgr::getIORString(obj);
    // clean
    cb->_remove_ref();
    if (refCallbackServer == NULL) return -1;
  }
  else {
    if (ORBMgr::init(myargc, (char**)myargv, false, 0)) {
      cerr << "ORB initialization failed.\n";
      return 1;
    }
  }

  /* Find Master Agent */
  MA_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::MANAME);
  MA = MasterAgent::_narrow(ORBMgr::getAgentReference(MA_name));
  if (CORBA::is_nil(MA)){
    cerr << "Cannot locate Master Agent " << MA_name << ".\n";
    return 1;
  }
  /* Initialize statistics module */
  stat_init();

  /* We do not need the parsing results any more */
  Parsers::endParsing();  

  return 0;
}

int
diet_finalize()
{
  stat_finalize();
  if (DIET_ct == 1){
    CallAsyncMgr * caMgr = CallAsyncMgr::Instance();
    while (caMgr->areThereWaitRules() > 0){
      omni_thread::sleep(1);
      // must be replace by a cal to waitall
      // must be a call to diet_finalyze_force ....
      // Maybe we must split async api from sync api ...
    }
    caMgr->release();
  }
  ORBMgr::destroy();
  return 0;
}


/****************************************************************************/
/* Data handles                                                             */

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


/****************************************************************************/
/* Argument Stack                                                           */

struct diet_argStack_elt_s {
  diet_arg_mode_t mode;
  diet_arg_t*     arg;
};
struct diet_argStack_s {
  size_t               maxsize, size;
  int                  first;
  diet_argStack_elt_t* stack;
};

diet_argStack_elt_t*
newArgStack_elt(diet_arg_mode_t mode, diet_arg_t* arg)
{
  diet_argStack_elt_t* res = new diet_argStack_elt_t;
  res->mode = mode;
  res->arg  = arg;
  return res;
}

diet_arg_mode_t
diet_argStack_elt_mode(diet_argStack_elt_t* argStack_elt)
{
  return argStack_elt->mode;
}

diet_arg_t*
diet_argStack_elt_arg(diet_argStack_elt_t* argStack_elt)
{
  return argStack_elt->arg;
}

diet_argStack_t*
newArgStack(size_t maxsize)
{
  diet_argStack_t* res = new diet_argStack_t;
  res->maxsize = maxsize;
  res->size = 0;
  res->first = -1;
  res->stack = new diet_argStack_elt_t[maxsize];
  return res;
}

int
pushArg(diet_argStack_t* stack, diet_argStack_elt_t* arg)
{
  if (stack->size == stack->maxsize)
    return 1;
  stack->first++;
  stack->stack[stack->first] = *arg;
  return 0;
}

/* !!! allocate a new diet_argStack_elt_t => to be freed !!! */
// FIXME: take care that argument copy is partial
diet_argStack_elt_t*
popArg(diet_argStack_t* stack)
{
  diet_argStack_elt_t* arg(NULL);
  if (stack->size == 0)
    return NULL;
  arg = new(diet_argStack_elt_t);
  *arg = stack->stack[stack->first];
  stack->first--;
  stack->size--;
  return arg;
}

int
destructArgStack(diet_argStack_t* stack)
{
  delete [] stack->stack;
  delete stack;
  return 0;
}



/****************************************************************************/
/* Function handles                                                         */

struct diet_function_handle_s {
  char* pb_name;
  SeD_var server;
};
#define DIET_DEFAULT_SERVER NULL


/* Allocate a function handle and set its server to DIET_DEFAULT_SERVER */
diet_function_handle_t*
diet_function_handle_default(char* pb_name)
{
  diet_function_handle_t* res = new diet_function_handle_t;
  res->pb_name = strdup(pb_name);
  res->server = DIET_DEFAULT_SERVER;
  return res;
}

/* Allocate a function handle and set its server */
/* This function is only added for GridRPC compatibility.
   Please, avoid using it !                              */
diet_function_handle_t*
diet_function_handle_init(char* server_name)
{
  /* Cannot be implemented until servers register in Naming Service */
  return NULL;
}

/* Free a function handle */
long int
diet_function_handle_destruct(diet_function_handle_t* handle)
{
  free(handle->pb_name);
  delete handle;
  return 0;
}

/* Get the function handle linked to reqID */
diet_function_handle_t*
diet_get_function_handle(diet_reqID_t reqID)
{
  return NULL;
}



/****************************************************************************/
/* GridRPC call functions                                                   */


/* Submit a request: the profile descriptor.
   Return the index of decision that can be used, -1 if none can be used.
   NB: (*decision)->length() == 0 if no service was found. */

int
submission(corba_pb_desc_t* pb, corba_response_t*& response)
{
  int server_OK(0);
  
  response = NULL;
  try {
    response = MA->submit(*pb, MAX_SERVERS);
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    cerr << "Caught a CORBA exception (" << tc->name()
	 << ") while submitting problem.\n";
    if (response)
      response->servers.length(0);
    return -2;
  }
  
  if (!response || response->servers.length() == 0) {
    cerr << "No server found for problem " << pb->path << ".\n";
    server_OK = -1;

  } else {

    if (TRACE_LEVEL >= TRACE_MAIN_STEPS) {
      cout << "The Master Agent found the following server(s):\n";
      for (size_t i = 0; i < response->servers.length(); i++) {
	int idx = response->sortedIndexes[i];
	cout << "    " << response->servers[idx].loc.hostName << ":"
	     << response->servers[idx].loc.port << "\n";
      }
    }

    server_OK = 0;
    while ((size_t) server_OK < response->servers.length()) {
      try {
	int           idx       = response->sortedIndexes[server_OK];
	SeD_ptr       server    = response->servers[idx].loc.ior;
	CORBA::Double totalTime = response->servers[idx].estim.totalTime;
	if (server->checkContract(response->servers[idx].estim, *pb)) {
	  server_OK++;
	  continue;
	}
	if ((totalTime == response->servers[idx].estim.totalTime) ||
	    ((response->servers[idx].estim.totalTime - totalTime)
	     < (ERROR_RATE *
		MAX(totalTime,response->servers[idx].estim.totalTime))))
	  break;
	server_OK++;
      } catch (...) {
	cerr << "Warning; exception catched\n";
	server_OK++;
	continue;
      }
    }
    if ((size_t) server_OK == response->servers.length())
      server_OK = -1;
  }
  return server_OK;
}


int
diet_call(diet_function_handle_t* handle, diet_profile_t* profile)
{
  //corba_pb_desc_t& corba_pb = *(new corba_pb_desc_t());
  corba_pb_desc_t corba_pb;
  corba_profile_t corba_profile;
  corba_response_t* response(NULL);
  int subm_count, server_OK, solve_res;
  static int nb_tries(3);
  
  stat_in("diet_call");

  if (mrsh_pb_desc(&corba_pb, profile, handle->pb_name))
    return 1;

  /* Request submission : try nb_tries times */
  stat_in("diet_call.submission");
  subm_count = 0;
  do {
    if ((server_OK = submission(&corba_pb, response)) == -2)
      break;
  } while ((response) && (response->servers.length() > 0) &&
	   (server_OK == -1) && (++subm_count < nb_tries));
  stat_out("diet_call.submission");

  if (!response || response->servers.length() == 0) {
    cerr << "Unable to find a server.\n";
    if (response)
      delete response;
    return 1;
  }
  if (server_OK == -1) {
    cerr << "Unable to find a server after " << nb_tries << " tries.\n";
    delete response;
    return 1;
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

  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    delete response;
    return 1;
  }

  stat_in("diet_call.solve");

  solve_res =
    response->servers[server_OK].loc.ior->solve(handle->pb_name, corba_profile);

  stat_out("diet_call.solve");

  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    delete response;
    return 1;
  }
  
  //delete &corba_pb;
  delete response;
  stat_out("diet_call");
  return solve_res;
}


diet_reqID_t
diet_call_async(diet_function_handle_t* handle, diet_profile_t* profile)
{
  corba_pb_desc_t corba_pb;
  corba_profile_t     corba_profile;
  corba_response_t* response(NULL);
  int subm_count, server_OK, solve_res, reqID;
  static int nb_tries(3);
  try {
    /* Request submission : try nb_tries times */
    displayProfile(profile, handle->pb_name);
    if (mrsh_pb_desc(&corba_pb, profile, handle->pb_name))
      return -1;
    subm_count = 0;

    stat_in("diet_call.submission.start");
    do {
      server_OK = submission(&corba_pb, response);
    }  while ((response->servers.length() > 0)
       && (server_OK == -1) && (++subm_count < nb_tries));
    stat_out("diet_call.submission.end");

    if (!response || response->servers.length() == 0) {
      cerr << "Unable to find a server.\n";
      return -1;
    }
    if (server_OK == -1) {
      cerr << "Unable to find a server after " << nb_tries << " tries.\n";
      return -1;
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
    
    if (mrsh_profile_to_in_args(&corba_profile, profile)) return -1;
    stat_in("diet_call.solve.start");

    // get sole CallAsyncMgr singleton
    CallAsyncMgr * caMgr = CallAsyncMgr::Instance();
    // create corba client callback serveur...
    if (caMgr->addAsyncCall(response->reqID, profile) != 0) return -1;

    response->servers[server_OK].loc.ior->solveAsync(handle->pb_name,
                                                      corba_profile, 
						      response->reqID, 
                                                      refCallbackServer);
  }catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "SeD async Caught exception : " << p << endl;
    else cout << "SeD async Caught exception : " << tc->id() << endl;
    fflush(stdout);
    reqID = -1;
  }
  catch (...){
    cout << "Exception in DIETCallAsync ..." << endl;
    reqID = -1;  
  }
  stat_out("diet_call.solve.end");
  reqID = response->reqID;
  delete response;
  return reqID;  

}



// FIXME: take care that argument copies are partial
int
argStack2profile(diet_profile_t* profile, diet_argStack_t* args)
{
  int tmp;
  
  profile->last_in    = 0;
  profile->last_inout = 0;
  profile->last_out = args->size - 1;
  profile->parameters = new diet_arg_t[profile->last_out + 1];
  tmp = 0;
  for (int mode = (int) IN; mode <= (int) OUT; mode++) {
    for (int i = args->first; i >= 0; i--) {
      if (args->stack[i].mode == (diet_arg_mode_t) mode) {
	profile->parameters[tmp] = (*args->stack[i].arg);
	tmp++;
      }
    }
    switch (mode) {
    case IN:    profile->last_in    = tmp - 1; break;
    case INOUT: profile->last_inout = tmp - 1; break;
    case OUT:   {
      if (profile->last_out != tmp - 1) {
	cerr << "DIET internal error: conversion argStack to profile.\n";
	return 1;
      }
    }
    }
  }
  return 0;
}

int
diet_call_argstack(diet_function_handle_t* handle, diet_argStack_t* args)
{
  int res;
  diet_profile_t profile;

  if (argStack2profile(&profile, args))
    return 1;
  
  res = diet_call(handle, &profile);

  delete [] profile.parameters;
  return res;
}


diet_reqID_t
diet_call_argstack_async(diet_function_handle_t* handle, diet_argStack_t* args)
{
  diet_reqID_t reqID;
  diet_profile_t profile;

  if (argStack2profile(&profile, args))
    return 1;
  
  reqID = diet_call_async(handle, &profile);

  delete [] profile.parameters;
  return reqID;
}


/****************************************************************************/
/* GridRPC control and wait functions                                       */


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
  int rst = ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[1];
    simpleWait[0].reqID = reqID;
    simpleWait[0].op = WAITOPERATOR(SOLE);
    Rule * rule = new Rule;
    rule->length = 1;
    rule->ruleElts = simpleWait;
    
    // get lock on condition/waitRule
    rst = CallAsyncMgr::Instance()->addWaitRule(rule);
    // NOTES: Be carefull, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, carefull using diet_cancel
    return rst;
    
  }
  catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "diet_wait async Caught exception : " << p << endl;
    else cout << "diet_wait async Caught exception : " << tc->id() << endl;
    fflush(stdout);
  }  
  catch (const exception& e){
    cout << "Unexpected exception , what=" << e.what() << endl;
    fflush(stdout);
  }
  return rst;
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
  STATE rst = ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (unsigned int k = 0; k < length; k++){
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
  catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "diet_wait_and async Caught exception : " << p << endl;
    else cout << "diet_wait_and async Caught exception : " << tc->id() << endl;
    fflush(stdout);
  }  
  catch (const exception& e){
    cout << "Unexpected exception, what=" << e.what() << endl;
    fflush(stdout);
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
  STATE rst = ERROR;
  try {
    // Create ruleElements table ...
    ruleElement * simpleWait = new ruleElement[length];
    for (unsigned int k = 0; k < length; k++){
      simpleWait[k].reqID = IDs[k];
      simpleWait[k].op = WAITOPERATOR(OR);
    }
    Rule * rule = new Rule;
    rule->length = length;
    rule->ruleElts = simpleWait;
    
    // get lock on condition/waitRule
    // and manage return rule status
    switch (CallAsyncMgr::Instance()->addWaitRule(rule)){
      case DONE:
	for (unsigned int k = 0; k < length; k++){
	  if (CallAsyncMgr::Instance()->getStatusReqID(IDs[k]) == DONE){
	    *IDptr = IDs[k];
	    return DONE;
	  }
	}
	return ERROR;
      case CANCEL:
	return CANCEL;
      case ERROR:
	return ERROR;
      default:
      return -1; // Unexcpected error, no value describing it (enum STATE)
    // NOTES: Be carefull, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, carefull using diet_cancel
    }
  }
  catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "diet_wait_or async Caught exception : " << p << endl;
    else cout << "diet_wait_or async Caught exception : " << tc->id() << endl;
    fflush(stdout);
  }  
  catch (const exception& e){
    cout << "Unexpected exception , what=" << e.what() << endl;
    fflush(stdout);
  }
  return rst;
}

/*****************************************************************************
* diet_wait_all GridRPC function
* return error status
* 
*****************************************************************************/
int
diet_wait_all()
{
  return CallAsyncMgr::Instance()->addWaitAllRule();
}

/*****************************************************************************
* diet_wait_any GridRPC function
* return the ID of the received request
* 
*****************************************************************************/
int
diet_wait_any(diet_reqID_t* IDptr)
{
  return CallAsyncMgr::Instance()->addWaitAnyRule(IDptr);
}



} // extern "C"
