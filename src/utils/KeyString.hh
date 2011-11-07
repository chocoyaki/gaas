/**
 * @file  KeyString.hh
 *
 * @brief  Hashed string object to be used with container
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _KEY_STRING_HH_
#define _KEY_STRING_HH_



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
  hash(const char *id);

  /**
   * creates a new KeyString.
   *
   * @param value value of the key. It value is copied by the
   * KeyString and the memory used as argument can be free by the
   * user. value must be not NULL.
   */
  KeyString(const char *value);

  /**
   * Creates a clone of the KeyString given in argument.
   *
   * @param masterAgentId the KeyString which is cloned
   */
  KeyString(const KeyString &masterAgentId);

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
  operator = (const KeyString &masterAgentId);

  /**
   * returnes a char* that containes the value of the
   * KeyString.
   */
  operator const char *() const;


  friend bool
    operator < (const KeyString &a, const KeyString &b);

  friend bool
    operator == (const KeyString &a, const KeyString &b);

  friend std::ostream &
  operator << (std::ostream & output, const KeyString &id);

private:
  /**
   * value of the key.
   */
  char *str;

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
  operator < (const KeyString &a, const KeyString &b);

/**
 * returns true if a and b represent the same ID.
 *
 * @param a a KeyString
 *
 * @param b a KeyString
 */
bool
  operator == (const KeyString &a, const KeyString &b);

/**
 * Writes an ascii description of an id.
 */
inline std::ostream &
operator << (std::ostream & output, const KeyString &key) {
  return output << key.str;
}


#endif  // _KEY_STRING_HH_
