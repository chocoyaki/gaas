/****************************************************************************/
/* GridRPC compliancy for the client interface of DIET                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Eddy CARON (Eddy.Caron@ens-lyon.fr)                                 */
/*    - Cedric Tedeschi (Cedric.Tedeschi@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.9  2007/06/28 18:23:19  rbolze
 * add dietReqID in the profile.
 * and propagate this change to all functions that  have both reqID and profile parameters.
 * TODO : look at the asynchronous mechanism (client->SED) to propage this change.
 *
 * Revision 1.8  2006/07/10 13:39:46  aamar
 * Correct some warnings
 *
 * Revision 1.7  2006/07/07 18:37:20  aamar
 * Correction of memory allocation problem in switch.
 *
 * Revision 1.6  2006/07/07 09:07:43  aamar
 * Add the implementation of the following functions:
 *   - grpc_finalize, grpc_cancel, grpc_cancel_all, grpc_probe, grpc_probe_or,
 *   grpc_wait, grpc_wait_and, grpc_wait_all and grpc_wait_any.
 * Other changes to be GRPC compliant: service call, results retrieving,
 *  profile management. Currently, only scalars types are supported.
 *
 * Revision 1.5  2006/06/29 15:02:41  aamar
 * Make change to handle the new type definition of grpc_function_handle_t (from a grpc_function_handle_s to grpc_function_handle_s*
 *
 * Revision 1.4  2006/06/29 13:05:43  aamar
 * Little change to handle function prototype (handle* to handle**)
 *
 * Revision 1.3  2006/06/29 12:35:06  aamar
 * Change the name of grpc_get_function_handle to grpc_get_handle and link it to diet_get_handle
 *
 * Revision 1.2  2006/06/21 23:14:11  ecaron
 * - New structure for grpc_function_handle_s to become compliant with
 * the client provides by gridrpc-wg to check the interoperability.
 *
 * - Add new grpc function : grpc_error_string
 *
 * Revision 1.1  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 ****************************************************************************/

#include "DIET_client.h" // includes DIET_grpc.h
#include <ORBMgr.hh>
#include <stdarg.h>
#include "debug.hh"
#include <vector>
#include <map>

#define BEGIN_API extern "C" {
#define END_API   } // extern "C"

using namespace std;

static vector<grpc_function_handle_t> handles;

static bool grpc_initialized = false;

extern bool 
getProfileDesc(const char * srvName, diet_profile_desc_t& profile);

extern diet_reqID_t*
get_all_session_ids(int& len);

grpc_error_t
diet_cancel_all();

void
set_req_error(diet_reqID_t sessionID,
	      diet_error_t error);

/****************************************************************************
 * check if the handle is initialized
 ***************************************************************************/
bool isInitialized(const grpc_function_handle_t handle) {
  for (unsigned int ix=0; ix < handles.size(); ix++) {
    if (handles[ix] == handle) 
      return true;
  }
  return false;
}
/****************************************************************************
 * remove the handle from the initialized handles vector
 ***************************************************************************/
bool removeFromInitialized(const grpc_function_handle_t handle) {
  for (vector<grpc_function_handle_t>::iterator p = handles.begin();
       p != handles.end();
       ++p) {
    if ( (*p) == handle) {
      handles.erase(p);
      return true;
    }
  }
  return false;
}
/****************************************************************************/
/* Initializing and Finalizing Functions                                    */
/****************************************************************************/

BEGIN_API
grpc_error_t
grpc_initialize(char* config_file_name)
{
  
  grpc_error_t err =  diet_initialize(config_file_name, 0, NULL);
  grpc_initialized = true;
  return err;
}

grpc_error_t
grpc_finalize() {
  if (!grpc_initialized) {
    return GRPC_NOT_INITIALIZED;
  }
  grpc_error_t err = diet_finalize();
  grpc_initialized = false;
  return err;
}

END_API


