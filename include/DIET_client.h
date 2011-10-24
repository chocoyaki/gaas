/****************************************************************************/
/* DIET client interface                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.45  2010/12/28 10:35:45  hguemar
 * move parsing error codes from src/utils/Parsers.hh to include/DIET_grpc.h
 *
 * Revision 1.44  2010/11/30 22:03:40  dloureir
 * Correcting headers to put more less-friendly information prior to doxygen documentation.
 *
 * Revision 1.43  2010/08/30 22:24:56  dloureir
 * Adding a lot od Doxygen documentation to the API.
 * There is a certain (huge) number of warnings, todos and remarks that must
 *  be corrected.
 * For example: function declared in DIET_server.h and implemented in
 * DIET_Data.cc ... Error code not understandable for users and not documented
 *
 * Revision 1.42  2010/07/27 10:24:31  glemahec
 * Improve robustness & general performance
 *
 * Revision 1.41  2010/07/26 00:05:20  bdepardo
 * Removed multiple declarations of diet_probe_or()
 *
 * Revision 1.40  2010/07/12 16:17:51  glemahec
 * DIET 2.5 beta 1 - Introduce DIET forwarders
 *
 * Revision 1.39  2009/09/25 12:34:01  bisnard
 * added dag cancellation API function
 *
 * Revision 1.38  2009/07/20 12:53:12  bisnard
 * obsoleted file storing persistent data IDs on client when using DTsM
 *
 * Revision 1.37  2008/04/16 08:27:07  ycaniou
 * C++ commentaries -> C commentaries to avoid warnings
 *
 * Revision 1.36  2008/04/14 09:07:21  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - diet_wf_call* functions no longer used with MaDag v2
 *
 * Revision 1.35  2008/04/10 09:13:30  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.34  2008/04/07 15:33:40  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.33  2008/04/07 12:19:13  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.32  2007/09/25 09:37:21  aamar
 * Nodes can notify the wf log service with the chosen hostname.
 *
 * Revision 1.31  2007/05/30 11:16:33  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.30  2007/04/16 22:43:42  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.29  2006/10/20 09:52:27  aamar
 * *** empty log message ***
 *
 * Revision 1.28  2006/09/21 09:11:31  ycaniou
 * Preliminary change in cmake to handle a BATCH variable
 *
 * Revision 1.27  2006/09/11 11:16:58  ycaniou
 * - client have now 3 kind of submission (default is conserved)
 * - server can be parallel in addition to batch (not yet tested)
 *
 * Revision 1.26  2006/08/30 11:56:13  ycaniou
 * Commit the changements on the API for batch/parallel submissions
 *
 * Revision 1.25  2006/07/10 11:24:22  aamar
 * Adding the following functions to the API:
 *    - enable_reordering, set_reordering_delta, nodeIsDone, nodeIsRunning,
 *     nodeIsWaiting
 *
 * Revision 1.24  2006/07/07 18:36:31  aamar
 * Add the diet_cancel_all to the API
 *
 * Revision 1.23  2006/06/29 15:01:51  aamar
 * Changing the type definition of grpc_function_handle_t from a grpc_function_handle_s to grpc_function_handle_t*
 *
 * Revision 1.22  2006/06/29 13:05:02  aamar
 * Little change of get_handle function prototype (handle* to handle** for parameter 1)
 *
 * Revision 1.21  2006/06/29 12:32:22  aamar
 * Adding the following functions to be GridRPC compliant :
 *    - diet_get_handle, diet_get_error, diet_error_string, diet_get_failed_session, diet_probe_or
 *    - diet_save_handle, set_req_error (These two functions can be removed from the header file
 *      but the DIET_grpc.cc needs their declaration)
 *
 * Revision 1.20  2006/04/18 13:10:17  ycaniou
 * C++ -> C commentaries to avoid warning
 *
 * Revision 1.19  2006/04/14 14:12:11  aamar
 * API extension for workflow support : adding the following methods
 *        diet_error_t diet_wf_call(diet_wf_desc_t* profile);
 *        void set_sched (struct AbstractWfSched * sched);
 *        void diet_wf_free(diet_wf_desc_t * profile);
 *        void set_madag_sched(int b);
 *
 * Revision 1.18  2006/04/12 16:13:11  ycaniou
 * Commentaries C++->C to avoid compilation warnings
 *
 * Revision 1.17  2005/05/02 14:51:53  ycaniou
 * The client API has now the functions diet_call_batch() and diet_call_async_batch(). The client has also the possibility to modify the profile so that it is a batch, parallel or if he wants a special number of procs.
 * Changes in diet_profile_t and diet_profile_desc_t structures
 * Functions to register a parallel or batch problem.
 * The SeD developper must end its profile solve function by a call to diet_submit_batch().
 *
 * Revision 1.16  2004/10/06 18:09:50  rbolze
 * fix a bug due to last commit :)
 *
 * Revision 1.15  2004/10/06 16:44:28  rbolze
 * add new function in API : get_diet_services which return a list of available services in the platform
 *
 * Revision 1.14  2004/05/04 11:45:21  bdelfabr
 * methods for creation of an id file modified
 *
 * Revision 1.13  2004/02/27 10:31:47  bdelfabr
 * modifications applied to corresponding header files
 *
 * Revision 1.12  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.11  2003/06/23 13:15:16  pcombes
 * diet_reqID_t back to int (GridRPC)
 *
 * Revision 1.10  2003/05/10 08:48:45  pcombes
 * log to clean at next time: Unify diet_initialize and diet_SeD prototypes.
 *
 * Revision 1.9  2003/04/10 11:27:29  pcombes
 * Add defines grpc_* for the GridRPC API.
 *
 * Revision 1.3  2002/10/15 18:36:03  pcombes
 * Remove the descriptors set functions.
 * Add convertors in API :
 *     Convertors let DIET build the sequence of arguments, that the
 *     correspunding solver needs, from the client sequence of arguments
 *     (which might match another service declared with the same solver).
 * For compatibility with old API, just add NULL for the convertor argument in
 * diet_service_table_add. But all the solvers needed before in programs using
 * DIET can be transformed into convertors.
 ****************************************************************************/
