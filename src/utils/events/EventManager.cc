/**
 * @file EventManager.cc
 *
 * @brief   Event Manager (Notifier class)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <iostream>
#include "EventManager.hh"

using namespace std;
using namespace events;

EventManager *EventManager::_instance = NULL;

EventManager *
EventManager::getEventMgr() {
  if (_instance == NULL) {
    _instance = new EventManager();
  }
  return _instance;
}

void
EventManager::addObserver(EventObserver *observer) {
  _observers.push_back(observer);
}

void
EventManager::sendEvent(EventBase *event) {
  // send notification to all observers
  for (unsigned int i = 0; i < _observers.size(); i++) {
    if (_observers[i]->isObserver(event)) {
      try {
        _observers[i]->handleEvent(event);
      } catch (std::ios_base::failure &e) {
        cerr << "Sending event failed: ios failure caught: " << e.what() <<
        endl;
      }
    }
  }
} // sendEvent
