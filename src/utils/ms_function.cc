#include "ms_function.hh"
#include <new> // definition of the bad_alloc exception

char* ms_strdup(const char* str) {
  char* copy = CORBA::string_dup(str) ;
  if (copy == NULL)
    throw bad_alloc() ;
  return copy ;
} 

char* ms_stralloc(int length) {
  char* str = CORBA::string_alloc(length) ;
  if (str == NULL)
    throw bad_alloc() ;
  return str ;
}
