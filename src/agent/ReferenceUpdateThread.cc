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

#include "DIET_config.h"

#if HAVE_MULTI_MA

#include "ReferenceUpdateThread.hh"
#include "MasterAgentImpl.hh"

#include <stdio.h>

ReferenceUpdateThread::ReferenceUpdateThread(MasterAgentImpl* owner,
					     unsigned int period) 
  : owner(owner), period(period) {
  start() ;
} // ReferenceUpdateThread(MasterAgent_impl*)


ReferenceUpdateThread::~ReferenceUpdateThread() { }


void ReferenceUpdateThread::run(void* ptr) {
  set_priority(omni_thread::PRIORITY_LOW) ;
  while(true) {
    owner->updateRefs() ;
    sleep(period) ;
  }
} // run(void*)

#endif // HAVE_MULTI_MA
