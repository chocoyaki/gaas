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
 * Revision 1.4  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
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

#include "MasterAgentImpl.hh"

#if HAVE_MULTI_MA

#include "ReferenceUpdateThread.hh"

#include <cstdio>

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
