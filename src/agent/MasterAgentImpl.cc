/****************************************************************************/
/* DIET master agent implementation source code                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2004/12/02 08:21:07  sdahan
 * bug fix:
 *   - file id leak in the BindService
 *   - can search an service that do not existe and having MA name different to
 *     its binding name.
 * warning message added in FloodRequest:
 *   - instead of just ignoring eronous message, a warning is print in the log
 * ALL_PRINT_STEP messages added to show some Multi-MAs request
 *
 * Revision 1.14  2004/11/29 15:22:16  sdahan
 * update the hash algorithm.
 * the ls6 and ls7 had the same hash value. So two differents request had the same id. By updating the hash function, this should not append presently.
 *
 * Revision 1.13  2004/10/06 16:40:24  rbolze
 * implement function to return the Profiles avialable on platform when a client ask it
 *
 * Revision 1.12  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.11  2004/06/09 15:10:38  mcolin
 * add stat_flush in statistics API in order to flush write access to
 * statistic file for agent and sed which never end and can't call
 * stat_finalize
 *
 * Revision 1.10  2004/03/01 18:46:08  rbolze
 * add logservice
 *
 * Revision 1.9  2004/02/27 10:25:11  bdelfabr
 * methods for data id creation and  methods to retrieve data descriptor are added
 *
 * Revision 1.8  2003/09/28 22:06:11  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.7  2003/09/22 21:19:49  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.4  2003/07/04 09:47:59  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:01:32  pcombes
 * Replace MasterAgent_impl.cc. Apply CS. Use ChildID, NodeDescription, Parsers,
 * Schedulers and TRACE_LEVEL. Update submit.
 * Multi-MA parts are still to be updated.
 ****************************************************************************/

#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "Parsers.hh"
#include "statistics.hh"
#include "ReferenceUpdateThread.hh"
#include "FloodRequestsList.hh"
#include <iostream>
using namespace std;
#include <stdio.h>

//#define aff_val(x)
#define aff_val(x) cout << #x << " = " << x << endl;

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define MA_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "MA::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

MasterAgentImpl::MasterAgentImpl() : AgentImpl()
{
  this->reqIDCounter = 0;
  this->num_session = 0;
  this->num_data = 0;
#ifdef HAVE_MULTI_MA
  this->floodRequestsList = new FloodRequestsList() ;
#endif
} // MasterAgentImpl


MasterAgentImpl::~MasterAgentImpl()
{
#ifdef HAVE_MULTI_MA
  delete floodRequestsList ;
  //  MAList.emptyIt();
#endif // HAVE_MULTI_MA
} // MasterAgentImpl::~MasterAgentImpl()


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
MasterAgentImpl::run()
{
  int res = this->AgentImpl::run();
  
  if (res)
    return res;
 
#ifdef HAVE_MULTI_MA

  /* launch the bind service */
  unsigned int* port = 
    static_cast<unsigned int*>(Parsers::Results::
	            getParamValue(Parsers::Results::BINDSERVICEPORT)) ;
  if (port != NULL) {
    bindSrv = new BindService(this, *port) ;
    char* bindName = ms_stralloc(strlen(localHostName) + 20);
    sprintf(bindName, "%s:%d", localHostName, (int)*port) ;
    this->bindName = bindName ;
    reqIDCounter = KeyString::hash(bindName) ;
  } else {
    reqIDCounter = KeyString::hash(localHostName) ;
  }
  reqIDCounter = ((reqIDCounter & 0xFFFFF) ^ ((reqIDCounter >> 12) & 0xFFF))
    * 1000 ;

  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ...\n");

  /* get the list of neighbours */
  char* neighbours = static_cast<char*>(Parsers::Results::
			   getParamValue(Parsers::Results::NEIGHBOURS)) ;
  neighbours = ms_strdup(neighbours) ;
  char* comma, *begin_copy ;
  begin_copy = neighbours ;
  while((comma = strchr(neighbours, ',')) != NULL) {
    comma[0] = '\0' ;
    MAIds.insert(CORBA::string_dup(neighbours)) ;
    neighbours = comma + 1 ;
  }
  MAIds.insert(CORBA::string_dup(neighbours)) ;
  free(begin_copy) ;

  /* initialize some variables */
  unsigned int* conf =
    static_cast<unsigned int*>(Parsers::Results::
	      	     getParamValue(Parsers::Results::MINNEIGHBOURS)) ;
  if (conf == NULL)
    minMAlinks = 2;
  else
    minMAlinks = *conf ;
  conf =
    static_cast<unsigned int*>(Parsers::Results::
	      	     getParamValue(Parsers::Results::MAXNEIGHBOURS)) ;
  if (conf == NULL)
    maxMAlinks = 10;
  else
    maxMAlinks = *conf ;

  conf = static_cast<unsigned int*>(Parsers::Results::
	      	     getParamValue(Parsers::Results::UPDATELINKPERIOD)) ;
  if (conf == NULL)
    new ReferenceUpdateThread(this, 300) ;
  else
    new ReferenceUpdateThread(this, *conf) ;
  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ... done.\n");
#endif // HAVE_MULTI_MA
  /* num_session thread safe*/
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nMaster Agent " << this->myName << " started.");
  return 0;
} // run(char* configFileName)



