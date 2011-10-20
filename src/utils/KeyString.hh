/****************************************************************************/
/* Hashed string object to be used with container                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN             - Sylvain.Dahan@lifc.univ-fcomte.fr       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.3  2006/06/30 15:37:35  ycaniou
 * Code presentation, commentaries (nothing really "touched")
 *
 * Revision 1.2  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.1  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 ****************************************************************************/

#ifndef _KEY_STRING_HH_
#define _KEY_STRING_HH_

/**
 * \page KeyString
 *
 * This class made easy the use of char* has container key. The use of
 * char* is not simple because a comparator must be write. Also, the
 * deallocation of the memory is not automatic.
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
 */

#include <cstdlib>
#include <sys/types.h>
#include <iostream>

class KeyString {
public:
  /**
   * calculate the hash value of the key
   *
   * @param id the string for which the hash value is calculated. Must
   * be not NULL.
   *
   * @return the hash value of id.
   */
  static u_int32_t
  hash(const char* id);

  /**
   * creates a new KeyString.
   *
   * @param value value of the key. It value is copied by the
   * KeyString and the memory used as argument can be free by the
   * user. value must be not NULL.
   */
  KeyString(const char* value);

  /**
   * Creates a clone of the KeyString given in argument.
   *
   * @param masterAgentId the KeyString which is cloned
   */
  KeyString(const KeyString & masterAgentId);

  /**
   * The KeyString is destroyed.
   */
  ~KeyString();

  /**
   * change the value of the key.
   *
   * @param masterAgentId the new value of the key.
   */
  KeyString &
  operator=(const KeyString & masterAgentId);

  /**
   * returnes a char* that containes the value of the
   * KeyString.
   */
  operator const char*() const;


  friend bool
  operator<(const KeyString & a, const KeyString & b);

  friend bool
  operator==(const KeyString & a, const KeyString & b);

  friend std::ostream&
  operator<<(std::ostream& output, const KeyString & id);

private:
  /**
   * value of the key.
   */
  char* str;

  /**
   * hash value of the identifier
   */
  u_int32_t hashValue;
};


/**
 * returns true if the representation of a is lesser than the
 * representation of b.
 *
 * @param a a KeyString
 *
 * @param b a KeyString
 */
bool
operator<(const KeyString & a, const KeyString & b);

/**
 * returns true if a and b represent the same ID.
 *
 * @param a a KeyString
 *
 * @param b a KeyString
 */
bool
operator==(const KeyString & a, const KeyString & b);

/**
 * Writes an ascii description of an id.
 */
inline std::ostream&
operator<<(std::ostream& output, const KeyString & key) {
  return output << key.str;
}


#endif  // _KEY_STRING_HH_
