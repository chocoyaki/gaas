/****************************************************************************/
/* Event Base class                                                         */
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

#ifndef EVENTBASE_HH_
#define EVENTBASE_HH_

#include <iostream>

class EventBase
{
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
    
  EventBase() : mySeverity(INFO) {}
  EventBase(Severity severity) : mySeverity(severity) {}
    
  short getSeverity() const { return mySeverity; }
  virtual std::string toString() const;
    
protected:
  virtual ~EventBase() {};
  Severity    mySeverity;
};

std::ostream& operator<<(std::ostream& out, const EventBase& e);

#endif  // EVENTBASE_HH_

