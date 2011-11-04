/**
 * @file  CallAsyncMgr.cc
 *
 * @brief   Asynchronized calls singleton Mgr
 *
 * @author   Christophe PERA (christophe.pera@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "CallAsyncMgr.hh"

#include <algorithm>
#include <boost/scoped_ptr.hpp>

#include "DagdaFactory.hh"
#include "DIET_Dagda.hh"
#include "debug.hh"
#include "marshalling.hh"

// locking object
static DietReadersWriterLock rwLock;
// initialize pointer
CallAsyncMgr *CallAsyncMgr::pinstance = 0;

/***********************************************************************
 * static methode managing singleton object
 * Return static sole instance of CallAsyncMgr
 * ********************************************************************/
CallAsyncMgr *
CallAsyncMgr::Instance() {
  if (pinstance == 0) {
    WriterLockGuard r(rwLock);
    if (pinstance == 0) {
      CallAsyncMgr::pinstance = new CallAsyncMgr;  // create sole instance
    }
  }
  return CallAsyncMgr::pinstance;  // address of sole instance
}

/**********************************************************************
 * client service API
 * add into internal list a new asynchronized reference
 * Return : 0 if OK, -1 if error
 * *******************************************************************/
int
CallAsyncMgr::addAsyncCall(diet_reqID_t reqID, diet_profile_t *dpt) {
  WriterLockGuard r(callAsyncListLock);
  // NOTE : maybe we do test if there is already this reqID registered
  if (caList.find(reqID) == caList.end()) {
    Data *data = new Data;
    data->profile = dpt;
    data->st = STATUS_RESOLVING;
    data->used = 0;
#ifdef HAVE_MULTICALL
    data->nbRequests = 0;
#endif  // HAVE_MULTICALL
    caList.insert(CallAsyncList::value_type(reqID, data));
  }
#ifdef HAVE_MULTICALL
  else {
    (caList.find(reqID))->second->nbRequests++;
  }
#endif  // HAVE_MULTICALL
  return 0;
} // addAsyncCall

int
CallAsyncMgr::deleteAsyncCall(diet_reqID_t reqID) {
  if (caList.find(reqID) == caList.end()) {
    return GRPC_INVALID_SESSION_ID;
  }

  WriterLockGuard r(callAsyncListLock);
  return deleteAsyncCallWithoutLock(reqID);
}

int
CallAsyncMgr::deleteAllAsyncCall() {
  WriterLockGuard r(callAsyncListLock);
  for (CallAsyncList::iterator p = this->caList.begin();
       p != caList.end();
       ++p) {
    deleteAsyncCallWithoutLock(p->first);
  }
  return GRPC_NO_ERROR;
}

/*********************************************************************
 * Notes : don't manage diet_profile_t free memory ...
 * When a reqID is canceled, all concerned rules are treated ...
 * Client must manage data memory if some clients wait for this
 * data reqID and others (not yet arrived) .....
 * Return 0 if OK, -1 if error, n if there were n rules about this
 * reqID
 * NOTES : caller must be released omni_semaphore and rules  !!!
 * job for awaken thread.. a call to deleteWaitRules is necesary ...
 * ******************************************************************/
int
CallAsyncMgr::deleteAsyncCallWithoutLock(diet_reqID_t reqID) {
  int k = -1;
  try {
    if (caList.find(reqID) != caList.end()) {
      k = 0;
      // delete reqID
      CallAsyncList::iterator h = caList.find(reqID);
      if (h->second->used != 0) {
        // delete h->second->profile;
        // NOTE: must be done -> test if others waitRules yet use this reqID
        // wait all threads locked by it get rules about this reqID.
        RulesReqIDMap::iterator j = rulesIDs.lower_bound(reqID);
        RulesConditionMap::iterator i = rulesConds.begin();
        while ((j != rulesIDs.end()) && (j != rulesIDs.upper_bound(reqID))) {
          j->second->status = STATUS_CANCEL;
          i = rulesConds.find(j->second);
          if (i != rulesConds.end()) {
            i->second->post();
          }
          ++j;
          k++;
        }
      }
      caList.erase(reqID);
    } else {
      WARNING(__FUNCTION__ << ":There is no request ID ("
                           << reqID << " registered");
      fflush(stderr);
    }
  } catch (std::exception &ex) {
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << ex.what());
    fflush(stderr);
  }
  return k;
} // deleteAsyncCallWithoutLock

