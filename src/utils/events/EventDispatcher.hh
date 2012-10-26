/**
 * @file EventDispatcher.hh
 *
 * @brief  Event Dispatcher (Listener class with template of event handler)
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef EVENTDISPATCHER_HH_
#define EVENTDISPATCHER_HH_

#include <string>
#include <map>
#include <iostream>
#include "TypeInfo.hh"
#include "EventObserver.hh"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif

namespace events {
class HandlerFunctionBase {
public:
  virtual ~HandlerFunctionBase() {
  }

  void
  exec(const EventBase *event) {
    call(event);
  }

private:
  virtual void
  call(const EventBase *) = 0;
};


template <class T, class EventT>
class DIET_API_LIB MemberFunctionHandler : public HandlerFunctionBase {
public:
  typedef void (T::*MemberFunc)(EventT *);

  MemberFunctionHandler(T * instance, MemberFunc memFn)
    : _instance(instance), _function(memFn) {
  }

  void
  call(const EventBase *event) {
    (_instance->*_function)(static_cast<EventT *>(event));
  }

private:
  T *_instance;
  MemberFunc _function;
};


class DIET_API_LIB EventDispatcher : public EventObserver {
public:
  ~EventDispatcher();

  virtual void
  handleEvent(const EventBase *);

  virtual bool
  isObserver(const EventBase *) const {
    return true;
  }

  template <class T, class EventT>
  void
  registerEventFunc(T *, void (T::*memFn) (EventT *));

private:
  typedef std::map<TypeInfo, HandlerFunctionBase *> Handlers;
  Handlers _handlers;
};


template <class T, class EventT>
void
EventDispatcher::registerEventFunc(T *obj, void (T::*memFn) (EventT *)) {
  _handlers[TypeInfo(typeid(EventT))] =
    new MemberFunctionHandler<T, EventT
                              >(obj, memFn);
}
}

#endif /* ifndef EVENTDISPATCHER_HH_ */
