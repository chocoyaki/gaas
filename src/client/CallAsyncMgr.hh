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
#include <omnithread.h>
#include <sys/types.h>
#include <assert.h>

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


class DietException { int zazoom ; };

typedef enum WAITOPERATOR{
  SOLE = 0,     // Wait rule is about a unique request
  OR,           // Wait rule is released if one of the OR element is avalaible
  AND,          // Wait rule is satisfied when all AND element are avalaible
  ANY,          // Wait rule is satisfied whatever request arrived
  ALL           // Wait rule is satisfied if all requests registered when the 
    // rule is created are available
};

typedef enum {
  STATUS_DONE = 0, // Result is available in local memory 
  STATUS_WAITING,       // End of solving on Server, result comes 
  STATUS_RESOLVING,          // Request is currently solving on Server
  STATUS_CANCEL,	// Cancel is called on a reqID.
  STATUS_ERROR		// Error caught
} request_status_t;


// Don't forget a data is link to a sole reqID
struct Data{
  diet_profile_t *profile;	// Ref on profile linked to a reqID
  request_status_t st;	   	// Info about reqID state 
  int used;			// Rules number using this profile/ReqID
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
    DietException dex;
};
#endif //CALLASYNCMGR
