/****************************************************************************/
/* MonitoringThread - actively collects monitoring information for          */
/* DIET, mainly from FAST                                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Georg Hoesch (hoesch@in.tum.de)                                     */
/*    - Cyrille Pontvieux (cyrille.pontvieux@edu.univ-fcomte.fr)            */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.1  2004/03/01 19:00:37  rbolze
 * Thread to enable monitoring from LogCentral
 *
 ****************************************************************************/

#include "MonitoringThread.hh"

MonitoringThread::MonitoringThread(DietLogComponent* DLC) {
  dietLogComponent = DLC;
}

void
MonitoringThread::startThread() {
  if (threadRunning) {
    return;
  } else {
    threadRunning = true;
    start_undetached();
  }
}

void
MonitoringThread::stopThread() {
  if (!threadRunning) {
    return;
  } else {
    threadRunning = false;
    join(NULL);
  }
}

void*
MonitoringThread::run_undetached(void* params) {
  while (threadRunning) {
    /* This is empty, as I do not have the time to implement and
     * test it. Whoever is interested in mem, load, latency and
     * bandwidth from FAST must gather the values here and use the
     * corresponding dietLogComponent functions to send them.
     */

    if (dietLogComponent != NULL) {
#if HAVE_FAST
      // check if FAST is available
      // gather data here
      // dietLogComponent->logMem(mem);
      // dietLogComponent->logLoad(load);
      // dietLogComponent->logBandwidth(bw);
      // dietLogComponent->logLatency(latency);
#endif
    }

    sleep(1,0);
  }
  return NULL;
}

MonitoringThread::~MonitoringThread() {
  // nothing to do
}