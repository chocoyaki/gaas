// $Id$

/*
 * $Log$
 * Revision 1.1  2002/12/27 15:57:39  sdahan
 * the log list becaume a ts_map<RequestID, Request*>
 *
 */

#ifndef _REQUEST_ID_
#define _REQUEST_ID_

#include <omniORB4/CORBA.h>

/**
 * This is the identifier type for a request.
 *
 * @author Sylvain DAHAN, LIFC Besancon (France)
 */

class RequestID {

private :
  /**
   * the value of RequestID
   */
  CORBA::Long value ;

public :

  /**
   * Creates a new RequestID.
   *
   * @param reqID the value of the RequestID.
   */
  inline RequestID(CORBA::Long reqID) : value(reqID) {}

  /**
   * Clones a RequestID.
   *
   * @param reqID the RequestID to clone.
   */
  inline RequestID(const RequestID & reqID) {
    value = reqID.value ;
  }

  /**
   * the current instance get the same value as \c reqID
   *
   * @param reqID The RequestID where the value is taken.
   */
  inline RequestID & operator=(const RequestID & reqID) {
    value = reqID.value ;
    return *this ;
  }

  /**
   * Converts the RequestID to a \c CORBA::Long value.
   */
  inline operator CORBA::Long() const {
    return value ;
  }

  friend bool operator< (const RequestID & a, const RequestID & b) ;

  friend bool operator== (const RequestID & a, const RequestID & b) ;

} ; // RequestID

/**
 * returns true if the representation of \c a is lesser than the
 * representation of \c b.
 *
 * @param a a RequestID
 *
 * @param b a RequestID
 */
inline bool operator< (const RequestID & a, const RequestID & b) {
  return a.value < b.value ;
}

/**
 * returns true if \c a and \c b represent the same ID.
 *
 * @param a a RequestID
 *
 * @param b a RequestID
 */
inline bool operator== (const RequestID & a, const RequestID & b) {
  return a.value == b.value ;
}

#endif // _REQUEST_ID_
