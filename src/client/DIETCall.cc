/****************************************************************************/
/* DIET call methods                                                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2011/04/20 14:16:28  bdepardo
 * Fixed a bug with inout in async calls
 *
 * Revision 1.7  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.6  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.5  2010/03/03 10:31:39  bdepardo
 * Changed \n into endl
 *
 * Revision 1.4  2009/10/13 15:09:37  bisnard
 * removed Dagda exceptions catch in diet_call_common
 *
 * Revision 1.3  2009/08/17 12:36:17  bdepardo
 * Be more consistent in stat messages
 *
 * Revision 1.2  2009/07/20 12:53:13  bisnard
 * obsoleted file storing persistent data IDs on client when using DTM
 *
 * Revision 1.1  2009/07/07 08:57:21  bisnard
 * Created new file by transferring functions from DIET_client.
 * Added new parameters to functions request_submission, diet_call_common
 * and diet_call_async_common to replace global variables.
 *
 ****************************************************************************/

#include "DIETCall.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "statistics.hh"
#include "CallAsyncMgr.hh"
#include "ORBMgr.hh"

#if HAVE_DAGDA
#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"
#endif // HAVE_DAGDA

#ifdef HAVE_MULTICALL
#include "MultiCall.hh"
#include <vector>
#endif // MULTICALL

#if HAVE_JUXMEM
#include "JuxMem.hh"
#endif // HAVE_JUXMEM

#ifdef HAVE_FD
#include "fd/fd.h"
#ifndef DIET_DEFAULT_FD_QOS
/* set a default QoS for fault detector
 * 30s detection time, 60s false detection, 1 month between two errors
 */
#define DIET_DEFAULT_FD_QOS 30.0, 2678400.0, 60.0
#endif // DIET_DEFAULT_FD_QOS

/* Recovery after failure func */
static void diet_call_failure_recover(fd_handle fd) { }
#endif // HAVE_FD

#ifdef HAVE_CCS
#include "SpecificClientScheduler.hh"
#endif // HAVE_CCS

/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

#ifdef HAVE_CCS
static omni_mutex      SCHED_MUTEX;
#endif

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

/****************************************
 * Request submission
 ****************************************/

