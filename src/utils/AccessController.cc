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

  while(1){
    this->resourceSemaphore->wait();
    // Threads seem to be released in order of their arrival at 
    // the semaphore.  To guarantee FIFO in order of arrival at
    // waitForAccess do a check and yield if not our turn.
    if( myReqID == (this->maxIDReleased + 1) ) {
      break;
    }
    (omni_thread::self())->yield();
  }

  if(this->numFreeSlots <= 0){
    fprintf(stderr, 
        "AccessController:: confusion between semaphore and numFreeSlots ...", 1);
  }
  if(this->numWaiting <= 0){
    fprintf(stderr, 
        "AccessController:: Unexplained problem counting waiting threads.", 1);
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