/****************************************************************************/
/* Remote Function Handle Management Functions                              */
/****************************************************************************/

BEGIN_API

struct diet_grpc_arg_s {
  diet_grpc_arg_mode_t mode;
  diet_arg_t*          arg;
};
struct grpc_arg_stack_s {
  size_t           maxsize;
  int              first;
  diet_grpc_arg_t* stack;
};


grpc_error_t
set_handle_param(grpc_function_handle_t* handle,
		 char* server_host_port) {
  // put this handle in the vector of initialized handles
  handles.push_back(*handle);

  return GRPC_NO_ERROR;
}

#define DIET_DEFAULT_SERVER NULL

/* Allocate a function handle and set its server to DIET_DEFAULT_SERVER */
grpc_error_t
grpc_function_handle_default(grpc_function_handle_t* handle, char* func_name)
{
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  //  grpc_function_handle_t* handle = new grpc_function_handle_t;
  (*handle) = new grpc_function_handle_s;
  (*handle)->func_name = strdup(func_name);
  (*handle)->pb = NULL;
  (*handle)->server = DIET_DEFAULT_SERVER;
  // put this handle in the vector of initialized handles
  handles.push_back(*handle);
  // Search for the service
  if (!getProfileDesc( (*handle)->func_name, 
		       (*handle)->profile))
    return GRPC_SERVER_NOT_FOUND;
  // put this handle in the vector of initialized handles
  handles.push_back(*handle);
  //  return handle;
  return GRPC_NO_ERROR;
}

/* Allocate a function handle and set its server */
/* This function is only added for GridRPC compatibility.
   Please, avoid using it !                              */
grpc_error_t
grpc_function_handle_init(grpc_function_handle_t* handle,
			  char* server_host_port, char* func_name)
{
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  // FIXME: to implement this function, we need to be able to convert a string
  // host:port into a CORBA IOR: host_portToIOR()
  WARNING(__FUNCTION__ << " is not fully implemented yet:"
	  << " it is redirected to grpc_handle_default)");
  (*handle) = new grpc_function_handle_s;
  (*handle)->func_name = strdup(func_name);
  (*handle)->pb = NULL;
#if IT_IS_DONE
  (*handle)->server = ORBMgr::stringToObject(host_portToIOR(server_host_port));
#else  // IT_IS_DONE
  (*handle)->server = DIET_DEFAULT_SERVER;
#endif // IT_IS_DONE

  // Search for the service
  if (!getProfileDesc( (*handle)->func_name, 
		       (*handle)->profile))
    return GRPC_SERVER_NOT_FOUND;

  // put this handle in the vector of initialized handles
  handles.push_back(*handle);

  return GRPC_NO_ERROR;
}

/* Free a function handle */
grpc_error_t
grpc_function_handle_destruct(grpc_function_handle_t* handle)
{
  if (!isInitialized(*handle)) {
    cout << "-- GRPC_NOT_INITIALIZED" << endl;
    return GRPC_NOT_INITIALIZED;
  }
  if ((handle) && (*handle) && ((*handle)->func_name)) {
    removeFromInitialized(*handle);
    // free the function name allocated by strdup
    free ((*handle)->func_name);
    // free the handle profile
    if ((*handle)->pb != NULL)
      diet_profile_free((*handle)->pb);
    // and finally free the handle
    delete (*handle);
    (*handle) = NULL;
  }
  else {
    return GRPC_OTHER_ERROR_CODE;
  }
  return GRPC_NO_ERROR;
}

/* Get the function handle linked to reqID */
grpc_error_t
grpc_get_handle(grpc_function_handle_t** handle,
		grpc_sessionid_t sessionID)
{
  // FIXME: Christophe ...
  //  ERROR(__FUNCTION__ << " is not implemented yet", 1);
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  return diet_get_handle(handle, sessionID);
}

END_API


/****************************************************************************/
/* GridRPC Argument Stack                                                   */
/****************************************************************************/

