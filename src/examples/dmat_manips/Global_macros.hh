/****************************************************************************/
/* DIET macros for use with thte asynchronous versions of dmat_manips.      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2003/06/16 17:12:49  pcombes
 * Move the examples using the asynchronous API into this directory.
 *
 ****************************************************************************/

#ifndef _GLOBAL_MACROS_HH_
#define _GLOBAL_MACROS_HH_

#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <omnithread.h>

using namespace std;

#define DIET_TRACE_WITH_THREAD_ID
#define DIET_TRACE_WITH_FILE_NAME
#define DIET_TRACE_WITH_FUNCTION_NAME

# if defined (DIET_NDEBUG)
#   define DIET_DEBUG(X)
# else
#   define DIET_DEBUG(X) X
# endif /* ACE_NDEBUG */

/*#define DIET_TEXT(X) do {
  (sprintf(

} while(0);*/

# define DIET_TRACE_IMPL(X) do { \
	  int id = omni_thread::self()->id(); \
	  timeval tval; \
	  gettimeofday(&tval, NULL); \
	  cout << tval.tv_sec << "," << tval.tv_usec << ":"; \
	  cout << id << ":"; \
	  cout << __FILE__ << ":"; \
	  cout <<__LINE__<< ":"; \
	  cout << __FUNCTION__ << ":"; \
	cout << X << "." << endl; \
	fflush(stdout); \
	fflush(stderr); \
  } while(0);


#define DIET_TRACE(X) DIET_TRACE_IMPL(X)

#endif /* _GLOBAL_MACROS_H_ */