/***********************************************************************
 * Add a new wait ALL rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled.
 * ********************************************************************/
int
CallAsyncMgr::addWaitAllRule() {
  try {
    Rule *rule = new Rule;
    {
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
      int size = caList.size();
      // Create ruleElements table ...
      ruleElement *simpleWait = new ruleElement[size];
      for (int k = 0; k < size; k++) {
        simpleWait[k].reqID = h->first;
        simpleWait[k].op = WAITOPERATOR(ALL);
        ++h;
      }
      rule->length = size;
      rule->ruleElts = simpleWait;
    }

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
  } catch (const std::exception &e) {
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return -1;
  }
} // addWaitAllRule

/***********************************************************************
 * Add a new wait ALL rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled.
 * ********************************************************************/
int
CallAsyncMgr::addWaitAnyRule(diet_reqID_t *IDptr) {
  try {
    boost::scoped_ptr<Rule> rule(new Rule);
    { // managing Reader lock
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
      int size = caList.size();
      // Create ruleElements table ...
      int doneReqCount = doneRequests.size();
      ruleElement *simpleWait = new ruleElement[size - doneReqCount];
      int ix = 0;
      for (int k = 0; k < size; k++) {
        if (find(doneRequests.begin(),
                 doneRequests.end(),
                 h->first) == doneRequests.end()) {
          simpleWait[ix].reqID = h->first;
          simpleWait[ix++].op = WAITOPERATOR(ANY);
        }
        ++h;
      }
      rule->length = size - doneReqCount;
      rule->ruleElts = simpleWait;
      rule->status = STATUS_RESOLVING;
    }

    // get lock on condition/waitRule
    switch (CallAsyncMgr::Instance()->addWaitRule(rule.get())) {
    case STATUS_DONE:
    {
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
      for (unsigned int k = 0; k < caList.size(); k++) {
        TRACE_TEXT(TRACE_ALL_STEPS, "status " << h->second->st << "\n");
        if ((h->second->st == STATUS_DONE) &&
            (find(doneRequests.begin(),
                  doneRequests.end(),
                  h->first) == doneRequests.end())) {
          TRACE_TEXT(TRACE_ALL_STEPS, "finding " << h->first << "\n");
          *IDptr = h->first;
          doneRequests.push_back(h->first);
          return STATUS_DONE;
        }
        ++h;
      }
      return STATUS_ERROR;
    }
    case STATUS_CANCEL:
      return STATUS_CANCEL;

    case STATUS_ERROR:
      return STATUS_ERROR;

    default:
      WARNING(__FUNCTION__ << "unexpected error in addWaitRule return value.");
      fflush(stderr);
      // Unexpected error, no value describing it (enum request_status_t)
      return -1;
      // NOTES: Be carefull, there may be others rules
      // using some of this reqID(AsyncCall)
      // So, carefull using diet_cancel
    } // switch
  } catch (const std::exception &e) {
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
  }
  return STATUS_ERROR;
} // addWaitAnyRule

/***********************************************************************
 * Add a new wait rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled.
 * ********************************************************************/
