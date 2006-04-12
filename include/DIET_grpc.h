/****************************************************************************/
/* GridRPC compliancy for the client interface of DIET                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/04/12 16:13:11  ycaniou
 * Commentaries C++->C to avoid compilation warnings
 *
 * Revision 1.2  2003/10/13 12:59:21  cpera
 * Change int to int32_t type in order to manage 32-64 bit system without using CORBA::Long into
 * Gridrpc API.
 *
 * Revision 1.1  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 ****************************************************************************/


#ifndef _DIET_GRPC_H_
#define _DIET_GRPC_H_

#include "DIET_data.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/****************************************************************************/
/* GridRPC Data Types                                                       */

/* Variables of this data type represent remote function that has been bound
   to a specific server. Until destroyed, a function handle may be used to
   invoke the remote function as many times as desired.                     */
typedef struct grpc_function_handle_s grpc_function_handle_t;

/* Variables of this data type represent a specific non-blocking GridRPC
   call. Session Ids are used to probe or wait for call completion, to
   cancel a call, or to check the error status of a call.                   */
typedef int32_t diet_reqID_t;
#define grpc_sessionid_t              diet_reqID_t

/* This data type is used for argument stacks. (see below)                  */
#define grpc_arg_stack_t              grpc_arg_stack_t

/* This data type is used for all error and return status codes from GridRPC
   functions.                                                               */
typedef int diet_error_t;
#define grpc_error_t                  diet_error_t


/****************************************************************************/
/* Initializing and Finalizing Functions                                    */

/* Read the configuration file and initialize the required modules.         */
grpc_error_t
grpc_initialize(char* config_file_name);
/* Release any resource being used by GridRPC.                              */
#define grpc_finalize                 diet_finalize


/****************************************************************************/
/* Remote Function Handle Management Functions                              */

/* Allocate a new function handle using a default server assoicated with the
   given function name. This default server is bnot valid, and tells DIET to
   determine it through the complete hierarchy at a call function. */
grpc_error_t
grpc_function_handle_default(grpc_function_handle_t* handle, char* pb_name);

/* Allocate a new function handle with a server explicitly specified by the
   user. This explicit server is specified by a string of the form
   "host:port". THIS FUNCTION IS STILL NOT IMPLEMENTED.                     */
grpc_error_t
grpc_function_handle_init(grpc_function_handle_t* handle,
			  char* server_host_port, char* func_name);

/* Release all information and resources associated with the specified
   function  handle.                                                        */
grpc_error_t
grpc_function_handle_destruct(grpc_function_handle_t* handle);

/* Return the function handle corresponding to the given session ID.        */
grpc_error_t
grpc_get_function_handle(grpc_function_handle_t* handle,
			 grpc_sessionid_t sessionID);


/****************************************************************************/
/* GridRPC Argument Stack                                                   */

/* 1. Elements of the stack */

typedef enum {IN, INOUT, OUT}  diet_grpc_arg_mode_t;
typedef struct diet_grpc_arg_s diet_grpc_arg_t;

/* Allocate a new diet_grpc_arg_t => to be freed !!! */
diet_grpc_arg_t*
diet_grpc_arg_alloc(diet_grpc_arg_mode_t mode, diet_arg_t* arg);
/* Return the mode of the argument */
diet_grpc_arg_mode_t
diet_grpc_arg_mode(diet_grpc_arg_t* arg);
/* Return the DIET version of the argument */
diet_arg_t*
diet_grpc_arg_diet_arg(diet_grpc_arg_t* arg);

/* 2. Stack functions */

typedef struct grpc_arg_stack_s grpc_arg_stack_t;

/* Create a new argument stack. maxsize is the maximum number of arguments
   that can be pushed onto this stack.                                      */
grpc_arg_stack_t*
grpc_stack_create(size_t maxsize);

/* Push the argument onto the stack. If the push operation exceeds the
   maxsize of the stack, then the stack is not changed, and -1 is returned.
   NB: arg is actually a (diet_grpc_arg_t *) and its fields are copied.     */
grpc_error_t
grpc_stack_push(grpc_arg_stack_t* stack, void* arg);

/* Remove the top element of the stack. If the stack is empty, return NULL.
   NB: the result is newly allocated (and thus to be freed)                 */
grpc_error_t
grpc_stack_pop(grpc_arg_stack_t* stack, diet_grpc_arg_t** arg);

/* Free all arguments of the stack. */
grpc_error_t
grpc_stack_destruct(grpc_arg_stack_t* stack);


/****************************************************************************/
/* GridRPC Call Functions                                                   */

/* Make a blocking call with a variable number of arguments.
 * NB: The GridRPC way of passing arguments is not DIET-compliant, since it
 *     does not take into account the qualifiers IN, INOUT or OUT. That is
 *     why it has been decided to use only pointers to diet_grpc_arg_t
 *     structures in the variable-length list of arguments.
 *     Moreover, the arguments must be given to grpc_call* functions in the
 *     same order as they are declared by the service, and the list must be
 *     terminated by a NULL argument.                                       */
grpc_error_t
grpc_call(grpc_function_handle_t* handle, ...);

/* Make a non-blocking remote procedure call with a variable number of
   arguments. A session ID is filled in that can be used to probe or wait
   for completion, cancellation, or to check the error status of the call.  */
grpc_error_t
grpc_call_async(grpc_function_handle_t* handle,
		grpc_sessionid_t* sessionID, ...);

/* Make a blocking call using the argument stack.                           */
grpc_error_t
grpc_call_argstack(grpc_function_handle_t* handle, grpc_arg_stack_t* args);

/* Make a non-blocking call using the argument stack. Similarly, a session
   ID is filled in that can be used to probe or wait for completion,
   cancellation, or to check the error status of the call.                  */
grpc_error_t
grpc_call_argstack_async(grpc_function_handle_t* handle,
			 grpc_sessionid_t* sessionID, grpc_arg_stack_t* args);


/****************************************************************************/
/* Asynchronous GridRPC Control Functions                                   */

/* Cancel the specified asynchronous GridRPC call.                          */
#define grpc_cancel                   diet_cancel
/* Cancel all outstanding asynchronous GridRPC calls.                       */
#define grpc_cancel_all               diet_cancel_all

/* Check whether the asynchronous GridRPC call has completed.               */
#define grpc_probe                    diet_probe
/* Check an array of session IDs for any GridRPC calls that have completed. */
#define grpc_probe_or                 diet_probe_or


/****************************************************************************/
/* Asynchronous GridRPC Control Functions                                   */

/* Block until the specified non-blocking request to complete.              */
#define grpc_wait                     diet_wait

/* Block until all of the specified non-blocking requests in a given set
   have completed.                                                          */
#define grpc_wait_and                 diet_wait_and

/* Block until any of the specified non-blocking requests in a given set
   have completed. Exactly one session ID is returned in idPtr.             */
#define grpc_wait_or                  diet_wait_or

/* Block until all previously issued non-blocking requests have completed.  */
#define grpc_wait_all                 diet_wait_all

/* Block until any previously issued non-blocking requests have completed.
   Exactly one session ID is returned in idPtr.                             */
#define grpc_wait_any                 diet_wait_any


/****************************************************************************/
/* Error Reporting Functions                                                */

#define grpc_perror                   diet_perror
#define grpc_error_string             diet_error_string
#define grpc_get_error                diet_get_error
#define grpc_get_last_error           diet_get_last_error


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_GRPC_H_ */
