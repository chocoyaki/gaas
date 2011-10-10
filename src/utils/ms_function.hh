/****************************************************************************/
/* Memory safe management header                                            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN             - Sylvain.Dahan@lifc.univ-fcomte.fr       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2006/11/16 09:55:56  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.9  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.8  2003/04/10 12:45:10  pcombes
 * Apply Coding Standards.
 *
 * Revision 1.6  2003/01/22 15:48:54  sdahan
 * add #include "omniorb.hh"
 *
 * Revision 1.5  2003/01/22 14:56:30  pcombes
 * Add omniORB3 compatibility
 *
 * Revision 1.4  2003/01/17 18:03:10  pcombes
 * Suppress warning on last #endif
 *
 * Revision 1.3  2002/12/23 17:02:06  sdahan
 * Add the documentation of the ms_function tool. This is a set of memory use
 * functions that throw an bad_alloc exception if there is not enough memory.
 *
 ****************************************************************************/

/**
 * \page ms_function
 *
 * This is a set of function that allocate and deallocate the memory
 * and send a \c bad_alloc() if there is not enough memory. By using
 * those functions the programmer don't have to check every return of
 * the memory functions.
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
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
char* ms_strdup(const char* str);

/**
 * Create an new string of length \c length with the \c
 * CORBA::string_alloc() function. If there is no enough memory to
 * allocate the string, it throws a \c bad_alloc exception.
 *
 * @param length the length of the new string.
 *
 * @return an undefined string of length \c length.
 */
char* ms_stralloc(int length);

/**
 * Frees the memory of the string given in argument with the function
 * \c CORBA::string_free.
 *
 * @param str the string who is destroyed
 */
#define ms_strfree(str) CORBA::string_free(str)

#endif // _MS_FUNCTION_HH_
