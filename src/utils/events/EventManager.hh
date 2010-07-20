/****************************************************************************/
/* Event Manager (Notifier class)                                           */
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

#ifndef EVENTMANAGER_HH_
#define EVENTMANAGER_HH_

#include "EventBase.hh"
#include "EventHandler.hh"
#include "EventObserver.hh"
#include <vector>

namespace events {

// Singleton class for the global event manager
// Implements the Mediator pattern to reduce coupling btw event producers and
// event observers

class EventManager {
  
  public:
    
    /**
     * Get the instance of the singleton
     * @return ref to the instance
     */
    static EventManager* getEventMgr();
    
    /**
     * Generic event generation method
     * To be used when no static method provided 
     * @param event
     */
    virtual void sendEvent(EventBase* event);
    
    /**
     * Add a new observer with a basic filter on severity
     * @param observer	the observer
     * @param severityLevel minimum level of severity
     */
    void addObserver(EventObserver* observer);
    
  protected:
    
    EventManager() : _parent(NULL) {}
    EventManager* _parent;
    
  private:
    
    static EventManager* 	_instance;
    std::vector<EventObserver*>	_observers;
};

}

#endif // EVENTMANAGER_HH_