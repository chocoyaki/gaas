// $Id$

/*
 * $Log$
 * Revision 1.2  2003/01/22 15:44:01  sdahan
 * separation of the LA and MA
 *
 * Revision 1.1  2002/12/27 15:57:39  sdahan
 * the log list become a ts_map<RequestID, Request*>
 *
 */

#include "Request.hh"

#include <assert.h>

#include <stdio.h>

void Request::freeResponses() {
  if (responses != NULL) {
    delete [] responses ;
    responses = NULL ;
    responsesSize = 0 ;
  }
}

Request::Request(const corba_request_t* request) {
  this->request = request ;
  gatheringEnded = new omni_condition(&respMutex) ;
  responses = NULL ;
  responsesSize = 0 ;
} // Request(...)


Request::~Request() {
  freeResponses() ;
  delete gatheringEnded ;
} // ~Request()


void Request::lock() {
  respMutex.lock() ;
} // lock()


void Request::unlock() {
  respMutex.unlock() ;
} // unlock()

void Request::waitResponses(int numberOfResponses) {
  assert(numberOfResponses > 0) ;
  freeResponses() ;
  responses = new corba_response_t[numberOfResponses] ;
  responsesSize = numberOfResponses ;
  nbWaitedSonsResponse = numberOfResponses ;
  gatheringEnded->wait() ;
} // waitResponses(int numverOfResponses)


void Request::addResponse(const corba_response_t* response) {
  assert(nbWaitedSonsResponse > 0) ;
  responses[--nbWaitedSonsResponse] = *response ;
  if(nbWaitedSonsResponse == 0)
    gatheringEnded->signal() ;
} // addResponse(const corba_response_t* response)


const corba_response_t* Request::getResponses() const {
  return responses ;
} // getResponses()


int Request::getResponsesSize() const {
  return responsesSize ;
} // getResponsesSize()


CORBA::Long Request::getNumberOfParameters() const {
  return request->pb.last_inout + 1 ;
} // getNumberOfParameters()

const corba_request_t* Request::getRequest() const {
  return request ;
} // getRequest()
