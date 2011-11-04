/**
 * @file EventDispatcher.cc
 *
 * @brief  Event Dispatcher (Listener class with template of event handler)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <iostream>
#include "EventDispatcher.hh"

using namespace std;
using namespace events;

// better use boost::ptr_map instead of std::map
EventDispatcher::~EventDispatcher() {
  Handlers::iterator it = _handlers.begin();
  while (it != _handlers.end()) {
    delete it->second;
    ++it;
  }
  _handlers.clear();
}

void
EventDispatcher::handleEvent(const EventBase *event) {
  Handlers::iterator it = _handlers.find(TypeInfo(typeid(*event)));
  if (it != _handlers.end()) {
    try {
      it->second->exec(event);
    } catch (...) {
      cerr << "Exception caught in event handling method - event=" <<
      event->toString() << endl;
    }
  }
}
