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


#ifndef _DIET_CLIENT_H_
#define _DIET_CLIENT_H_

#include "DIET_grpc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



  
/****************************************************************************/
/* A DIET client can submit several request in one session.                 */
/****************************************************************************/

diet_error_t
diet_initialize(char* config_file_name, int argc, char* argv[]);
diet_error_t
diet_finalize();


/****************************************************************************/
/* Data handles                                                             */
/****************************************************************************/

typedef diet_arg_t diet_data_handle_t;

/* The grpc_malloc function is useless, since it data descriptor allocation is
   useless and set functions aim at filling the structure.                  */
/* Once the data handle is allocated, please use the set functions to fill in
   the structure. */
diet_data_handle_t*
diet_data_handle_malloc();
void*
diet_get(diet_data_handle_t* handle);
/* Free data on server, and free handle if and if only allocated by
   diet_data_handle_malloc. */
diet_error_t
diet_free(diet_data_handle_t* handle);

void
create_file();

  /** stores identifier of a eprsistent data in a file in the client directory */
void
store_id(char* argID, char* msg);

void 
create_header();

int 
diet_free_persistent_data(char *id);

/****************************************************************************/
/* Call / submission functions                                              */
/****************************************************************************/

diet_error_t
diet_call(diet_profile_t* profile);
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call(diet_profile_t* profile) ;
diet_error_t
diet_sequential_call(diet_profile_t* profile) ;
#endif
diet_error_t
diet_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
#if defined HAVE_BATCH || defined HAVE_ALT_BATCH
diet_error_t
diet_parallel_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
diet_error_t
diet_sequential_call_async(diet_profile_t* profile, diet_reqID_t* reqID);
#endif

/****************************************************************************/
/* Request control and wait functions                                       */
/****************************************************************************/

/* Return 0 if the asynchronous call has completed. */
diet_error_t
diet_probe(diet_reqID_t reqID);
/* Return 0 if any asynchronous calls in the array of reqIDs have completed.
   Then exactly one reqID is returned in IDptr.                             */
diet_error_t
diet_probe_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr);

/* Get the function handle linked to reqID */
diet_error_t
diet_get_handle(grpc_function_handle_t** handle,
		diet_reqID_t sessionID);

/* Save the specified handle and associate it to a sessionID */
void
diet_save_handle(diet_reqID_t sessionID, 
		 grpc_function_handle_t* handle);

/* Set the error code of the defined session (reqID) */
void
set_req_error(diet_reqID_t sessionID,
	      diet_error_t error);
/* This function erases all persistent data that are manipulated by the reqID
   request. Do not forget to call diet_get_data_handle on data you would like
   to save.                                                                 */
diet_error_t
diet_cancel(diet_reqID_t reqID);

diet_error_t
diet_cancel_all();

diet_error_t
diet_wait(diet_reqID_t reqID);
diet_error_t
diet_wait_and(diet_reqID_t* IDs, size_t length);
diet_error_t
diet_wait_or(diet_reqID_t* IDs, size_t length, diet_reqID_t* IDptr);
diet_error_t
diet_wait_all();
diet_error_t
diet_wait_any(diet_reqID_t* IDptr);

/* 
 * return the error code of the asynchronous call identified by reqID
 */
diet_error_t
diet_get_error(diet_reqID_t reqID);
/*
 * return the corresponding error string
 */
char *
diet_error_string(diet_error_t error);
/*
 * return identifier of the failed session
 */
diet_error_t
diet_get_failed_session(diet_reqID_t* reqIdPtr);
/*
 * check if one of the requests contained in the array id reqIdArray has 
 * completed. 
 * Return the completed request ID if exist. Otherwise return an error code
 */
diet_error_t
diet_probe_or(diet_reqID_t* reqIdArray,
	      size_t length,
	      diet_reqID_t* reqIdPtr);
/****************************************************************************/
/*    get available Services in the DIET Platform                           */
/****************************************************************************/
char **
get_diet_services(int *services_number);

#ifdef HAVE_WORKFLOW
/*****************************************/
/* send a workflow description to ...... */
/*****************************************/
diet_error_t
diet_wf_call(diet_wf_desc_t* profile);
/*****************************************/
/* for workflow scheduling               */
/*****************************************/
struct AbstractWfSched;
void set_sched (struct AbstractWfSched * sched);
/*****************************************/
/* terminate a workflow session and      */
/*        free the memory                */
/*****************************************/
void
diet_wf_free(diet_wf_desc_t * profile);
/*****************************************/
/* define if the ma_dag return a         */
/* ordering and a scheduling (b = true)  */
/* or only and ordering (b = false)      */
/*****************************************/
void
set_madag_sched(int b);
/*****************************************/
/* enable/disable the reordering         */
/* reordering enabled (b = true)         */
/* reordering disabled (b = false)       */
/*****************************************/
void
enable_reordering(diet_wf_desc_t * profile,
                  const char * name, int b);
/***************************************************************/
/* set the reordering delta (time in seconds)                  */
/* and the number of nodes to trigger the                      */
/* reordering)                                                 */
/***************************************************************/
void set_reordering_delta(diet_wf_desc_t * profile,
                          const long int nb_sec, 
			  const unsigned long int nb_node);
/***************************************************************/
void
nodeIsDone(const char * node_id, const char * dag_id);

void
nodeIsRunning(const char * node_id);

void 
nodeIsStarting(const char * node_id);

void
nodeIsWaiting(const char * node_id);
/***************************************************************/

#endif /* HAVE_WORKFLOW */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DIET_CLIENT_H_ */

