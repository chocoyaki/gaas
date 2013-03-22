/**
 * @file EventLogger.hh
 *
 * @brief   Event Logger (Event listener with basic handler for logging events)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef EVENTLOGGER_HH_
#define EVENTLOGGER_HH_

#include <iostream>
#include <sstream>
#include "EventObserver.hh"
#include "EventTypes.hh"

namespace events {
class EventLogger : public EventObserver {
public:
  EventLogger(std::ostream & output, EventBase::Severity minSeverity)
  : myOutput(output), myMinSeverity(minSeverity) {
  }

  virtual bool
  isObserver(const EventBase *event) const {
    return (event->getSeverity() >= myMinSeverity);
  }

  virtual void
  handleEvent(const EventBase *event) {
    myOutput << event->toString() << std::endl;
  }

private:
  std::ostream &myOutput;
  EventBase::Severity myMinSeverity;
};
}

#endif  // EVENTLOGGER_HH_
