/****************************************************************************/
/* Thread safe counter implementation                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2010/07/30 14:44:26  glemahec
 * Temporary corrections for the new compilation process. Parallel compilation is still broken and there is a big mess on the CMakeLists files...
 *
 * Revision 1.2  2010/03/31 21:15:41  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.4  2007/06/28 14:55:05  ycaniou
 * Rien dans Counter.cc
 *
 * Ajout en inline de += et de -= car a = a +/- b n'est pas atomique.
 *
 * Revision 1.3  2003/09/04 14:49:24  ckochhof
 * CED: fix of an assert bug
 *
 * Revision 1.2  2003/04/10 12:45:10  pcombes
 * Apply Coding Standards.
 *
 * Revision 1.1  2002/12/17 17:07:32  sdahan
 * Add a new thread safe CORBA::Long counter and a new thread safe
 * LinkedList objects.
 ****************************************************************************/

#include "Counter.hh"
#include <cassert>


Counter::Counter(const Counter& aCounter) {
  value = static_cast<CORBA::ULong>(aCounter) ;
}

Counter Counter::operator++(int) {
  valueMutex.lock() ;
  assert(value < value + 1) ; // check for overflow
  Counter oldValue ;
  oldValue.value = value ;
  value++ ;
  valueMutex.unlock() ;
  return oldValue ;
}

Counter Counter::operator--(int) {
  valueMutex.lock() ;
  assert(value > 0) ;
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

Counter & Counter::operator--() {
  assert(value > 0) ;
  valueMutex.lock() ;
  value-- ;
  valueMutex.unlock() ;
  return *this ;
}

Counter & Counter::operator++() {
  assert(value < value + 1) ;
  valueMutex.lock() ;
  value++ ;
  valueMutex.unlock() ;
  return *this ;
}

Counter & Counter::operator-=(const Counter & aCounter) {
  assert(value > 0) ;
  valueMutex.lock() ;
  value = value - static_cast<CORBA::ULong>(aCounter) ;
  valueMutex.unlock() ;
  return *this ;
}

Counter & Counter::operator+=(const Counter & aCounter) {
  assert(value < value + 1) ;
  valueMutex.lock() ;
  value = value + static_cast<CORBA::ULong>(aCounter) ;
  valueMutex.unlock() ;
  return *this ;
}


