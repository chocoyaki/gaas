/****************************************************************************/
/* asynchronized calls singleton Mgr 					    */
/*                                  					    */
/*  Author(s):                                                              */
/*    - Christophe PERA (christophe.pera@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $ID$
 * $Log$
 * Revision 1.2  2003/06/02 08:56:56  cpera
 * Delete debug infos.
 *
 * Revision 1.1  2003/06/02 08:09:55  cpera
 * Beta version of asynchronize DIET API.
 *
 ****************************************************************************/

#include "marshalling.hh"
#include "CallAsyncMgr.hh"

using namespace std;

// locking object
static ReadersWritersLock rwLock; 
// initialize pointer
CallAsyncMgr* CallAsyncMgr::pinstance = 0;

/***********************************************************************
 * static methode managing singleton object
 * Return static sole instance of CallAsyncMgr
 * ********************************************************************/
CallAsyncMgr* CallAsyncMgr::Instance ()
{
  WriterLock r(rwLock);
  if (pinstance == 0)  // is it the first call?
  {  
    pinstance = new CallAsyncMgr; // create sole instance
  }
  return pinstance; // address of sole instance
}

/**********************************************************************
 * client service API
 * add into internal list a new asynchronized reference
 * Return : 0 if OK, -1 if error
 * *******************************************************************/
int CallAsyncMgr::addAsyncCall (int reqID, diet_profile_t* dpt) 
{
  WriterLock r(callAsyncListLock);
  // NOTE : maybe we do test if there is already this reqID registered
  if (caList.find(reqID) == caList.end()){
    Data * data = new Data;
    data->profile = dpt;
    data->st = STATE(RESOLVING);
    data->used = 0;
    caList.insert(CallAsyncList::value_type(reqID,data));
  }
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
 int CallAsyncMgr::deleteAsyncCall(int reqID) 
{
  WriterLock r(callAsyncListLock);
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
	  j->second->status=STATE(CANCEL);
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
  return k; 
}
  
/***********************************************************************
 * Add a new wait rule
 * Create ReaderLock allowing client thread waiting on...
 * Store it in a list, map it with rule
 * Wait on semaphore
 * Release waitRule 
 * Return STATE, -1 for an unexpected error or STATE(CANCEL)
 * if a reqID is cancelled. 
 * ********************************************************************/
int CallAsyncMgr::addWaitRule(Rule * rule)
{
  omni_semaphore * condRule = NULL;
  int status = -1;
  try {
    if (true){ // managing WriterLock
      WriterLock r(callAsyncListLock);
      // verify wait rule validity ...
      // for instance : at one state of all reqID is about RESOLVING..
      // else, signal client must not wait ...
      bool plenty = true;
      CallAsyncList::iterator h;
      for (int k = 0; k < rule->length; k++){
	h = caList.find(rule->ruleElts[k].reqID);
	if (h == caList.end()) return -1;
	else if (h->second->st == STATE(RESOLVING)) {
	  plenty = false;
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
	}
	condRule = new omni_semaphore(0);
	rulesConds.insert(RulesConditionMap::value_type(rule, condRule));
      }
    }	
    condRule->wait();
    status = rule->status;
    CallAsyncMgr::Instance()->deleteWaitRule(rule);
  }
  catch (const exception& e){
    WriterLock r(callAsyncListLock);
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
    return -1; // unexpected error
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
    if (rule == 0) return -1;
    WriterLock r(callAsyncListLock);
    RulesConditionMap::iterator i = rulesConds.find(rule);
    if (i != rulesConds.end()) delete i->second; 	// deleting semaphore
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
    rulesConds.erase(rule); 	// deleting Rule/Semaphore map elts
    delete[] rule->ruleElts;	// deleting RuleElement data table
    delete rule;		// deleting Rule
  }
    // ERREUR DE GESTION MEMOIRE. A CORRIGER ???????
  catch (const exception& e){
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
 * persistence of async call ID and corba callback IOR
 * Not implemented
 * *******************************************************************/
int CallAsyncMgr::areThereWaitRules() 
{
  ReaderLock r(callAsyncListLock);
  int size = rulesConds.size();
  return size;
}

/**********************************************************************
 * corba callback server service API
 *********************************************************************/ 
int CallAsyncMgr::notifyRst (int reqID, corba_profile_t * dp) 
{
  
  WriterLock r(callAsyncListLock);
  try {
    // update diet_profile datas linked to this reqId
    CallAsyncList::iterator h = caList.find(reqID);
    if (h == caList.end()){
      return -1;
    } // code de trace et debbug, a virer pour la version CVSise
    else { // update state of this reqID
      h->second->st = STATE(DONE); 
    }
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
	if (h == caList.end())continue; // ERROR, must be changed!!
	else if ((h->second->st != STATE(DONE)) && 
		((j->second->ruleElts[k].op == WAITOPERATOR(AND)) || 
		(j->second->ruleElts[k].op == WAITOPERATOR(SOLE)) ||
		(j->second->ruleElts[k].op == WAITOPERATOR(ALL)))) 
		plenty = false;
      /**********************************************************************
       * NOTES : rule parsing must be reimplemented ....
       * for performance and function 
       * ********************************************************************/
      }
     if (plenty == true){
	j->second->status=STATE(DONE);
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
  return 0;
}

/***********************************************************************
 * Provide current STATUS of reqID. See STATUS define in
 * CallAsyncMgr.hh
 * Return : enum STATUS
 * ********************************************************************/
int CallAsyncMgr::getStatusReqID(int reqID)
{
  ReaderLock r(callAsyncListLock);
  CallAsyncList::iterator h = caList.find(reqID);
  if (h == caList.end()){
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
  ReaderLock r(callAsyncListLock);
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
  WriterLock r(callAsyncListLock);
  // all list/map will be clean...
  CallAsyncList::iterator h = caList.begin();
  int rst = 0, tmp_rst = 0;
  while(h != caList.end()){
    if ((tmp_rst = deleteAsyncCall(h->first)) < 0) rst = tmp_rst;
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
