/****************************************************************************/
/* asynchronized calls singleton Manager                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.18  2011/03/16 14:54:06  hguemar
 * remove dead code: unused class DietException in src/client/CallAsyncMgr.{hh,cc}
 *
 * Revision 1.17  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.16  2008/10/22 14:16:37  gcharrie
 * Adding MultiCall. It is used to devide a profile and make several calls with just one SeD. Some documentation will be added soon.
 *
 * Revision 1.15  2008/03/28 13:17:16  rbolze
 * update code to avoid warning with the intel compiler
 *
 * Revision 1.14  2006/07/13 14:40:39  aamar
 * Adding the doneRequest vector for already tested requests by
 * grpc_wait_any.
 *
 * Revision 1.13  2006/07/07 09:20:03  aamar
 * Adding the public function getAddSessionIDs.
 *
 * Revision 1.12  2006/06/29 15:02:41  aamar
 * Make change to handle the new type definition of grpc_function_handle_t (from a grpc_function_handle_s to grpc_function_handle_s*
 *
 * Revision 1.11  2006/06/29 12:26:15  aamar
 * Adding the following functions to the CallAsyncMgr class:
 *    - deleteAllAsyncCall (to be able to do a diet_cancel_all).
 *    - setReqErrorCode and getReqErrorCode to set and get the error code
 *      associated to each asynchronous request.
 *    - getFailedSession
 *    - checkSessionID (test if session id is valid)
 *    - saveHandle and getHandle
 * To manage these function three data strutures were added:
 *    - map<diet_reqID_t, diet_error_t>, vector<diet_reqID_t> failedSessions
 *      and map<diet_reqID_t, grpc_function_handle_t *> handlesMap
 *
 * Revision 1.10  2006/06/03 21:12:13  ycaniou
 * Correct warning "'CallAsyncMgr::dex' contains empty classes"
 * -- tkx Injay
 *
 * Revision 1.9  2003/12/01 14:49:31  pcombes
 * Rename dietTypes.hh to DIET_data_internal.hh, for more coherency.
 *
 * Revision 1.8  2003/10/13 13:03:52  uid515
 * Replace int by int32_t for managing 32-64 bits system and CORBA::Long type.
 *
 * Revision 1.7  2003/09/22 13:10:54  cpera
 * Fix bugs and correct release function.
 *
 * Revision 1.6  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.5  2003/07/04 09:48:01  pcombes
 * enum STATE -> request_status_t, and its values are prefixed by STATUS.
 *
 * Revision 1.2  2003/06/04 14:40:05  cpera
 * Resolve bugs, change type of reqID (long int) and modify
 * diet_wait_all/diet_wait_any.
 *
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 ****************************************************************************/


#ifndef _CALLASYNCMGR_H_
#define _CALLASYNCMGR_H_

#include <map>
#include <vector>
#include <omnithread.h>
#include <sys/types.h>
#include <cassert>

#include "DIET_client.h"
#include "DIET_data_internal.hh"
#include "ReadersWriterLock.hh"


/****************************************************************************
 * A singleton class which manage asynchronnized call on SeD
 * Thread Safe using from client threads and corba callback server threads
 * Object model managing multiplex/demultiplex between client wait and corba
 * notify.
 * Use omni_conditionnal omnithread synchronized mecanism.
 ****************************************************************************/


typedef enum WAITOPERATOR{
  SOLE = 0,     // Wait rule is about a unique request
  OR,           // Wait rule is released if one of the OR element is avalaible
  AND,          // Wait rule is satisfied when all AND element are avalaible
  ANY,          // Wait rule is satisfied whatever request arrived
  ALL           // Wait rule is satisfied if all requests registered when the
  // rule is created are available
}WAITOPERATOR;

typedef enum {
  STATUS_DONE = 0, // Result is available in local memory
  STATUS_WAITING,       // End of solving on Server, result comes
  STATUS_RESOLVING,          // Request is currently solving on Server
  STATUS_CANCEL,        // Cancel is called on a reqID.
  STATUS_ERROR          // Error caught
} request_status_t;


