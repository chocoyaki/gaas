/**
* @file EventHandler.hh
* 
* @brief  Event Handler (Abstract interface class)  
* 
* @author - Benjamin Isnard (benjamin.isnard@ens-lyon.fr
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2010/11/24 15:36:59  bdepardo
 * New line at the end of file to prevent warnings
 *
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

class EventHandler {
public:
  virtual ~EventHandler() {
  }

  virtual void
  handleEvent(const EventBase*) = 0;
};
}

#endif