/**
 * Returns the identifier of a data by agreggation of numsession and numdata
 */
char * 
MasterAgentImpl::get_data_id()
{
  char* id = new char[100];
  (this->num_data)++;
  sprintf(id,"id.%s.%d.%d",myName,(int)(num_session), (int)(num_data));
  return CORBA::string_dup(id);
} // get_data_id()
 
/****************************************************************************/
/* Available Service                                                        */ 
/****************************************************************************/

/**
 * Returns the list of Profile available
 */ 
SeqCorbaProfileDesc_t*
 MasterAgentImpl::getProfiles(CORBA::Long& length)
{	
//        cout << "ask for list of services" << endl;
	return this->AgentImpl::SrvT->getProfiles(length);
}


/****************************************************************************/
/* Submission                                                               */
/****************************************************************************/

/**
 * Invoke Loc Manager method to get data presence information (call by client)
 */
CORBA::ULong 
MasterAgentImpl::dataLookUp(const char* argID){
  if(locMgr->dataLookUp(strdup(argID))==0)
    return 0;
  else
    return 1;
} // dataLookUp(const char* argID)

/**
 * invoke loc Manager method to get data descriptor of the data identified by argID 
 */
corba_data_desc_t* 
MasterAgentImpl::get_data_arg(const char* argID)
{
  corba_data_desc_t* resp = new corba_data_desc_t;
   resp = locMgr->set_data_arg(argID);  
   return resp;
}

/** Problem submission : remotely called by client. */
corba_response_t*
MasterAgentImpl::submit(const corba_pb_desc_t& pb_profile,
			CORBA::ULong maxServers)
{
  corba_request_t   creq;
  corba_response_t* decision(NULL);

  MA_TRACE_FUNCTION(pb_profile.path <<", " << maxServers);

  /* Initialize statistics module */
  stat_init();
  stat_in(this->myName,"start request");

  try {
    /* Initialize the corba request structure */
    creq.reqID = reqIDCounter++; // thread safe
    creq.pb = pb_profile;
    creq.max_srv = maxServers ;

#if HAVE_LOGSERVICE
    if (dietLogComponent!=NULL) {
      dietLogComponent->logAskForSeD(&creq);
    }
#endif

    decision = submit_local(creq) ;

#ifdef HAVE_MULTI_MA
    if (decision->servers.length() == 0) {
      FloodRequest& floodRequest =
	*(new FloodRequest(MADescription(), 
			   MADescription(_this(), myName),
			   creq, knownMAs)) ;

      bool requestAdded =
	floodRequestsList->add(floodRequest) ;
      assert(requestAdded) ;

      while((decision->servers.length() == 0) && (!floodRequest.flooded())) {  
	TRACE_TEXT(TRACE_ALL_STEPS, "multi-MAs search "
		   << creq.pb.path << " request (" << creq.reqID << ")\n") ;
	floodRequest.floodNextStep() ;
	*decision = floodRequest.getDecision() ;
      }

      stopFlooding(creq.reqID, NULL) ;
      floodRequest.releaseAccess() ;
    }
#endif // HAVE_MULTI_MA
  } catch(...) {
    WARNING("An exception was catched\n") ;
  }

#if HAVE_LOGSERVICE
  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&creq, decision);
  }