/**
   @brief DIET client interface

   @author Philippe COMBES (Philippe.Combes@ens-lyon.fr)

   @remarks License:
   $LICENSE$

   @details

   To use a \c DIET platform and access services implemented on the
   DIET Server Daemon a client program must use the DIET Client API
   described in this file.

   @page DIETclient DIET Client interface

   The most difficult part of building a client program is to understand how
   to describe the problem interface. Once this step is done, it is
   fairly easy to build calls to \b DIET.

   @section clientProgramStructure Structure of a client program

   Since the client side of \b DIET is a library, a client program has to
   define a main function that uses \b DIET through function calls.
   The complete data interface is described in \c DIET_data.

   Here is an exemple of code including \c DIET_client.h

   \code
   #include <stdio.h>
   #include <stdlib.h>

   #include "DIET_client.h"

   int main(int argc, char *argv[])
   {
   diet_initialize(configuration_file, argc, argv);
   // Successive DIET calls ...
   diet_finalize();
   }
   @endcode

   The client program must open its \b DIET session with a call to
   \c diet_initialize, which parses the configuration file to set
   all options and get a reference to the \b DIET \b Master \b Agent.
   The session is closed with a call to \c diet_finalize, which frees all
   resources associated with this session on the client. Note that
   memory allocated for all \b INOUT and \b OUT arguments brought back onto
   the client during the session is not freed during
   \c diet_finalize; this allows the user to continue to use the
   data, but also requires that the user explicitly free the memory.
   The user must also free the memory he or she allocated for \b IN
   arguments.

   @section ClientAPI Client API

   The client API follows the GridRPC definition: all
   \c diet_ functions are \b duplicated with \c grpc_
   functions.  Both \c diet_initialize \c grpc_initialize
   and \c diet_finalize \c grpc_finalize belong to the
   GridRPC API.

   A problem is managed through a @e function_handle, that
   associates a server to a problem name. For compliance with GridRPC
   \c DIET accepts \c diet_function_handle_init, but the server
   specified in the call will be ignored; \c DIET is designed to
   automatically select the best server. The structure allocation is
   performed through the function
   \c diet_function_handle_default.

   The @e function_handle returned is associated to the problem description,
   its profile, in the call to \c diet_call.
*/

#ifndef _DIET_CLIENT_H_
#define _DIET_CLIENT_H_