BEGIN_API


/** Allocate a new diet_grpc_arg_t => to be freed !!! */
diet_grpc_arg_t*
diet_grpc_arg_alloc(diet_grpc_arg_mode_t mode, diet_arg_t* arg)
{
  diet_grpc_arg_t* res = new diet_grpc_arg_t;
  res->mode = mode;
  res->arg  = arg;
  return res;
}

/** Return the mode of the argument */
diet_grpc_arg_mode_t
diet_grpc_arg_mode(diet_grpc_arg_t* arg)
{
  return arg->mode;
}

/** Return the DIET version of the argument */
diet_arg_t*
diet_grpc_arg_diet_arg(diet_grpc_arg_t* arg)
{
  return arg->arg;
}

/**
 * Create a new argument stack.
 * @param maxsize is the maximum number of arguments
 *                that can be pushed onto this stack.
 */
grpc_arg_stack_t*
grpc_stack_create(size_t maxsize)
{
  grpc_arg_stack_t* res = new grpc_arg_stack_t;
  res->maxsize = maxsize;
  res->first = -1;
  res->stack = new diet_grpc_arg_t[maxsize];
  return res;
}

/**
 * Push the argument onto the stack.
 * @return -1 if the pushing exceeds the maxsize,
 *            and then the stack is unchanged.
 * @param stack is the stack manipulated.
 * @param arg is actually a (diet_grpc_arg_t *) and its fields are copied.
 */
// FIXME: take care that argument copy is partial
grpc_error_t
grpc_arg_stack_push(grpc_arg_stack_t* stack, void* arg)
{
  if (stack->first == (int)(stack->maxsize) - 1)
    return 1;
  stack->first++;
  stack->stack[stack->first] = *((diet_grpc_arg_t*) arg);
  return 0;
}

/**
 * Remove the top element of the stack.
 * @return NULL if the stack is empty,
 *         a newly allocated argument (and thus to be freed) else.
 */
// FIXME: take care that argument copy is partial
grpc_error_t
grpc_arg_stack_pop(grpc_arg_stack_t* stack, diet_grpc_arg_t** arg)
{
  if (stack->first == -1) {
    *arg = NULL;
    ERROR(__FUNCTION__ << ": stack empty", 1);
  }
  *arg = new(diet_grpc_arg_t);
  **arg = stack->stack[stack->first];
  stack->first--;
  return 0;
}

/** Free all arguments of the stack and all resources related to it. */
grpc_error_t
grpc_stack_destruct(grpc_arg_stack_t* stack)
{
  delete [] stack->stack;
  delete stack;
  return 0;
}

END_API


/****************************************************************************/
/* Tools for GridRPC call functions    s                                    */
/****************************************************************************/

extern diet_error_t
diet_call_common(diet_profile_t* profile, SeD_var& chosenServer);

extern diet_error_t
diet_call_async_common(diet_profile_t* profile,
		       SeD_var& chosenServer);

