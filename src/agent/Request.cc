/****************************************************************************/
/* Request class source code                                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.9  2007/03/27 08:59:05  glemahec
 * Adapts the Request destructor to a dynamically loaded class.
 *
 * Revision 1.8  2006/05/15 19:43:21  ecaron
 * Previous version (the previous commit was an error)
 *
 * Revision 1.7  2006/05/15 19:37:42  ecaron
 * *** empty log message ***
 *
 * Revision 1.6  2005/05/15 15:46:40  alsu
 * minor change to the chooseGlobalScheduler method
 *
 * Revision 1.5  2005/04/25 08:57:17  hdail
 * Clean up memory for GS in Request destructor.
 *
 * Revision 1.4  2003/07/04 09:47:58  pcombes
 * Use new ERROR and WARNING macros.
 *
 * Revision 1.3  2003/04/10 13:02:50  pcombes
 * Apply Coding Standards. Add the GS (GlobalScheduler) private member.
 *
 * Revision 1.2  2003/01/22 15:44:01  sdahan
 * separation of the LA and MA
 *
 * Revision 1.1  2002/12/27 15:57:39  sdahan
 * the log list become a ts_map<RequestID, Request*>
 ****************************************************************************/

#include "Request.hh"

#include <assert.h>
#include <iostream>
using namespace std;
#include <stdio.h>

#include "debug.hh"

/* New : For scheduler load support. */
#ifdef USERSCHED
#include "UserScheduler.hh"
#endif
/*************************************/

void Request::freeResponses() {
  if (responses != NULL) {
    delete [] responses ;
    responses = NULL ;
    responsesSize = 0 ;
  }
}

Request::Request(const corba_request_t* request)
{
  this->request = request ;
  gatheringEnded = new omni_condition(&respMutex) ;
  responses = NULL ;
  responsesSize = 0 ;
  if (*(request->serialized_scheduler.in()) == '\0') {
    INTERNAL_WARNING("request " << request->reqID
		     << " has no scheduler associated");
    this->GS = GlobalScheduler::chooseGlobalScheduler();
  } else {
    this->GS = GlobalScheduler::deserialize(this->request->serialized_scheduler);
  }  
} // Request(...)

Request::Request(const corba_request_t* request, GlobalScheduler* GS)
{
  this->request = request ;
  gatheringEnded = new omni_condition(&respMutex) ;
  responses = NULL ;
  responsesSize = 0 ;

  this->GS = GS;
  if (GS && *(request->serialized_scheduler.in()) == '\0') {
    char* ser_sched = GlobalScheduler::serialize(this->GS);
    // deallocates old request->serialized_scheduler
    (const_cast<corba_request_t*>(request))->serialized_scheduler.out() =
      CORBA::string_dup(ser_sched);
    delete [] ser_sched;
  }  
} // Request(...)


Request::~Request() {
  freeResponses() ;
/* New : For scheduler load support. */
/* A loaded scheduler is not deleted in the same way than a normal scheduler.*/
#ifdef USERSCHED
  char * serializedScheduler = GlobalScheduler::serialize(this->GS);

  int nameLength = strlen(serializedScheduler);
  if (!strncmp(serializedScheduler, UserScheduler::stName,nameLength)) {
    (dynamic_cast<UserScheduler*> (this->GS))->destroy(this->GS);
  } else
#else
  delete this->GS;
#endif
/*************************************/
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


corba_response_t* Request::getResponses() const {
  return responses ;
} // getResponses()


size_t Request::getResponsesSize() const {
  return responsesSize ;
} // getResponsesSize()


CORBA::Long Request::getNumberOfParameters() const {
  return request->pb.last_inout + 1 ;
} // getNumberOfParameters()

const corba_request_t* Request::getRequest() const {
  return request ;
} // getRequest()


GlobalScheduler*
Request::getScheduler()
{
  return this->GS;
}