diet_error_t
request_submission(MasterAgent_var& MA,
                   diet_profile_t* profile,
                   SeD_var& chosenServer,
                   estVector_t& estim,
                   unsigned long maxServers)
{
  static int nb_tries(3);
  int server_OK(0), subm_count, data_OK(0);
  corba_pb_desc_t corba_pb;
  corba_response_t* response(NULL);
  char* bad_id(NULL);
  diet_reqID_t reqID;
  char statMsg[128];
  chosenServer = SeD::_nil();
	
  if (mrsh_pb_desc(&corba_pb, profile)) {
    ERROR("profile is wrongly built", 1);
  }
	
  /* Request submission : try nb_tries times */
	
  stat_in("Client","request_submission");
  subm_count = 0;
  do {
    response = NULL;
#if ! HAVE_JUXMEM && ! HAVE_DAGDA
    /* data property base_type and type retrieval : used for scheduler */
    int i = 0;
		
    for (i = 0, data_OK = 0 ;
         (i <= corba_pb.last_out && data_OK == 0) ;
         i++) {
      char* new_id = strdup(corba_pb.param_desc[i].id.idNumber);
			
      if(strlen(new_id) != 0) {
				/* then data is known. Check that it's still in plaform */
        corba_data_desc_t *arg_desc = new corba_data_desc_t;
        arg_desc = MA->get_data_arg(new_id);
        const char * tmp="-1";
        if( strcmp(CORBA::string_dup(arg_desc->id.idNumber),tmp) == 0 ) {
          bad_id = new_id;
          data_OK = 1;
        } else {
          const_cast<corba_data_desc_t&>(corba_pb.param_desc[i]) = *arg_desc;
        }
      }
    }
#endif // ! HAVE_JUXMEM && ! HAVE_DAGDA
		
#if HAVE_DAGDA && ! HAVE_JUXMEM
    {
      diet_error_t ret;
      if ((ret = dagda_get_data_desc(corba_pb, MA)))
        return ret;
    }
#endif // HAVE_DAGDA && ! HAVE_JUXMEM
		
    if(data_OK == 0) {
      /* Submit to the agent. */
      try {
        response = MA->submit(corba_pb, maxServers);
      } catch (CORBA::Exception& e) {
        CORBA::Any tmp;
        tmp <<= e;
        CORBA::TypeCode_var tc = tmp.type();
        if (response)
          delete response;
        ERROR("caught a CORBA exception (" << tc->name()
              << ") while submitting problem", 1);
      }
			
      /* set the req ID here before checking the errors */
      if (response != NULL) {
				reqID = response->reqID;
      }
			
      /* Check response */
      if (!response || response->servers.length() == 0) {
        WARNING("no server found for problem " << corba_pb.path);
        server_OK = -1;
				
      } else {
				
        if (TRACE_LEVEL >= TRACE_MAIN_STEPS) {
          TRACE_TEXT(TRACE_MAIN_STEPS,
                     "The Master Agent found the following server(s):" << endl);
          for (size_t i = 0; i < response->servers.length(); i++) {
            TRACE_TEXT(TRACE_MAIN_STEPS,
                       "    " << response->servers[i].loc.hostName << ":"
                       << response->servers[i].loc.port << endl);
          }
        }
        server_OK = 0;    // Use this when no contract checking
      } // end else  [if (!response || response->servers.length() == 0)]
    } // end if data ok
  } while ((response) && (response->servers.length() > 0) &&
           (server_OK == -1) && (++subm_count < nb_tries) && (data_OK == 0));
	
  if(data_OK == 1) {
    ERROR (" data with ID " <<  bad_id << " not inside the platform.", 1);
    delete (bad_id);
  } else {
		
    if (!response || response->servers.length() == 0) {
      if (response) {
        delete response;
      }
      ERROR("unable to find a server", GRPC_SERVER_NOT_FOUND);
    }
    if (server_OK == -1) {
      delete response;
      ERROR("unable to find a server after " << nb_tries << " tries."
						<< "The platform might be overloaded, try again later please", GRPC_SERVER_NOT_FOUND);
    }
		
    if (server_OK >= 0) {
      string serverName = string(response->servers[server_OK].loc.SeDName);
			chosenServer = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, serverName);

      /* The estimation vector of the chosen SeD is copied into the profile.
       * This is done because:
       * 1/ the SeD cannot store the estimations for all requests as many
       * won't be followed by a solve and it is not possible to know which ones
       * 2/ the estimation vector depends on the value of the parameters of the
       * profile (and not only on the description) so each request has a
       * different estimation vector.
       */
      estim   = new corba_estimation_t(response->servers[server_OK].estim);
			
      reqID = response->reqID;
#ifdef HAVE_FD
      fd_handle fd = fd_make_handle();
      fd_set_qos(fd, DIET_DEFAULT_FD_QOS);
      fd_set_service(fd, response->servers[server_OK].loc.hostName, 1);
      fd_observe(fd);
#endif
			
#ifdef HAVE_CCS
      if (SpecificClientScheduler::isEnabled()) {
				SCHED_MUTEX.lock();
				SpecificClientScheduler::start(chosenServer, response);
				SCHED_MUTEX.unlock();
      }
#endif // HAVE CCS
			
#ifdef HAVE_MULTICALL
      MultiCall::set_response(response);
#endif //HAVE_MULTICALL
			
    }
    sprintf(statMsg, "request_submission %ld", (unsigned long) reqID);
    stat_out("Client",statMsg);
    profile->dietReqID = reqID ;
  }
	
  return 0;
}

/****************************************
 *  JuxMem data management functions
 ****************************************/

#if HAVE_JUXMEM

/**
 * Performance measurement for bw and latency of JuxMem
 */
#define JUXMEM_LATENCY_THROUGHPUT 0

JuxMem::Wrapper * juxmem;

