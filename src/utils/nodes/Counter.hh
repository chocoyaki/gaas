/****************************************************************************/
/* Thread safe counter header                                               */
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
 * Revision 1.8  2007/06/28 14:55:05  ycaniou
 * Rien dans Counter.cc
 *
 * Ajout en inline de += et de -= car a = a +/- b n'est pas atomique.
 *
 * Revision 1.7  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.6  2005/04/28 13:14:32  eboix
 *    Inclusion of CORBA.h substituted with omniORB4/CORBA.h --- Injay 2461
 *
 * Revision 1.5  2003/04/10 12:45:10  pcombes
 * Apply Coding Standards.
 *
 ****************************************************************************/


#ifndef _COUNTER_HH_
#define _COUNTER_HH_

#include <omniORB4/CORBA.h>

#include <omnithread.h>
#include <sys/types.h>
#include <cassert>

/**
 * This is a thread safe counter. The operators ++, -- and = are
 * defined. The counter accept only 32 bits positive value. The prefix
 * versions of the ++ and -- operators are faster than the postfix
 * versions
 *
 * example :
 *
 * \code
 *   Counter c;         // c = 0
 *   ++c;               // c = 1
 *   c = 5;             // c = 5
 *   --c;               // c = 4
 *   // prints 4 on stdout. c = 3
 *   printf("%lu\n", (unsigned long)static_cast<CORBA::ULong>(c--));
 * \endcode
 *
 * @author Sylvain DAHAN, LIFC Besançon (France)
 */

class Counter {

private:
  
  /**
   * The counter value.
   */
  CORBA::ULong             value;

  /**
   * the counter critical zone mutex;
   */
  mutable omni_mutex    valueMutex;

public:
  
  /**
   * Creates a new Counter initialized with the value \c n. If no
   * arguments are given, the counter is initialized with the value
   * 0. There is no check for overflow
   *
   * @param n the initial value of the counter. It must be positive or
   * null.
   */
  Counter(CORBA::ULong n = 0)  : value(n) {}

  /**
   * Creates a copy of the counter given in arguments.
   *
   * @param aCounter the object that is copied
   */
  Counter(const Counter& aCounter);

  /**
   * Increments the the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  Counter operator++(int);

  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  Counter operator--(int);

  /**
   * Increments the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  Counter& operator++();
  /**
   * Increments the counter by a given value. 
   * An assert check that the counter does not overflow its capacity.
   *
   * @todo replace the assert by an exception.
   */
  Counter & operator+=(const Counter & aCounter);
  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  Counter & operator--();
  /**
   * Decrements the counter by a given value. 
   * An assert check that the counter is greater than 0.
   *
   * @todo replace the assert by an exception.
   */
  Counter & operator-=(const Counter & aCounter);
  /**
   * The instance get the same value as \c aCounter.
   *
   * @param aCounter The counter where the value is taken.
   */
  Counter & operator=(const Counter & aCounter);

  /**
   * Converts the counter to a \c CORBA::ULong value. The argument is not
   * used.
   */
  operator CORBA::ULong() const;
  

};

#endif  // _COUNTER_HH_