#include "DIET_grpc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /***************************************************************************/
  /* A DIET client can submit several request in one session.                */
  /***************************************************************************/
  /**
     Function used to initialize the \c DIET environment. This function sets up
     all the corba stuff and then connects the client to the
     \c DIET \c Master \c Agent.

     @param config_file_name configuration file name container the Client
     configuration. @see User's Manual for more information about the client
     configuration content.
     @param argc number of arguments passed to the client program
     @param argv arguments passed to the client program. \c DIET can look
     for special command-line arguments used for omniORB.
     @return error value specifying whether the initialization was successful
     or not:
     @arg GRPC_NO_ERROR on success
     @arg GRPC_NOT_INITIALIZED if the initialization failed, or if
     the MA/MADAG/Workflow LogService was not located
     @arg -1 if the CORBA servant callback of the client could not be activated
     @arg GRPC_ALREADY_INITIALIZED if DIET has already be initialized
     @arg DIET_FILE_IO_ERROR if an IO error happened during the config file
     parsing
     @arg DIET_PARSE_ERROR if the config file parsing happened

     @see the DIET_grpc.h file for more information about error values.
  */

  diet_error_t
  diet_initialize(const char* config_file_name, int argc, char* argv[]);
  /**
     Function used to finalize the \c DIET environment and free and
     the allocated memory by the DIET environment,
     this call will close all CORBA communications.

     @return error value specifying whether the finalization was successful
     or not: in fact GRPC_NO_ERROR in every case.

     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_finalize();

  /* DIET finalize call through signals catch function. */

  /**
     Function used to finalize the \c DIET environment and free and
     the allocated memory by the DIET environment,
     this call will close all CORBA communications.
     This finalization function is call like a callback when specified
     signals are thrown.

     The program will exit with the error value returned by the underlying
     diet_finalize() call.

     @param dummy the callback on which the finalize call will be done

     @sa diet_finalize()
  */
  void diet_finalize_sig(int dummy);

  /****************************************************************************/
  /* Data handles                                                             */
  /****************************************************************************/

  /**
     diet_arg_t is a handle on data

     @see diet_data_handle_t
  */
  typedef diet_arg_t diet_data_handle_t;

  /* The grpc_malloc function is useless, since its data descriptor allocation
     is useless and set functions aim at filling the structure. */
  /* Once the data handle is allocated, please use the set functions to fill in
     the structure. */

  /**
     This function is used to allocate memory for \c diet_data_handle_t.

     @return the \c diet_data_handle_t allocated

     @remarks The grpc_malloc function is useless, since its data descriptor
     allocation is useless and set functions aim at filling the structure.
     Once the data handle is allocated, please use the set functions to fill in
     the structure.
  */

  diet_data_handle_t*
  diet_data_handle_malloc();

  /**
     Function used to get a data handle present on the platform.

     @param handle handle of a data present in the platform and to be retrieved

     @return a void pointer on the value contained by the \c handle passed in
     argument
  */

  void*
  diet_get(diet_data_handle_t* handle);

  /**
     Free data on server, and free handle if and if only allocated by
     diet_data_handle_malloc.

     @param handle handle pointing to the data to free
     @return error value specifying whether the deallocation was successful or
     not: GRPC_NO_ERROR in every case.
     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_free(diet_data_handle_t* handle);

  /**
     Function used to store identifier of a persistent data in a file in
     the client directory

     @param argID id of the data to store
     @param msg message used for the data to store
  */
  void
  store_id(char* argID, char* msg);

  /* obsoleted
     void
     create_header();
  */

  /**
     Function used to free a persistent data on the platform with the
     specified \c id.

     @param id id of the persistent data to free
     @return error value corresponding the operation:
     @arg 1 if the data has been successfully freed
     @arg 0 if the data was not a persistent data or anything else

     @todo This function should probably return a diet_error_t value to
     correspond to other returned error values of api functions...
     @warning the error values are different from the ones found in DIET_grpc.h
     (0 is different from GRPC_NO_ERROR)
  */
  int
  diet_free_persistent_data(char *id);

  /****************************************************************************/
  /* Call / submission functions                                              */
  /****************************************************************************/

  /**
     Function used to perform a diet call. The main parameter is the
     \c profile describing the service to call and the in/inout/out parameters
     of the service corresponding to the service description.

     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_call(diet_profile_t* profile);

#if defined HAVE_ALT_BATCH
  /**
     Function used to perform a parallel call. The main parameter is the
     \c profile describing the service to call and the in/inout/out parameters
     of the service corresponding to the service description.

     @remark This function is only available when the batch functionnality is
     selected during compilation.

     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught
     while submitting the profile or if a profile to be sent was not inside
     the platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_parallel_call(diet_profile_t* profile);

  /**
     Function used to perform a sequential call. The main parameter is the
     \c profile describing the service to call and the in/inout/out parameters
     of the service corresponding to the service description.

     @remark This function is only available when the batch functionnality is
     selected during compilation.

     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_sequential_call(diet_profile_t* profile);
#endif

  /**
     Function used to perform a asynchronous call. The main parameter is the
     \c profile describing the service to call and the in/inout/out parameters
     of the service corresponding to the service description.
     It should be noted that the function directly returns.
     The end of the call must be controlled with the corresponding functions.
     @sa diet_probe()
     @sa diet_probe_or()
     @sa diet_wait()
     @sa diet_wait_and()
     @sa diet_wait_or()
     @sa diet_wait_all()
     @sa diet_wait_any()
     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @param reqID the request ID of the performed call.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
#if defined HAVE_ALT_BATCH
  /**
     Function used to perform a parrallel asynchronous call. The main parameter
     is the \c profile describing the service to call and the in/inout/out
     parameters of the service corresponding to the service description.
     It should be noted that the function directly returns. The end of the call
     must be controlled with the corresponding functions.
     @sa diet_probe()
     @sa diet_probe_or()
     @sa diet_wait()
     @sa diet_wait_and()
     @sa diet_wait_or()
     @sa diet_wait_all()
     @sa diet_wait_any()

     @remark This function is only available when the batch functionnality is
     selected during compilation.

     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @param reqID the request ID of the performed call.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_parallel_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
  /**
     Function used to perform a sequential asynchronous call.
     The main parameter is the \c profile describing the service to call and
     the in/inout/out parameters of the service corresponding to the service
     description.
     It should be noted that the function directly returns. The end of the call
     must be controlled with the corresponding functions.
     @sa diet_probe()
     @sa diet_probe_or()
     @sa diet_wait()
     @sa diet_wait_and()
     @sa diet_wait_or()
     @sa diet_wait_all()
     @sa diet_wait_any()

     @remark This function is only available when the batch functionnality is
     selected during compilation.

     @param profile the profile describing the service to call and the
     in/inout/out parameters of the service corresponding to the service
     description.
     @param reqID the request ID of the performed call.
     @return error value specifying whether the call was successful or not:
     @arg GRPC_NO_ERROR on success
     @arg 1 if the profile is wrongly built, if an CORBA error was caught while
     submitting the profile or if a profile to be sent was not inside the
     platform
     @arg GRPC_SERVER_NOT_FOUND if the server was not found

     @see the DIET_grpc.h file for more information about error values.
  */

  diet_error_t
  diet_sequential_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
