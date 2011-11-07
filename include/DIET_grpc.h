/**
* @file DIET_grpc.h
* 
* @brief  GridRPC compliancy for the client interface of DIET
* 
* @author Philippe COMBES (Philippe.Combes@ens-lyon.fr) 
*         Eddy CARON (Eddy.Caron@ens-lyon.fr
*         Cedric TEDESCHI (Cedric.Tedeschi@ens-lyon.fr)
* @section Licence
*   |LICENSE|                                                                
*/
<<<<<<< HEAD


=======
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7

#ifndef _DIET_GRPC_H_
#define _DIET_GRPC_H_

#include "DIET_data.h"
#include "DIET_server.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


  /****************************************************************************/
  /* GridRPC Data Types                                                       */
  /****************************************************************************/

  /**
     Structure representing a remote function that has been bound
     to a specific server. Until destroyed, a function handle may be used to
     invoke the remote function as many times as desired.                     */
  struct grpc_function_handle_s {
    char* func_name; /*!< function name */
    const char* server; /*!< server name */
    /*!< profile description for the corresponding server */
    diet_profile_desc_t profile;
    diet_profile_t * pb; /*!< problem profile */
    void ** args_refs; /*!< argument references */
  };

  /**
     Structure representing a pointer to a grpc_function_handle_t
  */
  typedef struct grpc_function_handle_s * grpc_function_handle_t;

  /**
      Value representing a specific non-blocking GridRPC
      call.                   */
  typedef int32_t diet_reqID_t;
  /**
     Session Ids are used to probe or wait for call completion, to
     cancel a call, or to check the error status of a call.
  */
#define grpc_sessionid_t              diet_reqID_t

  /**
      This data type is used for argument stacks.
  */
#define grpc_arg_stack_t              grpc_arg_stack_t

  /** This data type is used for all error and return status codes from GridRPC
      functions. */
  typedef int diet_error_t;
  /** This data type is used for all error and return status codes from GridRPC
      functions. */