// Don't forget a data is link to a sole reqID
struct Data{
  diet_profile_t *profile;      // Ref on profile linked to a reqID
  request_status_t st;          // Info about reqID state
  int used;                     // Rules number using this profile/ReqID
#ifdef HAVE_MULTICALL
  int nbRequests;  //saves the number of requests to wait
#endif //HAVE_MULTICALL
};

struct ruleElement{
  int32_t reqID;
  WAITOPERATOR op;
};

struct Rule {
  int length;
  ruleElement * ruleElts;
  request_status_t status;
};

// manage link between reqID and request datas
typedef std::map<int32_t ,Data *> CallAsyncList;
// manage link between reqID and waitRules about it.
typedef std::multimap<int32_t ,Rule *> RulesReqIDMap;
// Manage link between one rule and one omni semaphore
typedef std::map<Rule *, omni_semaphore *> RulesConditionMap;
// manage link between reqID and request datas
typedef std::map<int32_t ,request_status_t> ReqIDStateMap;

class CallAsyncMgr
{
public:
  // give a unique instance of CallAsyncMgr
  static CallAsyncMgr* Instance();
  // client service API
  // add into internal list a new asynchronized reference
  int addAsyncCall(diet_reqID_t reqID, diet_profile_t* dpt);
  int deleteAsyncCall(diet_reqID_t reqID);
  // used by gridRPC function cancel_all
  int deleteAllAsyncCall();
  // add a new wait rule
  int addWaitRule(Rule *);
  int addWaitAnyRule(diet_reqID_t* IDptr);
  int addWaitAllRule();
  int deleteWaitRule(Rule* rule);
  // persistence of async call ID and corba callback IOR
  // Not implemented
  int serialise();
  int areThereWaitRules();
  // corba callback server service API
  int notifyRst(diet_reqID_t reqID, corba_profile_t *dp);
  int getStatusReqID(diet_reqID_t reqID);
  int verifyRule(Rule *rule);
  // initialise all necessary corba services
  // call by the first add of asynchronized call
  int init(int argc, char* argv[]);
  // uninitialise all. end of corba servers ...
  // call when there is
  int release();

  /*
   * set the error code of a given request (session)
   */
  void
  setReqErrorCode(const diet_reqID_t reqID, const diet_error_t error);

  /*
   * get the error code of a given request (session)
   * if the request ID is not present (but valid) return -1
   */
  diet_error_t
  getReqErrorCode(const diet_reqID_t reqID);

  /*
   * return the failed session
   * successive call to this method (by get_failed_session in client API)
   * return the successives failed sessions
   */
  diet_error_t
  getFailedSession(diet_reqID_t * reqIdPtr);

  /*
   * check if the request ID is a valid
   */
  bool
  checkSessionID(const diet_reqID_t reqID);

  /*
   * Save a handle and associate it to a session ID
   */
  void
  saveHandle(diet_reqID_t sessionID,
             grpc_function_handle_t* handle);
  /*
   * get the handle associated to the provided sessionID
   */
  diet_error_t
  getHandle(grpc_function_handle_t** handle,
            diet_reqID_t sessionID);

  /*
   * get all the session IDs
   */
  diet_reqID_t*
  getAllSessionIDs(int& len);
protected:
  int deleteAsyncCallWithoutLock(diet_reqID_t reqID);
  // constructors
  CallAsyncMgr();

private:
  static CallAsyncMgr* pinstance;
  CallAsyncList caList;
  RulesReqIDMap rulesIDs;
  RulesConditionMap rulesConds;
  ReqIDStateMap iDState;
  DietReadersWriterLock callAsyncListLock;
  DietReadersWriterLock waitRulesReqIDStateLock;

  /*
   * A map to store the error code of the asynchronous requests
   */
  std::map<diet_reqID_t, diet_error_t> errorMap;
  /*
   * A vector to store the failed session IDs
   */
  std::vector<diet_reqID_t> failedSessions;
  /*
   * A map to save the function handles indexed by their session ID
   */
  std::map<diet_reqID_t, grpc_function_handle_t*> handlesMap;
  /*
   * A vector of already done requests (that the user do
   * a wait_any on)
   * (may be must be used also for requests that the user
   * do a wait on)
   */
  std::vector<diet_reqID_t> doneRequests;
};
#endif //CALLASYNCMGR
