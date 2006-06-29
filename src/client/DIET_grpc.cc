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


#define BEGIN_API extern "C" {
#define END_API   } // extern "C"


/****************************************************************************/
/* Initializing and Finalizing Functions                                    */
/****************************************************************************/

BEGIN_API
grpc_error_t
grpc_initialize(char* config_file_name)
{
  return diet_initialize(config_file_name, 0, NULL);
}
END_API


/****************************************************************************/
/* Remote Function Handle Management Functions                              */
/****************************************************************************/

BEGIN_API

#define DIET_DEFAULT_SERVER NULL

/* Allocate a function handle and set its server to DIET_DEFAULT_SERVER */
grpc_error_t
grpc_function_handle_default(grpc_function_handle_t* handle, char* func_name)
{
  //  grpc_function_handle_t* handle = new grpc_function_handle_t;
  handle = new grpc_function_handle_t;
  handle->func_name = strdup(func_name);
  handle->server = DIET_DEFAULT_SERVER;
    //  return handle;
  return 0;
}

/* Allocate a function handle and set its server */
/* This function is only added for GridRPC compatibility.
   Please, avoid using it !                              */
grpc_error_t
grpc_function_handle_init(grpc_function_handle_t* handle,
			  char* server_host_port, char* func_name)
{
  // FIXME: to implement this function, we need to be able to convert a string
  // host:port into a CORBA IOR: host_portToIOR()
  WARNING(__FUNCTION__ << " is not fully implemented yet:"
	  << " it is redirected to grpc_handle_default)");
  handle = new grpc_function_handle_t;
  handle->func_name = strdup(func_name);
#if IT_IS_DONE
  handle->server = ORBMgr::stringToObject(host_portToIOR(server_host_port));
#else  // IT_IS_DONE
  handle->server = DIET_DEFAULT_SERVER;
#endif // IT_IS_DONE
  return 0;
}

/* Free a function handle */
grpc_error_t
grpc_function_handle_destruct(grpc_function_handle_t* handle)
{
  free(handle->func_name);
  delete handle;
  return 0;
}

/* Get the function handle linked to reqID */
grpc_error_t
grpc_get_handle(grpc_function_handle_t** handle,
		grpc_sessionid_t sessionID)
{
  // FIXME: Christophe ...
  //  ERROR(__FUNCTION__ << " is not implemented yet", 1);
  return diet_get_handle(handle, sessionID);
}

END_API


/****************************************************************************/
/* GridRPC Argument Stack                                                   */
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
		       SeD_var& chosenServer, diet_reqID_t* reqID);

/** [internal] Convert the list of arguments into a diet_profile_t */
grpc_error_t
grpc_build_profile(diet_profile_t*& profile, char* pb_name, va_list arglist)
{

  typedef struct diet_grpc_arg_list_s {
    diet_grpc_arg_t*      grpc_arg;
    diet_grpc_arg_list_s* next;
  } diet_grpc_arg_list_t;

  diet_grpc_arg_t*      grpc_arg;
  diet_grpc_arg_list_t* first(NULL);
  diet_grpc_arg_list_t* last(NULL);
  int nb_in(0), nb_inout(0), nb_out(0);

  first = (diet_grpc_arg_list_t*)calloc(1, sizeof(diet_grpc_arg_list_t));
  last  = first;
  do {
    grpc_arg = va_arg(arglist, diet_grpc_arg_t*);
    if (grpc_arg) {
      switch(grpc_arg->mode) {
      case IN:    nb_in++;
      case INOUT: nb_inout++;
      case OUT:   nb_out++;
      }
      last->next = (diet_grpc_arg_list_t*)
	calloc(1, sizeof(diet_grpc_arg_list_t));
      last = last->next;
      last->grpc_arg = grpc_arg;
    }
  } while (grpc_arg != NULL);
  profile = diet_profile_alloc(pb_name,
			       nb_in - 1, nb_in + nb_inout - 1,
			       nb_in + nb_inout + nb_out - 1);
  last = first->next;
  for (int i = 0; i < nb_out; i++) {
    if (last == NULL) {
      INTERNAL_ERROR("in conversion of the arguments list "
		     << "into a DIET profile", 1);
    }
    try {
      profile->parameters[i] = *(last->grpc_arg->arg);
    } catch (...) {
      ERROR(__FUNCTION__ << " was passed arguments of wrong type", 1);
    }
    last = last->next;
  }
  if (last != NULL) {
    INTERNAL_ERROR("in conversion of the arguments list "
		   << "into a DIET profile", 1);
  }

  do {
    diet_grpc_arg_list_t* cur = first;
    first = first->next;
    free(cur);
  } while(first != NULL);
  
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
  corba_response_t* response(NULL);
  grpc_error_t res(0);

  va_start(ap, handle);
  if ((res = grpc_build_profile(profile, handle->func_name, ap)))
    return res;
  va_end(ap);

  res = diet_call_common(profile, server);
  handle->server = ORBMgr::getIORString(response->servers[0].loc.ior);
  diet_profile_free(profile);
  return res;
}

grpc_error_t
grpc_call_async(grpc_function_handle_t* handle,
		grpc_sessionid_t* sessionID, ...)
{
  diet_profile_t* profile;
  va_list ap;
  grpc_error_t res(0);
  CORBA::Object_var chosenObject;
  SeD_var chosenServer;
	  
  if (!sessionID) {
    ERROR(__FUNCTION__ << ": 2nd argument has not been allocated", 1);
  }

  // Store the allocated handle in the dedicated map
  diet_save_handle(*sessionID, handle);

  va_start(ap, sessionID);
  if ((res = grpc_build_profile(profile, handle->func_name, ap))) {
    set_req_error(*sessionID, res);
    return res;
  }
  va_end(ap);
  
  chosenObject = ORBMgr::stringToObject(handle->server);
  chosenServer = SeD::_narrow(chosenObject);
  res = diet_call_async_common(profile,chosenServer, sessionID);
  diet_profile_free(profile);
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

  if ((res = grpc_build_profile(profile, handle->func_name, args)))
    return res;

  chosenObject = ORBMgr::stringToObject(handle->server);
  chosenServer = SeD::_narrow(chosenObject);  
  res = diet_call_common(profile, chosenServer);
  diet_profile_free(profile);
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

  if ((res = grpc_build_profile(profile, handle->func_name, args)))
    return res;
  
  chosenObject = ORBMgr::stringToObject(handle->server);
  chosenServer = SeD::_narrow(chosenObject);  
  res = diet_call_async_common(profile, chosenServer, sessionID);
  diet_profile_free(profile);
  return 0;
}

END_API

/****************************************************************************/
/* GridRPC Error Reporting Functions                                        */
/****************************************************************************/

BEGIN_API

char *grpc_error_string(grpc_error_t error_code)
{

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
}		
																								
END_API