int
CallAsyncMgr::addWaitRule(Rule *rule) {
  omni_semaphore *condRule = NULL;
  request_status_t status = STATUS_ERROR;
  try {
    { // managing WriterLock
      WriterLockGuard r(callAsyncListLock);
      // verify wait rule validity ...
      // for instance : at one state of all reqID is about RESOLVING..
      // else, signal client must not wait ...
      bool plenty = true;
      bool tmpplenty = false;
      CallAsyncList::iterator h;
      for (int k = 0; k < rule->length; k++) {
        h = caList.find(rule->ruleElts[k].reqID);
        if (h == caList.end()) {
          WARNING(
            __FUNCTION__ << ": request ID ("
                         << rule->ruleElts[k].reqID <<
            ") is not registered.");
          fflush(stderr);
          return STATUS_ERROR;
        } else if (h->second->st == STATUS_RESOLVING) {
          plenty = false;  // one result is not yet ready, at least ...
        } else if (h->second->st == STATUS_DONE
                   && rule->ruleElts[k].op != WAITOPERATOR(ALL)
                   && rule->ruleElts[k].op != WAITOPERATOR(AND)) {
          tmpplenty = true;  // one result is finish yet
        }
        h->second->used++;  // NOTES : what to do if an exception ...
      }
      if (tmpplenty) {
        plenty = true;
      }
      if (plenty) {
        rule->status = STATUS_DONE;
        return STATUS_DONE;
      } else {
        for (int i = 0; i < rule->length; i++) {
          rulesIDs.insert(RulesReqIDMap::value_type(rule->ruleElts[i].reqID,
                                                    rule));
        }
        condRule = new omni_semaphore(0);
        rulesConds.insert(RulesConditionMap::value_type(rule, condRule));
      }
    }
    condRule->wait();
    {
      WriterLockGuard r(callAsyncListLock);
      status = rule->status;
      CallAsyncMgr::Instance()->deleteWaitRule(rule);
    }
  } catch (const std::exception &e) {
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    WriterLockGuard r(callAsyncListLock);

    RulesReqIDMap::iterator j = rulesIDs.begin();
    for (int k = 0; k < rule->length; k++) {
      while (j != rulesIDs.end()) {
        j = rulesIDs.find((rule->ruleElts[k]).reqID);
        if (j->second == rule) {
          rulesIDs.erase(j++);
        }
      }
    }
    delete condRule;
    rulesConds.erase(rule);
    delete[] rule->ruleElts;
    delete rule;
    return STATUS_ERROR;  // unexpected error
  }
  return status;  // Maybe OK, or a reqID should be deleted
  // or maybe an error in callback or Sed server ...
} // addWaitRule

/***********************************************************************
 * delete a wait rule
 * Be carefull : you must be sure there is no wait on
 * the rule and its condition before delete it ...
 **********************************************************************/
int
CallAsyncMgr::deleteWaitRule(Rule *rule) {
  try {
    // WriterLockGuard r(callAsyncListLock);
    if (rule == 0) {
      return -1;
    }

    RulesConditionMap::iterator i = rulesConds.find(rule);
    if (i != rulesConds.end()) {
      delete i->second;                                 // deleting semaphore
    }
    RulesReqIDMap::iterator j;
    // delete all elements in RulesReqIDMap about this rule/condition
    for (int k = 0; k < rule->length; k++) {
      while (
        (j = rulesIDs.find((i->first->ruleElts[k]).reqID)) != rulesIDs.end()) {
        if (j->second == rule) {
          rulesIDs.erase(j);
        }
      }
    }
    rulesConds.erase(rule);     // deleting Rule/Semaphore map elts
    delete[] rule->ruleElts;    // deleting RuleElement data table
    delete rule;                // deleting Rule
  } catch (const std::exception &e) {
    // ERREUR DE GESTION MEMOIRE. A CORRIGER ???????
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return STATUS_ERROR;
  }
  return 0;
} // deleteWaitRule

/***********************************************************************
 * persistence of async call ID and corba callback IOR
 * Not implemented
 **********************************************************************/
int
CallAsyncMgr::serialise() {
  return 0;
}

/**********************************************************************
   stence of async call ID and corba callback IOR
 * Not implemented
 * *******************************************************************/
int
CallAsyncMgr::areThereWaitRules() {
  ReaderLockGuard r(callAsyncListLock);
  return rulesConds.size();
}

/**********************************************************************
 * corba callback server service API
 *********************************************************************/
