/****************************************************************************/
/* Asynchronized calls singleton Mgr 					    */
/*                                  					    */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.14  2004/04/22 11:27:34  rbolze
 * make change in addWaitRule function to manage the case
 *  where a server has answered before the addWaitRule is done.
 * Correct a bug of Async. call.
 * to have more details see bugzilla (bug id=3)
 *
 * Revision 1.13  2003/09/25 10:00:52  cpera
 * Fix bugs on deleteAsyncCall function and add new WARNING messages.
 *
 * Revision 1.12  2003/09/23 14:56:50  cpera
 * Delete logs and correct a loop.
 *
 * Revision 1.11  2003/09/22 13:10:54  cpera
 * Fix bugs and correct release function.
 *
 * Revision 1.10  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.9  2003/07/04 09:48:00  pcombes
 * Use new ERROR and WARNING macros => each STATE value becomes STATUS_value.
 *
 * Revision 1.5  2003/06/23 08:05:45  cpera
 * Fix race condition that cause multiple initializations of pinstance.
 *
 * Revision 1.3  2003/06/04 14:40:05  cpera
 * Resolve bugs, change type of reqID (long int) and modify
 * diet_wait_all/diet_wait_any.
 *
 * Revision 1.2  2003/06/02 08:56:56  cpera
 * Delete debug infos.
 *
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 ****************************************************************************/

#include "marshalling.hh"
#include "CallAsyncMgr.hh"
#include "debug.hh"
using namespace std;

// locking object
static DietReadersWriterLock rwLock; 
// initialize pointer
CallAsyncMgr* CallAsyncMgr::pinstance = 0;

/***********************************************************************
 * static methode managing singleton object
 * Return static sole instance of CallAsyncMgr
 * ********************************************************************/
CallAsyncMgr* CallAsyncMgr::Instance ()
{
  if (pinstance == 0)  // is it the first call?
  {  
    WriterLockGuard r(rwLock);
    if (pinstance == 0) {
      CallAsyncMgr::pinstance = new CallAsyncMgr; // create sole instance
    }
  }
  return CallAsyncMgr::pinstance; // address of sole instance
}

/**********************************************************************
 * client service API
 * add into internal list a new asynchronized reference
 * Return : 0 if OK, -1 if error
 * *******************************************************************/
int CallAsyncMgr::addAsyncCall (diet_reqID_t reqID, diet_profile_t* dpt) 
{
  WriterLockGuard r(callAsyncListLock);
  // NOTE : maybe we do test if there is already this reqID registered
  if (caList.find(reqID) == caList.end()){
    Data * data = new Data;
    data->profile = dpt;
    data->st = STATUS_RESOLVING;
    data->used = 0;
    caList.insert(CallAsyncList::value_type(reqID,data));
  }
  return 0; 
}

int CallAsyncMgr::deleteAsyncCall(diet_reqID_t reqID) 
{
  WriterLockGuard r(callAsyncListLock);
  return deleteAsyncCallWithoutLock(reqID);
}

/*********************************************************************
 * Notes : don't manage diet_profile_t free memory ...
 * When a reqID is canceled, all concerned rules are treated ...
 * Client must manage data memory if some clients wait for this
 * data reqID and others (not yet arrived ) .....
 * Return 0 if OK, -1 if error, n if there were n rules about this
 * reqID
 * NOTES : caller must be released omni_semaphore and rules  !!!
 * job for awaken thread.. a call to deleteWaitRules is necesary ...
 * ******************************************************************/
int CallAsyncMgr::deleteAsyncCallWithoutLock(diet_reqID_t reqID) 
{
  int k = -1;
  try{
    if (caList.find(reqID) != caList.end()){
      k = 0;
      // delete reqID
      CallAsyncList::iterator h = caList.find(reqID);
      if (h->second->used != 0){
        // delete h->second->profile;
        // NOTE: must be done -> test if others waitRules yet use this reqID 
        // wait all threads locked by it get rules about this reqID.
        RulesReqIDMap::iterator j = rulesIDs.lower_bound(reqID);
        RulesConditionMap::iterator i = rulesConds.begin();
        while ((j != rulesIDs.end()) && (j != rulesIDs.upper_bound(reqID)))
        {
          j->second->status=STATUS_CANCEL;
          i = rulesConds.find(j->second);
          if (i != rulesConds.end()) i->second->post();
          j++;
          k++;
        }
      }
      caList.erase(reqID);
    }
    else {
      WARNING(__FUNCTION__ << ":There is no request ID (" << reqID << " registered");
      fflush(stderr);
    }
  }
  catch(exception ex){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << ex.what());
    fflush(stderr);
  }
  return k; 
}

/***********************************************************************
 * Add a new wait ALL rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule 
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled. 
 * ********************************************************************/
