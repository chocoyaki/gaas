// $Id$

/*
 * $Log$
 * Revision 1.1  2002/12/17 17:07:32  sdahan
 * Add a new thread safe CORBA::Long counter and a new thread safe
 * LinkedList objects.
 *
 */

#include "Counter.hh"
#include <assert.h>


Counter::Counter(const Counter& aCounter) {
  value = static_cast<CORBA::ULong>(aCounter) ;
}


Counter Counter::operator++(int) {
  assert(value < value + 1) ; // check for overflow
  valueMutex.lock() ;
  Counter oldValue ;
  oldValue.value = value ;
  value++ ;
  valueMutex.unlock() ;
  return oldValue ;
}


Counter Counter::operator--(int) {
  assert(value > 0) ;
  valueMutex.lock() ;
  Counter oldValue ;
  oldValue.value = value ;
  value-- ;
  valueMutex.unlock() ;
  return oldValue ;
}


Counter & Counter::operator=(const Counter & aCounter) {
  value = static_cast<CORBA::ULong>(aCounter) ;
  return *this ;
}


Counter::operator CORBA::ULong() const {
  CORBA::ULong    valueBuf ;
  valueMutex.lock() ;
  valueBuf = value ;
  valueMutex.unlock() ;
  return valueBuf ;
}
