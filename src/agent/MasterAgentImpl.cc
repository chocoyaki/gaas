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
 * Revision 1.32  2006/12/01 10:14:07  aamar
 * Workflow support: add a test to check if ALTPREDICT is activated.
 *   Workflow support still not working when use altpredict but diet
 *   compilation will not fail when combining these two options.
 *
 * Revision 1.31  2006/11/06 15:14:53  aamar
 * Workflow support: Correct some code about reqID
 *
 * Revision 1.30  2006/11/06 12:05:47  aamar
 * Workflow support: correct the lastReqID value.
 *
 * Revision 1.29  2006/11/02 17:11:17  rbolze
 * change information send by DietLogComponent
 *
 * Revision 1.28  2006/10/24 00:08:24  aamar
 * DietLogComponent used in submit_pb_set.
 *
 * Revision 1.27  2006/10/20 08:48:59  aamar
 * Remove the submit_wf function.
 * Handle the request ID in workflow submission.
 *
 * Revision 1.26  2006/08/31 05:47:50  ycaniou
 * Last fix wasn't complete (miss one line)
 *
 * Revision 1.25  2006/08/30 15:36:08  ycaniou
 * Correction for the MasterAgent to submit a "real" copy of parallel/sequential
 *   profile in HAVE_BATCH mode
 *
 * Revision 1.24  2006/06/30 15:37:34  ycaniou
 * Code presentation, commentaries (nothing really "touched")
 *
 * Revision 1.23  2006/05/12 12:12:32  sdahan
 * Add some documentation about multi-MA
 *
 * Bug fix:
 *  - segfault when the neighbours configuration line was empty
 *  - deadlock when a MA create a link on itself
 *
 * Revision 1.22  2006/04/14 14:17:38  aamar
 * Implementing the two methods for workflow support:
 *   - submit_wf (TO REMOVE)
 *   - submit_pb_set.
 *
 * Revision 1.21  2005/09/05 16:06:56  hdail
 * Addition of client hostname and location information to submit call.
 *
 * Revision 1.20  2005/07/11 16:31:56  hdail
 * Corrected bug wherein the MA would stop completely if a client requested
 * a service that didn't exist; this bug was introduced recently, so there should
 * be no reports of it from outside users.
 *
 * Revision 1.19  2005/05/15 15:48:50  alsu
 * minor changes from estimation vector reorganization
 *
 * Revision 1.18  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.17  2004/12/16 11:16:44  sdahan
 * adds multi-mas informations into the logService
 *
 * Revision 1.16  2004/12/15 15:57:08  sdahan
 * rewrite the FloodRequestsList to use a simplest implementation. The previous mutex bugs does not exist anymore.
 *
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

#ifdef HAVE_WORKFLOW
/*
  Workflow utilities header
 */

omni_mutex reqCount_mutex ;

#endif // HAVE_WORKFLOW

//#define aff_val(x)
#define aff_val(x) cout << #x << " = " << x << endl;

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
  if (neighbours == NULL)
    neighbours = "" ;
  neighbours = ms_strdup(neighbours) ;
  char* comma, *begin_copy ;
  begin_copy = neighbours ;
  while((comma = strchr(neighbours, ',')) != NULL) {
    comma[0] = '\0' ;
    if(neighbours[0] != '\0')
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
	     "\nMaster Agent " << this->myName << " started.\n");
  fflush(stdout);
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
#if ! HAVE_ALTPREDICT
corba_response_t*
MasterAgentImpl::submit(const corba_pb_desc_t& pb_profile,
			CORBA::ULong maxServers)
#else // HAVE_ALTPREDICT
corba_response_t*
MasterAgentImpl::submit(const corba_pb_desc_t& pb_profile,
			CORBA::ULong maxServers,
                        const char *clientHostname,
                        const char *clientLocID)
#endif
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
#if HAVE_ALTPREDICT
    creq.clientHostname = CORBA::string_dup(clientHostname);
    creq.clientLocationID = CORBA::string_dup(clientLocID);
