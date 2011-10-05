/****************************************************************************/
/* Event Dispatcher (Listener class with template of event handler)         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/07/28 12:06:38  bisnard
 * changed include file name
 *
 * Revision 1.1  2010/07/20 09:13:57  bisnard
 * Adding event dispatcher
 *
 */

#ifndef EVENTDISPATCHER_HH_
#define EVENTDISPATCHER_HH_

#include <string>
#include <map>
#include <iostream>
#include "TypeInfo.hh"
#include "EventObserver.hh"

namespace events {

  class HandlerFunctionBase
  {
  public:
    virtual ~HandlerFunctionBase() {};
    void exec(const EventBase* event) {call(event);}
    
  private:
    virtual void call(const EventBase*) = 0;
  };


  template <class T, class EventT>
  class MemberFunctionHandler : public HandlerFunctionBase
  {
  public:
    
    typedef void (T::*MemberFunc)(EventT*);
    MemberFunctionHandler(T* instance, MemberFunc memFn) : _instance(instance), _function(memFn) {};
    
    void call(const EventBase* event)
    {
      (_instance->*_function)(static_cast<EventT*>(event));
    }
    
  private:
    T*          _instance;
    MemberFunc  _function;
  };


  class EventDispatcher : public EventObserver
  {
  public:
    ~EventDispatcher();
    virtual void handleEvent(const EventBase*);
    virtual bool isObserver(const EventBase*) const { return true; }
    
    template <class T, class EventT>
    void registerEventFunc(T*, void (T::*memFn)(EventT*));
    
  private:
    typedef std::map<TypeInfo, HandlerFunctionBase*> Handlers;
    Handlers _handlers;
  };


  template <class T, class EventT>
  void EventDispatcher::registerEventFunc(T* obj, void (T::*memFn)(EventT*))
  {
    _handlers[TypeInfo(typeid(EventT))]= new MemberFunctionHandler<T, EventT>(obj, memFn);
  }

}

#endif