/** [internal] Convert the list of arguments into a diet_profile_t */
grpc_error_t
grpc_build_profile(grpc_function_handle_t* handle, diet_profile_t*& profile, 
		   char* pb_name, va_list arglist)
{

  typedef struct diet_grpc_arg_list_s {
    diet_grpc_arg_t*      grpc_arg;
    diet_grpc_arg_list_s* next;
  } diet_grpc_arg_list_t;

  // parameters call
  char char_arg, *char_arg_ptr;
  short short_arg, *short_arg_ptr;
  int int_arg, *int_arg_ptr;
  long long_arg, *long_arg_ptr;
  float float_arg, *float_arg_ptr;
  double double_arg, *double_arg_ptr;

  (*handle)->args_refs = new void*[(*handle)->profile.last_out+1];

  diet_arg_t* args[(*handle)->profile.last_out+1];
  for (int ix=0; ix<= (*handle)->profile.last_out; ix++)
     args[ix] = (diet_arg_t*)calloc(1, sizeof(diet_arg_t));

  profile = 
    diet_profile_alloc(pb_name,
		       (*handle)->profile.last_in, 
		       (*handle)->profile.last_inout,
		       (*handle)->profile.last_out);


  for (int ix=0; 
       ix <= (*handle)->profile.last_in; 
       ix++) {
    switch ((*handle)->profile.param_desc[ix].type) {
    case DIET_SCALAR:
      switch ((*handle)->profile.param_desc[ix].base_type) {
      case DIET_CHAR:
	char_arg = va_arg(arglist, int);
	char_arg_ptr = new char(char_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			char_arg_ptr, DIET_VOLATILE, DIET_CHAR);
	break;
      case DIET_SHORT:
	short_arg = va_arg(arglist, int);
	short_arg_ptr = new short(short_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			short_arg_ptr, DIET_VOLATILE, DIET_SHORT);
	break;
      case DIET_INT:
	int_arg = va_arg(arglist, int);
	int_arg_ptr = new int(int_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			int_arg_ptr, DIET_VOLATILE, DIET_INT);
	break;
      case DIET_LONGINT:
	long_arg = va_arg(arglist, long);
	long_arg_ptr = new long(long_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			long_arg_ptr, DIET_VOLATILE, DIET_LONGINT);
	break;
      case DIET_FLOAT:
	float_arg = va_arg(arglist, double);
	float_arg_ptr = new float(float_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			float_arg_ptr, DIET_VOLATILE, DIET_FLOAT);
	break;
      case DIET_DOUBLE:
	double_arg = va_arg(arglist, double);
	double_arg_ptr = new double(double_arg);
	diet_scalar_set(diet_parameter(profile,ix),
			double_arg_ptr, DIET_VOLATILE, DIET_DOUBLE);
	break;
      case DIET_SCOMPLEX:
	// TO FIX
	break;
      case DIET_DCOMPLEX:
	// TO FIX
	break;
      case DIET_BASE_TYPE_COUNT:
	// TO FIX
	break;
      } // end switch scalar
      break;
    case DIET_VECTOR:
      // FIXME
      break;
    case DIET_MATRIX:
      // FIXME
      break;
    case DIET_STRING:
      // FIXME
      break;
    case DIET_PARAMSTRING:
      // FIXME
      break;
    case DIET_FILE:
      // FIXME
      break;
    case DIET_DATA_TYPE_COUNT:
      // FIXME
      break;
    }
  } // end for last_in

  for (int ix= 1 + (*handle)->profile.last_in; 
       ix <= (*handle)->profile.last_out; 
       ix++) {
    switch ((*handle)->profile.param_desc[ix].type) {
    case DIET_SCALAR:
      switch ((*handle)->profile.param_desc[ix].base_type) {
      case DIET_CHAR:
	char_arg_ptr = va_arg(arglist, char*);
	diet_scalar_set(diet_parameter(profile,ix),
			char_arg_ptr, DIET_VOLATILE, DIET_CHAR);
	(*handle)->args_refs[ix] = (void*)(char_arg_ptr);
	break;
      case DIET_SHORT:
	short_arg_ptr = va_arg(arglist, short*);
	diet_scalar_set(diet_parameter(profile,ix),
			short_arg_ptr, DIET_VOLATILE, DIET_SHORT);
	(*handle)->args_refs[ix] = (void*)(short_arg_ptr);
	break;
      case DIET_INT:
	int_arg_ptr = va_arg(arglist, int*);
	diet_scalar_set(diet_parameter(profile,ix),
			int_arg_ptr, DIET_VOLATILE, DIET_INT);
	(*handle)->args_refs[ix] = (void*)(int_arg_ptr);
	break;
      case DIET_LONGINT:
	long_arg_ptr = va_arg(arglist, long*);
	diet_scalar_set(diet_parameter(profile,ix),
			long_arg_ptr, DIET_VOLATILE, DIET_LONGINT);	
	(*handle)->args_refs[ix] = (void*)(long_arg_ptr);
	break;
      case DIET_FLOAT:
	float_arg_ptr = va_arg(arglist, float*);
	diet_scalar_set(diet_parameter(profile,ix),
			float_arg_ptr, DIET_VOLATILE, DIET_FLOAT);
	(*handle)->args_refs[ix] = (void*)(float_arg_ptr);
	break;
      case DIET_DOUBLE:
	double_arg_ptr = va_arg(arglist, double*);
	diet_scalar_set(diet_parameter(profile,ix),
			double_arg_ptr, DIET_VOLATILE, DIET_DOUBLE);
	(*handle)->args_refs[ix] = (void*)(double_arg_ptr);
	break;
      case DIET_SCOMPLEX:
	// TO FIX
	break;
      case DIET_DCOMPLEX:
	// TO FIX
	break;
      case DIET_BASE_TYPE_COUNT:
	// TO FIX
	break;
      } // end switch scalar
      break;
    case DIET_VECTOR:
      // FIXME
      break;
    case DIET_MATRIX:
      // FIXME
      break;
    case DIET_STRING:
      // FIXME
      break;
    case DIET_PARAMSTRING:
      break;
    case DIET_FILE:
      // FIXME
      break;
    case DIET_DATA_TYPE_COUNT:
      // FIXME
      break;
    }
  } // end for last_out
  
  return 0;
}