#endif // HAVE_ALTPREDICT

    if (dietLogComponent != NULL) {
      dietLogComponent->logAskForSeD(&creq);
    }

    decision = submit_local(creq) ;

#ifdef HAVE_MULTI_MA
    if (decision->servers.length() == 0) {
      FloodRequest& floodRequest =
	*(new FloodRequest(MADescription(), 
			   MADescription(_this(), myName),
			   creq, knownMAs)) ;


      while((decision->servers.length() == 0) && (!floodRequest.flooded())) {  
	TRACE_TEXT(TRACE_ALL_STEPS, "multi-MAs search "
		   << creq.pb.path << " request (" << creq.reqID << ")\n") ;
	int flooded = floodRequest.floodNextStep() ;
	if (!flooded) {
	  bool requestAdded =
	    floodRequestsList->put(floodRequest) ;
	  assert(requestAdded) ;
	  floodRequest.waitResponses() ;
	  try {
	    floodRequestsList->get(creq.reqID) ;
	    *decision = floodRequest.getDecision() ;
	    TRACE_TEXT(TRACE_ALL_STEPS, decision->servers.length() <<
		       " SeD have been found for request (" << creq.reqID << ")\n") ;
	  } catch(FloodRequestNotFoundException f) {
	    WARNING("Can not found the requested decision in multi-MA search") ;
	  }
	}
      }
      try {
	floodRequest.stopFlooding() ;
      } catch (FloodRequestNotFoundException& e) {
	WARNING(e) ;
      }
      delete &floodRequest ;
    }
#endif // HAVE_MULTI_MA
  } catch(...) {
    WARNING("An exception was caught\n") ;
  }

  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&creq, decision);
  }
  
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
  /* Check that service exists */
  ServiceTable::ServiceReference_t sref;
  srvTMutex.lock();
  sref = this->SrvT->lookupService(&(creq.pb));

  if (sref == -1) {
    srvTMutex.unlock();

    /* Initialize the response */
    resp = new corba_response_t;
    resp->reqID = creq.reqID;
    resp->servers.length(0);
#if HAVE_ALTPREDICT
    resp->dataLoc.length(0);
#endif // HAVE_ALTPREDICT

  } else {
#ifndef HAVE_BATCH
    CORBA::Long numProfiles;
    SeqCorbaProfileDesc_t *profiles = this->SrvT->getProfiles(numProfiles);
    assert(sref < numProfiles);
    const corba_profile_desc_t profile = (*profiles)[sref];
#else
    /* I have defined, for batch cases, ServiceTable::getProfile( index )
       I use it here because of efficiency. 
       Can we replace previous non batch code?
       
    TODO: we can only manipulate reference here... look if we can change
    chooseGlobalScheduler() prototype */
    corba_profile_desc_t profile = this->SrvT->getProfile( sref ) ;
    /* Copy parallel flag of the client profile (reason why not const
       anymore) */
    profile.parallel_flag = creq.pb.parallel_flag ;
#endif
    srvTMutex.unlock();
  
    req = new Request(&creq,
                    GlobalScheduler::chooseGlobalScheduler(&creq, &profile));

    /** Forward request and schedule the responses */
    resp = findServer(req, creq.max_srv);
#ifndef HAVE_BATCH
    delete profiles;
#endif
  }

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
  //  cout << "updateRefs()\n" ;

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

  logNeighbors();

  //  cout << "--updateRefs()\n" ;

} // updateRefs()




/****************************************************************************/
/* MAs handshake                                                            */
/****************************************************************************/

