// $Id$

/*
 * This is part of DIET software.
 * Copyright (C) 2002 ReMaP/LIFC/INRIA
 */

#ifndef _COUNTER_HH_
#define _COUNTER_HH_

#ifdef __OMNIORB3__
#include <omniORB3/CORBA.h>
#else
#ifdef __OMNIORB4__
#include <omniORB4/CORBA.h>
#endif
#endif
#include <omnithread.h>
#include <sys/types.h>
#include <assert.h>

/**
 * This is a thread safe counter. The operators ++, -- and = are
 * defined. The counter accept only 32 bits positive value. The prefix
 * versions of the ++ and -- operators are faster than the postfix
 * versions
 *
 * example :
 *
 * \code
 *   Counter c ;         // c = 0
 *   ++c ;               // c = 1
 *   c = 5 ;             // c = 5
 *   --c ;               // c = 4
 *   // prints 4 on stdout. c = 3
 *   printf("%lu\n", (unsigned long)static_cast<CORBA::ULong>(c--)) ;
 * \endcode
 *
 * @author Sylvain DAHAN, LIFC Besançon (France)
 */

class Counter {

private :
  
  /**
   * The counter value.
   */
  CORBA::ULong             value ;

  /**
   * the counter critical zone mutex ;
   */
  mutable omni_mutex    valueMutex ;

public :
  
  /**
   * Creates a new Counter initialized with the value \c n. If no
   * arguments are given, the counter is initialized with the value
   * 0. There is no check for overflow
   *
   * @param n the initial value of the counter. It must be positive or
   * null.
   */
  inline Counter(CORBA::ULong n=0)  : value(n) {}

  /**
   * Creates a copy of the counter given in arguments.
   *
   * @param aCounter the object that is copied
   */
  Counter(const Counter& aCounter) ;

  /**
   * Increments the the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  Counter operator++(int) ;

  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  Counter operator--(int) ;

  /**
   * Increments the the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  inline Counter & operator++() {
    assert(value < value + 1) ;
    valueMutex.lock() ;
    value++ ;
    valueMutex.unlock() ;
    return *this ;
  }

  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  inline Counter & operator--() {
    assert(value > 0) ;
    valueMutex.lock() ;
    value-- ;
    valueMutex.unlock() ;
    return *this ;
  }

  /**
   * The instance get the same value as \c aCounter.
   *
   * @param aCounter The counter where the value is taken.
   */
  Counter & operator=(const Counter & aCounter) ;

  /**
   * Converts the counter to a \c CORBA::ULong value. The argument is not
   * used.
   */
  operator CORBA::ULong() const ;
  

} ;

#endif // _COUNTER_HH_
