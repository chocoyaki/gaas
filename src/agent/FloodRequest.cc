/****************************************************************************/
/* Register the state of a request which is send through the MAs graph      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2004/12/02 08:21:07  sdahan
 * bug fix:
 *   - file id leak in the BindService
 *   - can search an service that do not existe and having MA name different to
 *     its binding name.
 * warning message added in FloodRequest:
 *   - instead of just ignoring eronous message, a warning is print in the log
 * ALL_PRINT_STEP messages added to show some Multi-MAs request
 *
 * Revision 1.2  2004/10/15 08:19:56  hdail
 * Removed references to corba_response_t->sortedIndexes - no longer useful.
 *
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 ****************************************************************************/

#include "FloodRequest.hh"

#ifdef HAVE_MULTI_MA

#include <assert.h>

//#include <stdio.h>
//#define disp(act)  printf("-> %s %s\n", __ASSERT_FUNCTION, #act) ; act ; printf("-< %s %s\n", __ASSERT_FUNCTION, #act) ;
#define disp(act) act ;

void FloodRequest::getAccess() const {
  disp(accessMutex.lock()) ;
  assert (accessCpt >= -1) ;
  accessCpt++ ;
  disp(accessMutex.unlock()) ;
}

void FloodRequest::releaseAccess() const {
  disp(accessMutex.lock()) ;
  assert (accessCpt >= 0) ;
  accessCpt-- ;
  disp(accessMutex.unlock()) ;
}

void FloodRequest::addNewResponse() {
  nbOfWaitingResponse-- ;
  if(nbOfWaitingResponse == 0) {
    disp(allResponseReceived->signal()) ;
  }
}


FloodRequest::FloodRequest() : _defined(false) {
  nbOfWaitingResponse = 0 ;
  allResponseReceived = new omni_condition(&locker) ;
  accessCpt = 0 ;
}


FloodRequest::FloodRequest(const FloodRequest & request) {
  disp(request.locker.lock()) ;
  _defined = request._defined ;
  predecessor = request.predecessor ;
  nbOfWaitingResponse = 0 ;
  accessCpt = 0 ;
  owner = request.owner ;
  neighbouringMAs = request.neighbouringMAs ;
  this->request = request.request ;
  decisions = request.decisions ;
  allResponseReceived = new omni_condition(&locker) ;
  disp(request.locker.unlock()) ;
}


FloodRequest::FloodRequest(const MasterAgentImpl::MADescription& predecessor,
			   const MasterAgentImpl::MADescription& owner,
			   const corba_request_t& request,
			   MasterAgentImpl::MAList& knownMAs) :
  _defined(true), predecessor(predecessor), owner(owner),
  request(request) {
  nbOfWaitingResponse = 0 ;
  knownMAs.lock() ;
  accessCpt = 0 ;
  for(MasterAgentImpl::MAList::iterator iter = knownMAs.begin() ;
      iter != knownMAs.end() ; iter++) {
    NeighbouringMA& newNeighbouringMA = neighbouringMAs[iter->first] ;
    newNeighbouringMA.maDescription = iter->second ;
    if (predecessor.defined() && (iter->first ==
	KeyString(predecessor->getBindName())))
      newNeighbouringMA.state = NeighbouringMA::nmaStateAlreadyContacted ;
    else
      newNeighbouringMA.state = NeighbouringMA::nmaStateNotContacted ;
  }
  knownMAs.unlock() ;
  allResponseReceived = new omni_condition(&locker) ;
} // FloodRequest(...)


FloodRequest::~FloodRequest() {
  //printf("FloodRequest::~FloodRequest()\n") ;
  delete allResponseReceived ;
}


FloodRequest & FloodRequest::operator=(const FloodRequest & request) {
  disp(locker.lock()) ;
  accessCpt = 1 ; // initialise accessCpt at 0 + the operator= access
  _defined = request._defined ;
  predecessor = request.predecessor ;
  owner = request.owner ;
  neighbouringMAs = request.neighbouringMAs ;
  this->request = request.request ;
  decisions = request.decisions ;
  disp(locker.unlock()) ;
  return *this ; 
} // operator=(const FloodRequest&)


RequestID FloodRequest::getId(){
  return request.reqID ;
} // getId()


bool FloodRequest::flooded() {
  //printf("bool FloodRequest::flooded()\n") ;
  assert(defined()) ;
  bool result = true ;
  disp(locker.lock()) ;
  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      result && iter != neighbouringMAs.end() ; /* stop if the result
						    is found or if
						    there is no other
						    MA. */
      iter++) {
    if (iter->second.state != NeighbouringMA::nmaStateFlooded)
      result = false ;
  }
  disp(locker.unlock()) ;
  return result ;
} // flooded()


