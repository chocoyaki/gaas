#ifndef _MS_FUNCTION_HH_
#define _MS_FUNCTION_HH_

#include <omniORB3/CORBA.h>

char* ms_strdup(const char* str) ;
char* ms_stralloc(int length) ;

#define ms_strfree(str) CORBA::string_free(str)

#endif _MS_FUNCTION_HH_
