/****************************************************************************/
/* Event Handler (Abstract interface class)                                 */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/07/20 09:13:57  bisnard
 * Adding event dispatcher
 *
 */

#ifndef EVENTHANDLER_HH_
#define EVENTHANDLER_HH_

#include <string>
#include <map>
#include <iostream>

#include "EventBase.hh"

namespace events {
  
class EventHandler
{
  public:

    virtual void handleEvent(const EventBase*) = 0;
    
};

}

#endif