int
CallAsyncMgr::notifyRst(diet_reqID_t reqID, corba_profile_t *dp) {
  TRACE_TEXT(TRACE_ALL_STEPS, "notifyRst " << reqID << "\n");
  setReqErrorCode(reqID, GRPC_NO_ERROR);
  WriterLockGuard r(callAsyncListLock);

#ifdef HAVE_MULTICALL
  // Should just make one find, not two
  if (caList.find(reqID)->second->nbRequests != 0) {
    caList.find(reqID)->second->nbRequests--;
  } else {
#endif  // HAVE_MULTICALL

  try {
    TRACE_TEXT(TRACE_ALL_STEPS, "the service has computed the requestID="
               << reqID << " and notifies its answer" << "\n");
    fflush(stdout);
    // update diet_profile datas linked to this reqId
    CallAsyncList::iterator h = caList.find(reqID);
    if (h == caList.end()) {
      WARNING(__FUNCTION__
              << ":SeD notifies a result linked to a request ID ("
              << reqID << ") which is not registered");
      fflush(stderr);
      return -1;
    } else {   // update state of this reqID
      h->second->st = STATUS_DONE;
    }

    dagda_download_SeD_data(h->second->profile, dp);

    TRACE_TEXT(TRACE_ALL_STEPS, "Downloaded SeD data\n");

    // get rules about this reqID
    RulesReqIDMap::iterator j;
    if ((j = rulesIDs.lower_bound(reqID)) == rulesIDs.end()) {
      return 1;
    }
    RulesConditionMap::iterator i = rulesConds.begin();
    for (j = rulesIDs.lower_bound(reqID);
         j != rulesIDs.upper_bound(reqID);
         ++j) {
      bool plenty = true;
      for (int k = 0; k < j->second->length; k++) {
        h = caList.find(j->second->ruleElts[k].reqID);
        if ((h != caList.end())
            && (h->second->st != STATUS_DONE)
            && ((j->second->ruleElts[k].op == WAITOPERATOR(AND))
                || (j->second->ruleElts[k].op == WAITOPERATOR(SOLE))
                || (j->second->ruleElts[k].op == WAITOPERATOR(ALL))
                )
            ) {
          plenty = false;
        }
        /**********************************************************************
         * FIXME : rule parsing must be reimplemented ....
         * for performance and function
         * *******************************************************************/
      }

      if (plenty) {
        j->second->status = STATUS_DONE;
        i = rulesConds.find(j->second);
        if (i != rulesConds.end()) {
          i->second->post();
        } else {
        }
        // broadcast for that rule
      } else {
        // nothing. try another rule linked to this reqID
      }
    }
  } catch (const std::exception &e) {
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return -1;
  }

#ifdef HAVE_MULTICALL
}   // else (if the reqid nbRequests == 0)
#endif  // HAVE_MULTICALL

  return 0;
}

/***********************************************************************
 * Provide current STATUS of reqID. See STATUS define in
 * CallAsyncMgr.hh
 * Return : enum STATUS
 * ********************************************************************/
int
CallAsyncMgr::getStatusReqID(diet_reqID_t reqID) {
  ReaderLockGuard r(callAsyncListLock);
  CallAsyncList::iterator h = caList.find(reqID);
  if (h == caList.end()) {
    WARNING(__FUNCTION__ << ": reqID (" << reqID << ") is not registered.");
    fflush(stderr);
    return GRPC_INVALID_SESSION_ID;
  }
  int rst = h->second->st;
  return rst;
} // getStatusReqID

/**********************************************************************
 * Verify a rule, same as probe for a single reqID
 * Not implemented...
 * *******************************************************************/
int
CallAsyncMgr::verifyRule(Rule *rule) {
  ReaderLockGuard r(callAsyncListLock);
  return 0;
}

/**********************************************************************
 * initialize CallAsyncMgr
 * Not implemented
 * *******************************************************************/
int
CallAsyncMgr::init(int argc, char *argv[]) {
  return 0;
}

/**********************************************************************
 * release CallAsyncMgr datas.
 * Release datas in caList, Rules and semaphore will be released by
 * a call to deleteWaitRule. This call is performed by threads locked on
 * semaphore... deleteAsyncCall awaked it ...
 * *******************************************************************/
int
CallAsyncMgr::release() {
  WriterLockGuard r(callAsyncListLock);
  // all list/map will be clean...
  CallAsyncList::iterator h = caList.begin();
  int rst = 0, tmp_rst = 0;
  while (h != caList.end()) {
    if ((tmp_rst = deleteAsyncCallWithoutLock(h->first)) < 0) {
      rst = tmp_rst;
    }
    ++h;
  }
  return rst;
} // release

/**********************************************************************
 * Constructor
 * Private: currently do nothing
 * Not implemented
 * *******************************************************************/
CallAsyncMgr::CallAsyncMgr() {
  // ... perform necessary instance initializations
}

