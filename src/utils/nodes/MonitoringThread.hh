/**
 * @file MonitoringThread.hh
 *
 * @brief  actively collects monitoring information for DIET, mainly from FAST
 *
 * @author  Georg Hoesch (hoesch@in.tum.de)
 *          Cyrille Pontvieux (cyrille.pontvieux@edu.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MONITORINGTHREAD_HH_
#define _MONITORINGTHREAD_HH_

#include <omniconfig.h>
#include <omnithread.h>

#include "DietLogComponent.hh"

/**
 * This thread actively queries FAST every second for mem, load and
 * other passive values which cannot be measured directly in the
 * sorucecode
 */
class MonitoringThread : public omni_thread {
public:
  /**
   * Constructor.
   * @param DLC reference to the DietLogComponent of this thread.
   */
  explicit
  MonitoringThread(DietLogComponent *DLC);

  /**
   * Start the thread.
   */
  void
  startThread();

  /**
   * Stop the thread.
   * The object will be deleted by the orb.
   */
  void
  stopThread();

protected:
  /**
   * The main loop of the thread.
   * Logs FAST values with the dietLogComponent
   */
  void *
  run_undetached(void *params);

  /**
   * Destructor. Is called by the orb when the thread is
   * stopped. Do not call it yourself.
   */
  ~MonitoringThread();

private:
  /**
   * This threads dietLogComponent. NULL if no monitoring information
   * is to be gathered (although you don't need to start this thread then)
   */
  DietLogComponent *dietLogComponent;

  /**
   * controls the main loop
   */
  bool threadRunning;
};

#endif /* ifndef _MONITORINGTHREAD_HH_ */
