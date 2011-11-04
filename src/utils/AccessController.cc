/**
 * @file  AccessController.cc
 *
 * @brief  Interface for limiting access to a resource to a configurable consumers
 *
 * @author  - Holly DAIL (Holly.Dail@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "AccessController.hh"

#include <cstdio>
#include <iostream>
using namespace std;
#include <cstdlib>

#include "omnithread.h"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define ACCESS_TRACE_FUNCTION(formatted_text)           \
  TRACE_TEXT(TRACE_ALL_STEPS, "AccessControl::");       \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)

AccessController::AccessController(int initialResources) {
  if (initialResources >= 0) {
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
AccessController::waitForResource() {
  int myReqID = -1;

  this->globalLock.lock();      /** LOCK */
  myReqID = this->reqCounter++;
  this->numWaiting++;
  this->globalLock.unlock();    /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS,
             "Thread " << (omni_thread::self())->id()
                       << " / Request " << myReqID <<
             " enqueued ("
                       << this->numWaiting << " waiting, "
                       << this->numFreeSlots <<
             " slots free)" << endl);

  this->resourceSemaphore->wait();

  // TODO: On systems tested semaphore gives FIFO ordering and the following
  // test succeeds only in the case that tasks switch order between the receipt
  // of request IDs and the wait on the semaphore.  This case is not very
  // important because those tasks arrive at almost the same moment.
  // However, semaphores do not guarantee FIFO ordering so for portability
  // another solution must be found to guarantee FIFO.
  if (myReqID != (this->maxIDReleased + 1)) {
    WARNING(
      "Thread " << (omni_thread::self())->id()
                << " / Request " << myReqID <<
      " exiting queue out-of-order.");
  }

  if (this->numFreeSlots <= 0) {
    fprintf(stderr,
            "AccessController:: confusion between "
            "semaphore and numFreeSlots ...");
  }
  if (this->numWaiting <= 0) {
    fprintf(stderr,
            "AccessController:: Unexplained problem "
            "counting waiting threads.");
  }

  this->globalLock.lock();      /** LOCK */
  this->numFreeSlots--;
  this->numWaiting--;
  if (myReqID > this->maxIDReleased) {
    maxIDReleased = myReqID;
  }
  this->globalLock.unlock();    /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS,
             "Thread " << (omni_thread::self())->id()
                       << " / Request " << myReqID <<
             " got resource ("
                       << this->numWaiting << " waiting, "
                       << this->numFreeSlots <<
             " slots free)" << endl);
} // waitForResource

void
AccessController::releaseResource() {
  this->globalLock.lock();      /** LOCK */
  this->numFreeSlots++;
  this->globalLock.unlock();    /** UNLOCK */

  TRACE_TEXT(TRACE_ALL_STEPS,
             "Thread " << (omni_thread::self())->id()
                       << " released resource ("
                       << this->numWaiting << " waiting, "
                       << this->numFreeSlots <<
             " slots free)" << endl);

  this->resourceSemaphore->post();
} // releaseResource

int
AccessController::getFreeSlotCount() {
  return this->numFreeSlots;
}

int
AccessController::getNumWaiting() {
  return this->numWaiting;
}

int
AccessController::getTotalResourceCount() {
  return this->resourcesInitial;
}