/**********************************************************************
* set the error code of a given request (session)
**********************************************************************/
void
CallAsyncMgr::setReqErrorCode(const diet_reqID_t reqID,
                              const diet_error_t error) {
  TRACE_TEXT(TRACE_ALL_STEPS, "debug : setReqErrorCode; reqID =  " << reqID <<
             ", error = " << error << "\n");
  errorMap[reqID] = error;
  /*
     #define GRPC_NO_ERROR 0
     #define GRPC_NOT_INITIALIZED 1
     #define GRPC_CONFIGFILE_NOT_FOUND 2
     #define GRPC_CONFIGFILE_ERROR 3
     #define GRPC_SERVER_NOT_FOUND 4
     #define GRPC_FUNCTION_NOT_FOUND 5
     #define GRPC_INVALID_FUNCTION_HANDLE 6
     #define GRPC_INVALID_SESSION_ID 7
     #define GRPC_RPC_REFUSED 8
     #define GRPC_COMMUNICATION_FAILED 9
     #define GRPC_SESSION_FAILED 10
     #define GRPC_NOT_COMPLETED 11
     #define GRPC_NONE_COMPLETED 12
     #define GRPC_OTHER_ERROR_CODE 13
     #define GRPC_UNKNOWN_ERROR_CODE 14
     #define GRPC_ALREADY_INITIALIZED 15
     #define GRPC_LAST_ERROR_CODE 16
   */

  // if error represents a failed session, save the request ID in the
  // failed session vector
  if (error != GRPC_NO_ERROR) {
    failedSessions.push_back(reqID);
  }
} // setReqErrorCode

/**********************************************************************
* get the error code of a given request (session)
* if the request ID is not present (but valid) return -1
**********************************************************************/
diet_error_t
CallAsyncMgr::getReqErrorCode(const diet_reqID_t reqID) {
  // check if the request ID is valid
  if (caList.find(reqID) == caList.end()) {
    return GRPC_INVALID_SESSION_ID;
  }
  // search in the error map, if the request ID is not registred return -1
  if (errorMap.find(reqID) != errorMap.end()) {
    return (errorMap.find(reqID)->second);
  }
  return -1;
} // getReqErrorCode

/**********************************************************************
* return the failed session
* successive call to this method (by get_failed_session in client API)
* return the successives failed sessions
**********************************************************************/
diet_error_t
CallAsyncMgr::getFailedSession(diet_reqID_t *reqIdPtr) {
  if (failedSessions.empty()) {
    *reqIdPtr = GRPC_SESSIONID_VOID;
    return GRPC_NO_ERROR;
  }
  *reqIdPtr = failedSessions[0];
  // diet_error_t err = errorMap[failedSessions[0]];
  failedSessions.erase(failedSessions.begin());
  return GRPC_NO_ERROR;
}

/*
 * check if the request ID is a valid
 */
bool
CallAsyncMgr::checkSessionID(const diet_reqID_t reqID) {
  if (caList.find(reqID) == caList.end()) {
    return false;
  }
  return true;
}

/*
 * Save a handle and associate it to a session ID
 */
void
CallAsyncMgr::saveHandle(diet_reqID_t sessionID,
                         grpc_function_handle_t *handle) {
  handlesMap[sessionID] = handle;
}

/*
 * get the handle associated to the provided sessionID
 */
diet_error_t
CallAsyncMgr::getHandle(grpc_function_handle_t **handle,
                        diet_reqID_t sessionID) {
  if (!checkSessionID(sessionID)) {
    return GRPC_INVALID_SESSION_ID;
  }

  if (handlesMap.find(sessionID) == handlesMap.end()) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Implementation is not complete\n"
               << " The sessionID " << sessionID
               << " exists in the sessions map but not in the handles one\n");
    return GRPC_OTHER_ERROR_CODE;
  }

  *handle = handlesMap[sessionID];
  return GRPC_NO_ERROR;
} // getHandle

/*
 * get all the session IDs
 * the array must be deleted by the caller
 */
diet_reqID_t *
CallAsyncMgr::getAllSessionIDs(int &len) {
  diet_reqID_t *sessions = new diet_reqID_t[caList.size()];
  len = caList.size();
  int ix = 0;
  for (CallAsyncList::iterator p = caList.begin();
       p != caList.end();
       ++p) {
    sessions[ix] = p->first;
    ix++;
  }

  return sessions;
} // getAllSessionIDs