#endif

  /****************************************************************************/
  /* Request control and wait functions                                       */
  /****************************************************************************/

  /* Return 0 if the asynchronous call has completed. */
  /**
     Function used to test whether the asynchronous call has completed or not.

     @param reqID the request ID to test
     @return error value specifying whether the call was successful or not
     @arg GRPC_NO_ERROR on success
     @arg GRPC_NOT_COMPLETED if the call is not completed
     @arg GRPC_OTHER_ERROR_CODE if an error occured
     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_probe(diet_reqID_t reqID);
  /* Return 0 if any asynchronous calls in the array of reqIDs have completed.
     Then exactly one reqID is returned in IDptr.                             */

  /**
     Function used to test whether any of the asynchronous calls
     (the corresponding requests' ID are given in the diet_reqID_t array)
     has completed. Exactly one of the requests' ID is return in \c IDptr.

     @param IDs the requests' ID of the asynchronous calls to test.
     @param length length of the ID's array passed in argument
     @param IDptr pointer to the ID of exactly one asynchronous call completed

     @return error value specifying whether one of the calls was successful:
     @arg GRPC_INVALID_SESSION_ID is returned if one of the IDs is invalid
     @arg GRPC_NO_ERROR is returned if no error appeared
     @arg GRPC_NONE_COMPLETED is returned if no specified call has completed
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_probe_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr);

  /* Get the function handle linked to reqID */

  /**
     Function used to get a function handle on the service executed for the
     corresponding \c sessionID.

     @param handle handle of the executed function corresponding to the
     sessionID
     @param sessionID id of session for which we want to get the function
     handle

     @return error value specifying whether the handle could be retrieve or not:
     @arg GRPC_INVALID_SESSION_ID is returned if the session ID is invalid
     @arg GRPC_NO_ERROR is returned if no error appeared
     @arg GRPC_OTHER_ERROR_CODE error returned if the session ID is not
     associated to a function handle
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_get_handle(grpc_function_handle_t** handle,
                  diet_reqID_t sessionID);

  /* Save the specified handle and associate it to a sessionID */
  /**
     Function used to save a function handle and then associate it to a
     sessionID.

     @param sessionID ID of session to which the function handle is associated
     @param handle function handlewe want to associate to a session ID

  */
  void
  diet_save_handle(diet_reqID_t sessionID,
                   grpc_function_handle_t* handle);

  /* Set the error code of the defined session (reqID) */
  /**
     Function used to set the error code of the defined session.

     @param sessionID id of the session of which we want to set the error code
     @param error error code to set for the specified session
  */
  void
  set_req_error(diet_reqID_t sessionID,
                diet_error_t error);
  /* This function erases all persistent data that are manipulated by the reqID
     request. Do not forget to call diet_get_data_handle on data you would like
     to save. */
  /**
     Function used to cancel a request ID. It cuts the link between the SeD
     and the client.

     @remark This function erases all persistent data that are manipulated by
     the reqID request.
     Do not forget to call diet_get_data_handle on data you would like to save.

     @param reqID request ID we want to cancel
     @return error value specifying whether the call could be cancelled based
     on its ID or not:
     @arg GRPC_INVALID_SESSION_ID is returned if the session ID is invalid
     @arg GRPC_NO_ERROR is returned if no error appeared
     @arg -1 error returned if something when wrong during the cancel phase
     @arg > 0 something else
     @todo take a look at  CallAsyncMgr::deleteAsyncCallWithoutLock().
     Why are we returning values that could not be understand be the user ?
     Should better be something like error codes in the DIET_grph.h...
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_cancel(diet_reqID_t reqID);

  /**
     Function used to cancel all request ID.

     @remark This function erases all persistent data that are manipulated by
     the request IDs request.
     Do not forget to call diet_get_data_handle on data you would like to save.

     @return error value specifying whether calls have been cancelled based on
     their IDs:
     @arg GRPC_NO_ERROR in every cases
     @todo the error messages for each request ID cancelled are not managed.
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_cancel_all();

  /**
     Function used to wait for a call to complete based on its request id

     @param reqID request ID to wait for

     @return error values specifying whether the wait phase has been successful
     or not:
     @arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     @arg STATUS_ERROR if something went wrong when waiting
     @arg STATUS_DONE if the call has completed
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wait(diet_reqID_t reqID);

  /**
     Function used to wait for all of the calls to complete based on their
     request ids

     @param IDs IDs of the calls to wait for
     @param length the number of requests to wait for

     @return error values specifying whether the calls have been successfully
     completed:
     @arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     @arg STATUS_ERROR if something went wrong when waiting
     @arg STATUS_DONE if the call has completed
     @todo perhaps other error codes should be returned instead of STATUS_ERROR
     and STATUS_DONE which are located in CallAsyncMgr.hh. It should better be
     error codes located in DIET_grpc.h for the user to be able to take into
     account the output.
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wait_and(diet_reqID_t* IDs, size_t length);

  /**
     Function used to wait for the first of the calls to complete based on their
     request ids

     @param IDs IDs of the calls to wait for
     @param length the number of requests to wait for
     @param IDptr the first completed request

     @return error values specifying whether the calls have been successfully
     completed:
     @arg GRPC_INVALID_SESSION_ID is returned if the request ID is invalid
     @arg STATUS_CANCEL Cancel has been called on a reqID.
     @arg STATUS_ERROR if something went wrong when waiting
     @arg STATUS_DONE if the call has completed
     @arg -1 an unexpeted error happened
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes
     @todo something else than -1 should be returned ...
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr);

  /**
     Function used to wait for all of the calls performed in the plateform in
     a asynchronous manner.

     @return error values specifying whether the calls have been successfully
     completed:
     @arg STATUS_ERROR if something went wrong when waiting
     @arg STATUS_DONE if the call has completed
     @todo status_to_grpc_code() must be used to convert status codes to grpc
     error codes
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wait_all();

  /**
     Function used to wait for the first of the calls performed in the
     plateform in an asynchronous manner.

     @param IDptr the first completed request

     @return error values specifying whether the calls have been successfully
     completed:
     @arg GRPC_NO_ERROR Cancel has been called on a reqID.
     @arg GRPC_OTHER_ERROR_CODE if something went wrong when waiting
     @arg GRPC_NO_ERROR if the call has completed
     @arg -1 an unexpected error happened
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wait_any(diet_reqID_t* IDptr);

  /**
     Function used to return the error code of the asynchronous call identified
     by reqID

     @param reqID request ID for which we need the error code

     @return error code of the call for which we specify the request ID:
     @arg GRPC_INVALID_SESSION_ID is the id is not valid
     @arg -1 if the request id is not registered
     @arg GRPC_* corresponding error code

     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_get_error(diet_reqID_t reqID);

  /**
     Function used to return a string corresponding to the input error code

     @param error error code

     @return a string explaining the input error code

     @see the DIET_grpc.h file for more information about error values.
  */
  char *
  diet_error_string(diet_error_t error);


  /**
     Function used to return the identifier of the failed session.
     The failed session id is filled after the call

     @param reqIdPtr id of the failed session after the call.
     If no session failed, the reqIdPtr contains GRPC_SESSIONID_VOID.

     @return GRPC_NO_ERROR in every cases ...

     @todo that returned value is strange...
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_get_failed_session(diet_reqID_t* reqIdPtr);


  /****************************************************************************/
  /*    get available Services in the DIET Platform                           */
  /****************************************************************************/

  /**
     Function used to get the names of the services available on the platform.

     @remark The main problem of this function is due to collision of different
     services with same names. Use this function with caution !
     @warning The main problem of this function is due to collision of
     different services with same names. Use this function with caution !
     @todo this behaviour should be modified
     (perhaps the function should be deprecated and replaced by a function
     returning profiles and not service names)

     @param services_number after the call the value contains the number of
     services available on the platform.
     @return array of services' names
  */
  char **
  get_diet_services(int *services_number);

