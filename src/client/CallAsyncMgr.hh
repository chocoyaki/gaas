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
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 *
 ****************************************************************************/


#ifndef _CALLASYNCMGR_H_
#define _CALLASYNCMGR_H_

#include <map>
#include <omnithread.h>
#include <sys/types.h>
#include <assert.h>
#include "ReadersWritersLock.hh"
#include "dietTypes.hh"

/****************************************************************************
 * A singleton class which manage asynchronnized call on SeD                
 * Thread Safe using from client threads and corba callback server threads  
 * Object model managing multiplex/demultiplex between client wait and corba 
 * notify.
 * Use omni_conditionnal omnithread synchronized mecanism.
 ****************************************************************************/


class DietException {};

typedef enum WAITOPERATOR{
  SOLE = 0,     // Wait rule is about a unique request
  OR,           // Wait rule is released if one of the OR element is avalaible
  AND,          // Wait rule is satisfied when all AND element are avalaible
  ANY,          // Wait rule is satisfied whatever request arrived
  ALL           // Wait rule is satisfied if all requests registered when the 
                // rule is created are available
};
  
typedef enum STATE{
  RESOLVING = 0,        // Request is currently solving on Server
  WAITING,              // End of solving on Server, result comes 
  DONE,                 // Result is available in local memory on client
  CANCEL,		// Cancel is called on a reqID.
  ERROR			// Error catched
};


// Don't forget a data is link to a sole reqID
struct Data{
  diet_profile_t *profile;	// Ref on profile linked to a reqID
  STATE st;		   	// Info about reqID state 
  int used;			// Rules number using this profile/ReqID
};

struct ruleElement{
  int reqID;
  WAITOPERATOR op;
};

struct Rule {
  int length;
  ruleElement * ruleElts;
  STATE status;
};

// manage link between reqID and request datas
typedef std::map<int,Data *> CallAsyncList;
// manage link between reqID and waitRules about it.
typedef std::multimap<int,Rule *> RulesReqIDMap;
// Manage link between one rule and one omni semaphore 
typedef std::map<Rule *, omni_semaphore *> RulesConditionMap;
// manage link between reqID and request datas
typedef std::map<int,STATE> ReqIDStateMap;
 
class CallAsyncMgr 
{
  public:
    // give a unique instance of CallAsyncMgr 
    static CallAsyncMgr* Instance();
    // client service API
    // add into internal list a new asynchronized reference
    int addAsyncCall(int reqID, diet_profile_t* dpt);
    int deleteAsyncCall(int reqId);
    // add a new wait rule 
    int addWaitRule(Rule *);
    int deleteWaitRule(Rule* rule);
    // persistence of async call ID and corba callback IOR
    // Not implemented
    int serialise();
    int areThereWaitRules();
    // corba callback server service API
    int notifyRst(int reqID, corba_profile_t *dp);
    int getStatusReqID(int reqID);
    int verifyRule(Rule *rule);
    // initialise all necessary corba services 
    // call by the first add of asynchronized call 
    int init(int argc, char* argv[]);
    // uninitialise all. end of corba servers ...
    // call when there is 
    int release();
  protected:
    // constructors
    CallAsyncMgr();
    
  private:
    static CallAsyncMgr* pinstance;
    CallAsyncList caList;
    RulesReqIDMap rulesIDs;
    RulesConditionMap rulesConds;
    ReqIDStateMap iDState;
    ReadersWritersLock callAsyncListLock;
    ReadersWritersLock waitRulesReqIDStateLock;
    DietException dex;
};
#endif //CALLASYNCMGR