void FloodRequest::floodNextStep() {
  //printf("void FloodRequest::floodNextStep()\n") ;
  assert(defined()) ;
  disp(locker.lock()) ;
  KeyString ownerId = KeyString(owner->getBindName()) ;
  nbOfWaitingResponse = 0 ;
  bool completelyFlooded = true ;

  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      iter != neighbouringMAs.end() ; iter++) {
    MasterAgentImpl::MADescription& MA = iter->second.maDescription ;
    switch(iter->second.state) {
    case NeighbouringMA::nmaStateNotContacted :
      // first contact with the MA
      try {
	MA->searchService(owner.getIor(),ownerId, request) ;
	iter->second.state = NeighbouringMA::nmaStateFlooding ;
	completelyFlooded = false ;
	nbOfWaitingResponse++ ;
      } catch(CORBA::SystemException& ex) {
	iter->second.state = NeighbouringMA::nmaStateFlooded ;
      }
      break ;
    case NeighbouringMA::nmaStateFlooding :
      // the MA was already contacted, ask to continue the search
      try {
	MA->newFlood(request.reqID, ownerId) ;
	nbOfWaitingResponse++ ;
      } catch(CORBA::SystemException& ex) {
	iter->second.state = NeighbouringMA::nmaStateFlooded ;
      }
      completelyFlooded = false ;
      break ;
    case NeighbouringMA::nmaStateFlooded :
    case NeighbouringMA::nmaStateAlreadyContacted :
      // this area is already completely flooded
      break ;
    }
  }

  try {
    if (completelyFlooded) {
      if (predecessor.defined()) {
	predecessor->floodedArea(request.reqID, ownerId) ;
      }
    } else {
      // wait a maximum of 5 seconds
      unsigned long s, n ;
      omni_thread::get_time(&s, &n, 5, 0) ;
      disp(allResponseReceived->timedwait(s, n)) ;
      //disp(allResponseReceived->wait()) ;
      if (predecessor.defined()) {
	if(decisions.servers.length() != 0) {
	  predecessor->serviceFound(request.reqID, decisions) ;
	  decisions.servers.length(0) ;
	} else {
	  completelyFlooded = true ;
	  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
	      completelyFlooded && iter != neighbouringMAs.end() ; 
	      iter++) {
	    if (iter->second.state != NeighbouringMA::nmaStateFlooded)
	      completelyFlooded = false ;
	  }
	  if (completelyFlooded)
	    predecessor->floodedArea(request.reqID, ownerId) ;
	  else
	    predecessor->serviceNotFound(request.reqID, ownerId) ;
	}
      }
    }
  } catch(CORBA::SystemException& ex) {
    /* does nothing */
  }
  disp(locker.unlock()) ;
  //printf("--void FloodRequest::floodNextStep()\n") ;
} // floodNextStep()


corba_response_t FloodRequest::getDecision() {
  corba_response_t result ;
  disp(locker.lock()) ;
  result = decisions ;
  decisions.servers.length(0) ;
  disp(locker.unlock()) ;
  return result ;
} // getDecision()


void FloodRequest::stopFlooding() {
  //printf("void FloodRequest::stopFlooding()\n") ;
  assert(defined()) ;
  disp(locker.lock());

  /* tell to all the contacted MA that the service is found and the
     flooding must stop. */
  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      iter != neighbouringMAs.end() ; iter++) {
    if(iter->second.state != NeighbouringMA::nmaStateNotContacted &&
       iter->second.state != NeighbouringMA::nmaStateAlreadyContacted) {
      try {
	iter->second.maDescription->stopFlooding(request.reqID,
						 owner->getBindName()) ;
      } catch (CORBA::SystemException& ex) { 
	/* does nothing */
      }
    }
  }

  // if the floodNextStep is waiting some response, awaik it.
  if (nbOfWaitingResponse != 0) {
    nbOfWaitingResponse = 0 ;
    allResponseReceived->signal() ;
  }

  disp(locker.unlock()) ;
  _defined = false ;
} // stopFlooding


void FloodRequest::addResponseNotFound() {
  //printf("void FloodRequest::addResponseNotFound()\n") ;
  disp(locker.lock()) ;
  addNewResponse() ;
  disp(locker.unlock()) ;
}


void FloodRequest::addResponseFloodedArea(KeyString senderId) {
  //printf("void FloodRequest::addResponseFloodedArea(KeyString senderId)\n");
  disp(locker.lock()) ;
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId) ;
  if (senderIter != neighbouringMAs.end())
    senderIter->second.state = NeighbouringMA::nmaStateFlooded ;
  else
    WARNING("sender " << senderId << " is unknown from neighbours list") ;
  addNewResponse() ;
  disp(locker.unlock()) ;
}


void FloodRequest::addResponseAlreadyContacted(KeyString senderId) {
  //printf("void FloodRequest::addResponseAlreadyContacted(KeyString senderId)\n");
  disp(locker.lock()) ;
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId) ;
  if (senderIter != neighbouringMAs.end())
    senderIter->second.state = NeighbouringMA::nmaStateAlreadyContacted ;
  else
    WARNING("sender " << senderId << " is unknown from neighbours list") ;
  addNewResponse() ;
  disp(locker.unlock()) ;
}


void
FloodRequest::addResponseServiceFound(const corba_response_t& decision) {
  //printf("FloodRequest::addResponseServiceFound(const corba_response_t& decision)\n") ;
  disp(locker.lock()) ;

  CORBA::ULong decisionsLength = decisions.servers.length() ;
  CORBA::ULong newDecisionsLength =
    decisionsLength + decision.servers.length() ;
  decisions.servers.length(newDecisionsLength) ;

  for (CORBA::ULong decisionsIdx = decisionsLength ;
       decisionsIdx < newDecisionsLength ;
       decisionsIdx++) {
    decisions.servers[decisionsIdx] =
      decision.servers[decisionsIdx - decisionsLength] ;
  }

  addNewResponse() ;
  disp(locker.unlock()) ;
  //printf("sortie de addResponseServiceFound\n") ;
}

#endif // HAVE_MULTI_MA
