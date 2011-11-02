/**
* @file ChildID.hh
* 
* @brief  DIET ChildID implementation header
* 
* @author - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.1  2003/04/10 13:11:06  pcombes
 * Replace SonID.hh. Apply Coding Standards.
 *
 ****************************************************************************/


#ifndef _CHILD_ID_
#define _CHILD_ID_

#include <omniORB4/CORBA.h>

/**
 * This is the identifier type for the agent children. It is a CORBA::Long
 * with -1 instead of 0 as default value.
 */

class ChildID {
public:

  /**
   * Creates a new default ChildID with the value of -1.
   *
   * @param n the initial value of the ChildID.
   */
  explicit ChildID(CORBA::Long n = -1) : value(n) {}

  /**
   * Clones a ChildID.
   *
   * @param childID the ChildID to clone
   */
  ChildID(const ChildID& childID) {
    value = childID.value;
  }

  /**
   * The instance get the same value as \c aChildID.
   *
   * @param aChildID The ChildID where the value is taken.
   */
  inline ChildID&
  operator=(const ChildID& aChildID) {
    value = aChildID.value;
    return *this;
  }

  inline ChildID&
  operator=(const CORBA::Long& val) {
    value = val;
    return *this;
  }

  /**
   * Converts the ChildID to a \c CORBA::Long value.
   */
  inline operator CORBA::Long() const {
    return value;
  }

private:
  /**
   * the value of the ChildID
   */
  CORBA::Long value;
};

#endif  // _CHILD_ID_
