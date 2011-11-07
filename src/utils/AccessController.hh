/**
 * @file  AccessController.hh
 *
 * @brief  Interface for limiting access to a resource to a configurable consumers
 *
 * @author  Holly DAIL (Holly.Dail@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _ACCESSCONTROLLER_HH_
#define _ACCESSCONTROLLER_HH_

#include <cassert>
#include <sys/types.h>
#include <omniORB4/CORBA.h>
#include <omnithread.h>


class AccessController {
public:
  /**
   * Creates a new AccessController with a resource limit of \c
   * initialResources.
   *
   * @param initialResources The number of resources initially available
   *                  It must be a non-negative integer.
   */
  explicit
  AccessController(int initialResources);

  void
  waitForResource();

  void
  releaseResource();

  /**
   * Get count of resources immediately available.
   */
  int
  getFreeSlotCount();

  /**
   * Get count of number of threads waiting for a resource.
   */
  int
  getNumWaiting();

  /**
   * Get number of available resources.
   */
  int
  getTotalResourceCount();

private:
  // / The number of resources initially available.
  int resourcesInitial;

  // / Lock for thread-safe access to variables
  omni_mutex globalLock;

  // / Semaphore to provide low-level resource limit
  omni_semaphore *resourceSemaphore;

  /** Count of number of free resource slots.
   * [Needed because omni_semaphore will not report semaphore value. */
  int numFreeSlots;

  // / Number of threads waiting for resource access
  int numWaiting;

  // / Assign unique req ID to each access request
  int reqCounter;

  // / Largest ID of request that has been given resource
  int maxIDReleased;
};

#endif  // _ACCESSCONTROLLER_HH_
