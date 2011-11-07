/**
 * @file  ms_function.hh
 *
 * @brief  Memory safe management header
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _MS_FUNCTION_HH_
#define _MS_FUNCTION_HH_

#include <omniORB4/CORBA.h>

/**
 * Duplicates the string given in argument with the \c
 * CORBA::string_dup() function. If there is no enough memory to
 * allocate the string, it throws a \c bad_alloc exception.
 *
 * @param str the string to duplicate
 *
 * @return a clone of \c str.
 */
char *
ms_strdup(const char *str);

/**
 * Create an new string of length \c length with the \c
 * CORBA::string_alloc() function. If there is no enough memory to
 * allocate the string, it throws a \c bad_alloc exception.
 *
 * @param length the length of the new string.
 *
 * @return an undefined string of length \c length.
 */
char *
ms_stralloc(int length);

/**
 * Frees the memory of the string given in argument with the function
 * \c CORBA::string_free.
 *
 * @param str the string who is destroyed
 */
#define ms_strfree(str) CORBA::string_free(str)

#endif  // _MS_FUNCTION_HH_
