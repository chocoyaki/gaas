/**
 * @file ChildID.hh
 *
 * @brief  DIET ChildID implementation header
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _CHILD_ID_
#define _CHILD_ID_

#include <omniORB4/CORBA.h>



class ChildID {
public:

  /**
   * Creates a new default ChildID with the value of -1.
   *
   * @param n the initial value of the ChildID.
   */
  explicit
  ChildID(CORBA::Long n = -1): value(n) {
  }

  /**
   * Clones a ChildID.
   *
   * @param childID the ChildID to clone
   */
  ChildID(const ChildID &childID) {
    value = childID.value;
  }

  /**
   * The instance get the same value as \c aChildID.
   *
   * @param aChildID The ChildID where the value is taken.
   */
  inline ChildID &
  operator = (const ChildID &aChildID) {
    value = aChildID.value;
    return *this;
  }

  inline ChildID &
  operator = (const CORBA::Long & val) {
    value = val;
    return *this;
  }

  /**
   * Converts the ChildID to a \c CORBA::Long value.
   */
  inline operator
  CORBA::Long() const {
    return value;
  }

private:
  /**
   * the value of the ChildID
   */
  CORBA::Long value;
};

#endif  // _CHILD_ID_