#define grpc_error_t                  diet_error_t


  /****************************************************************************/
  /* Initializing and Finalizing Functions                                    */
  /****************************************************************************/

  /**
     Function used to read the configuration file and initialize the required
     modules.

     @param config_file_name configuration file name path

     @return error value specifying whether the initialization was successful
     or not:
     \arg GRPC_NO_ERROR on success
     \arg GRPC_NOT_INITIALIZED if the initialization failed, or if the
     MA/MADAG/Workflow LogService was not located
     \arg -1 if the CORBA servant callback of the client could not be activated
     \arg GRPC_ALREADY_INITIALIZED if DIET has already be initialized
     \arg DIET_FILE_IO_ERROR if an IO error happened during the config file
     parsing
     \arg DIET_PARSE_ERROR if the config file parsing happened

     @todo some error codes are not from the DIET_grpc.h file and should
     perhaps be transferred to it in order for the user to be able to manage
     them from the client code correctly

  */
  grpc_error_t
  grpc_initialize(char* config_file_name);

  /* #define grpc_finalize                 diet_finalize */
  /**
     Function used to release any resource being used by GridRPC.

     @return error value specifying whether the finalization was successful
     or not:
     \arg GRPC_NO_ERROR on success.
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all.

  */
  grpc_error_t
  grpc_finalize();



  /****************************************************************************/
  /* Remote Function Handle Management Functions                              */
  /****************************************************************************/

  /**
     Function used to allocate a new function handle using a default server
     associated with the
     given function name. This default server is not valid, and tells DIET to
     determine it through the complete hierarchy at a call function.

     @param handle function handle to allocate
     @param pb_name service name

     @return error value telling whether the call was sucessful or not:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all
     \arg GRPC_SERVER_NOT_FOUND if no server was found for that service
     \arg GRPC_NO_ERROR on success
  */
  grpc_error_t
  grpc_function_handle_default(grpc_function_handle_t* handle, char* pb_name);

  /**
     Function used to allocate a new function handle with a server explicitly
     specified by the
     user. This explicit server is specified by a string of the form
     "host:port".

     @warning THIS FUNCTION IS STILL NOT IMPLEMENTED.
     @param handle function handle to allocate
     @param server_host_port This explicit server is specified by a string of
     the form "host:port"
     @param func_name name of the function to get

     @return error value telling whether the call was sucessful or not:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all
     \arg GRPC_SERVER_NOT_FOUND if no server was found for that service
     \arg GRPC_NO_ERROR on success
  */
  grpc_error_t
  grpc_function_handle_init(grpc_function_handle_t* handle,
                            char* server_host_port, char* func_name);

  /**
     Function used to release all information and resources associated with the
     specified function handle.

     @param handle function handle to release
     @return error value telling whether the call was sucessful or not:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all
     \arg GRPC_OTHER_ERROR_CODE something else
     \arg GRPC_NO_ERROR on success

  */
  grpc_error_t
  grpc_function_handle_destruct(grpc_function_handle_t* handle);

  /**
     Function used to return the function handle corresponding to the given
     session ID.

     @param handle function handle to retrieve
     @param sessionID the id of the session of which we want to get a function
     handle
     @return error value telling whether the call was sucessful or not:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all
     \arg GRPC_INVALID_SESSION_ID is returned if the session ID is invalid
     \arg GRPC_NO_ERROR is returned if no error appeared
     \arg GRPC_OTHER_ERROR_CODE error returned if the session ID is not
     associated to a function handle
  */
  grpc_error_t
  grpc_get_handle(grpc_function_handle_t** handle,
                  grpc_sessionid_t sessionID);


  /***************************************************************************/
  /* GridRPC Argument Stack                                                  */
  /***************************************************************************/

  /* 1. Elements of the stack */

  /**
     Enumeration representing the argument mode
  */
  typedef enum {
    IN, /*!< Input argument */
    INOUT, /*!< Input/Output argument */
    OUT /*!< Output argument */
  }  diet_grpc_arg_mode_t;

  /**
     Structure representing the argument
  */
  typedef struct diet_grpc_arg_s diet_grpc_arg_t;

  /**
     Function used to allocate a new diet_grpc_arg_t

     @param mode argument mode
     @param arg argument

     @return new allocated grpc argument

     @warning The diet_grpc_arg_t should be freed !!!
  */
  diet_grpc_arg_t*
  diet_grpc_arg_alloc(diet_grpc_arg_mode_t mode, diet_arg_t* arg);

  /**
     Function used to get the argument mode of the input argument

     @param arg argument of which we want to get the argument mode
     @return argument mode of the input argument
  */
  diet_grpc_arg_mode_t
  diet_grpc_arg_mode(diet_grpc_arg_t* arg);

  /**
     Function used to return the DIET version of the argument

     @param arg argument of which we want to get the underlying argument
     @return the argument parameter corresponding

  */
  diet_arg_t*
  diet_grpc_arg_diet_arg(diet_grpc_arg_t* arg);

  /* 2. Stack functions */

  /**
     Structure representing the arguments stack
  */
  typedef struct grpc_arg_stack_s grpc_arg_stack_t;

  /**
      Function used to create a new argument stack.

      @param maxsize is the maximum number of arguments that can be pushed onto
      this stack.

      @return arguments stack created
  */
  grpc_arg_stack_t*
  grpc_stack_create(size_t maxsize);

  /**
      Function used to push the argument onto the stack. If the push operation
      exceeds the maxsize of the stack, then the stack is not changed,
      and -1 is returned.

      @param stack arguments stack on which we want to push an argument
      @param arg the argument we want to push

      @return error value telling whether the call was sucessful or not

      @warning function not implemented !

      @remark arg is actually a (diet_grpc_arg_t *) and its fields are copied.
  */
  grpc_error_t
  grpc_stack_push(grpc_arg_stack_t* stack, void* arg);

  /**
      Function used to remove the top element of the stack.
      If the stack is empty, return NULL.

      @param stack arguments stack on which we want to remove an argument
      @param arg the argument to remove from the arguments stack

      @return error value telling whether the call was sucessful or not

      @warning function not implemented !

      @remark the result is newly allocated (and thus to be freed)
  */
  grpc_error_t
  grpc_stack_pop(grpc_arg_stack_t* stack, diet_grpc_arg_t** arg);

  /**
     Function used to free all arguments of the stack.

     @param stack arguments stack
     @return 0 in every case.

  */
  grpc_error_t
  grpc_stack_destruct(grpc_arg_stack_t* stack);


  /****************************************************************************/
  /* GridRPC Call Functions                                                   */
  /****************************************************************************/

  /**
     Function used to make a blocking call with a variable number of arguments.

     @remark The GridRPC way of passing arguments is not DIET-compliant, since
     it does not take into account the qualifiers IN, INOUT or OUT. That is
     why it has been decided to use only pointers to diet_grpc_arg_t
     structures in the variable-length list of arguments.
     Moreover, the arguments must be given to grpc_call* functions in the
     same order as they are declared by the service, and the list must be
     terminated by a NULL argument.

     @param handle function handle to call

     @return error value telling whether the call was sucessful or not:
     \arg GRPC_NO_ERROR on success
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located
     \arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     \arg GRPC_SERVER_NOT_FOUND if the server was not found

  */
  grpc_error_t
  grpc_call(grpc_function_handle_t* handle, ...);

  /**
     Function used to make a non-blocking remote procedure call with a variable
     number of arguments.
     A session ID is filled in that can be used to probe or wait for completion,
     cancellation, or to check the error status of the call.

     @param handle function handle to call
     @param sessionID session ID of the async call to perform

     @return error value specifying whether the call was successful or not:
     \arg GRPC_NO_ERROR on success
     \arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     \arg GRPC_SERVER_NOT_FOUND if the server was not found
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located

  */
  grpc_error_t
  grpc_call_async(grpc_function_handle_t* handle,
                  grpc_sessionid_t* sessionID, ...);

  /**
      Function used to make a blocking call using the argument stack.

      @param handle function handle to call
      @param args arguments stack to provide to the function call

      @return error value specifying whether the call was successful or not:
      \arg GRPC_NO_ERROR on success
      \arg 1 if the profile is wrongly built, if an CORBA error was caught while
      submitting the profile or if a profile to be sent was not inside the
      platform
      \arg GRPC_SERVER_NOT_FOUND if the server was not found

  */
  grpc_error_t
  grpc_call_argstack(grpc_function_handle_t* handle, grpc_arg_stack_t* args);

  /**
      Function used to make a non-blocking call using the argument stack.
      Similarly, a session ID is filled in that can be used to probe or wait
      for completion, cancellation, or to check the error status of the call.

      @param handle function handle
      @param sessionID session ID of the async call to perform
      @param args arguments stack to provide to the function call

      @return error value specifying whether the call was successful or not:
      \arg GRPC_NO_ERROR on success
      \arg 1 if the profile is wrongly built, if an CORBA error was caught
      while submitting the profile or if a profile to be sent was not inside
      the platform
      \arg GRPC_SERVER_NOT_FOUND if the server was not found

  */
  grpc_error_t
  grpc_call_argstack_async(grpc_function_handle_t* handle,
                           grpc_sessionid_t* sessionID, grpc_arg_stack_t* args);


  /****************************************************************************/
  /* Asynchronous GridRPC Control Functions                                   */
  /****************************************************************************/

  /* Cancel the specified asynchronous GridRPC call.                          */
  /* #define grpc_cancel                   diet_cancel */
  /**
     Function used to cancel a specific asynchronous GridRPC call

     @param sessionID session ID of an asynchronous call

     @return error value specifying whether calls have been cancelled based
     on their IDs:
     \arg GRPC_NO_ERROR on success
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located

     @todo the error messages for each request ID cancelled are not managed.

  */
  grpc_error_t
  grpc_cancel(grpc_sessionid_t sessionID);

  /* Cancel all outstanding asynchronous GridRPC calls.                       */
  /* #define grpc_cancel_all               diet_cancel_all */
  /**
     Function used to cancel all outstanding asynchronous GridRPC calls

     @return error value specifying whether calls have been cancelled based on
     their IDs:
     \arg GRPC_NO_ERROR on success
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed, or if the
     MA/MADAG/Workflow LogService was not located

  */
  grpc_error_t
  grpc_cancel_all();

  /* Check whether the asynchronous GridRPC call has completed.               */
  /* #define grpc_probe                    diet_probe */
  /**
     Function used to check whether the asynchronous GridRPC call has completed.

     @param sessionID session ID of an asynchronous call

     @return error value specifying whether the call was successful or not
     \arg GRPC_NO_ERROR on success
     \arg GRPC_NOT_COMPLETED if the call is not completed
     \arg GRPC_OTHER_ERROR_CODE if an error occured

  */
  grpc_error_t
  grpc_probe(grpc_sessionid_t sessionID);


  /* Check an array of session IDs for any GridRPC calls that have completed. */
  /* #define grpc_probe_or                 diet_probe_or */
  /**
     Function used to check an array of session IDs for any GridRPC calls that
     have completed.

     @param reqIdArray array of request IDs
     @param length length of reqIdArray
     @param reqIdPtr pointer to the first completed request

     @return error value specifying whether one of the calls was successful:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed, or if the
     MA/MADAG/Workflow LogService was not located
     \arg GRPC_INVALID_SESSION_ID is returned if one of the IDs is invalid
     \arg GRPC_NO_ERROR is returned if no error appeared
     \arg GRPC_NONE_COMPLETED is returned if no specified call has completed

  */
  grpc_error_t
  grpc_probe_or(grpc_sessionid_t* reqIdArray, size_t length,
                grpc_sessionid_t* reqIdPtr);


  /****************************************************************************/
  /* Asynchronous GridRPC Control Functions                                   */
  /****************************************************************************/

  /* Block until the specified non-blocking request to complete.              */
  /* #define grpc_wait                     diet_wait */
  /**
     Function used to block until the specified non-blocking request to complete.

     @param reqID request ID

     @return error values specifying whether the wait phase has been successful
     or not:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located
     \arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     \arg STATUS_ERROR if something went wrong when waiting
     \arg STATUS_DONE if the call has completed
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes
  */
  grpc_error_t
  grpc_wait(grpc_sessionid_t reqID);

  /* Block until all of the specified non-blocking requests in a given set
     have completed.                                                          */
  /* #define grpc_wait_and                 diet_wait_and */
  /**
     Function used to block until all of the specified non-blocking requests in
     a given set
     have completed.

     @param IDs array of request IDs to wait for
     @param length length of the input array

     @return error values specifying whether the calls have been successfully
     ²completed:
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located
     \arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     \arg STATUS_ERROR if something went wrong when waiting
     \arg STATUS_DONE if the call has completed
     @todo perhaps other error codes should be returned instead of STATUS_ERROR
     and STATUS_DONE which are located in CallAsyncMgr.hh.
     It should better be error codes located in DIET_grpc.h for the user to be
     able to take into account the output.
  */
  grpc_error_t
  grpc_wait_and(diet_reqID_t* IDs, size_t length);

  /* Block until any of the specified non-blocking requests in a given set
     have completed. Exactly one session ID is returned in idPtr. */
  /* #define grpc_wait_or                  diet_wait_or */
  /**
     Function used to block until any of the specified non-blocking requests
     in a given set have completed.
     Exactly one session ID is returned in idPtr.

     @param IDs array or request Ids
     @param length length of IDs
     @param IDptr request that was the first one to complete
     \arg GRPC_NOT_INITIALIZED if a call to grpc_initialize() has not been
     performed successfully or at all, if the initialization failed,
     or if the MA/MADAG/Workflow LogService was not located
     \arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     \arg STATUS_CANCEL Cancel has been called on a reqID.
     \arg STATUS_ERROR if something went wrong when waiting
     \arg STATUS_DONE if the call has completed
     \arg -1 an unexpeted error happened
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes inside the underlying diet call
     @todo something else than -1 should be returned ...
  */
  grpc_error_t
  grpc_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr);


  /* Block until all previously issued non-blocking requests have completed. */
  /* #define grpc_wait_all                 diet_wait_all */
  /**
     Function used to block until all previously issued non-blocking requests
     have completed

     @return error values specifying whether the calls have been successfully
     completed:
     \arg STATUS_ERROR if something went wrong when waiting
     \arg STATUS_DONE if the call has completed
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes in the underlying diet call
  */
  grpc_error_t
  grpc_wait_all();


  /* Block until any previously issued non-blocking requests have completed.
     Exactly one session ID is returned in idPtr. */
  /* #define grpc_wait_any                 diet_wait_any */
  /**
     Function used to block until any previously issued non-blocking requests
     have completed.
     Exactly one session ID is returned in idPtr.

     @param IDptr the first completed request

     @return error values specifying whether the calls have been successfully
     completed:
     \arg GRPC_NO_ERROR Cancel has been called on a reqID.
     \arg GRPC_OTHER_ERROR_CODE if something went wrong when waiting
     \arg GRPC_NO_ERROR if the call has completed
     \arg -1 an unexpected error happened
  */
  grpc_error_t
  grpc_wait_any(diet_reqID_t* IDptr);

  /***************************************************************************/
  /* Error Reporting Functions                                               */
  /***************************************************************************/

  /**
     Function used to report any error
  */
