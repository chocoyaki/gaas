/****************************************************************************/
/* Event Dispatcher (Listener class with template of event handler)         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/11/24 15:38:05  bdepardo
 * New line at the end of the file to prevent warnings
 *
 * Revision 1.1  2010/07/20 09:13:57  bisnard
 * Adding event dispatcher
 *
 */

#include <iostream>
#include "EventDispatcher.hh"

using namespace std;
using namespace events;

//better use boost::ptr_map instead of std::map
EventDispatcher::~EventDispatcher()
{
  Handlers::iterator it = _handlers.begin();
  while(it != _handlers.end())
  {
    delete it->second;
    ++it;
  }
  _handlers.clear();
}

void EventDispatcher::handleEvent(const EventBase* event)
{
  Handlers::iterator it = _handlers.find(TypeInfo(typeid(*event)));
  if (it != _handlers.end())
  {
    try {
      it->second->exec(event);
    } catch (...) {
      cerr << "Exception caught in event handling method - event=" << event->toString() << endl;
    }
  }
}

