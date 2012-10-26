/**
 * @file  DIETCall.cc
 *
 * @brief   DIET call methods
 *
 * @author   Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETCall.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "statistics.hh"
#include "CallAsyncMgr.hh"
#include "ORBMgr.hh"

#include "DIET_Dagda.hh"
#include "DagdaFactory.hh"
#include "configuration.hh"

#ifdef HAVE_MULTICALL
#include "MultiCall.hh"
#include <vector>
#endif  // MULTICALL

#ifdef HAVE_CCS
#include "SpecificClientScheduler.hh"
#endif  // HAVE_CCS

/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

#ifdef HAVE_CCS
static omni_mutex SCHED_MUTEX;
#endif

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

/****************************************
* Request submission
****************************************/

diet_error_t
request_submission(MasterAgent_var &MA,
                   diet_profile_t *profile,
                   SeD_var &chosenServer,
                   estVector_t &estim,
                   unsigned long maxServers) {
  static int nb_tries(3);
  int server_OK(0), subm_count, data_OK(0);
  corba_pb_desc_t corba_pb;
  corba_response_t *response(NULL);
  char *bad_id(NULL);
  diet_reqID_t reqID;
  char statMsg[128];
  chosenServer = SeD::_nil();

  if (mrsh_pb_desc(&corba_pb, profile)) {
    ERROR_DEBUG("profile is wrongly built", 1);
  }

  /* Request submission : try nb_tries times */

  stat_in("Client", "request_submission");
  subm_count = 0;
  do {
    response = NULL;
    {
      diet_error_t ret;
      if ((ret = dagda_get_data_desc(corba_pb, MA))) {
        return ret;
      }
    }

    if (data_OK == 0) {
      /* Submit to the agent. */
      try {
        response = MA->submit(corba_pb, maxServers);
      } catch (CORBA::Exception &e) {
        CORBA::Any tmp;
        tmp <<= e;
        CORBA::TypeCode_var tc = tmp.type();
        if (response) {
          delete response;
        }
        ERROR_DEBUG("caught a CORBA exception (" << tc->name()
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
                     "The Master Agent found the following server(s):\n");
          for (size_t i = 0; i < response->servers.length(); i++) {
            TRACE_TEXT(TRACE_MAIN_STEPS,
                       "    " << response->servers[i].loc.hostName << ":"
                              << response->servers[i].loc.port << "\n");
          }
        }
        server_OK = 0;    // Use this when no contract checking
      } // end else  [if (!response || response->servers.length() == 0)]
    } // end if data ok
  } while ((response) && (response->servers.length() > 0) &&
           (server_OK == -1) && (++subm_count < nb_tries) && (data_OK == 0));

  if (data_OK == 1) {
    ERROR_DEBUG(" data with ID " << bad_id << " not inside the platform.", 1);
    delete (bad_id);
  } else {
    if (!response || response->servers.length() == 0) {
      if (response) {
        delete response;
      }
      ERROR_DEBUG("unable to find a server", GRPC_SERVER_NOT_FOUND);
    }
    if (server_OK == -1) {
      delete response;
      ERROR_DEBUG(
        "unable to find a server after " << nb_tries << " tries."
                                         <<
        "The platform might be overloaded, try again later please",
        GRPC_SERVER_NOT_FOUND);
    }

    if (server_OK >= 0) {
      std::string serverName(response->servers[server_OK].loc.SeDName);
      chosenServer =
        ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, serverName);

      /* The estimation vector of the chosen SeD is copied into the profile.
       * This is done because:
       * 1/ the SeD cannot store the estimations for all requests as many
       * won't be followed by a solve and it is not possible to know which ones
       * 2/ the estimation vector depends on the value of the parameters of the
       * profile (and not only on the description) so each request has a
       * different estimation vector.
       */
      estim = new corba_estimation_t(response->servers[server_OK].estim);

      reqID = response->reqID;

#ifdef HAVE_CCS
      if (SpecificClientScheduler::isEnabled()) {
        SCHED_MUTEX.lock();
        SpecificClientScheduler::start(chosenServer, response);
        SCHED_MUTEX.unlock();
      }
#endif  // HAVE CCS

#ifdef HAVE_MULTICALL
      MultiCall::set_response(response);
#endif  // HAVE_MULTICALL
    }
    sprintf(statMsg, "request_submission %ld", (unsigned long) reqID);
    stat_out("Client", statMsg);
    profile->dietReqID = reqID;
  }

  return 0;
} // request_submission


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
diet_call_common(MasterAgent_var &MA,
                 diet_profile_t *profile,
                 SeD_var &chosenServer,
                 estVector_t estimVect,
                 unsigned long maxServers) {
  diet_error_t res(0);
  int solve_res(1);
  corba_profile_t corba_profile;
  char statMsg[128];
  corba_estimation_t emptyEstimVect;
  stat_in("Client", "diet_call");

  // Retry loop
  unsigned long nbRetry = 0;  // by default we do not retry
  CONFIG_ULONG(diet::NBRETRY, nbRetry);
  for (unsigned int i = 0; i <= nbRetry && solve_res; ++i) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Trying to call the service for the "
               << i + 1 << " time\n");
    SeD_var cs = chosenServer;

    if (CORBA::is_nil(cs)) {
      if (!(res =
            request_submission(MA, profile, cs, estimVect,
                               maxServers))) {
        corba_profile.estim = *estimVect;  // copy estimation vector
        delete estimVect;  // vector was allocated in request_submission
      } else { // error in request_submission
        return res;
      }
      if (CORBA::is_nil(cs)) {
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
    cs->updateTimeSinceLastSolve();

    dagda_mrsh_profile(&corba_profile, profile, MA);

    /* Computation */
    sprintf(statMsg, "computation %ld", (unsigned long) profile->dietReqID);

    stat_in("Client", statMsg);
    TRACE_TEXT(TRACE_MAIN_STEPS, "Calling the ref Corba of the SeD\n");
    /* CORBA CALL to SED */
    solve_res = cs->solve(profile->pb_name, corba_profile);
  }  // end for retry

  stat_out("Client", statMsg);

  dagda_download_SeD_data(profile, &corba_profile);

  sprintf(statMsg, "diet_call %ld", (unsigned long) profile->dietReqID);
  stat_out("Client", statMsg);
  return solve_res;
} // diet_call_common


