/**
* @file  ReferenceUpdateThread.hh
* 
* @brief  A thread which updates the MultiMA links  
* 
* @author  - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.3  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.2  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 */


#ifndef _REFERENCE_UPDATE_THREAD_HH_
#define _REFERENCE_UPDATE_THREAD_HH_

#ifdef HAVE_MULTI_MA

#include "omnithread.h"

class MasterAgentImpl;

/**
 * This is an thead object that take a MasterAgent_impl in
 * argument. Each periods of time, it calls the updateRef of this
 * agent.
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
 */

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
  ReferenceUpdateThread(MasterAgentImpl* owner, unsigned int period);

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
  run(void* ptr);

  /**
   * A pointer on the master agent that own this thread. This is this
   * agent which are updated each 5 minutes.
   */
  MasterAgentImpl* owner;

  /**
   * amount of time that it wait between two references updates.
   */
  unsigned int period;
};

#endif  // HAVE_MULTI_MA

#endif  // _REFERENCE_UPDATE_THREAD_HH_
