/****************************************************************************/
/* MonitoringThread header - actively collects monitoring information for   */
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
 * Revision 1.2  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.1  2004/03/01 19:00:44  rbolze
 * Thread to enable monitoring from LogCentral
 *
 ****************************************************************************/

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
class MonitoringThread: public omni_thread
{
public:
  /**
   * Constructor.
   * @param DLC reference to the DietLogComponent of this thread.
   */
  MonitoringThread(DietLogComponent* DLC);

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
  void*
  run_undetached(void* params);

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
  DietLogComponent* dietLogComponent;

  /**
   * controls the main loop
   */
  bool threadRunning;
};

#endif