#define grpc_perror                   diet_perror

  /**
     Function used to return the error description string,
     given a GridRPC error code.
     If the error code is unrecognized for any reason, the string C
     GRPC_UNKNOWN_ERROR_CODE is returned

     @param error_code error code to translate to a string
     @return string corresponding to the input error_code

  */
  const char * grpc_error_string(grpc_error_t error_code);

  /**
     Function used to get an error
  */
#define grpc_get_error                diet_get_error
  /**
     Function used to get the last error
  */
#define grpc_get_last_error           diet_get_last_error
  /**
     Function used to get the last failed error
  */
#define grpc_get_failed_sessionid     diet_get_failed_session

  /***************************************************************************/
  /* GridRPC Error code                                                      */
  /***************************************************************************/
  /**
     Error code to use when no error appeared
  */
#define GRPC_NO_ERROR 0
  /**
     Error code specifying that the GRPC or DIET were not initialized.
     @remark This could come from a variety of reasons ...
     @todo specify other error codes to be more precise
  */
#define GRPC_NOT_INITIALIZED 1
  /**
     Error code specifying that the config file to be use for the
     initialization of the GRPC or DIET was not found
  */
#define GRPC_CONFIGFILE_NOT_FOUND 2
  /**
     Error code specifying that there was an error when reading the config file

     @todo during the parsing this error code could be used or some more
     precise error code that would be the equivalent of DIET_FILE_IO_ERROR for
     example
  */