/****************************************
* Asynchronous call
****************************************/

/**
 * This function unifies DIET and GridRPC APIs.
 * It is designed to be called from diet_call_async, grpc_call-async
 * and grpc_call_argstack_async.
 */
diet_error_t
diet_call_async_common(MasterAgent_var &MA,
                       diet_profile_t *profile,
                       SeD_var &chosenServer,
                       estVector_t estimVect,
                       unsigned long maxServers,
                       const char *refCallbackServer) {
  corba_profile_t corba_profile;
  corba_estimation_t emptyEstimVect;
  CallAsyncMgr *caMgr;
  diet_error_t res(0);
  // get sole CallAsyncMgr singleton
  caMgr = CallAsyncMgr::Instance();

  stat_in("Client", "diet_call_async");

  try {
    if (CORBA::is_nil(chosenServer)) {
      if (!(res =
              request_submission(MA, profile, chosenServer, estimVect,
                                 maxServers))) {
        corba_profile.estim = *estimVect;  // copy estimation vector
        delete estimVect;  // vector was allocated in request_submission
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
#endif  // HAVE_MULTICALL

    dagda_mrsh_profile(&corba_profile, profile, MA);

    // create corba client callback server...
    // TODO : modify addAsyncCall function because profile has the reqID
    if (caMgr->addAsyncCall(profile->dietReqID, profile) != 0) {
      return 1;
    }

    stat_in("Client", "computation_async");
    chosenServer->solveAsync(profile->pb_name, corba_profile,
                             refCallbackServer);

    stat_out("Client", "computation_async");

    if (unmrsh_out_args_to_profile(profile, &corba_profile)) {
      INTERNAL_ERROR("returned profile is wrongly built", 1);
    }

#ifdef HAVE_MULTICALL
  }     // endif (a call must be done)
}     // end for (for each SeD)
#endif
  } catch (const CORBA::Exception &e) {
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char *p = tc->name();
    if (*p != '\0') {
      WARNING("exception caught in " << __FUNCTION__ << '(' << p << ')');
    } else {
      WARNING("exception caught in " << __FUNCTION__ << '(' << tc->id() << ')');
    }
    profile->dietReqID = -1;
    return 1;
  } catch (...) {
    WARNING("exception caught in " << __FUNCTION__);
    profile->dietReqID = -1;
    return 1;
  }

  stat_out("Client", "diet_call_async");
  return 0;
}
