/****************************************************************************/
/* A thread which updates the MultiMA links                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 */


#ifndef _REFERENCE_UPDATE_THREAD_HH_
#define _REFERENCE_UPDATE_THREAD_HH_

#include "DIET_config.h"

#ifdef HAVE_MULTI_MA

#include "omnithread.h"

class MasterAgentImpl ;

/**
 * This is an thead object that take a MasterAgent_impl in
 * argument. Each periods of time, it calls the updateRef of this
 * agent.
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
 */

class ReferenceUpdateThread : public omni_thread {

private :

  /**
   * A pointer on the master agent that own this thread. This is this
   * agent which are updated each 5 minutes.
   */
  MasterAgentImpl* owner ;

  /**
   * amount of time that it wait between two references updates.
   */
  unsigned int period ;

  /**
   * destroys the thread.
   */
  ~ReferenceUpdateThread() ;

  /**
   * Updates the references of the owner, waits 5 minutes and do it
   * again.
   */
  void run(void* ptr) ;

public :
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
  ReferenceUpdateThread(MasterAgentImpl* owner, unsigned int period) ;

} ; // ReferenceUpdateThread

#endif // HAVE_MULTI_MA

#endif // _REFERENCE_UPDATE_THREAD_HH_