CORBA::Boolean
MasterAgentImpl::handShake(MasterAgent_ptr me, const char* myName)
{
  TRACE_TEXT(TRACE_ALL_STEPS, myName << " is shaking my hand (" << knownMAs.size() << "/" << maxMAlinks << ")\n") ;


  /* FIXME: There is probably a cleaner way to find if to IOR are equal */
  char* myior = ORBMgr::getIORString(_this());
  char* hisior = ORBMgr::getIORString(me);
  if (!strcmp(myior, hisior)) {
    TRACE_TEXT(TRACE_ALL_STEPS, "I refuse to handshake with myself\n") ;
    /* we need to return now, because the knownMA locker is already
       taken by the updateRefs function which call the handshake
       one. */
    /*    free(myior) ;
	  free(hisior) ;*/
    return false ;
  }
  
  /*  free(myior) ;
      free(hisior) ;*/

  knownMAs.erase(myName) ;
  
  // there is to much links to accept a new one.
  if (knownMAs.size() >= static_cast<size_t>(maxMAlinks))
    return false ;

  MAIds.insert(myName) ;
  knownMAs[myName] = MADescription(me, me->getHostname()) ;

  logNeighbors();
  return true ;
} // handShake(MasterAgent_ptr me, const char* myName)

/****************************************************************************/
/*                           Flooding Algorithm                             */
/****************************************************************************/

void MasterAgentImpl::searchService(MasterAgent_ptr predecessor,
				    const char* predecessorId,
				    const corba_request_t& request) {

  //printTime() ;
  //fprintf(stderr, ">>>>>searchService from %s, %d, %s\n", predecessorId,  (int)request.reqID, (const char*)myName) ;
  TRACE_TEXT(TRACE_ALL_STEPS, predecessorId << " search " 
	     << request.pb.path << " request (" << request.reqID << ")\n") ;

  reqIdList.lock() ;
  ReqIdList::iterator pos = reqIdList.find(request.reqID) ;
  bool found = (pos != reqIdList.end()) ;
  if (! found)
    reqIdList.insert(pos, request.reqID) ;
  reqIdList.unlock() ;

  if (found) {
    predecessor->alreadyContacted(request.reqID, bindName) ;  
    TRACE_TEXT(TRACE_ALL_STEPS, "already contacted for request (" << 
	       request.reqID << ")\n") ;
  } else {
    FloodRequest& floodRequest =
      *(new FloodRequest(MADescription(predecessor, predecessorId), 
			 MADescription(_this(), bindName),
			 request, knownMAs)) ;

    floodRequestsList->put(floodRequest) ;

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
    delete &floodRequest ;
    reqIdList.erase(reqId) ;
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
  }
}


void MasterAgentImpl::serviceNotFound(CORBA::Long reqId,
				      const char* senderId) {
  //fprintf(stderr, "serviceNotFound from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ; 
  try {
    TRACE_TEXT(TRACE_ALL_STEPS, "service not found by " << senderId
	       << " for request (" << reqId << ")\n") ;
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseNotFound() ;
    floodRequestsList->put(floodRequest) ;
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
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
    bool flooded = floodRequest.floodNextStep() ;
    floodRequestsList->put(floodRequest) ;
    if (flooded) {
      floodRequest.getPredecessor()->floodedArea(reqId, bindName) ;
    } else {
	floodRequest.waitResponses() ;
	floodRequestsList->get(reqId) ;
      try {
	corba_response_t decisions = floodRequest.getDecision() ;
	if(decisions.servers.length() != 0) {
	  floodRequest.getPredecessor()->serviceFound(reqId, decisions) ;
	  decisions.servers.length(0) ;
	} else if (floodRequest.flooded()) {
	  floodRequest.getPredecessor()->floodedArea(reqId, bindName) ;
	} else {
	  floodRequest.getPredecessor()->serviceNotFound(reqId, bindName) ;
	}
      } catch (FloodRequestNotFoundException& e) {
	WARNING(e) ;
      } catch(CORBA::SystemException& ex) {
	// does nothing
      }
      floodRequestsList->put(floodRequest) ;
    }
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
  } catch(CORBA::SystemException& ex) {
    // does nothing
  }
}

void MasterAgentImpl::floodedArea(CORBA::Long reqId,
				  const char* senderId) {
  //fprintf(stderr, "floodedArea from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName) ;
  TRACE_TEXT(TRACE_ALL_STEPS, "stop the flood of " << senderId
	     << " for request (" << reqId << ")\n") ;
  try {
    FloodRequest& floodRequest = 
      floodRequestsList->get(reqId) ;
    floodRequest.addResponseFloodedArea(senderId) ;
    floodRequestsList->put(floodRequest) ;
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
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
    floodRequestsList->put(floodRequest) ;
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
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
    floodRequestsList->put(floodRequest) ;
  } catch (FloodRequestNotFoundException& e) {
    WARNING(e) ;
  }
  //printf("<<<<<%d service found\n", (int)decision.length()) ;
}