/** [internal] Convert the stack of arguments into a diet_profile_t */
grpc_error_t
grpc_build_profile(diet_profile_t*& profile,
		   char* pb_name, grpc_arg_stack_t* args)
{
  int tmp;
  
  profile = diet_profile_alloc(pb_name, 0, 0, args->first);

  tmp = 0;
  for (int mode = (int) IN; mode >= (int) OUT; mode++) {
    for (int i = 0; i <= args->first; i++) {
      if (args->stack[i].mode == (diet_grpc_arg_mode_t) mode) {
	profile->parameters[tmp] = (*args->stack[i].arg);
	tmp++;
      }
    }
    switch (mode) {
    case IN:    profile->last_in    = tmp - 1; break;
    case INOUT: profile->last_inout = tmp - 1; break;
    case OUT:   {
      if (profile->last_out != tmp - 1) {
	ERROR("argStack could not be converted into a profile", 1);
      }
    }
    }
  }
  return 0;
}


/****************************************************************************/
/* GridRPC Call Functions                                                   */
/****************************************************************************/

BEGIN_API

grpc_error_t
grpc_call(grpc_function_handle_t* handle, ...)
{
  va_list ap;
  diet_profile_t* profile;
  SeD_var server;
  grpc_error_t res(0);

  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;


  va_start(ap, handle);
  if ((res = grpc_build_profile(handle, profile, (*handle)->func_name, ap)))
    return res;
  va_end(ap);

  res = diet_call_common(profile, server);
  (*handle)->server = ORBMgr::getIORString(server);
  //  diet_profile_free(profile);
  return res;
}

grpc_error_t
grpc_call_async(grpc_function_handle_t* handle,
		grpc_sessionid_t* sessionID, ...)
{
  va_list ap;
  grpc_error_t res(0);
  CORBA::Object_var chosenObject;
  SeD_var chosenServer;
	  
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;

  if (!sessionID) {
    ERROR(__FUNCTION__ << ": 2nd argument has not been allocated", 1);
  }

  va_start(ap, sessionID);
  if ((res = grpc_build_profile(handle, (*handle)->pb, 
				(*handle)->func_name, ap))) {
    set_req_error(*sessionID, res);
    return res;
  }
  va_end(ap);
  
  /*
  chosenObject = ORBMgr::stringToObject((*handle)->server);
  chosenServer = SeD::_narrow(chosenObject);
  */
  res = diet_call_async_common((*handle)->pb, chosenServer);
  *sessionID = (*handle)->pb->dietReqID;
  set_req_error(*sessionID, GRPC_NO_ERROR);

  // Store the allocated handle in the dedicated map
  diet_save_handle(*sessionID, handle);

  //  diet_profile_free(profile);
  return res;
}