#endif

  
  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "**************************************************\n");
  stat_out(this->myName,"stop request");
  stat_flush();

  return decision;
}


/** Problem submission. Looking for SeDs that can resolve the
    problem in the local domain. */
corba_response_t*
MasterAgentImpl::submit_local(const corba_request_t& creq)
{
  corba_response_t* resp(NULL);

  Request*          req(NULL);

  /* Initialize the request with a global scheduler */
  TRACE_TEXT(TRACE_ALL_STEPS, "Initialize the request " << creq.reqID << ".\n");	    
  req = new Request(&creq, GlobalScheduler::chooseGlobalScheduler(&creq));

  /* Forward request and schedule the responses */

  resp = findServer(req, creq.max_srv);

  resp->myID = (ChildID) -1;
  // Constructor initializes sequences with length == 0
  if ((resp != NULL) && (resp->servers.length() != 0)) {
    resp->servers.length(MIN(resp->servers.length(),
			     static_cast<size_t>(creq.max_srv)));
    TRACE_TEXT(TRACE_ALL_STEPS, "Decision signaled.\n");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
	       "No server found for problem " << creq.pb.path << ".\n");
  }
  
  reqList[creq.reqID] = NULL;
  delete req ;

  return resp;
} // submit_local(const corba_request_t& req, ...)


CORBA::Long 
MasterAgentImpl::get_session_num()
{
 (this->num_session)++;
  return num_session; 

}//get_session_num()

/**
 invoked by client : frees persistent data identified by argID, if not exists return NULL
*/
CORBA::Long
MasterAgentImpl::diet_free_pdata(const char* argID)
{
  if(this->dataLookUp(ms_strdup(argID)) == 0) {
    locMgr->rm_pdata(ms_strdup(argID));
    return 1;
  }
  else 
    return 0;
} //diet_free_pdata(const char* argID)


#ifdef HAVE_MULTI_MA
char* MasterAgentImpl::getBindName() {
  return CORBA::string_dup(bindName) ;
}


/* Update MAs references */
void
MasterAgentImpl::updateRefs()
{
  cout << "updateRefs()\n" ;

  MAIds.lock() ;
  MasterAgent_var ma ;

  int loopCpt = 0 ;
  
  for(StrList::iterator iter = MAIds.begin() ;
      iter != MAIds.end() ; iter++) {
    if(loopCpt < maxMAlinks) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Resolving " << *iter << "...");
      ma = bindSrv->lookup(*iter) ;
      if(CORBA::is_nil(ma)) {
	TRACE_TEXT(TRACE_ALL_STEPS, "not found\n") ;
      } else {
	TRACE_TEXT(TRACE_ALL_STEPS, "found\n") ;
	try {
	  bool result = ma->handShake(_this(), bindName) ;
	  if (result) {
	    TRACE_TEXT(TRACE_ALL_STEPS, "connection accepted\n") ;
	    knownMAs[*iter] = MADescription(ma, ma->getHostname()) ;
	    loopCpt++ ;
	  } else {
	    TRACE_TEXT(TRACE_ALL_STEPS, "connection refused\n") ;
	    knownMAs.erase(*iter) ;
	  }
	} catch(CORBA::SystemException& ex) {
	  TRACE_TEXT(TRACE_ALL_STEPS, "obsolete reference\n") ;
	  knownMAs.erase(*iter) ;
	}
      }
    } else {
      knownMAs.erase(*iter) ;
    }
  }
  MAIds.unlock() ;

  cout << "--updateRefs()\n" ;

} // updateRefs()




/****************************************************************************/
/* MAs handshake                                                            */
/****************************************************************************/

CORBA::Boolean
MasterAgentImpl::handShake(MasterAgent_ptr me, const char* myName)
{
  TRACE_TEXT(TRACE_ALL_STEPS, myName << " is shaking my hand (" << knownMAs.size() << "/" << maxMAlinks << ")\n") ;

  knownMAs.erase(myName) ;
  
  // there is to much links to accept a new one.
  if (knownMAs.size() >= static_cast<size_t>(maxMAlinks))
    return false ;

  MAIds.insert(myName) ;
  knownMAs[myName] = MADescription(me, me->getHostname()) ;
  return true ;
} // handShake(MasterAgent_ptr me, const char* myName)

