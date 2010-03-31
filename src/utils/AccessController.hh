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
 * Revision 1.4  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.3  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.2  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.1  2004/10/04 13:55:06  hdail
 * - Added AccessController class, an enhanced counting semaphore.
 * - Added config file options for controlling concurrent SeD access.
 *
 ****************************************************************************/

#ifndef _ACCESSCONTROLLER_HH_
#define _ACCESSCONTROLLER_HH_

#include <omniORB4/CORBA.h>

#include <omnithread.h>
#include <sys/types.h>
#include <cassert>

/**
 * The AccessController class coordinates access to a resource between
 * multiple threads.  It can be thought of as an enhanced counting
 * semaphore:  threads wanting a resource call waitForResource() and after
 * they are finished with the resource they call releaseResource().  FIFO 
 * order is enforced, where priority is based on the order of arrival at
 * the waitForResource call.  
 *
 * @author Holly DAIL
 */
class AccessController{

public :
  
  /**
   * Creates a new AccessController with a resource limit of \c 
   * initialResources.
   *
   * @param initialResources The number of resources initially available
   *                  It must be a non-negative integer.
   */
  AccessController(int initialResources);

  void waitForResource();

  void releaseResource();

  /** 
   * Get count of resources immediately available.
   */
  int getFreeSlotCount();

  /**
   * Get count of number of threads waiting for a resource.
   */
  int getNumWaiting();

  /**
   * Get number of available resources.
   */
  int getTotalResourceCount();

private :
  /// The number of resources initially available.
  int resourcesInitial;

  /// Lock for thread-safe access to variables
  omni_mutex globalLock;

  /// Semaphore to provide low-level resource limit
  omni_semaphore* resourceSemaphore;

  /** Count of number of free resource slots.  
   * [Needed because omni_semaphore will not report semaphore value. */
  int numFreeSlots;

  /// Number of threads waiting for resource access
  int numWaiting;

  /// Assign unique req ID to each access request
  int reqCounter;

  /// Largest ID of request that has been given resource
  int maxIDReleased;
} ;

#endif // _ACCESSCONTROLLER_HH_
