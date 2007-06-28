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
#include <assert.h>


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