int CallAsyncMgr::addWaitAllRule()
{
  try {
    Rule * rule = new Rule;
    {
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
      int size = caList.size();
      // Create ruleElements table ...
      ruleElement * simpleWait = new ruleElement[size];
      for (int k = 0; k < size; k++){
        simpleWait[k].reqID = h->first;
        simpleWait[k].op = WAITOPERATOR(ALL);
        ++h;
      }
      rule->length = size;
      rule->ruleElts = simpleWait;
    }

    // get lock on condition/waitRule
    return CallAsyncMgr::Instance()->addWaitRule(rule);
  }
  catch (const exception& e){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return -1;
  }
}

/***********************************************************************
 * Add a new wait ALL rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule 
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled. 
 * ********************************************************************/
int CallAsyncMgr::addWaitAnyRule(diet_reqID_t* IDptr)
{
  try {
    Rule * rule = new Rule;
    { //managing Reader lock
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
      int size = caList.size();
      // Create ruleElements table ...
      ruleElement * simpleWait = new ruleElement[size];
      for (int k = 0; k < size; k++){
        simpleWait[k].reqID = h->first;
        simpleWait[k].op = WAITOPERATOR(ANY);
        ++h;
      }
      rule->length = size;
      rule->ruleElts = simpleWait;
      rule->status = STATUS_RESOLVING;
    }

    // get lock on condition/waitRule
    switch (CallAsyncMgr::Instance()->addWaitRule(rule)){
      case STATUS_DONE:
        {
          ReaderLockGuard r(callAsyncListLock);
          CallAsyncList::iterator h = caList.begin();
          for (unsigned int k = 0; k < caList.size(); k++){
            if (h->second->st == STATUS_DONE){
              *IDptr = h->first;
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
        {
          WARNING(__FUNCTION__ << "unexpected error in addWaitRule return value.");
          fflush(stderr);
          return -1; // Unexcpected error, no value describing it (enum request_status_t)
          // NOTES: Be carefull, there may be others rules
          // using some of this reqID(AsyncCall)
          // So, carefull using diet_cancel
        }
    }
  }
  catch (const exception& e){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
  }
  return STATUS_ERROR;
}

/***********************************************************************
 * Add a new wait rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule 
 * Return request_status_t, -1 for an unexpected error or STATUS_CANCEL
 * if a reqID is cancelled. 
 * ********************************************************************/
int CallAsyncMgr::addWaitRule(Rule * rule)
{
  omni_semaphore * condRule = NULL;
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
      for (int k = 0; k < rule->length; k++){
        h = caList.find(rule->ruleElts[k].reqID);
        if (h == caList.end()){
          WARNING(__FUNCTION__ << ": request ID (" << rule->ruleElts[k].reqID << ") is not registered.");
          fflush(stderr);
          return STATUS_ERROR;
        }
        else if (h->second->st == STATUS_RESOLVING) {
          plenty = false; // one result is not yet ready, at least ...
        }
	else if(h->second->st == STATUS_DONE 
		&& rule->ruleElts[k].op != WAITOPERATOR(ALL) 
		&& rule->ruleElts[k].op != WAITOPERATOR(AND) ){
	tmpplenty = true; // one result is finish yet
	}
        h->second->used++; // NOTES : what to do if an exception ...	
      }
      if (tmpplenty==true){
      	plenty=true;
       }
      if (plenty == true){
        rule->status = STATUS_DONE;
        return STATUS_DONE;
      }
      else {
        for (int i = 0; i < rule->length; i++){
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
  }
  catch (const exception& e){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    WriterLockGuard r(callAsyncListLock);
    RulesReqIDMap::iterator j;
    for (int k = 0; k < rule->length; k++){
      while ( j != rulesIDs.end()){
        j = rulesIDs.find((rule->ruleElts[k]).reqID);	
        if (j->second == rule){
          rulesIDs.erase(j);
        }
      }
    }
    delete condRule;
    rulesConds.erase(rule);
    delete[] rule->ruleElts;
    delete rule;
    return STATUS_ERROR; // unexpected error
  }
  return status; // Maybe OK, or a reqID should be deleted
  // or maybe an error in callback or Sed server ...
}

/***********************************************************************
 * delete a wait rule
 * Be carefull : you must be sure there is no wait on
 * the rule and its condition before delete it ...
 **********************************************************************/
int CallAsyncMgr::deleteWaitRule(Rule* rule)
{
  try {
    //WriterLockGuard r(callAsyncListLock);
    if (rule == 0) return -1;
    RulesConditionMap::iterator i = rulesConds.find(rule);
    if (i != rulesConds.end()) delete i->second; 	// deleting semaphore
    RulesReqIDMap::iterator j;
    // delete all elements in RulesReqIDMap about this rule/condition
    for (int k = 0; k < rule->length; k++){
      while ((j = rulesIDs.find((i->first->ruleElts[k]).reqID)) != rulesIDs.end()){
        if (j->second == rule){
          rulesIDs.erase(j);
        }
      }
    }
    rulesConds.erase(rule); 	// deleting Rule/Semaphore map elts
    delete[] rule->ruleElts;	// deleting RuleElement data table
    delete rule;		// deleting Rule
  }
  // ERREUR DE GESTION MEMOIRE. A CORRIGER ???????
  catch (const exception& e){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return STATUS_ERROR;
  }
  return 0;
}

/***********************************************************************
 * persistence of async call ID and corba callback IOR
 * Not implemented
 **********************************************************************/
int CallAsyncMgr::serialise () 
{
  return 0; 
}

/**********************************************************************
  stence of async call ID and corba callback IOR
 * Not implemented
 * *******************************************************************/
int CallAsyncMgr::areThereWaitRules() 
{
  ReaderLockGuard r(callAsyncListLock);
  return rulesConds.size();
}

/**********************************************************************
 * corba callback server service API
 *********************************************************************/ 
int CallAsyncMgr::notifyRst (diet_reqID_t reqID, corba_profile_t * dp) 
{
  WriterLockGuard r(callAsyncListLock);
  try {
	cout<< "the service has computed the requestID=" << reqID << " and notify his answer\n";
	fflush(stdout);
    // update diet_profile datas linked to this reqId
    CallAsyncList::iterator h = caList.find(reqID);
    if (h == caList.end()){
      WARNING(__FUNCTION__ << ":SeD notify a result linked to a request ID (" << reqID << ") which is not registered");
      fflush(stderr);
      return -1;
    } // code de trace et debbug, a virer pour la version CVSise
    else { // update state of this reqID
      h->second->st = STATUS_DONE; 
    }
    if (unmrsh_out_args_to_profile(h->second->profile, dp)){
      INTERNAL_WARNING(__FUNCTION__ << ":unmrsh_out_args_to_profile failed");
      fflush(stderr);
      return -1;
    }
    // get rules about this reqID
    RulesReqIDMap::iterator j;
    if ((j = rulesIDs.lower_bound(reqID)) == rulesIDs.end()) return 1; 
    RulesConditionMap::iterator i = rulesConds.begin();
    for(j = rulesIDs.lower_bound(reqID);
        j != rulesIDs.upper_bound(reqID);
        j++)
    {  
      bool plenty = true;
      for (int k = 0; k < j->second->length; k++){
        h = caList.find(j->second->ruleElts[k].reqID);
        if (h == caList.end())
          continue; // FIXME: must be changed!!
        else if ((h->second->st != STATUS_DONE) && 
            ((j->second->ruleElts[k].op == WAITOPERATOR(AND)) || 
             (j->second->ruleElts[k].op == WAITOPERATOR(SOLE)) ||
             (j->second->ruleElts[k].op == WAITOPERATOR(ALL)))) 
          plenty = false;
        /**********************************************************************
         * NOTES : rule parsing must be reimplemented ....
         * for performance and function 
         * *******************************************************************/
      }
      if (plenty == true){
        j->second->status=STATUS_DONE;
        i = rulesConds.find(j->second);
        if (i != rulesConds.end()){
          i->second->post();
        }
        else {
        }
        // broadcast for that rule
      }
      else {
        // nothing. try another rule linked to this reqID
      }
    }
  }
  catch (const exception& e){
    WARNING("exception caught in " << __FUNCTION__ << " , what=" << e.what());
    fflush(stderr);
    return -1;
  }
  return 0;
}

/***********************************************************************
 * Provide current STATUS of reqID. See STATUS define in
 * CallAsyncMgr.hh
 * Return : enum STATUS
 * ********************************************************************/
int CallAsyncMgr::getStatusReqID(diet_reqID_t reqID)
{
  ReaderLockGuard r(callAsyncListLock);
  CallAsyncList::iterator h = caList.find(reqID);
  if (h == caList.end()){
    WARNING(__FUNCTION__ << ": reqID (" << reqID << ") is not registered.");
    fflush(stderr);  
    return -1;
  }
  int rst = h->second->st;
  return rst; 
}

/**********************************************************************
 * Verify a rule, same as probe for a single reqID
 * Not implemented...
 * *******************************************************************/
int CallAsyncMgr::verifyRule(Rule *rule)
{
  ReaderLockGuard r(callAsyncListLock);
  return 0;
}

/**********************************************************************
 * initialize CallAsyncMgr
 * Not implemented
 * *******************************************************************/ 
int CallAsyncMgr::init(int argc, char* argv[])
{
  return 0;
}

/**********************************************************************
 * release CallAsyncMgr datas.
 * Release datas in caList, Rules and semaphore will be released by
 * a call to deleteWaitRule. This call is performed by threads locked on
 * semaphore... deleteAsyncCall awaked it ...
 * *******************************************************************/ 
int CallAsyncMgr::release()
{
  WriterLockGuard r(callAsyncListLock);
  // all list/map will be clean...
  CallAsyncList::iterator h = caList.begin();
  int rst = 0, tmp_rst = 0;
  while(h != caList.end()){
    if ((tmp_rst = deleteAsyncCallWithoutLock(h->first)) < 0) rst = tmp_rst;
    h++;
  }
  return rst;
}

/**********************************************************************
 * Constructor
 * Private: currently do nothing
 * Not implemented
 * *******************************************************************/ 
CallAsyncMgr::CallAsyncMgr() 
{ 
  //... perform necessary instance initializations 
}
