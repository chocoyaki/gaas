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
 * Revision 1.10  2011/04/20 14:17:31  bdepardo
 * Removed a cout
 *
 * Revision 1.9  2011/01/21 17:27:31  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.8  2010/07/12 20:14:32  glemahec
 * DIET 2.5 beta 1 - Forwarders with Multi-MAs bug correction
 *
 * Revision 1.7  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.6  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.5  2006/05/12 12:12:32  sdahan
 * Add some documentation about multi-MA
 *
 * Bug fix:
 *  - segfault when the neighbours configuration line was empty
 *  - deadlock when a MA create a link on itself
 *
 * Revision 1.4  2004/12/15 15:57:08  sdahan
 * rewrite the FloodRequestsList to use a simplest implementation. The previous mutex bugs does not exist anymore.
 *
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

#include <cassert>

//#include <stdio.h>
//#define disp(act)  printf("-> %s %s\n", __ASSERT_FUNCTION, #act) ; act ; printf("-< %s %s\n", __ASSERT_FUNCTION, #act) ;
#define disp(act) act ;

void FloodRequest::addNewResponse() {
  nbOfWaitingResponse-- ;
  if(nbOfWaitingResponse == 0) {
    disp(allResponseReceived->signal()) ;
  }
}


FloodRequest::FloodRequest() {
  hop = 0 ;
  nbOfWaitingResponse = 0 ;
  allResponseReceived = new omni_condition(&locker) ;
}


FloodRequest::FloodRequest(const FloodRequest & request) {
  predecessor = request.predecessor ;
  hop = 0 ;
  nbOfWaitingResponse = 0 ;
  owner = request.owner ;
  neighbouringMAs = request.neighbouringMAs ;
  this->request = request.request ;
  decisions = request.decisions ;
  allResponseReceived = new omni_condition(&locker) ;
}


FloodRequest::FloodRequest(const MasterAgentImpl::MADescription& predecessor,
                           const MasterAgentImpl::MADescription& owner,
                           const corba_request_t& request,
                           MasterAgentImpl::MAList& knownMAs) :
  predecessor(predecessor), owner(owner), request(request) {
  hop = 0 ;
  nbOfWaitingResponse = 0 ;
  knownMAs.lock() ;
  // copy the list of the neighboring MAs but without the MA that had
  // send this request. 
  for(MasterAgentImpl::MAList::iterator iter = knownMAs.begin() ;
      iter != knownMAs.end() ; ++iter) {
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
  predecessor = request.predecessor ;
  owner = request.owner ;
  neighbouringMAs = request.neighbouringMAs ;
  this->request = request.request ;
  decisions = request.decisions ;
  return *this ; 
} // operator=(const FloodRequest&)


RequestID FloodRequest::getId(){
  return request.reqID ;
} // getId()


bool FloodRequest::flooded() {
  //printf("bool FloodRequest::flooded()\n") ;
  bool result = true ;
  if (hop >= 8)
    return true ;
  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      result && iter != neighbouringMAs.end() ; /* stop if the result
						   is found or if
						   there is no other
						   MA. */
      ++iter) {
    if (iter->second.state != NeighbouringMA::nmaStateFlooded)
      result = false ;
  }
  return result ;
} // flooded()


bool FloodRequest::floodNextStep() {
  //printf("void FloodRequest::floodNextStep()\n") ;
  KeyString ownerId = KeyString(owner->getBindName()) ;
  hop = hop+1 ;
  nbOfWaitingResponse = 0 ;
  bool completelyFlooded = true ;

  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      iter != neighbouringMAs.end() ; ++iter) {
    MasterAgentImpl::MADescription& MA = iter->second.maDescription ;
    switch(iter->second.state) {
    case NeighbouringMA::nmaStateNotContacted :
      // first contact with the MA
      try {
	MA->searchService(ownerId, ownerId, request) ;
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
  return completelyFlooded ;
  //printf("--void FloodRequest::floodNextStep()\n") ;
}


void FloodRequest::waitResponses() {
  // wait a maximum of 5 seconds
  unsigned long s, n ;
  locker.lock() ;
  omni_thread::get_time(&s, &n, 5, 0) ;
  disp(allResponseReceived->timedwait(s, n)) ;
  locker.unlock() ;
}


corba_response_t FloodRequest::getDecision() {
  corba_response_t result ;
  result = decisions ;
  decisions.servers.length(0) ;
  return result ;
} // getDecision()


void FloodRequest::stopFlooding() {
  //printf("void FloodRequest::stopFlooding()\n") ;

  /* tell to all the contacted MA that the service is found and the
     flooding must stop. */
  for(NeighbouringMAs::iterator iter = neighbouringMAs.begin() ;
      iter != neighbouringMAs.end() ; ++iter) {
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

} // stopFlooding


void FloodRequest::addResponseNotFound() {
  //printf("void FloodRequest::addResponseNotFound()\n") ;
  addNewResponse() ;
}


void FloodRequest::addResponseFloodedArea(KeyString senderId) {
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId) ;
  if (senderIter != neighbouringMAs.end())
    senderIter->second.state = NeighbouringMA::nmaStateFlooded ;
  else
    WARNING("sender " << senderId << " is unknown from neighbours list") ;
  addNewResponse() ;
}


void FloodRequest::addResponseAlreadyContacted(KeyString senderId) {
  //printf("void FloodRequest::addResponseAlreadyContacted(KeyString senderId)\n");
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId) ;
  if (senderIter != neighbouringMAs.end())
    senderIter->second.state = NeighbouringMA::nmaStateAlreadyContacted ;
  else
    WARNING("sender " << senderId << " is unknown from neighbours list") ;
  addNewResponse() ;
}


void
FloodRequest::addResponseServiceFound(const corba_response_t& decision) {
  //printf("FloodRequest::addResponseServiceFound(const corba_response_t& decision)\n") ;

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
  //printf("sortie de addResponseServiceFound\n") ;
}

#endif // HAVE_MULTI_MA
