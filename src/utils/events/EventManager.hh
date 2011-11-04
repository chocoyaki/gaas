/**
 * @file EventManager.hh
 *
 * @brief   Event Manager (Notifier class)
 *
 * @author Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef EVENTMANAGER_HH_
#define EVENTMANAGER_HH_

#include <vector>
#include "EventBase.hh"
#include "EventHandler.hh"
#include "EventObserver.hh"

namespace events {
// Singleton class for the global event manager
// Implements the Mediator pattern to reduce coupling btw event producers and
// event observers

class EventManager {
public:
  virtual ~EventManager() {
  }

  /**
   * Get the instance of the singleton
   * @return ref to the instance
   */
  static EventManager *
  getEventMgr();

  /**
   * Generic event generation method
   * To be used when no static method provided
   * @param event
   */
  virtual void
  sendEvent(EventBase *event);

  /**
   * Add a new observer with a basic filter on severity
   * @param observer  the observer
   */
  void
  addObserver(EventObserver *observer);

protected:
  EventManager(): _parent(NULL) {
  }
  EventManager *_parent;

private:
  static EventManager *_instance;
  std::vector<EventObserver *> _observers;
};
}

#endif  // EVENTMANAGER_HH_
