/**
 * @file EventObserver.hh
 *
 * @brief   Event Observer (Abstract Listener interface class)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef EVENTOBSERVER_HH_
#define EVENTOBSERVER_HH_

#include <iostream>
#include "EventHandler.hh"

namespace events {
class EventObserver : public EventHandler {
public:
  virtual ~EventObserver() {
  }

  virtual bool
  isObserver(const EventBase *event) const = 0;
};
}

#endif  // EVENTOBSERVER_HH_