/****************************************************************************/
/*                           Flooding Algorithm                             */
/****************************************************************************/

void MasterAgentImpl::searchService(MasterAgent_ptr predecessor,
				    const char* predecessorId,
				    const corba_request_t& request) {

  //printTime() ;
  //fprintf(stderr, "searchService from %s, %s:%d, %s\n", predecessorId, (const char*)request.reqId.maId, (int)request.reqId.idNumber, (const char*)myName) ;
  TRACE_TEXT(TRACE_ALL_STEPS, predecessor << " search " 
	     << request.pb.path << " request (" << request.reqID << ")\n") ;

  
  FloodRequest& floodRequest =
    *(new FloodRequest(MADescription(predecessor, predecessorId), 
		       MADescription(_this(), bindName),
		       request, knownMAs)) ;

  bool requestAdded =
    floodRequestsList->add(floodRequest) ;

  if(requestAdded) {  
    corba_response_t* decision = submit_local(request) ;

    if (decision->servers.length() == 0) {
      predecessor->serviceNotFound(request.reqID, bindName) ;
      TRACE_TEXT(TRACE_ALL_STEPS, "no server for request (" <<
		 request.reqID << ")\n") ;
    } else {
      predecessor->serviceFound(request.reqID, *decision) ;
      TRACE_TEXT(TRACE_ALL_STEPS, decision->servers.length() 
		 << " server(s) found for request (" <<
		 request.reqID << ")\n") ;
    }

    floodRequest.releaseAccess() ;
  } else {
    predecessor->alreadyContacted(request.reqID, bindName) ;  
    TRACE_TEXT(TRACE_ALL_STEPS, "already contacted for request (" << 
	       request.reqID << ")\n") ;

  }
  
  //printf("<<<<<search service from %s\n", predecessorId) ;

} // searchService(...)


void MasterAgentImpl::stopFlooding(CORBA::Long reqId,
				   const char* senderId) {
  //fprintf(stderr, "stopFlooding from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.stopFlooding() ;
    // two releaseAccess means that the floodRequest can be destroyed
    floodRequest.releaseAccess() ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
  }
}


void MasterAgentImpl::serviceNotFound(CORBA::Long reqId,
				      const char* senderId) {
  //fprintf(stderr, "serviceNotFound from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ; 
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseNotFound() ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
  }
}


void MasterAgentImpl::newFlood(CORBA::Long reqId,
			       const char* senderId){
  //fprintf(stderr, "newFlood from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
    TRACE_TEXT(TRACE_ALL_STEPS, senderId << " continue the search for "
	       << " request (" << reqId << ")\n") ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.floodNextStep() ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
    knownMAs[senderId]->alreadyContacted(reqId, bindName) ;
  }
}

void MasterAgentImpl::floodedArea(CORBA::Long reqId,
				  const char* senderId) {
  //fprintf(stderr, "floodedArea from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
  TRACE_TEXT(TRACE_ALL_STEPS, "stop the flood of " << senderId << " for "
	     << " request (" << reqId << ")\n") ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseFloodedArea(senderId) ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
  }
}


void MasterAgentImpl::alreadyContacted(CORBA::Long reqId,
				       const char* senderId) {
  //fprintf(stderr, "alreadyContacted from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
  TRACE_TEXT(TRACE_ALL_STEPS, "already contacted for request (" << reqId << ")") ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseAlreadyContacted(senderId) ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
  }
}


void MasterAgentImpl::serviceFound(CORBA::Long reqId,
				   const corba_response_t& decision) {
  //printTime() ;
  //fprintf(stderr, "%d serviceFound, %s:%d, %s\n", (int)decision.length(), (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseServiceFound(decision) ;
    floodRequest.releaseAccess() ;
  } catch (FloodRequestNotFoundException& e) {
    cerr << e << endl ;
  }
  //printf("<<<<<%d service found\n", (int)decision.length()) ;
}

#endif // HAVE_MULTI_MA
