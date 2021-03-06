/**
 * @file  FloodRequest.cc
 *
 * @brief  Register the state of a request which is send through the MAs graph
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "FloodRequest.hh"

#ifdef HAVE_MULTI_MA

#include <cassert>

// #include <stdio.h>
// #define disp(act)  printf("-> %s %s\n", __ASSERT_FUNCTION, #act); act; printf("-< %s %s\n", __ASSERT_FUNCTION, #act);
#define disp(act) act;

void
FloodRequest::addNewResponse() {
  nbOfWaitingResponse--;
  if (nbOfWaitingResponse == 0) {
    disp(allResponseReceived->signal());
  }
}


FloodRequest::FloodRequest() {
  hop = 0;
  nbOfWaitingResponse = 0;
  allResponseReceived = new omni_condition(&locker);
}


FloodRequest::FloodRequest(const FloodRequest &request) {
  predecessor = request.predecessor;
  hop = 0;
  nbOfWaitingResponse = 0;
  owner = request.owner;
  neighbouringMAs = request.neighbouringMAs;
  this->request = request.request;
  decisions = request.decisions;
  allResponseReceived = new omni_condition(&locker);
}


FloodRequest::FloodRequest(const MasterAgentImpl::MADescription &predecessor,
                           const MasterAgentImpl::MADescription &owner,
                           const corba_request_t &request,
                           MasterAgentImpl::MAList &knownMAs):
  predecessor(predecessor), owner(owner), request(request) {
  hop = 0;
  nbOfWaitingResponse = 0;
  knownMAs.lock();
  // copy the list of the neighboring MAs but without the MA that had
  // send this request.
  for (MasterAgentImpl::MAList::iterator iter = knownMAs.begin();
       iter != knownMAs.end(); ++iter) {
    NeighbouringMA &newNeighbouringMA = neighbouringMAs[iter->first];
    newNeighbouringMA.maDescription = iter->second;
    if (predecessor.defined() &&
        (iter->first == KeyString(predecessor->getBindName()))) {
      newNeighbouringMA.state = NeighbouringMA::nmaStateAlreadyContacted;
    } else {
      newNeighbouringMA.state = NeighbouringMA::nmaStateNotContacted;
    }
  }
  knownMAs.unlock();
  allResponseReceived = new omni_condition(&locker);
}


FloodRequest::~FloodRequest() {
  delete allResponseReceived;
}


FloodRequest &
FloodRequest::operator=(const FloodRequest &request) {
  predecessor = request.predecessor;
  owner = request.owner;
  neighbouringMAs = request.neighbouringMAs;
  this->request = request.request;
  decisions = request.decisions;
  return *this;
}


RequestID
FloodRequest::getId() {
  return request.reqID;
}


bool
FloodRequest::flooded() {
  bool result = true;
  if (hop >= 8) {
    return true;
  }
  /* stop if the result is found or if there is no other MA. */
  NeighbouringMAs::iterator iter = neighbouringMAs.begin();
  for (; result && iter != neighbouringMAs.end(); ++iter) {
    if (iter->second.state != NeighbouringMA::nmaStateFlooded) {
      result = false;
    }
  }
  return result;
} // flooded


bool
FloodRequest::floodNextStep() {
  KeyString ownerId = KeyString(owner->getBindName());
  hop = hop + 1;
  nbOfWaitingResponse = 0;
  bool completelyFlooded = true;

  for (NeighbouringMAs::iterator iter = neighbouringMAs.begin();
       iter != neighbouringMAs.end(); ++iter) {
    MasterAgentImpl::MADescription &MA = iter->second.maDescription;
    switch (iter->second.state) {
    case NeighbouringMA::nmaStateNotContacted:
      // first contact with the MA
      try {
        MA->searchService(ownerId, ownerId, request);
        iter->second.state = NeighbouringMA::nmaStateFlooding;
        completelyFlooded = false;
        nbOfWaitingResponse++;
      } catch (CORBA::SystemException &ex) {
        iter->second.state = NeighbouringMA::nmaStateFlooded;
      }
      break;
    case NeighbouringMA::nmaStateFlooding:
      // the MA was already contacted, ask to continue the search
      try {
        MA->newFlood(request.reqID, ownerId);
        nbOfWaitingResponse++;
      } catch (CORBA::SystemException &ex) {
        iter->second.state = NeighbouringMA::nmaStateFlooded;
      }
      completelyFlooded = false;
      break;
    case NeighbouringMA::nmaStateFlooded:
    case NeighbouringMA::nmaStateAlreadyContacted:
      // this area is already completely flooded
      break;
    } // switch
  }
  return completelyFlooded;
} // floodNextStep


void
FloodRequest::waitResponses() {
  // wait a maximum of 5 seconds
  unsigned long s, n;
  locker.lock();
  omni_thread::get_time(&s, &n, 5, 0);
  disp(allResponseReceived->timedwait(s, n));
  locker.unlock();
}


corba_response_t
FloodRequest::getDecision() {
  corba_response_t result;
  result = decisions;
  decisions.servers.length(0);
  return result;
}


void
FloodRequest::stopFlooding() {
  /* tell to all the contacted MA that the service is found and the
     flooding must stop. */
  for (NeighbouringMAs::iterator iter = neighbouringMAs.begin();
       iter != neighbouringMAs.end(); ++iter) {
    if (iter->second.state != NeighbouringMA::nmaStateNotContacted &&
        iter->second.state != NeighbouringMA::nmaStateAlreadyContacted) {
      try {
        iter->second.maDescription->stopFlooding(request.reqID,
                                                 owner->getBindName());
      } catch (CORBA::SystemException &ex) {
        /* does nothing */
      }
    }
  }

  // if the floodNextStep is waiting some response, awaik it.
  if (nbOfWaitingResponse != 0) {
    nbOfWaitingResponse = 0;
    allResponseReceived->signal();
  }
} // stopFlooding


void
FloodRequest::addResponseNotFound() {
  addNewResponse();
}


void
FloodRequest::addResponseFloodedArea(KeyString senderId) {
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId);
  if (senderIter != neighbouringMAs.end()) {
    senderIter->second.state = NeighbouringMA::nmaStateFlooded;
  } else {
    WARNING("sender " << senderId << " is unknown from neighbours list");
  }
  addNewResponse();
}


void
FloodRequest::addResponseAlreadyContacted(KeyString senderId) {
  NeighbouringMAs::iterator senderIter = neighbouringMAs.find(senderId);
  if (senderIter != neighbouringMAs.end()) {
    senderIter->second.state = NeighbouringMA::nmaStateAlreadyContacted;
  } else {
    WARNING("sender " << senderId << " is unknown from neighbours list");
  }
  addNewResponse();
}


void
FloodRequest::addResponseServiceFound(const corba_response_t &decision) {
  CORBA::ULong decisionsLength = decisions.servers.length();
  CORBA::ULong newDecisionsLength =
    decisionsLength + decision.servers.length();
  decisions.servers.length(newDecisionsLength);

  for (CORBA::ULong decisionsIdx = decisionsLength;
       decisionsIdx < newDecisionsLength;
       decisionsIdx++) {
    decisions.servers[decisionsIdx] =
      decision.servers[decisionsIdx - decisionsLength];
  }

  addNewResponse();
} // addResponseServiceFound

#endif  // HAVE_MULTI_MA