END_API

/****************************************************************************/
/* GridRPC Argument Stack Call Functions                                    */
/****************************************************************************/

BEGIN_API

grpc_error_t
grpc_call_argstack(grpc_function_handle_t* handle, grpc_arg_stack_t* args)
{
  grpc_error_t res;
  diet_profile_t* profile;
  CORBA::Object_var chosenObject;
  SeD_var chosenServer;

  if ((res = grpc_build_profile(profile, (*handle)->func_name, args)))
    return res;

  chosenObject = ORBMgr::stringToObject((*handle)->server);
  chosenServer = SeD::_narrow(chosenObject);  
  res = diet_call_common(profile, chosenServer);
  //  diet_profile_free(profile);
  return res;
}


grpc_error_t
grpc_call_argstack_async(grpc_function_handle_t* handle,
			 grpc_sessionid_t* sessionID, grpc_arg_stack_t* args)
{
  grpc_error_t res;
  diet_profile_t* profile;
  CORBA::Object_var chosenObject;
  SeD_var chosenServer;

  if ((res = grpc_build_profile(profile, (*handle)->func_name, args)))
    return res;
  
  chosenObject = ORBMgr::stringToObject((*handle)->server);
  chosenServer = SeD::_narrow(chosenObject);  
  res = diet_call_async_common(profile, chosenServer);
  *sessionID = profile->dietReqID;
  //  diet_profile_free(profile);
  return 0;
}

END_API

/****************************************************************************/
/* GridRPC Error Reporting Functions                                        */
/****************************************************************************/

BEGIN_API

char *
grpc_error_string(grpc_error_t error_code) {
  return diet_error_string(error_code);
  /*  
  switch(error_code)
    {
    case  0 : return(CORBA::string_dup("GRPC_NO_ERROR"));
    case  1 : return(CORBA::string_dup("GRPC_NOT_INITIALIZED"));
    case  2 : return(CORBA::string_dup("GRPC_CONFIGFILE_NOT_FOUND"));
    case  3 : return(CORBA::string_dup("GRPC_CONFIGFILE_ERROR"));
    case  4 : return(CORBA::string_dup("GRPC_SERVER_NOT_FOUND"));
    case  5 : return(CORBA::string_dup("GRPC_FUNCTION_NOT_FOUND"));
    case  6 : return(CORBA::string_dup("GRPC_INVALID_FUNCTION_HANDLE"));
    case  7 : return(CORBA::string_dup("GRPC_INVALID_SESSION_ID"));
    case  8 : return(CORBA::string_dup("GRPC_RPC_REFUSED"));
    case  9 : return(CORBA::string_dup("GRPC_COMMUNICATION_FAILED"));
    case 10 : return(CORBA::string_dup("GRPC_SESSION_FAILED"));
    case 11 : return(CORBA::string_dup("GRPC_NOT_COMPLETED"));
    case 12 : return(CORBA::string_dup("GRPC_NONE_COMPLETED"));
    default : return(CORBA::string_dup("GRPC_OTHER_ERROR_CODE"));
    }
  */	
} 

