/**
 * @file  KeyString.cc
 *
 * @brief  Hashed string object to be used with container
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <cassert>
#include <cstring>
#include "ms_function.hh"
#include "KeyString.hh"

/* disp_state is a macro that displayed the attribut's value of the
   object and the name of the called methods */
#define disp_state(fct)

u_int32_t
KeyString::hash(const char *id) {
  assert(id != NULL);
  u_int32_t result = 0;
  size_t length = strlen(id);

  for (size_t idIdx = 0; idIdx < length; idIdx++) {
    result = ((result << 1) | (result >> 31)) + id[idIdx];
  }

  return result;
} // hash


KeyString::KeyString(const char *value) {
  assert(value != NULL);
  disp_state("KeyString(const char* value)");
  this->str = ms_strdup(value);
  hashValue = hash(str);
  disp_state("--KeyString(const char* value)");
}


KeyString::KeyString(const KeyString &value) {
  disp_state("KeyString(const KeyString & value)");
  str = ms_strdup(value.str);
  hashValue = value.hashValue;
  disp_state("--KeyString(const KeyString & value)");
}

KeyString::~KeyString() {
  disp_state("~KeyString()");
  ms_strfree(str);
}


KeyString &
KeyString::operator=(const KeyString &value) {
  disp_state("operator= (const KeyString & value)");
  ms_strfree(str);
  str = ms_strdup(value.str);
  hashValue = value.hashValue;
  disp_state("--operator= (const KeyString & value)");
  return *this;
}

KeyString::operator const char *() const {
  disp_state("KeyString::operator corba_ma_identifier_t()");
  return str;
}


bool
operator<(const KeyString &a, const KeyString &b) {
  bool result;
  if (a.hashValue == b.hashValue) {
    result = strcmp(a.str, b.str) < 0;
  } else {
    result = a.hashValue < b.hashValue;
  }
  return result;
}


bool
operator==(const KeyString &a, const KeyString &b) {
  return ((a.hashValue == b.hashValue) &&
          (strcmp(a.str, b.str) == 0));
}
