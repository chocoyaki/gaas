/**
 * @file Counter.hh
 *
 * @brief  Thread safe counter header
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _COUNTER_HH_
#define _COUNTER_HH_

#include <omniORB4/CORBA.h>

#include <omnithread.h>
#include <sys/types.h>
#include <cassert>
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif
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
 * @author Sylvain DAHAN, LIFC Besan�on (France)
 */

class DIET_API_LIB Counter {
public:
  /**
   * Creates a new Counter initialized with the value \c n. If no
   * arguments are given, the counter is initialized with the value
   * 0. There is no check for overflow
   *
   * @param n the initial value of the counter. It must be positive or
   * null.
   */
  Counter(CORBA::ULong n = 0): value(n) {
  }

  /**
   * Creates a copy of the counter given in arguments.
   *
   * @param aCounter the object that is copied
   */
  Counter(const Counter &aCounter);

  /**
   * Increments the the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  Counter
    operator++ (int i);

  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  Counter
    operator-- (int i);

  /**
   * Increments the counter. An assert check that the counter does
   * not overflow its capacity. The argument is not used.
   *
   * @todo replace the assert by an exception.
   */
  Counter &
  operator++ ();
  /**
   * Increments the counter by a given value.
   * An assert check that the counter does not overflow its capacity.
   *
   * @todo replace the assert by an exception.
   */
  Counter &
  operator += (const Counter &aCounter);
  /**
   * Decrements the counter. An assert check that the counter is
   * greater than 0. The argument is not used.
   */
  Counter &
  operator-- ();
  /**
   * Decrements the counter by a given value.
   * An assert check that the counter is greater than 0.
   *
   * @todo replace the assert by an exception.
   */
  Counter &
  operator -= (const Counter &aCounter);
  /**
   * The instance get the same value as \c aCounter.
   *
   * @param aCounter The counter where the value is taken.
   */
  Counter &
  operator = (const Counter &aCounter);

  /**
   * Converts the counter to a \c CORBA::ULong value. The argument is not
   * used.
   */
  operator
  CORBA::ULong() const;

private:
  CORBA::ULong value; /**<! counter value */
  mutable omni_mutex valueMutex; /**<! counter mutex guard */
};

#endif  // _COUNTER_HH_
