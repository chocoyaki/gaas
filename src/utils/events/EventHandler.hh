/**
 * @file EventHandler.hh
 *
 * @brief  Event Handler (Abstract interface class)
 *
 * @author Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef EVENTHANDLER_HH_
#define EVENTHANDLER_HH_

#include <string>
#include <map>
#include <iostream>

#include "EventBase.hh"

namespace events {
class EventHandler {
public:
  virtual ~EventHandler() {
  }

  virtual void
  handleEvent(const EventBase *) = 0;
};
}

#endif /* ifndef EVENTHANDLER_HH_ */
