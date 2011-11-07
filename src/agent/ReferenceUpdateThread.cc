/**
 * @file  ReferenceUpdateThread.cc
 *
 * @brief  A thread which updates the MultiMA links
 *
 * @author   Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "MasterAgentImpl.hh"

#if HAVE_MULTI_MA

#include "ReferenceUpdateThread.hh"

#include <cstdio>

ReferenceUpdateThread::ReferenceUpdateThread(MasterAgentImpl *owner,
                                             unsigned int period)
  : owner(owner), period(period) {
  start();
} // ReferenceUpdateThread(MasterAgent_impl*)


ReferenceUpdateThread::~ReferenceUpdateThread() {
}


void
ReferenceUpdateThread::run(void *ptr) {
  set_priority(omni_thread::PRIORITY_LOW);
  while (true) {
    owner->updateRefs();
    sleep(period);
  }
} // run(void*)

#endif  // HAVE_MULTI_MA