void
MasterAgentImpl::logNeighbors() {
  char * str ;
  size_t str_len = 1 ;
  knownMAs.lock() ;
  for(MasterAgentImpl::MAList::iterator iter = knownMAs.begin() ;
      iter != knownMAs.end() ; iter++)
    str_len += strlen(iter->first) + 1 ;
  str = new char[str_len] ;
  str[0] = 0 ;
  for(MasterAgentImpl::MAList::iterator iter = knownMAs.begin() ;
      iter != knownMAs.end() ; iter++) {
    strcat(str, iter->first) ;
    strcat(str, " ") ;
  }
  knownMAs.unlock() ;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Multi-MAs neighbors " << str << "\n");

  if (dietLogComponent!=NULL) {
    dietLogComponent->logNeighbors(str);
  }
}

#endif // HAVE_MULTI_MA

#ifdef HAVE_WORKFLOW

/** 
 * Workflow submission function. *
 * called by the MA_DAG or a client to submit a set of problems *
 */
wf_response_t *
MasterAgentImpl::  submit_pb_set  (const corba_pb_desc_seq_t& seq_pb,
				   const CORBA::Long setSize,
				   const bool used) {
  struct timeval tbegin;
  struct timeval tend;
  gettimeofday(&tbegin, NULL);

  static CORBA::Long dag_id = 0;
  wf_response_t * wf_response = new wf_response_t;
  wf_response->firstReqID = reqIDCounter;
  unsigned int len = seq_pb.length();
  wf_response->wfn_seq_resp.length(0);
  corba_response_t * corba_response = NULL;
  Counter initialReqIdCounter = this->reqIDCounter;
  wf_response->complete = false;
  TRACE_TEXT (TRACE_MAIN_STEPS, 
	      "The MasterAgent receives a set of "<< len << " problems" << 
	      " for " << setSize << " nodes" <<
	      endl);
  for (unsigned int ix=0; ix<len; ix++) {
#if ! HAVE_ALTPREDICT
    corba_response = this->submit(seq_pb[ix], 1024);
#endif
    if ((corba_response == NULL) || (corba_response->servers.length() == 0)) {
      TRACE_TEXT (TRACE_MAIN_STEPS, 
		  "The problem set can't be solved (one or more services are "
		  << "missing) " << endl);
      if (!used) 
	this->reqIDCounter = initialReqIdCounter;
      return wf_response;
    }
    else {
      wf_response->wfn_seq_resp.length(ix+1);
      wf_response->wfn_seq_resp[ix].node_id = 
	CORBA::string_dup(seq_pb[ix].path);    
      wf_response->wfn_seq_resp[ix].response = *corba_response;    
    }
  }

  TRACE_TEXT (TRACE_MAIN_STEPS, 
	      "The pb set can be solved (all services available) " << endl);
  wf_response->complete = true;

  reqCount_mutex.lock();

  wf_response->dag_id = dag_id;
  // increment the dag id
  dag_id += 1; 
  // increment the reqIDCounter
  reqIDCounter = reqIDCounter + (setSize-seq_pb.length());

  wf_response->lastReqID =  wf_response->firstReqID + setSize - 1 ;
  reqCount_mutex.unlock();

  gettimeofday(&tend, NULL);
  // calculate the processing time in ms
  time_t ptime = (tend.tv_sec - tbegin.tv_sec)* 1000 +
    (tend.tv_usec - tbegin.tv_usec)/1000;

  if (dietLogComponent != NULL) {
    dietLogComponent->logDagSubmit(wf_response, 
				   ptime);
  }
  
  if (!used)
    this->reqIDCounter =  initialReqIdCounter;
  return wf_response;
}

#endif // HAVE_WORKFLOW
