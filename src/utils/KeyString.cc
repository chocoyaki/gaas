/****************************************************************************/
/* Hashed string object to be used with container                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.2  2004/10/08 12:04:02  hdail
 * Corrected mis-matched return values for hash in .cc and .hh.  Caused problem
 * for comptability with 64-bit machines.
 *
 * Revision 1.1  2004/09/29 13:35:32  sdahan
 * Add the Multi-MAs feature.
 *
 ****************************************************************************/

#include "ms_function.hh"
#include "KeyString.hh"
#include <cassert>
#include <cstring>

/* disp_state is a macro that displayed the attribut's value of the
   object and the name of the called methods */
//#include <cstdio>
//#define disp_state(fct) printf("KeyString::%s %s (%lx)\n", fct, (const char*)this->value ? (const char*)this->value : "empty-string", (long)(const char*)this->value)
#define disp_state(fct)

u_int32_t KeyString::hash(const char* id) {
  assert(id != NULL);
  u_int32_t result = 0;
  size_t length = strlen(id);
  for (size_t idIdx = 0; idIdx < length; idIdx++)
    result = ((result<<1) | (result>>31))+ id[idIdx];
  return result;
}


KeyString::KeyString(const char* value) {
  assert(value != NULL);
  disp_state("KeyString(const char* value)");
  this->str = ms_strdup(value);
  hashValue = hash(str);
  disp_state("--KeyString(const char* value)");
}


KeyString::KeyString(const KeyString & value) {
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
KeyString::operator= (const KeyString & value) {
  disp_state("operator= (const KeyString & value)");
  ms_strfree(str);
  str = ms_strdup(value.str);
  hashValue = value.hashValue;
  disp_state("--operator= (const KeyString & value)");
  return *this;
}

KeyString::operator const char*() const {
  disp_state("KeyString::operator corba_ma_identifier_t()");
  return str;
}


bool operator< (const KeyString & a, const KeyString & b) {
  bool result;
  if (a.hashValue == b.hashValue)
    result = strcmp(a.str, b.str) < 0;
  else
    result = a.hashValue < b.hashValue;
  return result;
}


bool operator== (const KeyString & a, const KeyString & b) {
  return (a.hashValue == b.hashValue)
    && (strcmp(a.str, b.str) == 0);
}
