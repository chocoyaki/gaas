/****************************************************************************/
/* Interface for limiting access to a resource to a configurable number of  */
/* consumers.                                                               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Holly DAIL (Holly.Dail@ens-lyon.fr)                                 */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2005/04/25 09:08:12  hdail
 * Print warning if jobs exit SeD-level queue out of order, but don't re-insert
 * them.  This approach needs to be redone later.
 *
 * Revision 1.2  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.1.2.1  2004/12/01 14:53:44  alsu
 * removing spurious argument that caused compilation warning (and thus
 * failure in maintainer mode) on certain "sensitive" versions of gcc.
 *
 * Revision 1.1  2004/10/04 13:55:06  hdail
 * - Added AccessController class, an enhanced counting semaphore.
 * - Added config file options for controlling concurrent SeD access.
 *
 ****************************************************************************/

#include "AccessController.hh"

#include <stdio.h>
#include <iostream>
using namespace std;
#include <stdlib.h>

#include "debug.hh"
#include "omnithread.h"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define ACCESS_TRACE_FUNCTION(formatted_text) \
  TRACE_TEXT(TRACE_ALL_STEPS, "AccessControl::"); \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

AccessController::AccessController(int initialResources) {
  if(initialResources >= 0) {
    this->resourcesInitial = initialResources;
  } else {
    this->resourcesInitial = 0;
    fprintf(stderr, "AccessController:: invalid initialResources of %d.\n",
        initialResources);
    fprintf(stderr, "AccessController:: setting initialResources to 0.\n");
  } 

  resourceSemaphore = new omni_semaphore(this->resourcesInitial);
  numFreeSlots = this->resourcesInitial;
  numWaiting = 0;

  reqCounter = 0;
  maxIDReleased = -1;
}

void
AccessController::waitForResource(){
  int myReqID = -1;
    
  this->globalLock.lock();      /** LOCK */
  myReqID = this->reqCounter++;
  this->numWaiting++;
  this->globalLock.unlock();    /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS, "Thread " <<  (omni_thread::self())->id()
        << " / Request " << myReqID << " enqueued ("
        << this->numWaiting << " waiting, "
        << this->numFreeSlots << " slots free)" << endl);

  this->resourceSemaphore->wait();

  // TODO: On systems tested semaphore gives FIFO ordering and the following
  // test succeeds only in the case that tasks switch order between the receipt
  // of request IDs and the wait on the semaphore.  This case is not very
  // important because those tasks arrive at almost the same moment.
  // However, semaphores do not guarantee FIFO ordering so for portability
  // another solution must be found to guarantee FIFO.
  if( myReqID != (this->maxIDReleased + 1) ) {
    WARNING("Thread " << (omni_thread::self())->id()
          << " / Request " << myReqID << " exiting queue out-of-order.");
  }

  if(this->numFreeSlots <= 0){
    fprintf(stderr, 
            "AccessController:: confusion between "
            "semaphore and numFreeSlots ...");
  }
  if(this->numWaiting <= 0){
    fprintf(stderr, 
            "AccessController:: Unexplained problem "
            "counting waiting threads.");
  }

  this->globalLock.lock();      /** LOCK */
  this->numFreeSlots--;
  this->numWaiting--;
  if(myReqID > this->maxIDReleased){
    maxIDReleased = myReqID;
  }
  this->globalLock.unlock();    /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS, "Thread " <<  (omni_thread::self())->id()
      << " / Request " << myReqID << " got resource ("
      << this->numWaiting << " waiting, "
      << this->numFreeSlots << " slots free)" << endl);
}

void
AccessController::releaseResource(){
  this->globalLock.lock();      /** LOCK */
  this->numFreeSlots++;
  this->globalLock.unlock();    /** UNLOCK */  

  TRACE_TEXT(TRACE_ALL_STEPS, "Thread " <<  (omni_thread::self())->id()
      << " released resource ("
      << this->numWaiting << " waiting, "
      << this->numFreeSlots << " slots free)" << endl);

  this->resourceSemaphore->post();
}

int AccessController::getFreeSlotCount(){
  return this->numFreeSlots;
}

int AccessController::getNumWaiting(){
  return this->numWaiting;
}

int AccessController::getTotalResourceCount(){
  return this->resourcesInitial;
}

