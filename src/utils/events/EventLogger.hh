/****************************************************************************/
/* Event Logger (Event listener with basic handler for logging events)      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/11/24 15:36:59  bdepardo
 * New line at the end of file to prevent warnings
 *
 * Revision 1.1  2010/07/20 09:13:57  bisnard
 * Adding event dispatcher
 *
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
  EventLogger(std::ostream& output, EventBase::Severity minSeverity)
    : myOutput(output), myMinSeverity(minSeverity) {}

  virtual bool isObserver(const EventBase* event) const {
    return (event->getSeverity() >= myMinSeverity);
  }

  virtual void handleEvent(const EventBase* event) {
    myOutput << *event << std::endl;
  }

private:
  std::ostream&       myOutput;
  EventBase::Severity myMinSeverity;

};

}

#endif  // EVENTLOGGER_HH_

