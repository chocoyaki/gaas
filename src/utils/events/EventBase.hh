/**
 * @file EventBase.hh
 *
 * @brief  Event Base class
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef EVENTBASE_HH_
#define EVENTBASE_HH_

#include <iostream>
#include <string>

class EventBase {
public:
  enum Severity {
    TRACE,
    DEBUG,
    INFO,     // default severity (application-level events)
    NOTICE,
    WARNING,
    ERROR,
    CRITICAL,
    FATAL
  };

  EventBase(): mySeverity(INFO) {
  }

  explicit
  EventBase(Severity severity): mySeverity(severity) {
  }

  short
  getSeverity() const {
    return mySeverity;
  }

  virtual std::string
  toString() const;

protected:
  virtual ~EventBase() {
  }

  Severity mySeverity;
};

std::ostream &
operator << (std::ostream & out, const EventBase &e);

#endif  // EVENTBASE_HH_