#define GRPC_CONFIGFILE_ERROR 3
  /**
     Error code specifying that no server was found to solve the provided
     problem
  */
#define GRPC_SERVER_NOT_FOUND 4
  /**
     Error code specifying that the called function was not found
  */
#define GRPC_FUNCTION_NOT_FOUND 5
  /**
     Error code specifying that the function handle provided as invalid
  */
#define GRPC_INVALID_FUNCTION_HANDLE 6
  /**
     Error code specifying that the session id provided is invalid
  */
#define GRPC_INVALID_SESSION_ID 7
  /**
     Error code specifying that the RPC call is refused
  */
#define GRPC_RPC_REFUSED 8
  /**
     Error code specifying that the communication failed
  */
#define GRPC_COMMUNICATION_FAILED 9
  /**
     Error code specifying that the session failed
  */
#define GRPC_SESSION_FAILED 10
  /**
     Error code specifying that the performed call was not completed
  */
#define GRPC_NOT_COMPLETED 11
  /**
     Error code specifying that the none of the calls were completed
  */
#define GRPC_NONE_COMPLETED 12
  /**
     Error code specifying that the error is none of the other error code ...
     @todo perhaps more error codes are necessary th avoid using this one ...
  */
#define GRPC_OTHER_ERROR_CODE 13
  /**
     Error code specifying that the error thrown is unknown
  */