inline void
uploadClientDataJuxMem(diet_profile_t* profile)
{
  int i = 0;
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif
	
  for (i = 0; i <= profile->last_inout; i++) {
    /**
     * If there is no data id (for both IN or INOUT case), this a new
     * data.  Therefore, attach the user input and do a msync on
     * JuxMem of the data.  Of course do that only if data are
     * persistent! Else let CORBA move them
     */
    if (profile->parameters[i].desc.id == NULL &&
				profile->parameters[i].desc.mode == DIET_PERSISTENT) {
      profile->parameters[i].desc.id =
			juxmem->attach(profile->parameters[i].value,
										 data_sizeof(&(profile->parameters[i].desc)),
										 1, 1, EC_PROTOCOL, BASIC_SOG);
      TRACE_TEXT(TRACE_MAIN_STEPS, "A data space with ID = "
								 << profile->parameters[i].desc.id
								 << " for IN data has been attached inside JuxMem!" << endl);
      /* The local memory is flush inside JuxMem */
#if JUXMEM_LATENCY_THROUGHPUT
			gettimeofday(&t_begin, NULL);
#endif
      juxmem->msync(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
			gettimeofday(&t_end, NULL);
			timersub(&t_end, &t_begin, &t_result);
			latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
			throughput = (data_sizeof(&(profile.parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
			sprintf(statMsg, "IN/INOUT %s msync. Latency: %f, Throughput: %f\n", profile->parameters[i].desc.id, latency, throughput);
			stat_out("JuxMem", statMsg);
#endif
			if (i <= profile->last_in) {
				juxmem->detach(profile->parameters[i].value);
			}
    }
  }
}

inline void
downloadClientDataJuxMem(diet_profile_t* profile)
{
  int i = 0;
#if JUXMEM_LATENCY_THROUGHPUT
  float latency = 0;
  float throughput = 0;
  /**
   * To store time
   */
  struct timeval t_begin;
  struct timeval t_end;
  struct timeval t_result;
#endif
	
  for (i = profile->last_in + 1; i <= profile->last_out; i++) {
    /**
     * Retrieve INOUT or OUT data only if DIET_PERSISTENT_RETURN.
     * Note that for INOUT data, the value can be already initialized.
     * In this case, JuxMem will use this address to store the
     * data. If value is NULL, a memory area will allocated.
     */
    if (profile->parameters[i].desc.id != NULL &&
				profile->parameters[i].desc.mode == DIET_PERSISTENT_RETURN) {
      if (i <= profile->last_inout) {
				TRACE_TEXT(TRACE_MAIN_STEPS, "Reading IN_OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ..." << endl);
      } else {
				TRACE_TEXT(TRACE_MAIN_STEPS, "Retrieving OUT data with ID = " << profile->parameters[i].desc.id << " from JuxMem ..." << endl);
				profile->parameters[i].value = juxmem->mmap(profile->parameters[i].value, data_sizeof(&(profile->parameters[i].desc)), profile->parameters[i].desc.id, 0);
      }
#if JUXMEM_LATENCY_THROUGHPUT
			gettimeofday(&t_begin, NULL);
#endif
			juxmem->acquireRead(profile->parameters[i].value);
			juxmem->release(profile->parameters[i].value);
#if JUXMEM_LATENCY_THROUGHPUT
			gettimeofday(&t_end, NULL);
			timersub(&t_end, &t_begin, &t_result);
			latency = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
			timersub(&t_begin, &(this->t_begin), &t_result);
			time = (t_result.tv_usec + (t_result.tv_sec * 1000. * 1000)) / 1000.;
			throughput = (data_sizeof(&(profile.parameters[i].desc)) / (1024. * 1024.)) / (latency / 1000.);
			fprintf(stderr, "%f INOUT %s read. Latency: %f, Throughput: %f\n", time, profile.parameters[i].desc.id, latency, throughput);
#endif
      juxmem->detach(profile->parameters[i].value);
    }
  }
}
#endif // HAVE_JUXMEM

/****************************************************************************/
/* Public methods                                                           */
/****************************************************************************/

/****************************************
 * Synchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call, grpc_call and grpc_call_argstack.
 */
diet_error_t
diet_call_common(MasterAgent_var& MA,
                 diet_profile_t* profile,
                 SeD_var& chosenServer,
                 estVector_t estimVect,
                 unsigned long maxServers)
{
  diet_error_t res(0);
  int solve_res(0);
  corba_profile_t corba_profile;
  char statMsg[128];
  corba_estimation_t emptyEstimVect;
  stat_in("Client","diet_call");
	
  if (CORBA::is_nil(chosenServer)) {
    if (!(res = request_submission(MA, profile, chosenServer, estimVect, maxServers))) {
      corba_profile.estim = *estimVect; // copy estimation vector
      delete estimVect; // vector was allocated in request_submission
    } else { // error in request_submission
      return res;
    }
    if (CORBA::is_nil(chosenServer)) {
      return 1;
    }
  }
  /* Add estimation vector to the corba_profile */
  /* (use an empty vector in case it is not provided, eg for grpc calls) */
  else if (estimVect != NULL) {
    corba_profile.estim = *estimVect;
  } else {
    corba_profile.estim = emptyEstimVect;
  }
	
  // Server is chosen, update its timeSinceLastSolve
  chosenServer->updateTimeSinceLastSolve() ;

#if HAVE_JUXMEM
  uploadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM
	
#if HAVE_DAGDA
  dagda_mrsh_profile(&corba_profile, profile, MA);
#else
  /* Convert profile (with data) in corba_profile (with data) */
  if (mrsh_profile_to_in_args(&corba_profile, profile)) {
    ERROR("profile is wrongly built", 1);
  }
	
  int j = 0;
  bool found = false;
  while ((j <= corba_profile.last_out) && (found == false)) {
    if (diet_is_persistent(corba_profile.parameters[j])) {
			// && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
			found = true;
    }
    j++;
  }
  if(found == true){
		//     create_file();
  }
#if ! HAVE_JUXMEM
  /* data property base_type and type retrieval: used for scheduler */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    if(strlen(new_id) != 0) {
      corba_data_desc_t* arg_desc = new corba_data_desc_t;
      arg_desc = MA->get_data_arg(new_id);
      const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
    }
  }
	
  /* generate new ID for data if not already existant */
  for(int i = 0 ; i <= corba_profile.last_out ; i++) {
    if((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
       (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT) &&
       (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
		{
			char* new_id = MA->get_data_id();
			corba_profile.parameters[i].desc.id.idNumber = new_id;
		}
  }
#endif // ! HAVE_JUXMEM
#endif // HAVE_DAGDA

  /* Computation */
  sprintf(statMsg, "computation %ld", (unsigned long) profile->dietReqID);
	
  stat_in("Client",statMsg);
  TRACE_TEXT(TRACE_MAIN_STEPS, "Calling the ref Corba of the SeD" << endl);
#if HAVE_FD
  fd_set_transition_handler(diet_call_failure_recover);
#endif
  /* CORBA CALL to SED */
  solve_res = chosenServer->solve(profile->pb_name, corba_profile);
	
  stat_out("Client",statMsg);
	
#if ! HAVE_DAGDA
  /* reaffect identifier */
  for(int i = 0;i <= profile->last_out;i++) {
    if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
        (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)) {
      profile->parameters[i].desc.id = strdup(corba_profile.parameters[i].desc.id.idNumber);
    }
  }
	
  if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
    INTERNAL_ERROR("returned profile is wrongly built", 1);
  }
#else
  dagda_download_SeD_data(profile, &corba_profile);
#endif // ! HAVE_DAGDA
	
#if HAVE_JUXMEM
  downloadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM
	
  sprintf(statMsg, "diet_call %ld", (unsigned long) profile->dietReqID);
  stat_out("Client",statMsg);
  return solve_res;
}


/****************************************
 * Asynchronous call
 ****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call_async, grpc_call-async
 * and grpc_call_argstack_async.
 */
diet_error_t
diet_call_async_common(MasterAgent_var& MA,
                       diet_profile_t* profile,
                       SeD_var& chosenServer,
                       estVector_t estimVect,
                       unsigned long maxServers,
                       const char *refCallbackServer)
{
  corba_profile_t corba_profile;
  corba_estimation_t emptyEstimVect;
  CallAsyncMgr* caMgr;
  diet_error_t res(0);
  // get sole CallAsyncMgr singleton
  caMgr = CallAsyncMgr::Instance();
	
  stat_in("Client","diet_call_async");
	
  try {
		
    if (CORBA::is_nil(chosenServer)) {
      if (!(res = request_submission(MA, profile, chosenServer, estimVect, maxServers))) {
        corba_profile.estim = *estimVect; // copy estimation vector
        delete estimVect; // vector was allocated in request_submission
      } else { // error in request_submission
        caMgr->setReqErrorCode(profile->dietReqID, res);
        return res;
      }
      if (CORBA::is_nil(chosenServer)) {
        caMgr->setReqErrorCode(profile->dietReqID, GRPC_SERVER_NOT_FOUND);
        return GRPC_SERVER_NOT_FOUND;
      }
    }
    /* Add estimation vector to the corba_profile */
    /* (use an empty vector in case it is not provided, eg for grpc calls) */
    else if (estimVect != NULL) {
      corba_profile.estim = *estimVect;
    } else {
      corba_profile.estim = emptyEstimVect;
    }
		
#ifdef HAVE_MULTICALL
    int max = MultiCall::get_response()->servers.length();
    for (int counter = 0; counter < max; counter++) {
      if (MultiCall::updateCall(profile, chosenServer)) {
#endif //HAVE_MULTICALL
				
#if HAVE_JUXMEM
        uploadClientDataJuxMem(profile);
#endif
				
#if HAVE_DAGDA
        dagda_mrsh_profile(&corba_profile, profile, MA);
#else
        if (mrsh_profile_to_in_args(&corba_profile, profile)) {
          caMgr->setReqErrorCode(profile->dietReqID, GRPC_INVALID_FUNCTION_HANDLE);
          ERROR("profile is wrongly built", 1);
        }
				
        int j = 0;
        bool found = false;
        while ((j <= corba_profile.last_out) && (found == false)) {
          if (diet_is_persistent(corba_profile.parameters[j])) {
            // && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
            found = true;
          }
          j++;
        }
        if(found == true){
          //       create_file();
        }
#endif // HAVE_DAGDA
				
#if ! HAVE_JUXMEM
#if ! HAVE_DAGDA
        int i = 0;
        /* data property base_type and type retrieval : used for scheduler */
        for(i = 0;i <= corba_profile.last_out;i++) {
          char* new_id = strdup(corba_profile.parameters[i].desc.id.idNumber);
          if(strlen(new_id) != 0) {
            corba_data_desc_t *arg_desc = new corba_data_desc_t;
            arg_desc = MA->get_data_arg(new_id);
            const_cast<corba_data_desc_t&>(corba_profile.parameters[i].desc) = *arg_desc;
          }
        }
        /* generate new ID for data if not already existant */
        for(i = 0;i <= corba_profile.last_out;i++) {
          if ((corba_profile.parameters[i].desc.mode > DIET_VOLATILE ) &&
              (corba_profile.parameters[i].desc.mode < DIET_PERSISTENCE_MODE_COUNT)
              && (MA->dataLookUp(strdup(corba_profile.parameters[i].desc.id.idNumber))))
            {
              char* new_id = MA->get_data_id();
              corba_profile.parameters[i].desc.id.idNumber = new_id;
            }
        }
#endif // ! HAVE_DAGDA
#endif // ! HAVE_JUXMEM
				
        // create corba client callback server...
        // TODO : modify addAsyncCall function because profile has the reqID
        if (caMgr->addAsyncCall(profile->dietReqID, profile) != 0) {
          return 1;
        }
				
        stat_in("Client","computation_async");
        chosenServer->solveAsync(profile->pb_name, corba_profile,
                                 refCallbackServer);
				
        stat_out("Client","computation_async");
				
        if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
          INTERNAL_ERROR("returned profile is wrongly built", 1);
        }
				
#if HAVE_JUXMEM
        downloadClientDataJuxMem(profile);
#endif // HAVE_JUXMEM
				
#ifdef HAVE_MULTICALL
      } //endif (a call must be done)
    } //end for (for each SeD)
#endif
		
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') {
      WARNING("exception caught in " << __FUNCTION__ << '(' << p << ')');
    } else {
      WARNING("exception caught in " << __FUNCTION__ << '(' << tc->id() << ')');
    }
    profile->dietReqID = -1;
    return 1;
  }
  catch (...) {
    WARNING("exception caught in " << __FUNCTION__);
    profile->dietReqID = -1;
    return 1;
  }
	
  stat_out("Client","diet_call_async");
  return 0;
}


/****************************************
 * Juxmem
 ****************************************/

#if HAVE_JUXMEM
void
initJuxMem() {
  juxmem = new JuxMem::Wrapper(NULL);
}

void
terminateJuxMem() {
  delete(juxmem);
}
#endif // HAVE_JUXMEM
