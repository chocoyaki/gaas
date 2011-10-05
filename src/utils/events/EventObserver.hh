/****************************************************************************/
/* Event Observer (Abstract Listener interface class)                       */
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

#ifndef EVENTOBSERVER_HH_
#define EVENTOBSERVER_HH_

#include <iostream>
#include "EventHandler.hh"

namespace events {

  class EventObserver : public EventHandler {
  
  public:
    
    virtual bool isObserver(const EventBase* event) const = 0;

  };

}

#endif // EVENTOBSERVER_HH_