#define GRPC_UNKNOWN_ERROR_CODE 14
  /**
     Error code specifying that the initialization phase was already performed
  */
#define GRPC_ALREADY_INITIALIZED 15
  /**
     Error code to ask for the last error code
  */
#define GRPC_LAST_ERROR_CODE 16

  /***************************************************************************/
  /* Workflow error code                                                     */
  /***************************************************************************/
#ifdef HAVE_WORKFLOW
  /**
     Error code specifying that the provided XML file describing the workflow
     was malformed
  */
#define XML_MALFORMED 17
  /**
     Error code specifying that the DAG structure was not correct
  */
#define DAG_BADSTRUCT 18
  /**
     Error code specifying that the functional workflow was not correct
  */
#define WFL_BADSTRUCT 19
  /**
     Error code specifying that the server was missing
  */
#define SRV_MISS      20
#endif
  /***************************************************************************/
  /* DIET Config Parser error codes                                          */
  /***************************************************************************/

#define DIET_PARSE_ERROR        21
#define DIET_FILE_IO_ERROR      22
#define DIET_MISSING_PARAMETERS 23

  /* don't forget to update ErrorCodeStr in DIET_client.cc
     each time, you add a new error code */
#define DIET_LAST_ERROR_CODE    24

  /***************************************************************************/
  /* GridRPC Session code                                                    */
  /***************************************************************************/
  /**
     Session code used to provide a void session
  */
#define GRPC_SESSIONID_VOID -1

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_GRPC_H_ */