grpc_error_t
grpc_get_results(grpc_sessionid_t reqID) {
  grpc_function_handle_t* handle;
  grpc_error_t err;
  err = grpc_get_handle(&handle, reqID);
  if (err == GRPC_NO_ERROR) {
    for (int ix=(*handle)->pb->last_in + 1;
	 ix <= (*handle)->pb->last_out;
	 ix++) {
      if ((*handle)->pb->parameters[ix].desc.generic.type == DIET_SCALAR) {
	switch ((*handle)->pb->parameters[ix].desc.generic.base_type) {
	case DIET_CHAR:
	  *((char*)((*handle)->args_refs[ix])) = 
	    *((char*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_SHORT:
	  *((short*)((*handle)->args_refs[ix])) = 
	    *((short*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_INT:
	  *((int*)((*handle)->args_refs[ix])) = 
	    *((int*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_LONGINT:
	  *((long*)((*handle)->args_refs[ix])) = 
	    *((long*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_FLOAT:
	  *((float*)((*handle)->args_refs[ix])) = 
	    *((float*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_DOUBLE:
	  *((double*)((*handle)->args_refs[ix])) = 
	    *((double*)((*handle)->pb->parameters[ix].desc.specific.scal.value));
	  break;

	case DIET_SCOMPLEX:
	  // TO FIX
	  break;

	case DIET_DCOMPLEX:
	  // TO FIX
	  break;

	case DIET_BASE_TYPE_COUNT:
	  // TO FIX
	  break;
	} // end switch
      } // end if DIET_SCALAR
    }
  }
  else {
    cout << grpc_error_string(err) << endl;
  }

  return GRPC_NO_ERROR;
}

grpc_error_t
grpc_wait(grpc_sessionid_t reqID) {
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  grpc_error_t err = diet_wait(reqID);

  if (err == GRPC_NO_ERROR) {
    grpc_get_results(reqID);
  }

  return err;
}

grpc_error_t
grpc_wait_and(diet_reqID_t* IDs, size_t length) {
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  grpc_error_t err = diet_wait_and(IDs, length);

  if (err == GRPC_NO_ERROR) {
    for (unsigned int ix=0; ix<length; ix++) {
      grpc_get_results(IDs[ix]);
    }
  }

  return err;
}

grpc_error_t
grpc_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr) {
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;
  return diet_wait_or(IDs, length, IDptr);
}

/* Block until all previously issued non-blocking requests have completed.  */
grpc_error_t
grpc_wait_all() {
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  
  grpc_error_t err = diet_wait_all();

  if (err == GRPC_NO_ERROR) {
    int len;
    diet_reqID_t * sessions = get_all_session_ids(len);
    for (int ix=0; ix<len; ix++) {
      grpc_get_results(sessions[ix]);
    }
    if (sessions != NULL)
      delete sessions;
  }

  return err;
}

/* Block until any previously issued non-blocking requests have completed.
   Exactly one session ID is returned in idPtr.                             */
grpc_error_t
grpc_wait_any(diet_reqID_t* IDptr) {
  // check if GRPC is initialized
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  
  grpc_error_t err = diet_wait_any(IDptr);
  if (err == GRPC_NO_ERROR) {
      grpc_get_results(*IDptr);
  }

  return err;
}

/* Check whether the asynchronous GridRPC call has completed.               */
grpc_error_t
grpc_probe(grpc_sessionid_t sessionID) {
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  

  grpc_error_t err = diet_probe(sessionID);

  return err;
}

/* Check an array of session IDs for any GridRPC calls that have completed. */
grpc_error_t
grpc_probe_or(grpc_sessionid_t* reqIdArray,
	      size_t length,
	      grpc_sessionid_t* reqIdPtr) {
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  
  
  grpc_error_t err = diet_probe_or(reqIdArray, length, reqIdPtr);

  return err;
}

/* Cancel the specified asynchronous GridRPC call.                          */
grpc_error_t
grpc_cancel(grpc_sessionid_t sessionID) {
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  

  grpc_error_t err = diet_cancel(sessionID);

  return err;
}

/* Cancel all outstanding asynchronous GridRPC calls.                       */
grpc_error_t 
grpc_cancel_all() {
  if (!grpc_initialized)
    return GRPC_NOT_INITIALIZED;  

  grpc_error_t err = diet_cancel_all();

  return err;
}

END_API
