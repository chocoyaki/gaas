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
	DIET_DEBUG(VARIABLE_OUTPUT(pinstance))	
  if (pinstance == 0)  // is it the first call?
  {  
	cout << "on va acquerir le lock" << endl;
    WriterLockGuard r(rwLock);
	cout << "on va peut etre initialiser pinstance = " << pinstance << endl;
    if (pinstance == 0) {
	CallAsyncMgr::pinstance = new CallAsyncMgr; // create sole instance
	cout << "on a initialisé pinstance : " << pinstance << endl;
    }
  }
	DIET_DEBUG(TEXT_OUTPUT(("FIN CallAsyncMgr::Instance")))
  return CallAsyncMgr::pinstance; // address of sole instance
}

/**********************************************************************
 * client service API
 * add into internal list a new asynchronized reference
 * Return : 0 if OK, -1 if error
 * *******************************************************************/
int CallAsyncMgr::addAsyncCall (diet_reqID_t reqID, diet_profile_t* dpt) 
{
	DIET_DEBUG()
  WriterLockGuard r(callAsyncListLock);
  // NOTE : maybe we do test if there is already this reqID registered
  if (caList.find(reqID) == caList.end()){
	DIET_DEBUG()
    Data * data = new Data;
    data->profile = dpt;
    data->st = STATUS_RESOLVING;
    data->used = 0;
    caList.insert(CallAsyncList::value_type(reqID,data));
	DIET_DEBUG()
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return 0; 
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
 int CallAsyncMgr::deleteAsyncCall(diet_reqID_t reqID) 
{
	DIET_DEBUG()
  WriterLockGuard r(callAsyncListLock);
  int k = -1;
  try{
    if (caList.find(reqID) != caList.end()){
	DIET_DEBUG()
      k = 0;
      // delete reqID
      CallAsyncList::iterator h = caList.find(reqID);
	DIET_DEBUG()
      if (h->second->used != 0){
	// delete h->second->profile;
	// NOTE: must be done -> test if others waitRules yet use this reqID 
	// wait all threads locked by it get rules about this reqID.
	RulesReqIDMap::iterator j = rulesIDs.lower_bound(reqID);
	RulesConditionMap::iterator i = rulesConds.begin();
	DIET_DEBUG()
	while ((j != rulesIDs.end()) && (j != rulesIDs.upper_bound(reqID)))
	{
	  j->second->status=STATUS_CANCEL;
	  i = rulesConds.find(j->second);
	  if (i != rulesConds.end()) i->second->post();
	  ++j;
	  k++;
	}
      }
      caList.erase(reqID);
    }
  }
  catch(exception ex){
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
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
	DIET_DEBUG()
  try {
    Rule * rule = new Rule;
    {
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
	DIET_DEBUG()
      int size = caList.size();
      // Create ruleElements table ...
      ruleElement * simpleWait = new ruleElement[size];
	DIET_DEBUG()
      for (int k = 0; k < size; k++){
	simpleWait[k].reqID = h->first;
	simpleWait[k].op = WAITOPERATOR(ALL);
	++h;
      }
      rule->length = size;
      rule->ruleElts = simpleWait;
    }
    
    // get lock on condition/waitRule
	DIET_DEBUG(TEXT_OUTPUT(("END")))
    return CallAsyncMgr::Instance()->addWaitRule(rule);
  }
  catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "diet_wait_or async Caught exception : " << p << endl;
    else cout << "diet_wait_or async Caught exception : " << tc->id() << endl;
    fflush(stdout);
    return -1;
  }  
  catch (const exception& e){
    cout << "Unexpected exception , what=" << e.what() << endl;
    fflush(stdout);
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
	DIET_DEBUG()
  try {
    Rule * rule = new Rule;
    { //managing Reader lock
      ReaderLockGuard r(callAsyncListLock);
      CallAsyncList::iterator h = caList.begin();
	DIET_DEBUG()
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
	DIET_DEBUG(TEXT_OUTPUT(("END")))
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
      return -1; // Unexcpected error, no value describing it (enum request_status_t)
    // NOTES: Be carefull, there may be others rules
    // using some of this reqID(AsyncCall)
    // So, carefull using diet_cancel
    }
  }
  catch (const CORBA::Exception &e){
    // Process any other User exceptions. Use the .id() method to
    // record or display useful information
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char * p = tc->name();
    if (*p != '\0') cout << "diet_wait_or async Caught exception : " << p << endl;
    else cout << "diet_wait_or async Caught exception : " << tc->id() << endl;
    fflush(stdout);
  }  
  catch (const exception& e){
    cout << "Unexpected exception , what=" << e.what() << endl;
    fflush(stdout);
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
	DIET_DEBUG()
  omni_semaphore * condRule = NULL;
  request_status_t status = STATUS_ERROR;
  try {
	DIET_DEBUG()
    if (true){ // managing WriterLock
      WriterLockGuard r(callAsyncListLock);
      // verify wait rule validity ...
      // for instance : at one state of all reqID is about RESOLVING..
      // else, signal client must not wait ...
      bool plenty = true;
      CallAsyncList::iterator h;
      for (int k = 0; k < rule->length; k++){
	h = caList.find(rule->ruleElts[k].reqID);
	if (h == caList.end()) return STATUS_ERROR;
	else if (h->second->st == STATUS_RESOLVING) {
	  plenty = false; // one result is not yet ready, at least ...
	}
	h->second->used++; // NOTES : what to do if an exception ...	
      }
      if (plenty == true){
				return 0;
      }
      else {
				for (int i = 0; i < rule->length; i++){
	  			rulesIDs.insert(RulesReqIDMap::value_type(rule->ruleElts[i].reqID,
	  						rule));
					DIET_DEBUG()
				}
				condRule = new omni_semaphore(0);
				rulesConds.insert(RulesConditionMap::value_type(rule, condRule));
				DIET_DEBUG()
      }
    }	
		DIET_DEBUG()
    condRule->wait();
		DIET_DEBUG()
    status = rule->status;
    CallAsyncMgr::Instance()->deleteWaitRule(rule);
		DIET_DEBUG()
  }
  catch (const exception& e){
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
	DIET_DEBUG()
    rulesConds.erase(rule);
	DIET_DEBUG()
    delete[] rule->ruleElts;
	DIET_DEBUG()
    delete rule;
	DIET_DEBUG()
    return STATUS_ERROR; // unexpected error
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
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
	DIET_DEBUG()
  try {
    if (rule == 0) return -1;
	DIET_DEBUG()
    WriterLockGuard r(callAsyncListLock);
    RulesConditionMap::iterator i = rulesConds.find(rule);
	DIET_DEBUG()
    if (i != rulesConds.end()) delete i->second; 	// deleting semaphore
	DIET_DEBUG()
    RulesReqIDMap::iterator j;
    // delete all elements in RulesReqIDMap about this rule/condition
    for (int k = 0; k < rule->length; k++){
      while ( j != rulesIDs.end()){
	j = rulesIDs.find((i->first->ruleElts[k]).reqID);	
	if (j->second == rule){
	  rulesIDs.erase(j);
	}
      }
    }
	DIET_DEBUG()
    rulesConds.erase(rule); 	// deleting Rule/Semaphore map elts
	DIET_DEBUG()
    delete[] rule->ruleElts;	// deleting RuleElement data table
	DIET_DEBUG()
    delete rule;		// deleting Rule
	DIET_DEBUG()
  }
    // ERREUR DE GESTION MEMOIRE. A CORRIGER ???????
  catch (const exception& e){
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return 0;
}

/***********************************************************************
 * persistence of async call ID and corba callback IOR
 * Not implemented
 **********************************************************************/
int CallAsyncMgr::serialise () 
{
	DIET_DEBUG()
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return 0; 
}

/**********************************************************************
 * persistence of async call ID and corba callback IOR
 * Not implemented
 * *******************************************************************/
int CallAsyncMgr::areThereWaitRules() 
{
	DIET_DEBUG()
  ReaderLockGuard r(callAsyncListLock);
  int size = rulesConds.size();
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return size;
}

/**********************************************************************
 * corba callback server service API
 *********************************************************************/ 
int CallAsyncMgr::notifyRst (diet_reqID_t reqID, corba_profile_t * dp) 
{
  
	DIET_DEBUG()
  WriterLockGuard r(callAsyncListLock);
  try {
    // update diet_profile datas linked to this reqId
    CallAsyncList::iterator h = caList.find(reqID);
	DIET_DEBUG()
    if (h == caList.end()){
      return -1;
    } // code de trace et debbug, a virer pour la version CVSise
    else { // update state of this reqID
      h->second->st = STATUS_DONE; 
    }
	DIET_DEBUG()
    if (unmrsh_out_args_to_profile(h->second->profile, dp))
      return -1;
    // get rules about this reqID
    RulesReqIDMap::iterator j = rulesIDs.lower_bound(reqID);
    RulesConditionMap::iterator i = rulesConds.begin();
    while ((j != rulesIDs.end()) && (j != rulesIDs.upper_bound(reqID)))
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
      ++j;
    }
  }
  catch (...){
    return -1;
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return 0;
}

/***********************************************************************
 * Provide current STATUS of reqID. See STATUS define in
 * CallAsyncMgr.hh
 * Return : enum STATUS
 * ********************************************************************/
int CallAsyncMgr::getStatusReqID(diet_reqID_t reqID)
{
	DIET_DEBUG()
  ReaderLockGuard r(callAsyncListLock);
  CallAsyncList::iterator h = caList.find(reqID);
  if (h == caList.end()){
    return -1;
  }
  int rst = h->second->st;
	DIET_DEBUG(TEXT_OUTPUT(("END")))
  return rst; 
}

/**********************************************************************
 * Verify a rule, same as probe for a single reqID
 * Not implemented...
 * *******************************************************************/
int CallAsyncMgr::verifyRule(Rule *rule)
{
	DIET_DEBUG()
  ReaderLockGuard r(callAsyncListLock);
	DIET_DEBUG(TEXT_OUTPUT(("END")))
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
	DIET_DEBUG()
  WriterLockGuard r(callAsyncListLock);
  // all list/map will be clean...
  CallAsyncList::iterator h = caList.begin();
  int rst = 0, tmp_rst = 0;
  while(h != caList.end()){
    if ((tmp_rst = deleteAsyncCall(h->first)) < 0) rst = tmp_rst;
    h++;
  }
	DIET_DEBUG(TEXT_OUTPUT(("END")))
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