#ifdef HAVE_WORKFLOW
  /*****************************************/
  /* send a workflow description to ...... */
  /*****************************************/
  /**
     Function used to realize a workflow call based on the specified profile.

     @param profile workflow profile
     @return error telling whether the workflow call was successful or not:
     @arg 1 if the MaDag was not created during the initialization, something
     was wrong with the type of workflow to launch
     (it must be DIET_WF_DAG or DIET_WF_FUNCTIONAL).
     Depending on the type of workflow the following problems may had occured:
     \c if \c it's \c a \c DAG, the DAG was cancelled or and error occured
     during its execution; \c if \c it's \c a \c functional \c workflow,
     the DAG execution failed or the workflow instantiation or execution failed.
     @arg XML_MALFORMED if the workflow file was malformed
     @arg WFL_BADSTRUCT if the structure of the workflow was incorrect

     @todo the returned value corresponds to a lot of different possible
     errors ...
     @see the DIET_grpc.h file for more information about error values.
  */
  diet_error_t
  diet_wf_call(diet_wf_desc_t* profile);

  /********************************************/
  /* cancel a dag sent to the MaDag.          */
  /********************************************/

  /**
     Function used to cancel a DAG sent to the MaDag

     @param dagId Id of the DAG to cancel
     @return error value telling whether the cancellation was successful or not:
     @arg 1 if the MaDag was not created during the initialization or if it's
     an invalid Dag
     @arg 0 or GRPC_NO_ERROR it no problem appeared during the cancellation of
     the Dag

     @todo the returned value should perhaps use the ones defined in DIET_grpc.h
     @see the DIET_grpc.h file for more information about error values.

  */
  diet_error_t
  diet_wf_cancel_dag(const char* dagId);

  /*****************************************/
  /* terminate a workflow session and      */
  /*        free the memory                */
  /*****************************************/

  /**
     Function used to terminate a workflow session and free the memory
     allocated for the workflow profile.

     @param profile profile of the workflow
  */
  void
  diet_wf_free(diet_wf_desc_t * profile);

  /***************************************************************/

  /**
     Enumeration describing the workflow submission mode

     @todo some more documentation should be added to explain what each value
     means
  */
  typedef enum {
    MA_MODE, /*!< Master Agent Mode */
    MADAG_MODE, /*!< MaDag Mode */
    MULTIWF_MODE /*!< Multi Workflow Mode */
  } wf_sub_mode_t;

#endif /* HAVE_WORKFLOW */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_CLIENT_H_ */

