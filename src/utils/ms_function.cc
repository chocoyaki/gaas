/*
 * $Log$
 * Revision 1.3  2002/12/23 17:02:06  sdahan
 * Add the documentation of the ms_function tool. This is a set of memory use
 * functions that throw an bad_alloc exception if there is not enough memory.
 *
 * Revision 1.2  2002/12/23 16:45:58  sdahan
 * uses the absolute name of the exception "std::bad_alloc" instead of the
 * local name "bad_alloc". This changes nothing about the code but now it
 * compile on darwin.
 */

#include "ms_function.hh"
#include <new> // definition of the bad_alloc exception

char* ms_strdup(const char* str) {
  char* copy = CORBA::string_dup(str) ;
  if (copy == NULL)
    throw std::bad_alloc() ;
  return copy ;
} 

char* ms_stralloc(int length) {
  char* str = CORBA::string_alloc(length) ;
  if (str == NULL)
    throw std::bad_alloc() ;
  return str ;
}
