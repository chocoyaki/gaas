/**
 * @file  ReferenceUpdateThread.hh
 *
 * @brief  A thread which updates the MultiMA links
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */



#ifndef _REFERENCE_UPDATE_THREAD_HH_
#define _REFERENCE_UPDATE_THREAD_HH_

#ifdef HAVE_MULTI_MA

#include "omnithread.h"

class MasterAgentImpl;


class ReferenceUpdateThread : public omni_thread {
public:
  /**
   * Creates and launch a new thread which update the reference of
   * owner each 5 minutes.
   *
   * @param owner A pointer on the master agent that own this
   * thread. This is this agent which are updated each 5 minutes.
   *
   * @param period amount of time that it wait between two references
   * updates.
   */
  ReferenceUpdateThread(MasterAgentImpl * owner, unsigned int period);

  /**
   * destroys the thread.
   */
  ~ReferenceUpdateThread();

private:
  /**
   * Updates the references of the owner, waits 5 minutes and do it
   * again.
   */
  void
  run(void *ptr);

  /**
   * A pointer on the master agent that own this thread. This is this
   * agent which are updated each 5 minutes.
   */
  MasterAgentImpl *owner;

  /**
   * amount of time that it wait between two references updates.
   */
  unsigned int period;
};

#endif  // HAVE_MULTI_MA

#endif  // _REFERENCE_UPDATE_THREAD_HH_
